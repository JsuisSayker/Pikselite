vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO ocornut/imgui
    REF v1.91.9
    SHA512 c9393bd9f6b49b036ad6ab3ba4d972876c6f60ce7f5c13a7a56ff11b3559ea3211b0caa03eed10b4f4fbe9c371e14f7f24866bd476652f543f3ed3aa878ea930
)

# ------------------------------------------------------------------
# Inject a minimal CMakeLists.txt into the ImGui source folder
# ------------------------------------------------------------------
file(WRITE "${SOURCE_PATH}/CMakeLists.txt" "
cmake_minimum_required(VERSION 3.15)
project(imgui_backends CXX)

set(IMGUI_SRC
    imgui.cpp
    imgui_draw.cpp
    imgui_tables.cpp
    imgui_widgets.cpp
    backends/imgui_impl_sdl2.cpp
    backends/imgui_impl_opengl3.cpp
)

add_library(imgui_backends STATIC \${IMGUI_SRC})

target_include_directories(imgui_backends PUBLIC
    \${CMAKE_CURRENT_SOURCE_DIR}
    \${CMAKE_CURRENT_SOURCE_DIR}/backends
)

find_package(SDL2 REQUIRED)
find_package(OpenGL REQUIRED)

target_link_libraries(imgui_backends PUBLIC SDL2::SDL2 OpenGL::GL)

# ✅ Add install rules so vcpkg_install_cmake() works
install(TARGETS imgui_backends
    ARCHIVE DESTINATION lib
)
install(DIRECTORY \${CMAKE_CURRENT_SOURCE_DIR}/
    DESTINATION include/imgui
    FILES_MATCHING PATTERN \"*.h\"
)
install(DIRECTORY \${CMAKE_CURRENT_SOURCE_DIR}/backends/
    DESTINATION include/imgui/backends
    FILES_MATCHING PATTERN \"*.h\"
)
")

# ------------------------------------------------------------------
# Configure, build, and install using vcpkg helpers
# ------------------------------------------------------------------
# --- Configure the build ---
vcpkg_configure_cmake(
    SOURCE_PATH ${SOURCE_PATH}
    PREFER_NINJA
    OPTIONS
        -DBUILD_SHARED_LIBS=OFF
)

vcpkg_build_cmake()

vcpkg_install_cmake()

# ------------------------------------------------------------------
# Copy resulting static lib to your project’s lib/ folder
# ------------------------------------------------------------------
file(GLOB LIB_FILES "${CURRENT_BUILDTREES_DIR}/${TARGET_TRIPLET}-rel/*.lib")
set(PROJECT_LIB_DIR "${CURRENT_BUILDTREES_DIR}/../../../../lib")
file(MAKE_DIRECTORY "${PROJECT_LIB_DIR}")

foreach(LIB_FILE ${LIB_FILES})
    file(COPY "${LIB_FILE}" DESTINATION "${PROJECT_LIB_DIR}")
endforeach()
