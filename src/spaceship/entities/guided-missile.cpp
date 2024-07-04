#include "guided-missile.h"

#include <spaceship/components/health-component.h>
#include <spaceship/entities/spaceship.h>
#include <spaceship/entities/explosion-effect.h>

#include <suprengine/assets.h>
#include <suprengine/random.h>

using namespace spaceship;

GuidedMissile::GuidedMissile( 
	SharedPtr<Spaceship> owner, 
	WeakPtr<HealthComponent> wk_target,
	Color color 
)
	: _wk_owner( owner ), _wk_target( wk_target ), _color( color )
{}

void GuidedMissile::setup()
{
	_model_renderer = create_component<StylizedModelRenderer>(
		Assets::get_model( "projectile" ),
		_color
	);
	_model_renderer->draw_only_outline = true;

	_lifetime_component = create_component<LifetimeComponent>( LIFETIME );
	_lifetime_component->on_time_out.listen( "owner", 
		std::bind( &GuidedMissile::explode, this ) );

	_current_move_speed = move_speed * STARTING_MOVE_SPEED_RATIO;

	//  set initial target direction
	if ( auto target = _wk_target.lock() )
	{
		_desired_direction = 
			( target->transform->location - transform->location ).normalized();
	}

}

void GuidedMissile::update_this( float dt )
{
	//  accelerate speeds
	_current_move_speed = math::lerp( 
		_current_move_speed, 
		move_speed, 
		dt * move_acceleration 
	);
	if ( LIFETIME - _lifetime_component->life_time > TIME_LOCKED_ROTATION ) 
	{
		_current_rotation_speed = math::lerp(
			_current_rotation_speed,
			rotation_speed,
			dt * rotation_acceleration
		);
	}

	_update_target( dt );

	//  move forward
	transform->set_location( 
		transform->location 
	  + transform->get_forward() * _current_move_speed * dt
	);

	_check_impact();
}

void GuidedMissile::explode()
{
	//  spawn explosion effect
	{
		Color color = Color::white;
		if ( auto owner = _wk_owner.lock() )
		{
			color = owner->get_color();
		}

		float size = explosion_size
			+ random::generate( EXPLOSION_SIZE_DEVIATION.x, EXPLOSION_SIZE_DEVIATION.y );

		auto& engine = Engine::instance();
		auto effect = engine.create_entity<ExplosionEffect>( explosion_size, color );
		effect->transform->location = transform->location;
	}

	kill();
}

void GuidedMissile::_update_target( float dt )
{
	if ( auto target = _wk_target.lock() )
	{
		//  invalidate target if dead
		if ( !target->is_alive() )
		{
			_wk_target.reset();
		}
		else
		{
			//  update direction
			_desired_direction = 
				( target->transform->location - transform->location ).normalized();
		}
	}

	//  rotate towards target
	Quaternion look_rotation = Quaternion::look_at( 
		_desired_direction, up_direction );
	transform->set_rotation(
		Quaternion::slerp(
			transform->rotation,
			look_rotation,
			dt * _current_rotation_speed
		)
	);
}

void GuidedMissile::_check_impact()
{
	auto& engine = Engine::instance();
	auto physics = engine.get_physics();

	//  setup raycast
	Ray ray( 
		transform->location, 
		transform->get_forward(), 
		impact_distance 
	);
	RayParams params {};
	params.can_hit_from_origin = false;

	//  check collisions
	RayHit hit {};
	if ( !physics->raycast( ray, &hit, params ) ) return;
	
	//  check entity is not owner
	auto entity = hit.collider->get_owner();
	if ( entity == _wk_owner.lock() ) return;

	//  check entity has health component
	auto health = entity->get_component<HealthComponent>();
	if ( !health ) return;
	
	//  damage it
	_damage( health );
	//printf( "hit %p (self:%p)\n", hit.collider->get_owner(), this );
}

void GuidedMissile::_damage( SharedPtr<HealthComponent> target )
{
	if ( !target ) return;
	
	Vec3 diff = target->transform->location - transform->location;

	//  damage
	DamageInfo info {};
	info.attacker = _wk_owner.lock();
	info.damage = damage_amount;
	info.knockback = diff.normalized() * knockback_force;
	DamageResult result = target->damage( info );

	//  alert owner
	if ( result.is_valid )
	{
		if ( auto owner = _wk_owner.lock() )
		{
			owner->on_hit.invoke( result );
		}
	}

	explode();
	//printf( "boom\n" );
}
