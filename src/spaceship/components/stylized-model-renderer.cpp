#include "stylized-model-renderer.h"

#include <suprengine/core/engine.h>

#include <gl/glew.h>

using namespace spaceship;

StylizedModelRenderer::StylizedModelRenderer(
	const SharedPtr<Model>& model,
	const Color modulate,
	const int priority_order
)
	: ModelRenderer( 
		model,
		"stylized",
		modulate,
		priority_order
	) {}

void StylizedModelRenderer::render( RenderBatch* render_batch )
{
	// Get offset scale
	float offset_scale = 1.0f;
	if ( dynamic_camera_distance_settings.is_active )
	{
		const SharedPtr<Camera> camera = render_batch->get_camera();

		// Compute distances
		const Vec3 dist = transform->location - camera->transform->location;
		const float dist_sqr = math::min(
			dynamic_camera_distance_settings.max_distance_sqr, 
			dist.length_sqr()
		);

		// Add offset scale
		const float offset = math::lerp(
			outline_scale, 
			dynamic_camera_distance_settings.max_outline_scale,
			dist_sqr / dynamic_camera_distance_settings.max_distance_sqr
		);
		offset_scale += offset;
	}
	else
	{
		offset_scale += outline_scale;
	}

	// Draw outline mesh
	if ( !math::near_value( offset_scale, 1.0f ) )
	{
		//  compute outline matrix
		const Mtx4 outline_matrix = Mtx4::create_from_transform(
			transform->scale * offset_scale,
			transform->rotation,
			transform->location
		);

		glFrontFace( draw_outline_ccw ? GL_CCW : GL_CW );
		render_batch->draw_model(
			outline_matrix,
			model,
			shader_name,
			modulate
		);
	}

	// Draw inner mesh
	if ( !draw_only_outline )
	{
		glFrontFace( GL_CW );
		render_batch->draw_model(
			transform->get_matrix(),
			model,
			shader_name,
			inner_modulate
		);
	}
}
