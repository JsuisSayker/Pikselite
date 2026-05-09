cmake_minimum_required(VERSION 3.20)
# -------------------------------------------------
# Set overlay ports directory
# -------------------------------------------------
set(VCPKG_OVERLAY_PORTS "${CMAKE_SOURCE_DIR}/external/overlay-ports")

set(CMAKE_MAP_IMPORTED_CONFIG_RELWITHDEBINFO "Debug" "Release")
set(CMAKE_MAP_IMPORTED_CONFIG_MINSIZEREL "Debug" "Release")
project(PikseliteEngine)

# Use C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable testing
option(ENABLE_COVERAGE "Enable coverage reporting" ON)
enable_testing()

# -------------------------------------------------
# Automatically detect system vcpkg installation
# -------------------------------------------------
if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    if(EXISTS "$ENV{USERPROFILE}/vcpkg/scripts/buildsystems/vcpkg.cmake")
        set(CMAKE_TOOLCHAIN_FILE "$ENV{USERPROFILE}/vcpkg/scripts/buildsystems/vcpkg.cmake"
            CACHE STRING "Vcpkg toolchain file")
    elseif(EXISTS "C:/vcpkg/scripts/buildsystems/vcpkg.cmake")
        set(CMAKE_TOOLCHAIN_FILE "C:/vcpkg/scripts/buildsystems/vcpkg.cmake"
            CACHE STRING "Vcpkg toolchain file")
    endif()
endif()

# -------------------------------------------------
# Find dependencies
# -------------------------------------------------
find_package(SDL2 CONFIG REQUIRED)
find_package(GLEW CONFIG REQUIRED)
find_package(OpenGL REQUIRED)
find_package(Stb REQUIRED)
find_package(Lua REQUIRED)
find_path(SOL2_INCLUDE_DIRS "sol/sol.hpp")
find_package(nlohmann_json CONFIG REQUIRED)
find_package(imgui_backends CONFIG REQUIRED)
find_package(ZLIB REQUIRED)

find_package(box2d CONFIG REQUIRED)
find_package(nfd CONFIG REQUIRED)
# find_package(imgui CONFIG REQUIRED)

# -------------------------------------------------
# Add subdirectories
# -------------------------------------------------
add_subdirectory(interface)
add_subdirectory(src/graphics)
add_subdirectory(src/editors)
add_subdirectory(src/engine)
add_subdirectory(tests)

# -------------------------------------------------
# Build executable
# -------------------------------------------------
add_executable(${PROJECT_NAME} src/main.cpp)

# -------------------------------------------------
# Add ImGui (manual linking)
# -------------------------------------------------
message(STATUS "CMAKE_SOURCE_DIR:${CMAKE_SOURCE_DIR}")

# add_library(imgui STATIC
#     ${CMAKE_SOURCE_DIR}/external/imgui/imgui.cpp
#     ${CMAKE_SOURCE_DIR}/external/imgui/imgui_draw.cpp
#     ${CMAKE_SOURCE_DIR}/external/imgui/imgui_widgets.cpp
#     ${CMAKE_SOURCE_DIR}/external/imgui/imgui_tables.cpp
# )
# target_include_directories(imgui PUBLIC ${CMAKE_SOURCE_DIR}/external/imgui)


# -------------------------------------------------
# Link manually built ImGui backends static library
# -------------------------------------------------
# Add the directory where imgui_backends.lib is generated
link_directories(${CMAKE_BINARY_DIR}/lib)

# Declare the static library manually if not found by vcpkg
# add_library(imgui_backends STATIC IMPORTED)
# set(VCPKG_INSTALLED_DIR "${CMAKE_BINARY_DIR}/vcpkg_installed/${VCPKG_TARGET_TRIPLET}/include")
# set_target_properties(imgui_backends PROPERTIES
#     IMPORTED_LOCATION "${CMAKE_BINARY_DIR}/lib/imgui_backends.lib"
#     INTERFACE_INCLUDE_DIRECTORIES "${VCPKG_INSTALLED_DIR}/imgui;${VCPKG_INSTALLED_DIR}/imgui/backends"
# )

option(PIKSELITE_ENABLE_PROFILING "Enable Tracy profiling" ON)

if(PIKSELITE_ENABLE_PROFILING)
    message(STATUS "TRACY PROFILING ENABLED")
    find_package(tracy CONFIG REQUIRED)
    target_link_libraries(engine PRIVATE Tracy::TracyClient)
    # -------------------------------------------------
    # Define the TRACY_ENABLE macro for the entire project to enable Tracy profiling
    # -------------------------------------------------
    # target_compile_definitions(${PROJECT_NAME} PRIVATE TRACY_ENABLE)
    target_compile_definitions(engine PRIVATE
        $<$<CONFIG:Debug>:TRACY_ENABLE>
    )

    # Move TracyClient.dll to the Debug folder after build
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "$<TARGET_FILE:Tracy::TracyClient>"
            $<TARGET_FILE_DIR:${PROJECT_NAME}>
        COMMENT "Copying TracyClient.dll to output directory"
    )
endif()

target_link_libraries(${PROJECT_NAME}
    PRIVATE
    SDL2::SDL2
    SDL2::SDL2main
    GLEW::GLEW
    OpenGL::GL
    imgui::imgui_backends
    pikselite_interface
    graphics
    engine
    editor
    nlohmann_json::nlohmann_json
    nfd::nfd
)
