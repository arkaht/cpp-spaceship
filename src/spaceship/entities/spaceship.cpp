#include "spaceship.h"

#include <spaceship/entities/guided-missile.h>
#include <spaceship/entities/explosion-effect.h>

#include <suprengine/core/assets.h>
#include <suprengine/core/engine.h>
#include <suprengine/utils/random.h>

using namespace spaceship;

std::vector<WeakPtr<Spaceship>> Spaceship::_all_spaceships;

Spaceship::Spaceship() 
{}

Spaceship::~Spaceship()
{
	if ( const SharedPtr<Entity> entity = _trail_renderer->get_owner())
	{
		entity->kill();
	}

	if ( const SharedPtr<SpaceshipController> controller = wk_controller.lock())
	{
		controller->unpossess();
	}

	//  remove from list
	std::erase_if( _all_spaceships, 
		[this]( const WeakPtr<Spaceship>& wk_ship )
		{
			const SharedPtr<Spaceship> ship = wk_ship.lock();
			if ( ship == nullptr ) return false;
			
			return ship.get() == this;
		}
	);
}

void Spaceship::setup()
{
	auto& engine = Engine::instance();

	CameraDynamicDistanceSettings dcd_settings {};
	dcd_settings.is_active = true;
	dcd_settings.max_distance_sqr = math::pow( 256.0f, 2.0f );
	dcd_settings.max_outline_scale = 1.0f;

	// Setup components
	_model_renderer = create_component<StylizedModelRenderer>(
		Assets::get_model( "spaceship" ),
		_color
	);
	_model_renderer->dynamic_camera_distance_settings = dcd_settings;
	_model_renderer->outline_scale = MODEL_OUTLINE_SCALE;
	_collider = create_component<BoxCollider>( Box::one * 2.0f );

	// Initialize trail
	const SharedPtr<Entity> trail_entity = engine.create_entity<Entity>();
	_trail_renderer = trail_entity->create_component<StylizedModelRenderer>(
		_model_renderer->model,
		_color
	);
	_trail_renderer->dynamic_camera_distance_settings = dcd_settings;
	_trail_renderer->outline_scale = MODEL_OUTLINE_SCALE;
	_trail_renderer->draw_only_outline = true;
	_trail_renderer->draw_outline_ccw = false;

	// Health
	_health = create_component<HealthComponent>();
	_health->on_damage.listen( &Spaceship::_on_damage, this );

	// Add to list
	_all_spaceships.push_back( as<Spaceship>() );
}

void Spaceship::update_this( const float dt )
{
	_update_movement( dt );
	_update_trail( dt );

	// Reduce shoot cooldown
	_shoot_time = math::max( 0.0f, _shoot_time - dt );
}

SharedPtr<Spaceship> Spaceship::find_lockable_target( 
	const Vec3& view_direction 
) const
{
	SharedPtr<Spaceship> target = nullptr;

	float best_view_alignment = -1.0f;
	float best_distance = MISSILE_LOCK_MAX_DISTANCE;

	for ( const WeakPtr<Spaceship>& wk_ship : _all_spaceships )
	{
		const SharedPtr<Spaceship> ship = wk_ship.lock();
		if ( ship == nullptr || ship.get() == this ) continue;
		
		// Check health
		const SharedPtr<HealthComponent> health = ship->get_health_component();
		if ( !health->is_alive() ) continue;

		Vec3 diff = ship->transform->location - transform->location;
		
		// Check distance
		float distance = diff.length();
		if ( distance >= MISSILE_LOCK_MAX_DISTANCE ) continue;

		// Check direction
		Vec3 direction = diff * ( 1.0f / distance );
		float view_alignment = Vec3::dot( direction, view_direction );
		if ( view_alignment <= MISSILE_LOCK_DOT_THRESHOLD ) continue;
	
		if ( distance >= best_distance && view_alignment < best_view_alignment ) continue;

		best_distance = math::min( distance, best_distance );
		best_view_alignment = math::max( view_alignment, best_view_alignment );
		target = ship;
	}

	return target;
}

void Spaceship::shoot()
{
	Engine& engine = Engine::instance();
	const SharedPtr<Spaceship> shared_this = as<Spaceship>();

	// Spawn projectile
	for ( int i = 0; i < 2; i++ )
	{
		const SharedPtr<Projectile> projectile = engine.create_entity<Projectile>(
			shared_this,
			_color
		);
		projectile->transform->scale = Vec3( 1.5f );
		projectile->transform->location = 
			get_shoot_location( 
				Vec3 {
					projectile->transform->scale.x,
					i == 0 ? -1.0f : 1.0f,
					1.0f
				} 
			);
		projectile->transform->rotation = transform->rotation;
		projectile->damage_amount = 25.0f;
	}

	//  put on cooldown
	_shoot_time = SHOOT_TIME;
}

void Spaceship::launch_missiles( 
	const WeakPtr<HealthComponent>& wk_target
)
{
	Engine& engine = Engine::instance();
	const SharedPtr<Spaceship> shared_this = as<Spaceship>();

	constexpr int MISSILES_COUNT = 6;
	for ( int i = 0; i < MISSILES_COUNT; i++ )
	{
		float row = math::floor( static_cast<float>( i ) / 2.0f );
		Timer timer(
			[&engine, shared_this, wk_target, this, i, row]{
				const SharedPtr<GuidedMissile> missile = engine.create_entity<GuidedMissile>(
					shared_this,
					wk_target,
					_color
				);
				missile->transform->location = transform->location 
					+ transform->get_right() * ( i % 2 == 0 ? 1.0f : -1.0f ) * 2.0f
					+ transform->get_forward() * row * 3.0f;
				missile->transform->rotation = Quaternion::look_at( transform->get_up(), Vec3::up );
				missile->up_direction = transform->get_up();
			},
			row * 0.1f
		);
		engine.add_timer( timer );
	}
}

