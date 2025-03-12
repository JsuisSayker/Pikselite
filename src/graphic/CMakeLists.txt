cmake_minimum_required(VERSION 3.10)

add_library(graphic
  Graphic.cpp
  GraphicDraw.cpp
  GraphicEvents.cpp
  GraphicInterface.cpp
)

find_package(SDL2 REQUIRED)
find_package(SDL2_image REQUIRED)

target_link_libraries(graphic
  SDL2::SDL2
  SDL2::SDL2main
  SDL2_image::SDL2_image
  imgui
  pikselite_interface
)