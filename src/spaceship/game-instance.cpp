#include "game-instance.h"

#include <spaceship/scenes/game-scene.h>

#include <suprengine/core/assets.h>

using namespace spaceship;

void GameInstance::load_assets()
{
    // Shaders
	Assets::load_shader(
		"stylized",
		"assets/spaceship/shaders/stylized.vert",
		"assets/spaceship/shaders/stylized.frag"
	);

	// Textures
	Assets::load_texture( 
		"crosshair-line", 
		"assets/spaceship/sprites/crosshair-line.png" 
	);
	Assets::load_texture( 
		"kill-icon", 
		"assets/spaceship/sprites/kill-icon.png" 
	);

	// Models
	Assets::load_model( "spaceship", "assets/spaceship/models/spaceship2.fbx" );
	Assets::load_model( "projectile", "assets/spaceship/models/projectile.fbx" );
	Assets::load_model( "planet-ring", "assets/spaceship/models/planet-ring.fbx" );
	Assets::load_model( "asteroid0", "assets/spaceship/models/asteroid0.fbx" );
	Assets::load_model( "asteroid1", "assets/spaceship/models/asteroid1.fbx" );
	Assets::load_model( "explosion0", "assets/spaceship/models/explosion0.fbx" );
	Assets::load_model( "explosion1", "assets/spaceship/models/explosion1.fbx" );
	Assets::load_model( "explosion2", "assets/spaceship/models/explosion2.fbx" );

	// Curves
	Assets::load_curves_in_folder( "assets/spaceship/curves/", true, true );
}

void GameInstance::init()
{
	Engine& engine = Engine::instance();
	InputManager* inputs = engine.get_inputs();

    // Setup inputs
    inputs->set_relative_mouse_mode( true );

    // Setup render batch
	OpenGLRenderBatch* render_batch = get_render_batch();
	render_batch->set_background_color( Color::from_0x( 0x00000000 ) );

    // Load scene
	engine.create_scene<GameScene>( this );
}

void GameInstance::release()
{}

GameInfos GameInstance::get_infos() const
{
    GameInfos infos {};
    infos.window.title = "Spaceship X";
    infos.window.width = 1280;
	infos.window.height = 720;
    return infos;
}
