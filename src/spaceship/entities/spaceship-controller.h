#pragma once

#include <suprengine/core/entity.h>
#include <suprengine/utils/event.h>

namespace spaceship
{
	using namespace suprengine;

	class Spaceship;

	struct SpaceshipControlInputs
	{
		float throttle_delta = 0.0f;

		Quaternion desired_rotation = Quaternion::identity;
		bool should_smooth_rotation = true;
		float smooth_rotation_speed = 1.0f;
	};

	class SpaceshipController : public Entity
	{
	public:
		virtual ~SpaceshipController();

		void possess(const SharedPtr<Spaceship>& ship);
		void unpossess();

		SharedPtr<Spaceship> get_ship() const { return _possessed_ship.lock(); }
		const SpaceshipControlInputs& get_inputs() const { return _inputs; }

	public:
		virtual void on_possess() {};
		virtual void on_unpossess() {};

		virtual void update_inputs( float dt ) = 0;

	public:
		/*
		 * Called when the possessed spaceship has been changed.
		 * 
		 * Params:
		 * - SharedPtr<Spaceship> previous
		 * - SharedPtr<Spaceship> current
		 */
		Event<SharedPtr<Spaceship>, SharedPtr<Spaceship>> on_possess_changed;

	protected:
		WeakPtr<Spaceship> _possessed_ship;
		SpaceshipControlInputs _inputs {};

	private:
		bool _suppress_event = false;
	};
}