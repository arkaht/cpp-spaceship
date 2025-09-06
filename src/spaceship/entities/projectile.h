#pragma once

#include <spaceship/components/stylized-model-renderer.h>

#include <suprengine/core/entity.h>
#include <suprengine/components/lifetime-component.h>
#include <suprengine/utils/ray.h>

namespace spaceship
{
	using namespace suprengine;

	class Spaceship;

	class Projectile : public Entity
	{
	public:
		Projectile(const SharedPtr<Spaceship>& owner, Color color );

		void setup() override;
		void update_this( float dt ) override;

	public:
		float move_speed = 750.0f;

		float damage_amount = 5.0f; 
		float knockback_force = 80.0f;

	private:
		bool _check_collisions( float movement_speed );
		void _on_hit( const RayHit& result );

	private:
		const float LIFETIME = 3.0f;
	
	private:
		Color _color;

		WeakPtr<Spaceship> _wk_owner;

		SharedPtr<StylizedModelRenderer> _model_renderer;
		SharedPtr<LifetimeComponent> _lifetime_component;
	};
}

