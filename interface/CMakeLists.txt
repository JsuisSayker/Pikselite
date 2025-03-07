cmake_minimum_required(VERSION 3.10)

project(
  SDL2_POC_INTERFACE
  VERSION 0.1
  DESCRIPTION "SDL2 interface"
  LANGUAGES CXX
)

add_library(sdl2_poc_interface INTERFACE)

target_include_directories(sdl2_poc_interface
  INTERFACE "include"
)