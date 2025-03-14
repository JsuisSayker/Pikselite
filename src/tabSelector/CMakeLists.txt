cmake_minimum_required(VERSION 3.10)

add_library(tab_selector
  TabSelector.cpp
)

target_link_libraries(tab_selector
  pikselite_interface
  imgui
  graphic
  sprite_editor
  project_editor
)