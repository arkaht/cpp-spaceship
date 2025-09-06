#include "projectile.h"

#include <spaceship/entities/asteroid.h>
#include <spaceship/entities/spaceship.h>

#include <suprengine/core/assets.h>
#include <suprengine/core/engine.h>

using namespace spaceship;

Projectile::Projectile( const SharedPtr<Spaceship>& owner, const Color color )
	: _wk_owner( owner ), _color( color )
{}

void Projectile::setup()
{
	_model_renderer = create_component<StylizedModelRenderer>(
		Assets::get_model( "projectile" ),
		_color
	);
	_model_renderer->draw_only_outline = true;

	_lifetime_component = create_component<LifetimeComponent>( LIFETIME );
	_lifetime_component->on_time_out.listen( &Entity::kill, this );
}

void Projectile::update_this( const float dt )
{
	const float movement_speed = move_speed * dt;
	const Vec3 movement = transform->get_forward() * movement_speed;
	const Vec3 new_location = transform->location + movement;
	if ( _check_collisions( movement_speed ) ) return;

	//  movement
	transform->set_location( new_location );
}

bool Projectile::_check_collisions( const float movement_speed )
{
	const Engine& engine = Engine::instance();
	Physics* physics = engine.get_physics();

	//  setup ray
	const Ray ray(
		transform->location, 
		transform->get_forward(), 
		movement_speed 
	);
	const RayParams params {};

	//  check safe movement 
	RayHit result;
	if ( physics->raycast( ray, &result, params ) )
	{
		if ( result.collider->get_owner() != _wk_owner.lock() )
		{
			_on_hit( result );
			kill();
			return true;
		}
	}

	return false;
}

void Projectile::_on_hit( const RayHit& result )
{
	const SharedPtr<Entity> entity = result.collider->get_owner();

	// Damage health component
	if ( const SharedPtr<HealthComponent> health = entity->find_component<HealthComponent>())
	{
		DamageInfo info {};
		info.attacker = _wk_owner.lock();
		info.damage = damage_amount;
		info.knockback = -result.normal * knockback_force;

		const DamageResult damage_result = health->damage( info );

		// Alert owner
		if ( damage_result.is_valid )
		{
			if ( const SharedPtr<Spaceship> owner = _wk_owner.lock() )
			{
				owner->on_hit.invoke( damage_result );
			}
		}
	}
}
