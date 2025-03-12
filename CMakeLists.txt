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

include(FetchContent)
FetchContent_Declare(
  SDL2
  GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
  GIT_TAG release-2.32.2
)
FetchContent_MakeAvailable(SDL2)

add_executable(pikselite
  src/main.cpp
)

# Add your project subdirectories
add_subdirectory(interface)
add_subdirectory(src/graphic)
add_subdirectory(src/editor)
add_subdirectory(src/home)
add_subdirectory(extern/imgui)

target_link_libraries(pikselite
  pikselite_interface
  graphic
  editor
  home
  SDL2::SDL2
  SDL2::SDL2main
  imgui
)

add_custom_command(
  TARGET pikselite
  POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:pikselite> ${CMAKE_SOURCE_DIR}
)
