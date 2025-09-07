#include "player-spaceship-controller.h"

#include <spaceship/components/player-hud.h>

#include <suprengine/core/engine.h>
#include <suprengine/math/easing.h>

#include "spaceship/inputs.h"
#include "suprengine/components/input-component.h"

using namespace spaceship;

PlayerSpaceshipController::PlayerSpaceshipController()
{}

PlayerSpaceshipController::~PlayerSpaceshipController()
{
	camera->get_owner()->kill();
}

void PlayerSpaceshipController::setup()
{
	auto& engine = Engine::instance();

	hud = create_component<PlayerHUD>( as<PlayerSpaceshipController>() );

	//  setup camera settings
	CameraProjectionSettings projection_settings {};
	projection_settings.znear = 1.0f;
	projection_settings.zfar = 10000.0f;

	//  initialize camera
	auto camera_owner = engine.create_entity<Entity>();
	camera = camera_owner->create_component<Camera>( projection_settings );
	camera->set_active();

	_input_component = create_component<InputComponent>(
		InputContext {
			.use_mouse_and_keyboard = true,
			.gamepad_id = 0
		}
	);

	//  test: second camera
	camera_owner = engine.create_entity<Entity>();
	camera_owner->transform->scale = Vec3( 10.0f );
}

void PlayerSpaceshipController::update_this( const float dt )
{
	const SharedPtr<Spaceship> ship = get_ship();
	if ( !ship || ship->state != EntityState::Active ) return;

	_update_shoot( dt );
	_update_camera( dt );
}

void PlayerSpaceshipController::on_possess()
{
}

void PlayerSpaceshipController::on_unpossess()
{
}

void PlayerSpaceshipController::update_inputs( const float dt )
{
	if ( !is_inputs_enabled ) return;

	const Engine& engine = Engine::instance();
	const InputManager* inputs = engine.get_inputs();
	const SharedPtr<Spaceship> ship = get_ship();

	const InputAction<Vec2>* move_input_action = inputs->get_action<Vec2>( MOVE_INPUT_ACTION_NAME );
	const InputAction<Vec2>* look_input_action = inputs->get_action<Vec2>( LOOK_INPUT_ACTION_NAME );

	const Vec2 move_value = _input_component->read_value( move_input_action );
	const Vec2 look_value = _input_component->read_value( look_input_action );

	// Movement inputs
	_inputs.throttle_delta = move_value.y;

	// Handle aim velocity
	{
		// Rotation inputs
		const float yaw_delta = move_value.x;
		const float roll_delta = -look_value.x;
		const float pitch_delta = -look_value.y;

		// Add inputs
		_aim_velocity.x = math::clamp( 
			_aim_velocity.x + roll_delta * AIM_SENSITIVITY.x, 
			-MAX_AIM_VELOCITY, 
			MAX_AIM_VELOCITY 
		);
		_aim_velocity.y = math::clamp( 
			_aim_velocity.y + pitch_delta * AIM_SENSITIVITY.y, 
			-MAX_AIM_VELOCITY, 
			MAX_AIM_VELOCITY 
		);
		_aim_velocity.z = math::clamp(
			_aim_velocity.z + yaw_delta * AIM_SENSITIVITY.z,
			-MAX_AIM_VELOCITY,
			MAX_AIM_VELOCITY
		);

		// Drag to zero
		_aim_velocity = Vec3::lerp( 
			_aim_velocity, 
			Vec3::zero,
			dt * AIM_VELOCITY_DECREASE
		);
	}

	// Input rotation
	Quaternion rotation = ship->transform->rotation;
	rotation = rotation + Quaternion( 
		ship->transform->get_forward(), 
		_aim_velocity.x * dt 
	);
	rotation = rotation + Quaternion( 
		ship->transform->get_right(), 
		_aim_velocity.y * dt 
	);
	rotation = rotation + Quaternion(
		ship->transform->get_up(),
		_aim_velocity.z * dt
	);
	_inputs.desired_rotation = rotation;

	// We want the player to directly control its rotation, so no smoothing.
	_inputs.should_smooth_rotation = false;
}

