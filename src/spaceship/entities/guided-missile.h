#pragma once

#include <spaceship/components/stylized-model-renderer.h>
#include <spaceship/components/health-component.h>

#include <suprengine/core/entity.h>
#include <suprengine/components/lifetime-component.h>

namespace spaceship
{
	using namespace suprengine;

	class Spaceship;
	class HealthComponent;

	class GuidedMissile : public Entity
	{
	public:
		GuidedMissile(
			const SharedPtr<Spaceship>& owner,
			const WeakPtr<HealthComponent>& wk_target,
			Color color 
		);

		void setup() override;
		void update_this( float dt ) override;

		void explode();

	public:
		float move_speed = 175.0f;
		float move_acceleration = 16.0f;

		float rotation_speed = 30.0f;
		float rotation_acceleration = 6.0f;

		float impact_distance = 4.0f;
		float damage_amount = 17.0f;
		float knockback_force = 45.0f;

		float explosion_size = 2.0f;

		Vec3 up_direction { Vec3::up };

	private:
		void _update_target( float dt );
		void _check_impact();

		void _damage(const SharedPtr<HealthComponent>& target );

	private:
		const float LIFETIME = 6.0f;

		//  Time during which rotation is locked
		const float TIME_LOCKED_ROTATION = 0.04f;
		//  Ratio of move speed to apply at start
		const float STARTING_MOVE_SPEED_RATIO = 0.6f;

		const Vec2 EXPLOSION_SIZE_DEVIATION { -1.0f, 1.5f };

	private:
		float _current_move_speed { 0.0f };
		float _current_rotation_speed { 0.0f };
		
		Vec3 _desired_direction { Vec3::forward };
		WeakPtr<HealthComponent> _wk_target;
		WeakPtr<Spaceship> _wk_owner;

		Color _color;

		SharedPtr<StylizedModelRenderer> _model_renderer;
		SharedPtr<LifetimeComponent> _lifetime_component;
	};
}