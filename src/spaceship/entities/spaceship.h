#pragma once

#include <spaceship/components/stylized-model-renderer.h>
#include <spaceship/components/health-component.h>
#include <spaceship/entities/spaceship-controller.h>
#include <spaceship/entities/projectile.h>

#include <suprengine/components/colliders/box-collider.h>

namespace spaceship
{
	using namespace suprengine;

	class Spaceship : public Entity
	{
	public:
		Spaceship();
		~Spaceship();

		void setup() override;
		void update_this( float dt ) override;

		SharedPtr<Spaceship> find_lockable_target(
			const Vec3& view_direction 
		) const;

		void shoot();
		void launch_missiles(const WeakPtr<HealthComponent>& wk_target);
		
		void die();
		void respawn();

		Vec3 get_shoot_location( const Vec3& axis_scale ) const;
		float get_shoot_time() const { return _shoot_time; }

		float get_throttle() const { return _throttle; }

		void set_color( const Color& color );
		Color get_color() const { return _color; }

		SharedPtr<HealthComponent> get_health_component() const { return _health; }

	public:
		/*
		 * Called when an entity has been damaged by the spaceship.
		 * 
		 * Parameters:
		 * - const DamageResult& result
		 */
		Event<const DamageResult&> on_hit;

	public:
		WeakPtr<SpaceshipController> wk_controller;

	private:
		//  Outline scale on models renderers
		const float MODEL_OUTLINE_SCALE = 0.03f;

		//  Throttle gain per second
		const float THROTTLE_GAIN_SPEED = 0.65f;
		//  Max throttle offset by keeping forward pressed
		const float MAX_THROTTLE_FORWARD_OFFSET = 0.25f;
		/*
		 * Speed at maximum throttle (i.e. 1.0), 
		 * proportionally scaled before and beyond 
		 */
		const float MAX_THROTTLE_SPEED = 90.0f;

		//  Trail starting throttle threshold 
		const float TRAIL_THROTTLE_START = 0.3f;
		//  Trail intensity smooth speed
		const float TRAIL_INTENSITY_SPEED = 2.0f;
		//  Trail model wave animation frequency
		const float TRAIL_WAVE_FREQUENCY = 15.0f;
		//  Trail model wave animation amplitude
		const float TRAIL_WAVE_AMPLITUDE = 0.25f;
		//  Trail model scale offset for intensity factor
		const float TRAIL_MODEL_SCALE_INTENSITY_OFFSET = 0.6f;
		//  Trail model scale
		const Vec3  TRAIL_MODEL_SCALE { 1.0f, 1.0f, 0.5f };
		//  Trail model outline scale added as an offset to scale
		const float TRAIL_MODEL_OUTLINE_SCALE_OFFSET = 2.0f;

		//  Shoot time interval
		const float SHOOT_TIME = 0.15f;

		//  Maximum distance for locking missiles to a target
		const float MISSILE_LOCK_MAX_DISTANCE = 500.0f;
		//  View-Direction dot product threshold for locking a target
		const float MISSILE_LOCK_DOT_THRESHOLD = 0.95f;

		//  Explosion size bounds evaluated by a linear 
		//  interpolation of last damage dealt
		const Vec2  EXPLOSION_SIZE { 9.0f, 15.0f };
		//  Explosion last max damage used to calculate its size
		const float EXPLOSION_DAMAGE_FOR_MAX_SIZE = 15.0f;
		//  Explosion random size deviation
		const Vec2  EXPLOSION_SIZE_DEVIATION { -1.0f, 2.0f };

	private:
		void _update_movement( float dt );
		void _update_trail( float dt );

		void _on_damage( const DamageResult& result );

	private:
		float _throttle = 0.0f;
		float _trail_intensity = 0.0f;
		Color _color = Color::green;

		float _shoot_time = 0.0f;

		SharedPtr<StylizedModelRenderer> _model_renderer;
		SharedPtr<StylizedModelRenderer> _trail_renderer;
		SharedPtr<BoxCollider> _collider;
		SharedPtr<HealthComponent> _health;

		static std::vector<WeakPtr<Spaceship>> _all_spaceships;
	};
}