void PlayerSpaceshipController::_update_shoot( float dt )
{
	const Engine& engine = Engine::instance();
	const InputManager* inputs = engine.get_inputs();
	const SharedPtr<Spaceship> ship = get_ship();

	const InputAction<bool>* shoot_input_action = inputs->get_action<bool>( SHOOT_INPUT_ACTION_NAME );
	const InputAction<bool>* missile_input_action = inputs->get_action<bool>( MISSILE_INPUT_ACTION_NAME );

	// Shoot
	if ( _input_component->read_value( shoot_input_action ) && ship->get_shoot_time() <= 0.0f )
	{
		ship->shoot();
	}

	// Missile
	_wk_locked_target = ship->find_lockable_target( camera->transform->get_forward() );

	const bool missile_input = _input_component->read_value( missile_input_action );
	if ( missile_input && !_last_missile_input )
	{
		if ( const SharedPtr<Spaceship> target = _wk_locked_target.lock() )
		{
			ship->launch_missiles( target->get_health_component() );
		}
	}

	_last_missile_input = missile_input;
}

void PlayerSpaceshipController::_update_camera( const float dt )
{
	const Engine& engine = Engine::instance();
	const InputManager* inputs = engine.get_inputs();
	const SharedPtr<Spaceship> ship = get_ship();

	// Get parameters
	const float throttle = ship->get_throttle();
	const float throttle_ratio = easing::in_out_cubic( throttle / 1.0f );
	const float smooth_move_speed = math::lerp( CAMERA_MOVE_SPEED.x, CAMERA_MOVE_SPEED.y, throttle_ratio );
	const float up_distance = math::lerp( CAMERA_UP_RANGE.x, CAMERA_UP_RANGE.y, throttle_ratio );

	// Get axes
	const Vec3 up = ship->transform->get_up();
	const Vec3 forward = ship->transform->get_forward();

	const InputAction<bool>* rearview_input_action = inputs->get_action<bool>( REARVIEW_INPUT_ACTION_NAME );

	// Rearview feature
	Vec3 target_location = ship->transform->location + up * up_distance;
	if ( _input_component->read_value( rearview_input_action ) )
	{
		const float distance = math::lerp(
			CAMERA_REARVIEW_DISTANCE.x,
			CAMERA_REARVIEW_DISTANCE.y,
			throttle  // Looks better on linearly progressive distance
		);
		target_location += forward * distance;
		camera->transform->set_location( target_location );

		const Quaternion rearview_rotation = Quaternion::concatenate(
			ship->transform->rotation,
			Quaternion( up, math::PI ) 
		);
		camera->transform->set_rotation( rearview_rotation );

		_is_rearview_enabled = true;
	}
	else
	{
		const float distance = math::lerp(
			CAMERA_BACKWARD.x, 
			CAMERA_BACKWARD.y, 
			throttle_ratio 
		);
		target_location += forward * -distance;

		const Quaternion target_rotation = ship->transform->rotation;
		const Quaternion rotation = Quaternion::lerp(
			camera->transform->rotation,
			target_rotation,
			dt * CAMERA_ROTATION_SPEED
		);

		// Apply location
		const Vec3 location = Vec3::lerp(
			camera->transform->location,
			target_location,
			dt * smooth_move_speed
		);

		// Teleport camera first frame when stopping the rearview,
		// this could also be handled better with two distinct cameras.
		if ( _is_rearview_enabled )
		{
			camera->transform->set_location( target_location );
			camera->transform->set_rotation( target_rotation );
			_is_rearview_enabled = false;
		}
		else
		{
			camera->transform->set_location( location );
			camera->transform->set_rotation( rotation );
		}
	}

	// Update up direction for roll
	camera->up_direction = Vec3::lerp( 
		camera->up_direction, 
		up, 
		dt * CAMERA_ROTATION_SPEED 
	);

	// Update fov
	const float fov = math::lerp(
		CAMERA_FOV.x, 
		CAMERA_FOV.y, 
		throttle_ratio
	);
	/*float time = _game->get_timer()->get_accumulated_seconds();
	fov += throttle_ratio 
		 * math::sin( time * 5.0f )
		 * 1.0f;*/
	camera->set_fov( fov );
}
