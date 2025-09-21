#pragma once

#include "suprengine/utils/usings.h"
#include <vector>

#include "suprengine/components/camera.h"
#include "suprengine/math/vec3.h"
#include "suprengine/tools/vis-debug.h"
#include "suprengine/utils/memory.h"

namespace suprengine
{
    class InputManager;
}

namespace spaceship
{
    using namespace suprengine;

    class PlayerSpaceshipController;

    class PlayerManager
    {
    public:
        explicit PlayerManager( InputManager& inputs );
        ~PlayerManager();

        SharedPtr<PlayerSpaceshipController> create_player( Vec3 location, Quaternion rotation, int gamepad_id );
        void destroy_player(const SharedPtr<PlayerSpaceshipController>& player );

        SharedPtr<PlayerSpaceshipController> get_player_from_gamepad_id( int gamepad_id ) const;

        void update_viewports();

        SharedPtr<Camera> get_camera( int player_id ) const;

    private:
        void on_gamepad_connected( int gamepad_id );
        void on_gamepad_disconnected( int gamepad_id );

    private:
        std::vector<SharedPtr<PlayerSpaceshipController>> _controllers {};

        InputManager& _inputs;
    };
}