#pragma once

#include <suprengine/core/game.h>
#include <suprengine/rendering/opengl/opengl-render-batch.h>

#include "suprengine/input/input-manager.h"

namespace spaceship
{
	using namespace suprengine;

	class GameInstance : public Game<OpenGLRenderBatch>
	{
	public:
		void load_assets() override;

		void init() override;

		void release() override;

		GameInfos get_infos() const override;

	private:
		void setup_input_actions(InputManager* inputs);
	};
}
