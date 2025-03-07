cmake_minimum_required(VERSION 3.10)

project(
  PIKSELITE
  VERSION 0.1
  DESCRIPTION "Pikselite Engine"
  LANGUAGES CXX
)

# Set C++ Standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# check if sdl2 is installed
find_package(SDL2 REQUIRED CONFIG REQUIRED COMPONENTS SDL2)
find_package(SDL2 REQUIRED CONFIG COMPONENTS SDL2main)

add_executable(pikselite
  src/main.cpp
  src/graphic/Graphic.cpp
)

add_subdirectory(interface)

target_link_libraries(
  pikselite
  pikselite_interface
  SDL2::SDL2
  SDL2::SDL2main
)

add_custom_command(
  TARGET pikselite
  POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:pikselite> ${CMAKE_SOURCE_DIR}
)
