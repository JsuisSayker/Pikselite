cmake_minimum_required(VERSION 3.10)

add_library(project_editor
  ProjectEditor.cpp
  ProjectEditorCamera.cpp
)

target_link_libraries(project_editor
  pikselite_interface
  engine_interface
  imgui
  graphic
)