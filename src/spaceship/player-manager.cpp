#include "player-manager.h"

#include "suprengine/core/engine.h"
#include "suprengine/input/input-manager.h"

#include "entities/player-spaceship-controller.h"
#include "entities/spaceship.h"
#include "suprengine/utils/random.h"

using namespace spaceship;

constexpr int PLAYER_COLORS_COUNT = 5;
constexpr uint32 PLAYER_COLORS[PLAYER_COLORS_COUNT]
{
	0xF2CD13FF, // Yellow
	0xFF6978FF, // Bright pink
	0xB1EDE8FF, // Light blue
	0x3E2F5BFF, // Violet
	0x26B6A6FF, // Duck blue
};

PlayerManager::PlayerManager( InputManager& inputs )
	: _inputs( inputs )
{
	_inputs.on_gamepad_connected.listen( &PlayerManager::on_gamepad_connected, this );
	_inputs.on_gamepad_disconnected.listen( &PlayerManager::on_gamepad_disconnected, this );
}

static Color get_next_player_color()
{
	static int color_id = random::generate( 0, PLAYER_COLORS_COUNT - 1 );
	const Color player_color = Color::from_0x( PLAYER_COLORS[color_id] );
	color_id = ( color_id + 1 ) % PLAYER_COLORS_COUNT;

	return player_color;
}

SharedPtr<PlayerSpaceshipController> PlayerManager::create_player(
	const Vec3 location, const Quaternion rotation,
	const int gamepad_id
)
{
	if ( const SharedPtr<PlayerSpaceshipController> controller =
			get_player_from_gamepad_id( gamepad_id ) )
	{
		Logger::info( "Player with gamepad %d already exists, skipping creation.", gamepad_id );
		return controller;
	}

	const int player_id = static_cast<int>(_controllers.size());
	ASSERT( player_id < 4 );

	Engine& engine = Engine::instance();

	const SharedPtr<Spaceship> ship = engine.create_entity<Spaceship>();
	ship->set_color( get_next_player_color() );
	ship->transform->location = location;
	ship->transform->rotation = rotation;

	const InputContext input_context{
		.use_mouse_and_keyboard = gamepad_id == 0,
		.gamepad_id = gamepad_id
	};

	SharedPtr<PlayerSpaceshipController> controller =
		engine.create_entity<PlayerSpaceshipController>( input_context );
	controller->possess( ship );

	Logger::info( "Player using gamepad %d has been created.", gamepad_id );

	_controllers.push_back( controller );
	update_viewports();

	return controller;
}

void PlayerManager::destroy_player( const SharedPtr<PlayerSpaceshipController>& player )
{
	const auto itr = std::ranges::find( _controllers, player );
	ASSERT( itr != _controllers.end() );

	player->get_ship()->kill();
	player->kill();

	_controllers.erase( itr );
}

SharedPtr<PlayerSpaceshipController> PlayerManager::get_player_from_gamepad_id(
	const int gamepad_id ) const
{
	for ( const SharedPtr<PlayerSpaceshipController>& controller : _controllers )
	{
		const SharedPtr<InputComponent> input_component = controller->get_input_component();
		if ( input_component->get_input_context().gamepad_id == gamepad_id )
		{
			return controller;
		}
	}

	return nullptr;
}

void PlayerManager::update_viewports()
{
	if ( _controllers.empty() ) return;

	constexpr Rect DEFAULT_VIEWPORT { 0.0f, 0.0f, 1.0f, 1.0f };

	// Initialize first player viewport
	const SharedPtr<Camera> first_camera = get_camera( 0 );
	first_camera->set_viewport( DEFAULT_VIEWPORT );

	// Update other players by splitting viewports iteratively
	SplitDirection next_split_direction = SplitDirection::Vertical;
	SharedPtr<Camera> next_split_camera = first_camera;
	for ( int i = 1; i < _controllers.size(); i++ )
	{
		const SharedPtr<Camera> camera = get_camera( i );

		Rect lhs = next_split_camera->get_viewport();
		Rect rhs = lhs.split( next_split_direction );

		if ( next_split_direction == SplitDirection::Vertical )
		{
			// Swapping viewports in vertical since OpenGL's Y origin is at the bottom.
			std::swap( lhs, rhs );
		}

		// Apply viewports
		next_split_camera->set_viewport( lhs );
		camera->set_viewport( rhs );

		if ( i == 2 )
		{
			// The fourth player need to split the first player's viewport instead of the third
			// (which, otherwise, would make three screens on the bottom part of the window).
			next_split_camera = first_camera;
		}
		else
		{
			next_split_camera	 = camera;
			next_split_direction = SplitDirection::Horizontal;
		}
	}
}

SharedPtr<Camera> PlayerManager::get_camera( const int player_id ) const
{
	const SharedPtr<PlayerSpaceshipController> controller = _controllers[player_id];
	return controller->get_camera();
}

void PlayerManager::on_gamepad_connected( const int gamepad_id )
{
	create_player(
		Vec3 {
			0.0f,
			-100.0f * static_cast<float>( gamepad_id ),
			0.0f
		},
		Quaternion::identity,
		gamepad_id
	);
}

void PlayerManager::on_gamepad_disconnected( const int gamepad_id )
{
	const SharedPtr<PlayerSpaceshipController> controller =
		get_player_from_gamepad_id( gamepad_id );

	destroy_player( controller );
	update_viewports();
}
