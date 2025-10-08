cmake_minimum_required(VERSION 3.20)
project(PikseliteEngine)

# Use C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

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

# -------------------------------------------------
# Add subdirectories
# -------------------------------------------------
add_subdirectory(interface)
add_subdirectory(src/graphics)

# -------------------------------------------------
# Build executable
# -------------------------------------------------
add_executable(${PROJECT_NAME} src/main.cpp)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        SDL2::SDL2
        SDL2::SDL2main
        GLEW::GLEW
        OpenGL::GL
        pikselite_interface
        graphics
    
)
