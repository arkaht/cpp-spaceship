#include "game-instance.h"

#include <spaceship/scenes/game-scene.h>

#include <suprengine/core/assets.h>

#include "inputs.h"

using namespace spaceship;

void GameInstance::load_assets()
{
    // Shaders
	Assets::load_shader(
		"stylized",
		"assets/spaceship/shaders/stylized.vert",
		"assets/spaceship/shaders/stylized.frag"
	);

	// Textures
	Assets::load_texture( 
		"crosshair-line", 
		"assets/spaceship/sprites/crosshair-line.png" 
	);
	Assets::load_texture( 
		"kill-icon", 
		"assets/spaceship/sprites/kill-icon.png" 
	);

	// Models
	Assets::load_model( "spaceship", "assets/spaceship/models/spaceship2.fbx" );
	Assets::load_model( "projectile", "assets/spaceship/models/projectile.fbx" );
	Assets::load_model( "planet-ring", "assets/spaceship/models/planet-ring.fbx" );
	Assets::load_model( "asteroid0", "assets/spaceship/models/asteroid0.fbx" );
	Assets::load_model( "asteroid1", "assets/spaceship/models/asteroid1.fbx" );
	Assets::load_model( "explosion0", "assets/spaceship/models/explosion0.fbx" );
	Assets::load_model( "explosion1", "assets/spaceship/models/explosion1.fbx" );
	Assets::load_model( "explosion2", "assets/spaceship/models/explosion2.fbx" );

	// Curves
	Assets::load_curves_in_folder( "assets/spaceship/curves/", true, true );
}

void GameInstance::init()
{
	Engine& engine = Engine::instance();
	InputManager* inputs = engine.get_inputs();

    // Setup inputs
    inputs->set_relative_mouse_mode( true );
	setup_input_actions( inputs );

    // Setup render batch
	OpenGLRenderBatch* render_batch = get_render_batch();
	render_batch->set_background_color( Color::from_0x( 0x00000000 ) );

    // Load scene
	engine.create_scene<GameScene>( this );
}

void GameInstance::release()
{}

GameInfos GameInstance::get_infos() const
{
    GameInfos infos {};
    infos.window.title = "Spaceship X";
    infos.window.width = 1280;
	infos.window.height = 720;
    return infos;
}

void GameInstance::setup_input_actions(InputManager* inputs)
{
	InputAction<Vec2>* move_action = inputs->create_action<Vec2>( MOVE_INPUT_ACTION_NAME );
	move_action->assign_keys( Axis2D::Y, PhysicalKey::S, PhysicalKey::W );
	move_action->assign_keys( Axis2D::X, PhysicalKey::A, PhysicalKey::D );
	move_action->assign_gamepad_joystick( JoystickSide::Left );

	InputAction<Vec2>* look_action = inputs->create_action<Vec2>( LOOK_INPUT_ACTION_NAME );
	look_action->assign_mouse_delta();
	look_action->assign_gamepad_joystick( JoystickSide::Right, JoystickInputModifier::NegateY );

	InputAction<bool>* rearview_action = inputs->create_action<bool>( REARVIEW_INPUT_ACTION_NAME );
	rearview_action->assign_mouse_button( MouseButton::Middle );
	rearview_action->assign_gamepad_button( GamepadButton::LeftShoulder );

	InputAction<bool>* shoot_action = inputs->create_action<bool>( SHOOT_INPUT_ACTION_NAME );
	shoot_action->assign_mouse_button( MouseButton::Left );
	shoot_action->assign_gamepad_button( GamepadButton::RightTrigger );

	InputAction<bool>* missile_action = inputs->create_action<bool>( MISSILE_INPUT_ACTION_NAME );
	missile_action->assign_mouse_button( MouseButton::Right );
	missile_action->assign_gamepad_button( GamepadButton::LeftTrigger );
}