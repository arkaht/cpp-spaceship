#include "game-instance.h"

#include <spaceship/scenes/game-scene.h>

#include <suprengine/core/assets.h>

using namespace spaceship;

void GameInstance::load_assets()
{
    //  shaders
	Assets::load_shader(
		"stylized",
		"assets/spaceship/shaders/stylized.vert",
		"assets/spaceship/shaders/stylized.frag"
	);

	//  textures
	Assets::load_texture( 
		"crosshair-line", 
		"assets/spaceship/sprites/crosshair-line.png" 
	);
	Assets::load_texture( 
		"kill-icon", 
		"assets/spaceship/sprites/kill-icon.png" 
	);

	//  models
	Assets::load_model( 
		"spaceship", "assets/spaceship/models/spaceship2.fbx" );
	Assets::load_model( 
		"projectile", "assets/spaceship/models/projectile.fbx" );
	Assets::load_model( 
		"planet-ring", "assets/spaceship/models/planet-ring.fbx" );
	Assets::load_model( 
		"asteroid0", "assets/spaceship/models/asteroid0.fbx" );
	Assets::load_model( 
		"asteroid1", "assets/spaceship/models/asteroid1.fbx" );
	Assets::load_model( 
		"explosion0", "assets/spaceship/models/explosion0.fbx" );
	Assets::load_model( 
		"explosion1", "assets/spaceship/models/explosion1.fbx" );
	Assets::load_model( 
		"explosion2", "assets/spaceship/models/explosion2.fbx" );

	//  curves
	Assets::load_curves_in_folder( "assets/spaceship/curves/", true, true );
}

void GameInstance::init()
{
    auto& engine = Engine::instance();
    auto inputs = engine.get_inputs();

    //  setup inputs
    inputs->set_relative_mouse_mode( true );

    //  setup render batch
    auto render_batch = get_render_batch();
	render_batch->set_background_color( Color::from_0x( 0x00000000 ) );

    //  load scene
	engine.create_scene<GameScene>( this );
}

void GameInstance::release()
{}

GameInfos GameInstance::get_infos() const
{
    GameInfos infos {};
    infos.title = "Spaceship X";
    infos.width = 1280;
	infos.height = 720;
    return infos;
}
