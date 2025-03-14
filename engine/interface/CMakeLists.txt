cmake_minimum_required(VERSION 3.10)

project(
  ENGINE_INTERFACE
  VERSION 0.1
  DESCRIPTION "engine interface"
  LANGUAGES CXX
)

add_library(engine_interface INTERFACE)

target_include_directories(engine_interface
  INTERFACE "include"
  pikselite_interface
)
