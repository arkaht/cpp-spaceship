# Spaceship
**Spaceship is a stylized C++ 3D game about spaceship movement, asteroids and the game feel of speed.**

It is built using [suprengine](https://github.com/arkaht/cpp-suprengine), my custom engine which uses OpenGL and SDL2.

![image](https://github.com/user-attachments/assets/e96ac9e4-6ac7-4cab-83c1-22017f746614)

## Dependencies
### Project
+ C++20 compiler
+ OpenGL 3.3.0
+ CMake 3.11

### Libraries
+ [cpp-suprengine](https://github.com/arkaht/cpp-suprengine)

## Features
+ Stylized rendering with flat colors for outlines.
+ Feedback for going at max speed such as with the camera and a thruster effect behind the spaceship.
+ Shootable asteroids roaming on their own.
+ Shoot with bullets and guided missiles, destroy spaceships or get destroyed and respawn.
+ Pawn-Controller architecture for spaceships similar to Unreal Engine, allowing to separate code for Player and AIs.

## Project Structure
This project only holds the engine code since it is de-coupled from games code.

**Folder structure:**
+ **`assets/`** contains game assets, such as models, textures, curves for explosion effects, etc.
+ **`src/`** contains source files of the game.

## Build the project
This project is built using **CMake 3.11**, ensure you have already installed a compatible version.

### Steps
0. Create a folder anywhere for the installation, we'll name it `spaceship`. The reason for this is that the game's `CMakeLists.txt` will attempt to finds the engine's folder directly from its own parent folder, if you don't want that, look at the [Troubleshooting](#troubleshooting) section.
1. Clone the engine's [repository](https://github.com/arkaht/cpp-suprengine) in `spaceship/cpp-suprengine`.
2. Run command `git submodule update --init` in the engine's folder.
3. Clone the game's repository (this one) in `spaceship/cpp-spaceship`.
4. Run the game's `CMakeLists.txt` either by using CMake's command line interpreter, CMake's GUI or your favorite IDE.

### Troubleshooting

<details><summary><b>Change engine's folder location</b></summary>

If for some reasons you want to move the engine's or game's folder apart from each other,
you can configure the location of the engine using the CMake variable `SUPRENGINE_PATH`.

Using command line:
```cmd
cd build
cmake .. -DSUPRENGINE_PATH=C:/Path/To/Engine/
```

You can also use **cmake-gui** to change this variable.
</details>

<details><summary><b>Couldn't update engine's Git Submodules</b></summary>

If running the git submodule update command didn't work for any reasons, replace the folder `libs/curve-x` by cloning [arkaht/cpp-curve-x](https://github.com/arkaht/cpp-curve-x).
</details>

<details><summary><b>Need administrator privileges to run the CMakeLists.txt</b></summary>

I'm using symbolic links to avoid having to copy the assets folder each time I compile. The problem is that on Windows, the symlink command requires administrator privileges. To resolve that, you can either:
+ Enable Windows developper mode (not recommended).
+ Run your IDE or CLI with administrator privileges.
+ Edit the `CMakeLists.txt` by replacing `suprengine_symlink_assets` with `suprengine_copy_assets`
</details>
