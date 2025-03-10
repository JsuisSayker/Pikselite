cmake_minimum_required(VERSION 3.10)

project(
  PIKSELITE_INTERFACE
  VERSION 0.1
  DESCRIPTION "SDL2 interface"
  LANGUAGES CXX
)

add_library(pikselite_interface INTERFACE)

target_include_directories(pikselite_interface
  INTERFACE "include"
  ../../extern/imgui
  ../../extern/imgui/backends
)