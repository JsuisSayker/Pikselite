cmake_minimum_required(VERSION 3.10)

add_library(managers
  SystemManager.cpp
)

target_link_libraries(managers
  engine_interface
  graphic
)