cmake_minimum_required(VERSION 3.21)
project(imgui_backends)

# Path to real imgui source (passed from portfile)
set(IMGUI_DIR ${IMGUI_SOURCE_PATH})

include(GNUInstallDirs)

set(CMAKE_DEBUG_POSTFIX d)

# -------------------------
# IMGUI CORE
# -------------------------
add_library(imgui STATIC
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
)

target_include_directories(imgui PUBLIC
    $<BUILD_INTERFACE:${IMGUI_DIR}>
    $<INSTALL_INTERFACE:include>
)

# -------------------------
# IMGUI BACKENDS
# -------------------------
add_library(imgui_backends STATIC
    ${IMGUI_DIR}/backends/imgui_impl_sdl2.cpp
    ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
)

target_include_directories(imgui_backends PUBLIC
    $<BUILD_INTERFACE:${IMGUI_DIR}>
    $<BUILD_INTERFACE:${IMGUI_DIR}/backends>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(imgui_backends PUBLIC imgui)

# Dependencies
find_package(SDL2 CONFIG REQUIRED)
find_package(OpenGL REQUIRED)

target_link_libraries(imgui_backends PUBLIC
    SDL2::SDL2
    OpenGL::GL
)

set_target_properties(imgui imgui_backends PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    ARCHIVE_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/lib"
    ARCHIVE_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/debug/lib"
)

# Optional but safe (config mapping)
# foreach(t imgui imgui_backends)
#     set_target_properties(${t} PROPERTIES
#         MAP_IMPORTED_CONFIG_RELWITHDEBINFO Release
#         MAP_IMPORTED_CONFIG_MINSIZEREL Release
#     )
# endforeach()

# -------------------------
# INSTALL
# -------------------------
install(TARGETS imgui imgui_backends
    EXPORT imgui_backendsTargets
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)

# Headers
file(GLOB IMGUI_HEADERS ${IMGUI_DIR}/*.h)
install(FILES ${IMGUI_HEADERS}
    DESTINATION include
)

file(GLOB IMGUI_BACKEND_HEADERS ${IMGUI_DIR}/backends/*.h)
install(FILES ${IMGUI_BACKEND_HEADERS}
    DESTINATION include/imgui/backends
)

# -------------------------
# EXPORT CONFIG
# -------------------------
install(EXPORT imgui_backendsTargets
    FILE imgui_backendsTargets.cmake
    NAMESPACE imgui::
    DESTINATION lib/cmake/imgui_backends
)

include(CMakePackageConfigHelpers)

configure_package_config_file(
    ${CMAKE_CURRENT_LIST_DIR}/imgui_backendsConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/imgui_backendsConfig.cmake
    INSTALL_DESTINATION lib/cmake/imgui_backends
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/imgui_backendsConfig.cmake
    DESTINATION lib/cmake/imgui_backends
)