cmake_minimum_required(VERSION 3.16)
project(game_of_life)

# set the output directory for built objects.
# This makes sure that the dynamic library goes into the build directory automatically.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")

# This assumes the SDL source is available in vendored/SDL
add_subdirectory(vendored/SDL EXCLUDE_FROM_ALL)

add_library(vec vec.c)

# Create your game executable target as usual
add_executable(${PROJECT_NAME} game_of_life.c)

target_link_libraries(${PROJECT_NAME} PRIVATE vec)
target_link_libraries(${PROJECT_NAME} PRIVATE SDL3::SDL3)
