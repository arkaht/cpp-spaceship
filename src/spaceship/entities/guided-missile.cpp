#include "guided-missile.h"

#include <spaceship/components/health-component.h>
#include <spaceship/entities/spaceship.h>
#include <spaceship/entities/explosion-effect.h>

#include <suprengine/core/assets.h>
#include <suprengine/core/engine.h>
#include <suprengine/utils/random.h>

using namespace spaceship;

GuidedMissile::GuidedMissile(
	const SharedPtr<Spaceship>& owner,
	const WeakPtr<HealthComponent>& wk_target,
	const Color color
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
	_lifetime_component->on_time_out.listen( &GuidedMissile::explode, this );

	_current_move_speed = move_speed * STARTING_MOVE_SPEED_RATIO;

	//  set initial target direction
	if ( const SharedPtr<HealthComponent> target = _wk_target.lock())
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
	// Spawn explosion effect
	{
		Color color = Color::white;
		if ( const SharedPtr<Spaceship> owner = _wk_owner.lock() )
		{
			color = owner->get_color();
		}

		const float size = explosion_size
			+ random::generate( EXPLOSION_SIZE_DEVIATION.x, EXPLOSION_SIZE_DEVIATION.y );

		Engine& engine = Engine::instance();
		const SharedPtr<ExplosionEffect> effect = engine.create_entity<ExplosionEffect>( size, color );
		effect->transform->location = transform->location;
	}

	kill();
}

void GuidedMissile::_update_target( const float dt )
{
	if ( const SharedPtr<HealthComponent> target = _wk_target.lock())
	{
		// Invalidate target if dead
		if ( !target->is_alive() )
		{
			_wk_target.reset();
		}
		else
		{
			// Update direction
			_desired_direction = ( target->transform->location - transform->location ).normalized();
		}
	}

	// Rotate towards target
	const Quaternion look_rotation = Quaternion::look_at( _desired_direction, up_direction );
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
	const Engine& engine = Engine::instance();
	Physics* physics = engine.get_physics();

	// Setup raycast
	const Ray ray(
		transform->location, 
		transform->get_forward(), 
		impact_distance 
	);
	RayParams params {};
	params.can_hit_from_origin = false;

	// Check collisions
	RayHit hit {};
	if ( !physics->raycast( ray, &hit, params ) ) return;
	
	// Check entity is not owner
	const SharedPtr<Entity> entity = hit.collider->get_owner();
	if ( entity == _wk_owner.lock() ) return;

	// Check entity has health component
	const SharedPtr<HealthComponent> health = entity->find_component<HealthComponent>();
	if ( !health ) return;
	
	//  damage it
	_damage( health );
}

void GuidedMissile::_damage( const SharedPtr<HealthComponent>& target )
{
	if ( !target ) return;
	
	const Vec3 diff = target->transform->location - transform->location;

	// Damage
	DamageInfo info {};
	info.attacker = _wk_owner.lock();
	info.damage = damage_amount;
	info.knockback = diff.normalized() * knockback_force;
	const DamageResult result = target->damage( info );

	// Alert owner
	if ( result.is_valid )
	{
		if ( const SharedPtr<Spaceship> owner = _wk_owner.lock())
		{
			owner->on_hit.invoke( result );
		}
	}

	explode();
}
