cmake_minimum_required(VERSION 3.10)

add_library(sprite_editor
  SpriteEditor.cpp
  SpriteEditorCamera.cpp
  SpriteEditorEvents.cpp
)

target_link_libraries(sprite_editor
  pikselite_interface
  imgui
  graphic
)