cmake_minimum_required(VERSION 3.10)

add_library(home
  Home.cpp
)

target_link_libraries(home
  pikselite_interface
  imgui
)