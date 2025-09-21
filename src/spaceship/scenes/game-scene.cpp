#include "game-scene.h"

#include <spaceship/game-instance.h>
#include <spaceship/entities/explosion-effect.h>
#include <spaceship/components/player-hud.h>

#include <suprengine/core/assets.h>

#include "spaceship/player-manager.h"

using namespace spaceship;

GameScene::GameScene( GameInstance* game_instance )
	: _game_instance( game_instance )
{
}

void GameScene::init()
{
	Engine& engine = Engine::instance();

	random::seed( _seed );

	// Setup planet
	const SharedPtr<Entity> planet = engine.create_entity<Entity>();
	planet->transform->location = Vec3 { 2000.0f, 500.0f, 30.0f };
	planet->transform->rotation = Quaternion( DegAngles { -6.0f, 0.0f, 12.0f } );
	planet->transform->scale = Vec3( 10.0f );
	planet->create_component<StylizedModelRenderer>(
		Assets::get_model( "planet-ring" ),
		Color::from_0x( 0x1c6cF0FF )
	);

	// Spawn asteroids
	constexpr int ASTEROID_COUNT = 32;
	constexpr Vec3 ASTEROIDS_LOCATION { 500.0f, 100.0f, 50.0f };

	for ( int i = 0; i < ASTEROID_COUNT; i++ )
	{
		const SharedPtr<Asteroid> asteroid = engine.create_entity<Asteroid>();
		asteroid->transform->location = ASTEROIDS_LOCATION + random::generate_location(
			-300.0f, -300.0f, -300.0f,
			300.0f, 300.0f, 300.0f
		);
		asteroid->transform->rotation = Quaternion::look_at( random::generate_direction(), Vec3::up );
		asteroid->transform->scale = random::generate_scale( 4.0f, 30.0f );
		asteroid->linear_direction = Vec3::right * 2.0f * random::generate( 0.8f, 1.2f );
		asteroid->update_collision_to_transform();
	}

	_player_manager = std::make_unique<PlayerManager>( *engine.get_inputs() );

	// Spawn first player
	player_controller = _player_manager->create_player( _player_location, _player_rotation, 0 );
	spaceship1 = player_controller->get_ship();

	// Spawn second spaceship
	spaceship2 = engine.create_entity<Spaceship>();
	spaceship2->set_color( Color::from_0x( 0x9213f2FF ) );
	spaceship2->transform->location = Vec3 { 50.0f, 0.0f, 0.0f };

	// Possess it by AI
	ai_controller = engine.create_entity<AISpaceshipController>();
	ai_controller->possess( spaceship2 );
	//ai_controller->wk_target = spaceship1;

	// Instantiate temporary camera
	CameraProjectionSettings projection_settings = player_controller->get_camera()->get_projection_settings();
	projection_settings.fov = 50.0f;
	projection_settings.znear = 10.0f;

	const SharedPtr<Entity> camera_owner = engine.create_entity<Entity>();
	_temporary_camera = camera_owner->create_component<Camera>( projection_settings );
	_temporary_camera_model = camera_owner->create_component<ModelRenderer>( Assets::get_model( MESH_ARROW ) );

	//generate_ai_spaceships( 1 );
}

