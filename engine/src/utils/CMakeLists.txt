cmake_minimum_required(VERSION 3.10)

add_library(utils
  Clock.cpp
)

target_link_libraries(utils
  engine_interface
)