void Spaceship::die()
{
	Engine& engine = Engine::instance();

	if ( _health->health > 0.0f )
	{
		_health->health = 0.0f;
	}

	_throttle = 0.0f;

	_model_renderer->is_active = false;
	_trail_renderer->is_active = false;
	_collider->is_active = false;

	state = EntityState::Paused;

	// Spawn explosion effect
	{
		const float size_ratio_over_damage = math::clamp(
			math::abs( _health->health / EXPLOSION_DAMAGE_FOR_MAX_SIZE ), 
			0.0f, 1.0f 
		);
		float size = math::lerp( EXPLOSION_SIZE.x, EXPLOSION_SIZE.y, size_ratio_over_damage );
		size += random::generate( EXPLOSION_SIZE_DEVIATION.x, EXPLOSION_SIZE_DEVIATION.y );

		const SharedPtr<ExplosionEffect> effect = engine.create_entity<ExplosionEffect>(
			size,
			_color
		);
		effect->transform->location = transform->location;
	}
	printf( "Spaceship[%d] is killed!\n", get_unique_id() );

	TIMER( 5.0f, { respawn(); } );
}

void Spaceship::respawn()
{
	transform->set_location( Vec3::zero );
	transform->set_rotation( Quaternion::identity );

	_model_renderer->is_active = true;
	_trail_renderer->is_active = true;
	_collider->is_active = true;

	state = EntityState::Active;

	_health->heal_to_full();

	printf( "Spaceship[%d] has respawned!\n", get_unique_id() );
}

Vec3 Spaceship::get_shoot_location( const Vec3& axis_scale ) const
{
	// TODO: Would be nice if we could reference an attachment inside a model.
	return transform->location
		+ transform->get_forward() * 3.7f * axis_scale.x
		+ transform->get_right() * 2.0f * axis_scale.y
		+ transform->get_up() * 0.25f * axis_scale.z;
}

void Spaceship::set_color( const Color& color )
{
	_color = color;

	// Update renderers
	_model_renderer->modulate = _color;
	_trail_renderer->modulate = _color;
}

void Spaceship::_update_movement( const float dt )
{
	// Get inputs
	SpaceshipControlInputs inputs {};
	if ( const SharedPtr<SpaceshipController> controller = wk_controller.lock())
	{
		controller->update_inputs( dt );
		inputs = controller->get_inputs();
	}

	const float throttle_delta = inputs.throttle_delta;
	const float throttle_speed = THROTTLE_GAIN_SPEED;

	// Find throttle target
	float throttle_target = math::clamp( _throttle, 0.0f, 1.0f );
	if ( throttle_delta > 0.0f )
	{
		throttle_target = 1.0f + MAX_THROTTLE_FORWARD_OFFSET;
	}
	else if ( throttle_delta < 0.0f )
	{
		throttle_target = 0.0f;
	}

	// Throttle
	_throttle = math::approach( 
		_throttle,
		throttle_target,
		dt * throttle_speed
	);

	// Apply forward movement
	const float move_speed = dt * _throttle * MAX_THROTTLE_SPEED;
	const Vec3 movement = transform->get_forward() * move_speed;
	transform->set_location( transform->location + movement );

	// Apply rotation and avoid identity rotation when no controller
	if ( SharedPtr<SpaceshipController> controller = wk_controller.lock())
	{
		const Quaternion rotation = inputs.should_smooth_rotation
			? Quaternion::slerp( transform->rotation, inputs.desired_rotation, dt * inputs.smooth_rotation_speed )
			: inputs.desired_rotation;

		transform->set_rotation( rotation );
	}
}

void Spaceship::_update_trail( float dt )
{
	Engine& engine = Engine::instance();
	const float time = engine.get_updater()->get_accumulated_seconds();

	//  intensity
	float trail_intensity_target = 0.0f;
	if ( _throttle > TRAIL_THROTTLE_START )
	{
		trail_intensity_target = math::remap( 
			_throttle, 
			TRAIL_THROTTLE_START, 1.0f, 
			0.0f, 1.0f 
		);
	}
	_trail_intensity = math::lerp(
		_trail_intensity,
		trail_intensity_target,
		dt * TRAIL_INTENSITY_SPEED
	);
	_trail_renderer->is_active = _trail_intensity > 0.01f;

	// Update visual
	if ( _trail_renderer->is_active )
	{
		const SharedPtr<Transform>& trail_transform = _trail_renderer->transform;

		// Location
		Vec3 location = transform->location;
		location += -transform->get_forward()
			* _trail_intensity
			* math::abs( 0.1f + math::sin( time * _throttle * TRAIL_WAVE_FREQUENCY ) * TRAIL_WAVE_AMPLITUDE );
		trail_transform->set_location( location );

		// Rotation
		trail_transform->set_rotation( transform->rotation );

		//  scale
		Vec3 scale = transform->scale * TRAIL_MODEL_SCALE;
		scale *= math::min( TRAIL_MODEL_SCALE_INTENSITY_OFFSET + _trail_intensity, 1.0f ) 
			   + _trail_renderer->outline_scale * TRAIL_MODEL_OUTLINE_SCALE_OFFSET;
		trail_transform->set_scale( scale );
	}
}

void Spaceship::_on_damage( const DamageResult& result )
{
	if ( !result.is_alive )
	{
		die();
	}
}
