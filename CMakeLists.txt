cmake_minimum_required(VERSION 3.21)
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

# -------------------------------------------------
# Options
# -------------------------------------------------
option(BUILD_EDITORS "Build editor tools (sprite editor, project editor, ImGui)" ON)
option(ENABLE_COVERAGE "Enable coverage reporting" ON)
option(PIKSELITE_ENABLE_PROFILING "Enable Tracy profiling" ON)

# Enable testing
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
# Find dependencies (core - needed by all targets)
# -------------------------------------------------
find_package(SDL2 CONFIG REQUIRED)
find_package(GLEW CONFIG REQUIRED)
find_package(OpenGL REQUIRED)
find_package(Stb REQUIRED)
find_package(Lua REQUIRED)
find_path(SOL2_INCLUDE_DIRS "sol/sol.hpp")
find_package(nlohmann_json CONFIG REQUIRED)
find_package(ZLIB REQUIRED)
find_package(box2d CONFIG REQUIRED)

# Editor-only dependencies
find_package(imgui_backends CONFIG REQUIRED)
if(BUILD_EDITORS)
    find_package(nfd CONFIG REQUIRED)
endif()

link_directories(${CMAKE_BINARY_DIR}/lib)

# -------------------------------------------------
# Core libraries (always built)
# -------------------------------------------------
add_subdirectory(interface)
add_subdirectory(src/graphics)
add_subdirectory(src/engine)
add_subdirectory(src/game)

# Profiling (optional, links to engine target — must be after engine subdirectory)
if(PIKSELITE_ENABLE_PROFILING)
    message(STATUS "TRACY PROFILING ENABLED")
    find_package(tracy CONFIG REQUIRED)
    target_link_libraries(engine PRIVATE Tracy::TracyClient)
    target_link_libraries(game PRIVATE Tracy::TracyClient)
    target_link_libraries(graphics PRIVATE Tracy::TracyClient)
    target_link_libraries(pikselite_interface INTERFACE Tracy::TracyClient)
    target_compile_definitions(engine PRIVATE
        $<$<CONFIG:Debug>:TRACY_ENABLE>
    )
    target_compile_definitions(game PRIVATE
        $<$<CONFIG:Debug>:TRACY_ENABLE>
    )
endif()

# -------------------------------------------------
# Editor executable
# -------------------------------------------------
if(BUILD_EDITORS)
    add_subdirectory(src/editors)

        add_subdirectory(tests)

    add_executable(${PROJECT_NAME} src/main.cpp)

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
        game
        editor
        nlohmann_json::nlohmann_json
        nfd::nfd
    )

    # Copy all runtime DLLs to the executable output directory
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "$<TARGET_RUNTIME_DLLS:${PROJECT_NAME}>"
            "$<TARGET_FILE_DIR:${PROJECT_NAME}>"
        COMMAND_EXPAND_LISTS
        COMMENT "Copying runtime DLLs to output directory"
    )
endif()

# -------------------------------------------------
# Game projects (auto-discovered from games/)
# -------------------------------------------------
add_subdirectory(games)
