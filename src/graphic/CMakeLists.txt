cmake_minimum_required(VERSION 3.10)

add_library(graphic
  Graphic.cpp
  GraphicDraw.cpp
  GraphicEvents.cpp
  GraphicInterface.cpp
)

target_link_libraries(graphic
  SDL2::SDL2
  SDL2::SDL2main
  imgui
  pikselite_interface
)