cmake_minimum_required(VERSION 3.10)

add_library(core
  Core.cpp
)

target_link_libraries(core
  pikselite_interface
  graphic
)