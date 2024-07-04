#include "player-hud.h"

#include <spaceship/entities/player-spaceship-controller.h>

#include <suprengine/assets.h>
#include <suprengine/easing.h>

using namespace spaceship;

PlayerHUD::PlayerHUD( 
	SharedPtr<PlayerSpaceshipController> owner
)
	: _controller( owner )
{
	_controller->on_possess_changed.listen( "player-hud",
		[&]( SharedPtr<Spaceship> previous, SharedPtr<Spaceship> current )
		{
			_unbind_from_spaceship( previous );
			_bind_to_spaceship( current );
		}
	);

	_crosshair_line_texture = Assets::get_texture( "crosshair-line" );
	_kill_icon_texture = Assets::get_texture( "kill-icon" );
	KILL_ICON_TEXTURE_SCALE = KILL_ICON_SIZE / _kill_icon_texture->get_size().x;
}

void PlayerHUD::update( float dt )
{
	auto spaceship = _controller->get_ship();
	if ( !spaceship ) return;

	_hit_time = math::max( 0.0f, _hit_time - dt );
	_crosshair_color = Color::lerp( 
		_crosshair_color, 
		spaceship->get_color(), 
		dt * CROSSHAIR_COLOR_SMOOTH_SPEED 
	);

	//  update kill icons
	int column = 0;
	for ( auto itr = _kill_icons.begin(); itr != _kill_icons.end(); )
	{
		auto& data = *itr;

		//  update x-offset
		float target_x = column * ( KILL_X_GAP + KILL_ICON_SIZE );
		if ( data.x_offset == -1.0f )
		{
			data.x_offset = target_x;
		}
		else 
		{
			data.x_offset = math::lerp( 
				data.x_offset, 
				target_x,
				dt * KILL_X_SPEED
			);
		}

		if ( data.life_time > 0.0f )
		{
			data.life_time = math::max( 0.0f, data.life_time - dt );

			float color_speed = dt * KILL_COLOR_IN_SPEED;
			data.color.r = math::lerp( data.color.r, data.target_color.r, color_speed );
			data.color.g = math::lerp( data.color.g, data.target_color.g, color_speed );
			data.color.b = math::lerp( data.color.b, data.target_color.b, color_speed );
			data.color.a = math::lerp( data.color.a, data.target_color.a, dt * KILL_ALPHA_IN_SPEED );

			data.scale = easing::out_expo( 
				math::min( KILL_SCALE_TIME, KILL_TIME - data.life_time ) / KILL_SCALE_TIME );
			
			column++;
		}
		else
		{
			data.color.a = math::lerp( 
				data.color.a, 
				(uint8_t)0,
				dt * KILL_ALPHA_OUT_SPEED
			);

			if ( data.color.a == 0 )
			{
				itr = _kill_icons.erase( itr );
				break;
			}
		}

		itr++;
	}
}

void PlayerHUD::render( RenderBatch* render_batch )
{
	auto& engine = Engine::instance();
	auto window = engine.get_window();
	auto camera = engine.camera;

	auto spaceship = _controller->get_ship();
	if ( !spaceship ) return;

	Vec2 window_size = window->get_size();

	//  render crosshair
	{
		Vec3 aim_location = spaceship->get_shoot_location( Vec3 { 1.0f, 0.0f, 1.0f } );
		aim_location += spaceship->transform->get_forward() * CROSSHAIR_DISTANCE;

		Vec3 crosshair_pos = camera->world_to_viewport( aim_location );
		if ( crosshair_pos.z > 0.0f )
		{
			_draw_crosshair( render_batch, crosshair_pos );
		}
	}

	//  render kill icons
	int count = (int)_kill_icons.size();
	for ( int i = 0; i < count; i++ )
	{
		const auto& data = _kill_icons[i];

		render_batch->draw_texture( 
			Vec2 { 
				window_size.x * 0.5f + data.x_offset,
				window_size.y * 0.25f,
			},
			Vec2::one * data.scale * KILL_ICON_TEXTURE_SCALE,
			0.0f,
			Vec2 { 0.5f, 0.5f },
			_kill_icon_texture,
			data.color
		);
	}

	//  render missile-locking target
	SharedPtr<Spaceship> target = _controller->get_locked_target();
	if ( target )
	{
		Vec3 target_pos = camera->world_to_viewport( target->transform->location );
		if ( target_pos.z > 0.0f )
		{
			render_batch->draw_texture( 
				(Vec2)target_pos,
				Vec2::one * 1.0f,
				engine.get_updater()->get_accumulated_seconds() * 3.0f,
				Vec2 { 0.5f, 0.5f },
				_crosshair_line_texture,
				target->get_color()
			);
		}
	}
}

void PlayerHUD::_bind_to_spaceship( SharedPtr<Spaceship> spaceship )
{
	if ( !spaceship ) return;

	//  color
	_crosshair_color = spaceship->get_color();

	//  events
	spaceship->on_hit.listen( "player-hud", 
		std::bind( &PlayerHUD::_on_spaceship_hit, this, std::placeholders::_1 ) );
}

void PlayerHUD::_unbind_from_spaceship( SharedPtr<Spaceship> spaceship )
{
	if ( !spaceship ) return;

	spaceship->on_hit.unlisten( "player-hud" );
}

void PlayerHUD::_draw_crosshair( 
	RenderBatch* render_batch, 
	const Vec2& pos
)
{
	auto spaceship = _controller->get_ship();

	const float angle_iter = math::DOUBLE_PI / CROSSHAIR_LINES_COUNT;
	
	float shoot_ratio = 
		easing::in_out_cubic( spaceship->get_shoot_time() / 0.15f );
	float hit_ratio =
		easing::in_out_cubic( _hit_time / HIT_TIME );

	float distance = CROSSHAIR_LINES_DISTANCE 
		+ CROSSHAIR_LINES_SHOOT_DISTANCE * shoot_ratio;
	Vec2 scale = CROSSHAIR_LINE_SCALE
		+ CROSSHAIR_LINES_SHOOT_SCALE * hit_ratio;

	float angle = CROSSHAIR_START_ANGLE;
	for ( int i = 0; i < CROSSHAIR_LINES_COUNT; i++ )
	{
		Vec2 offset {
			math::cos( angle ) * distance,
			math::sin( angle ) * distance,
		};

		render_batch->draw_texture( 
			pos + offset,
			scale,
			angle,
			Vec2 { 0.5f, 0.5f }, 
			_crosshair_line_texture,
			_crosshair_color
		);
		angle += angle_iter;
	}
}

void PlayerHUD::_on_spaceship_hit( const DamageResult& result )
{
	_hit_time = HIT_TIME;
	_crosshair_color = Color::white;

	auto health = result.victim.lock();
	if ( !health ) return;

	auto spaceship = health->get_owner()->cast<Spaceship>();
	if ( spaceship && !result.is_alive )
	{
		KillIconData data {};
		data.life_time = KILL_TIME;
		data.target_color = spaceship->get_color();

		_kill_icons.push_back( data );
	}
}
