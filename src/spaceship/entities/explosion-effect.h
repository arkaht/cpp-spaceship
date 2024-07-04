#pragma once

#include <spaceship/components/stylized-model-renderer.h>

#include <suprengine/curve.h>
#include <suprengine/components/lifetime-component.h>

namespace spaceship
{
	using namespace suprengine;

	class ExplosionEffect : public Entity
	{
	public:
		ExplosionEffect( 
			float explosion_size = 1.0f, 
			Color color = Color::white,
			int model_id = -1
		);

		void setup() override;
		void update_this( float dt ) override;

	public:
		float explosion_size = 1.0f;
		Color color = Color::white;

	private:
		//  Lifetime base value
		const float LIFETIME = 1.5f;
		//  Lifetime random deviation to add
		const float LIFETIME_DEVIATION = 0.2f;

		//  Maximum outline scale
		const float OUTLINE_SCALE = 0.3f;

		//  Random scale-bounds per axis to apply
		const Vec2 RANDOM_SCALE[3] =
		{
			Vec2 { 0.8f, 1.2f },  //  x-axis
			Vec2 { 0.8f, 1.2f },  //  y-axis
			Vec2 { 0.8f, 1.2f },  //  z-axis
		};

	private:
		float _max_lifetime = 0.0f;
		Vec3 _scale = Vec3::one;
		int _model_id;

		SharedPtr<LifetimeComponent> _lifetime_component;
		SharedPtr<StylizedModelRenderer> _model_renderer;

		SharedPtr<Curve> _curve_transform_scale;
		SharedPtr<Curve> _curve_outline_scale;
		SharedPtr<Curve> _curve_outline_color;
		SharedPtr<Curve> _curve_inner_color;
	};
}