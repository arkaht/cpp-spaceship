#pragma once

#include <suprengine/components/renderers/model-renderer.hpp>

namespace spaceship
{
	using namespace suprengine;

	struct CameraDynamicDistanceSettings
	{
		bool is_active = false;
		float max_distance_sqr = math::pow( 1024.0f, 2.0f );
		float max_outline_scale = 0.1f;
	};

	class StylizedModelRenderer : public ModelRenderer
	{
	public:
		StylizedModelRenderer(
			const SharedPtr<Model>& model,
			Color modulate = Color::white,
			int priority_order = 0
		);

		void render( RenderBatch* render_batch ) override;

	public:
		float outline_scale = 0.025f;
		bool draw_only_outline = false;
		bool draw_outline_ccw = true;

		Color inner_modulate = Color::black;

		CameraDynamicDistanceSettings dynamic_camera_distance_settings;
	};
}