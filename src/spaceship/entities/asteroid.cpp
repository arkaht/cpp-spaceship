#include "asteroid.h"
#include <suprengine/random.h>

using namespace spaceship;

Asteroid::Asteroid()
{}

Asteroid::~Asteroid() {}

void Asteroid::setup()
{
	_model_renderer = create_component<StylizedModelRenderer>( 
		Assets::get_model( "asteroid" + std::to_string( random::generate( 0, 1 ) ) ),
		Color::from_0x( 0xeb6e3dFF )
	);
	_collider = create_component<SphereCollider>( 1.0f );

	_health = create_component<HealthComponent>();
	_health->on_damage.listen( "owner", 
		std::bind( &Asteroid::_on_damage, this, std::placeholders::_1 ) 
	);
}

void Asteroid::update_this( float dt )
{
	Vec3 movement = linear_direction * dt;
	RadAngles rotation = RadAngles( linear_direction * math::DEG2RAD * dt );

	transform->set_location( transform->location + movement );
	transform->set_rotation( transform->rotation + Quaternion( rotation ) );
}

void Asteroid::update_collision_to_transform()
{
	_health->max_health = 5.0f * transform->scale.x;
	_health->health = _health->max_health;

	//printf( "%f\n", _health);
	//_collider->radius = 
	//	( transform->scale.x + transform->scale.y + transform->scale.z ) / 3.0f;
}

void Asteroid::_on_damage( const DamageResult& result )
{
	linear_direction += result.info.knockback * ( 1.0f / transform->scale.x );

	//  check death
	if ( !result.is_alive )
	{
		//  split
		if ( split_times > 0 )
		{
			split();
		}

		kill();
	}
}

void Asteroid::split()
{
	auto& engine = Engine::instance();

	split_times--;

	float linear_force = linear_direction.length();
	Vec3 right_force = transform->get_right() 
		* linear_force 
		* random::generate( 1.5f, 2.5f );

	const int COUNT = random::generate( 2, 4 );
	for ( int i = 0; i < 2; i++ )
	{
		float angle = 360.0f / COUNT * i;

		auto half = engine.create_entity<Asteroid>();
		half->linear_direction = Vec3::transform( 
			Vec3 { 
				math::cos( angle ), 
				math::sin( angle ), 
				0.0f 
			}, 
			transform->rotation 
		) * linear_force * random::generate( 1.1f, 1.5f );

		/*float dot =	Vec3::dot( 
			half->linear_direction.normalized(), 
			linear_direction.normalized() 
		);
		if ( dot > 0.0f )
		{
			half->linear_direction *= ( 1.0f + dot ) * random::generate( 1.1f, 1.5f );
		}*/

		half->transform->location = transform->location 
			+ half->linear_direction.normalized();
		half->transform->rotation = Quaternion::look_at( 
			random::generate_direction(), 
			Vec3::up 
		);
		half->transform->scale = transform->scale * ( random::generate( 0.9f, 1.2f ) / COUNT );
		half->split_times = split_times;
		half->update_collision_to_transform();
	}
}
