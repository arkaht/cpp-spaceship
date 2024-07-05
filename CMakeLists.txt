cmake_minimum_required(VERSION 3.11)

#  Setup Spaceship
project(SPACESHIP)
set(CMAKE_CXX_STANDARD 20)
set(SPACESHIP_INCLUDE "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(SPACESHIP_SOURCE "${SPACESHIP_INCLUDE}/spaceship")
set(SPACESHIP_ASSETS "${CMAKE_CURRENT_SOURCE_DIR}/assets")

#  Require Suprengine
set(SUPRENGINE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../cpp-suprengine/" CACHE FILEPATH "Path to the suprengine project")
add_subdirectory("${SUPRENGINE_PATH}" "suprengine")

#  Find source files
file(GLOB_RECURSE SPACESHIP_SOURCES CONFIGURE_DEPENDS "${SPACESHIP_SOURCE}/*.cpp")

#  Declare test project
add_executable(SPACESHIP)
set_target_properties(SPACESHIP PROPERTIES OUTPUT_NAME "spaceship")
target_include_directories(SPACESHIP PRIVATE "${SPACESHIP_INCLUDE}")
target_sources(SPACESHIP PRIVATE "${SPACESHIP_SOURCES}")
target_link_libraries(SPACESHIP PRIVATE SUPRENGINE)

#  Copy DLLs and assets
suprengine_copy_dlls(SPACESHIP)
suprengine_copy_assets(SPACESHIP "spaceship")