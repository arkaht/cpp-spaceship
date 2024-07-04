#pragma once

#include <suprengine/scene.h>
#include <suprengine/components/colliders/box-collider.h>
#include <suprengine/components/mover.hpp>
#include <suprengine/components/mouse-looker.hpp>
#include <suprengine/random.h>

#include <spaceship/entities/player-spaceship-controller.h>
#include <spaceship/entities/ai-spaceship-controller.h>
#include <spaceship/entities/asteroid.h>

namespace spaceship 
{
	class GameInstance;

	class GameScene : public Scene
	{
	public:
		GameScene( GameInstance* game_instance );

		void init() override;
		void update( float dt ) override;

		void generate_ai_spaceships( int count );

	private:
		SharedPtr<Spaceship> spaceship1;
		SharedPtr<Spaceship> spaceship2;

		GameInstance* _game_instance { nullptr };

		SharedPtr<PlayerSpaceshipController> player_controller;
		SharedPtr<AISpaceshipController> ai_controller;

		float spawn_time { 0.0f };

		unsigned int _seed = (unsigned int)std::time( nullptr );

		SharedPtr<Camera> _temporary_camera;
		SharedPtr<ModelRenderer> _temporary_camera_model;

		Vec3 _player_location = Vec3::zero;
		Quaternion _player_rotation = Quaternion::identity;
	};
}