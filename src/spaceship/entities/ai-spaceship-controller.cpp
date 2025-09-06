#include "ai-spaceship-controller.h"

using namespace spaceship;

AISpaceshipController::AISpaceshipController()
{}

void AISpaceshipController::update_inputs( float dt )
{
	const SharedPtr<Spaceship> ship = get_ship();

	if ( const SharedPtr<Spaceship> target = wk_target.lock())
	{
		const Vec3 dir = target->transform->location - ship->transform->location;
		const Vec3 normalized_dir = dir.normalized();

		const Quaternion desired_rotation = Quaternion::look_at( 
			normalized_dir,
			Vec3::up
		);

		const float forward_alignement = Vec3::dot(
			normalized_dir, 
			ship->transform->get_forward() 
		);

		//  shoot if aligned
		if ( ship->get_shoot_time() <= 0.0f && forward_alignement >= 0.9f )
		{
			ship->shoot();
		}

		_inputs.throttle_delta = forward_alignement;

		_inputs.desired_rotation = desired_rotation;

		_inputs.should_smooth_rotation = true;
		_inputs.smooth_rotation_speed = 2.0f;
	}
	else
	{
		_inputs.throttle_delta = 0.0f;
	}
}
