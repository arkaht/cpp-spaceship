#include "player-spaceship-controller.h"

#include <spaceship/components/player-hud.h>

#include <suprengine/core/engine.h>
#include <suprengine/math/easing.h>

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

	// Movement inputs
	_inputs.throttle_delta = inputs->get_keys_as_axis( PhysicalKey::S, PhysicalKey::W, 1.0f );

	// Handle aim velocity
	{
		// Rotation inputs
		const Vec2 mouse_delta = -inputs->mouse_delta;
		const float yaw_delta = inputs->get_keys_as_axis( PhysicalKey::A, PhysicalKey::D, 1.0f );
		const float roll_delta = mouse_delta.x;
		const float pitch_delta = mouse_delta.y;

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

	// Shoot
	if ( inputs->is_mouse_button_down( MouseButton::Left ) && ship->get_shoot_time() <= 0.0f )
	{
		ship->shoot();
	}

	// Missile
	_wk_locked_target = ship->find_lockable_target( camera->transform->get_forward() );

	if ( inputs->is_mouse_button_just_pressed( MouseButton::Right ) )
	{
		if ( const SharedPtr<Spaceship> target = _wk_locked_target.lock() )
		{
			ship->launch_missiles( target->get_health_component() );
		}
	}
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
	
	// Find targets location & rotation
	Vec3 target_location = ship->transform->location + up * up_distance;
	Quaternion target_rotation = ship->transform->rotation;
	if ( inputs->is_mouse_button_down( MouseButton::Middle ) )
	{
		const float distance = math::lerp(
			CAMERA_LOOK_BACKWARD_DISTANCE.x, 
			CAMERA_LOOK_BACKWARD_DISTANCE.y, 
			throttle  //  looks better on linearly progressive distance
		);
		target_location += forward * distance;
		target_rotation = Quaternion::concatenate( 
			target_rotation, 
			Quaternion( up, math::PI ) 
		);
	}
	else
	{
		const float distance = math::lerp(
			CAMERA_BACKWARD.x, 
			CAMERA_BACKWARD.y, 
			throttle_ratio 
		);
		target_location += forward * -distance;
	}

	// Apply location & rotation
	const Vec3 location = Vec3::lerp(
		camera->transform->location,
		target_location,
		dt * smooth_move_speed
	);
	Quaternion rotation = Quaternion::lerp(
		camera->transform->rotation,
		target_rotation,
		dt * CAMERA_ROTATION_SPEED
	);
	camera->transform->set_location( location );
	camera->transform->set_rotation( rotation );

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