void GameScene::update( const float dt )
{
	Engine& engine = Engine::instance();
	const InputManager* inputs = engine.get_inputs();

	// Switch spaceship possession
	if ( inputs->is_key_just_pressed( PhysicalKey::One ) )
	{
		player_controller->possess( spaceship1 );
		ai_controller->possess( spaceship2 );
	}
	if ( inputs->is_key_just_pressed( PhysicalKey::Two ) )
	{
		player_controller->possess( spaceship2 );
	}

	if ( ( spawn_time -= dt ) <= 0.0f )
	{
		SharedPtr<ExplosionEffect> explosion = engine.create_entity<ExplosionEffect>(
			15.0f,
			random::generate_color()
		);
		explosion->transform->location = Vec3 { 0.0f, 100.0f, 0.0f };
		spawn_time += 2.5f;
	}
	
	// Window mode toggle
	if ( inputs->is_key_just_pressed( PhysicalKey::F1 ) )
	{
		Window* window = engine.get_window();
		if ( window->get_mode() != WindowMode::Windowed )
		{
			window->set_mode( WindowMode::Windowed );
		}
		else
		{
			window->set_mode( WindowMode::BorderlessFullscreen );
		}
	}
	if ( inputs->is_key_just_pressed( PhysicalKey::F2 ) )
	{
		OpenGLRenderBatch* renderer = _game_instance->get_render_batch();
		renderer->set_samples( renderer->get_samples() == 0 ? 8 : 0 );
	}
	if ( inputs->is_key_just_pressed( PhysicalKey::F3 ) )
	{
		const SharedPtr<Spaceship> spaceship = player_controller->get_ship();
		_player_location = spaceship->transform->location;
		_player_rotation = spaceship->transform->rotation;

		engine.clear_entities();
		init();
	}
	if ( inputs->is_key_just_pressed( PhysicalKey::F4 ) )
	{
		if ( spaceship1->get_color().a == 0 )
		{
			spaceship1->set_color( Color::from_0x( 0xf2cd13FF ) );
		}
		else
		{
			spaceship1->set_color( Color { 0, 0, 0, 0 } );
		}
	}
	// F5: place temporary camera
	if ( inputs->is_key_just_pressed( PhysicalKey::F5 ) )
	{
		const SharedPtr<Spaceship> spaceship = player_controller->get_ship();
		_temporary_camera->transform->set_location( spaceship->transform->location );
		_temporary_camera->transform->set_rotation( spaceship->transform->rotation );
		printf( "Temporary camera has been moved!\n" );
	}
	// F6: switch from spaceship camera to temporary camera
	if ( inputs->is_key_just_pressed( PhysicalKey::F6 ) )
	{
		if ( _temporary_camera->is_active() )
		{
			player_controller->get_camera()->set_active();
			player_controller->get_hud()->is_active = true;
			_temporary_camera->set_inactive();
			_temporary_camera_model->is_active = true;
			printf( "Switch camera to spaceship camera!\n" );
		}
		else
		{
			_temporary_camera->set_active();
			player_controller->get_camera()->set_inactive();
			player_controller->get_hud()->is_active = false;
			_temporary_camera_model->is_active = false;
			printf( "Switch camera to temporary camera!\n" );
		}
	}
	// F7: lock player inputs
	if ( inputs->is_key_just_pressed( PhysicalKey::F7 ) )
	{
		player_controller->is_inputs_enabled = !player_controller->is_inputs_enabled;
	}
	// F8: generate explosions around the player
	if ( inputs->is_key_just_pressed( PhysicalKey::F8 ) )
	{
		const int count = random::generate( 7, 14 );
		for ( int i = 0; i < count; i++ )
		{
			TIMER( i * random::generate( 0.1f, 0.25f ), {
				const SharedPtr<Spaceship> spaceship = player_controller->get_ship();

				const SharedPtr<ExplosionEffect> effect = engine.create_entity<ExplosionEffect>(
					random::generate( 15.0f, 20.0f ), 
					random::generate_color() 
				);
				effect->transform->location = 
					spaceship->transform->location 
					+ random::generate_direction() * random::generate( 100.0f, 200.0f );
			} );
		}
	}
}

void GameScene::generate_ai_spaceships( const int count )
{
	Engine& engine = Engine::instance();

	std::vector<SharedPtr<Spaceship>> potential_targets;
	std::vector<SharedPtr<AISpaceshipController>> controllers;

	for ( int i = 0; i < count; i++ )
	{
		auto spaceship = engine.create_entity<Spaceship>();
		spaceship->set_color( random::generate_color() );
		spaceship->transform->location = Vec3 
		{ 
			100.0f + static_cast<float>( i ) * 50.0f,
			0.0f,
			0.0f
		};
		potential_targets.push_back( spaceship );

		auto controller = engine.create_entity<AISpaceshipController>();
		controller->possess( spaceship );
		controllers.push_back( controller );
	}

	std::random_device rd {};
	std::default_random_engine rng( rd() );
	std::shuffle( potential_targets.begin(), potential_targets.end(), rng );

	for ( const SharedPtr<AISpaceshipController>& controller : controllers )
	{
		controller->wk_target = potential_targets[potential_targets.size() - 1];
		potential_targets.pop_back();
	}
}
