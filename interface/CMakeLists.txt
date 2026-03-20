cmake_minimum_required(VERSION 3.10)

project(
  PIKSELITE_INTERFACE
  VERSION 0.1
  DESCRIPTION "pikselite interface"
  LANGUAGES CXX
)

add_library(pikselite_interface INTERFACE)

# -------------------------------------------------
# Link manually built ImGui backends static library
# -------------------------------------------------
# Add the directory where imgui_backends.lib is generated
link_directories(${CMAKE_BINARY_DIR}/lib)

# Declare the static library manually if not found by vcpkg
add_library(imgui_backends STATIC IMPORTED)
set(VCPKG_INSTALLED_DIR "${CMAKE_BINARY_DIR}/vcpkg_installed/${VCPKG_TARGET_TRIPLET}/include")
set_target_properties(imgui_backends PROPERTIES
    IMPORTED_LOCATION "${CMAKE_BINARY_DIR}/lib/imgui_backends.lib"
    INTERFACE_INCLUDE_DIRECTORIES "${VCPKG_INSTALLED_DIR}/imgui;${VCPKG_INSTALLED_DIR}/imgui/backends"
)

target_include_directories(pikselite_interface
  INTERFACE "include"
)

target_link_libraries(pikselite_interface
  INTERFACE
    imgui_backends
)