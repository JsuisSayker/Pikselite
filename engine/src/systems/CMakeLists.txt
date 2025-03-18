cmake_minimum_required(VERSION 3.10)

add_library(systems
  MovementSystem.cpp
  InputSystem.cpp
  FireSystem.cpp
)

target_link_libraries(systems
  engine_interface
  graphic
)