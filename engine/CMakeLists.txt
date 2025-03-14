cmake_minimum_required(VERSION 3.10)

add_subdirectory(interface)
# add_subdirectory(src/systems)

add_library(engine
  src/Core.cpp
)

target_link_libraries(engine
  engine_interface
  pikselite_interface
  # systems
  graphic
)