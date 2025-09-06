#include "explosion-effect.h"

#include <suprengine/core/assets.h>

#include <suprengine/math/easing.h>
#include <suprengine/utils/random.h>

using namespace spaceship;

ExplosionEffect::ExplosionEffect( 
	const float explosion_size,
	const Color color,
	int model_id
)
	: explosion_size( explosion_size ),
	  color( color )
{
	// Randomize model if unspecified
	if ( model_id < 0 )
	{
		model_id = random::generate( 0, 2 );
	}
	_model_id = model_id;
}

void ExplosionEffect::setup()
{
	_model_renderer = create_component<StylizedModelRenderer>(
		Assets::get_model( "explosion" + std::to_string( _model_id ) ),
		Color::white/*color*/
	);
	_model_renderer->inner_modulate = color/*Color::white*/;
	//_model_renderer->draw_only_outline = true;

	_max_lifetime = LIFETIME
		+ random::generate( -LIFETIME_DEVIATION, LIFETIME_DEVIATION );

	_lifetime_component = create_component<LifetimeComponent>( _max_lifetime );
	_lifetime_component->on_time_out.listen( &Entity::kill, this );
	
	_scale = Vec3 {
		random::generate( RANDOM_SCALE[0] ),
		random::generate( RANDOM_SCALE[1] ),
		random::generate( RANDOM_SCALE[2] ),
	};
	transform->rotation = random::generate_rotation();

	// Get curves
	_curve_transform_scale = Assets::get_curve( "explosion/transform-scale" );
	_curve_outline_scale = Assets::get_curve( "explosion/outline-scale" );
	_curve_outline_color = Assets::get_curve( "explosion/outline-color" );
	_curve_inner_color = Assets::get_curve( "explosion/inner-color" );
}

void ExplosionEffect::update_this( const float dt )
{
	const float lifetime = _max_lifetime - _lifetime_component->life_time;
	const float t = lifetime / _max_lifetime;

	// Lerp outline color
	_model_renderer->modulate = Color::lerp(
		Color::white, 
		color,
		_curve_outline_color->evaluate_by_time( t )
	);

	// Lerp inner color
	_model_renderer->inner_modulate = Color::lerp( 
		color,
		Color::black,
		_curve_inner_color->evaluate_by_time( t )
	);
	
	// Apply outline scale
	_model_renderer->outline_scale = 
		_curve_outline_scale->evaluate_by_time( t ) * OUTLINE_SCALE;
	
	// Apply transform scale
	transform->set_scale( 
			explosion_size 
		* _curve_transform_scale->evaluate_by_time( t ) 
		* _scale
	);
}
