#pragma once

#include <suprengine/components/colliders/sphere-collider.h>

#include <suprengine/core/entity.h>
#include <spaceship/components/stylized-model-renderer.h>
#include <spaceship/components/health-component.h>

namespace spaceship
{
	using namespace suprengine;

	class Asteroid : public Entity
	{
	public:
		Asteroid();
		~Asteroid();

		void setup() override;
		void update_this( float dt ) override;

		void update_collision_to_transform();

		void split();
			
	public:
		Vec3 linear_direction = Vec3::forward * 5.0f;
		int split_times = 2;

	private:
		void _on_damage( const DamageResult& result );

	private:
		SharedPtr<StylizedModelRenderer> _model_renderer;
		SharedPtr<SphereCollider> _collider;
		SharedPtr<HealthComponent> _health;
	};
}