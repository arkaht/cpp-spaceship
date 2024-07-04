#pragma once

#include <spaceship/entities/spaceship.h>

namespace spaceship
{
	using namespace suprengine;

	class AISpaceshipController : public SpaceshipController
	{
	public:
		AISpaceshipController();

		void update_inputs( float dt ) override;

	public:
		WeakPtr<Spaceship> wk_target;
	};
}