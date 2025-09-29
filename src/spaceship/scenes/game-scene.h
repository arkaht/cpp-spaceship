#pragma once

#include <suprengine/core/scene.h>
#include <suprengine/components/mover.hpp>
#include <suprengine/utils/random.h>

#include <spaceship/entities/player-spaceship-controller.h>
#include <spaceship/entities/ai-spaceship-controller.h>
#include <spaceship/entities/asteroid.h>

namespace spaceship
{
	class GameInstance;
	class PlayerManager;

	class GameScene : public Scene
	{
	public:
		GameScene( GameInstance* game_instance );

		void init() override;
		void update( float dt ) override;

		void generate_ai_spaceships( int count );

	private:
		WeakPtr<Spaceship> _spaceship1 {};
		WeakPtr<Spaceship> _spaceship2 {};

		GameInstance* _game_instance = nullptr;
		std::unique_ptr<PlayerManager> _player_manager = nullptr;

		WeakPtr<PlayerSpaceshipController> _player_controller {};
		WeakPtr<AISpaceshipController> _ai_controller {};

		float spawn_time { 0.0f };

		uint32 _seed = static_cast<unsigned int>(std::time(nullptr));

		SharedPtr<Camera> _temporary_camera {};
		SharedPtr<ModelRenderer> _temporary_camera_model {};

		Vec3 _player_location = Vec3::zero;
		Quaternion _player_rotation = Quaternion::identity;
	};
}
