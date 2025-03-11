cmake_minimum_required(VERSION 3.10)

add_library(editor
  Editor.cpp
  EditorCamera.cpp
  EditorEvents.cpp
)

target_link_libraries(editor
  pikselite_interface
  imgui
  graphic
)