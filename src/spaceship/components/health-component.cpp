#include "health-component.h"

#include <suprengine/math/math.h>

using namespace spaceship;

HealthComponent::HealthComponent( float health )
	: health( health ), max_health( health )
{}

DamageResult HealthComponent::damage( const DamageInfo& info )
{
	DamageResult result( info );

	//  check component can receive damage
	if ( !is_alive() ) return result;

	//  check damage infos
	if ( info.attacker == nullptr ) return result;
	if ( info.damage <= 0.0f ) return result;

	health -= info.damage;
	if ( health <= 0.0f )
	{
		//printf( "dead!\n" );
	}
	//printf( "%f -> %f\n", health + info.damage, health );

	//  validate result
	result.is_valid = true;
	result.victim = as<HealthComponent>();
	result.is_alive = is_alive();

	//  trigger event
	on_damage.invoke( result );

	return result;
}

bool HealthComponent::is_alive() const
{
	return health > 0.0f;
}

void HealthComponent::heal( float amount )
{
	health = math::min( max_health, health + amount );
}

void HealthComponent::heal_to_full()
{
	health = max_health;
}
