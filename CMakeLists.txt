cmake_minimum_required(VERSION 3.20)
project(PikseliteEngine)

# Utiliser le standard C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# -------------------------------------------------
# Si vcpkg est dans le dossier du projet, indiquer le toolchain
# (Optionnel si l'intégration vcpkg est activée globalement)
set(CMAKE_TOOLCHAIN_FILE "${CMAKE_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake"
    CACHE STRING "Vcpkg toolchain file")

# -------------------------------------------------
# Chercher les packages via vcpkg
find_package(SDL2 CONFIG REQUIRED)
find_package(GLEW CONFIG REQUIRED)
find_package(OpenGL REQUIRED)

# -------------------------------------------------
# Ajouter l'exécutable
add_executable(PikseliteEngine main.cpp)

# Lier les bibliothèques nécessaires
target_link_libraries(PikseliteEngine
    PRIVATE
        SDL2::SDL2
        SDL2::SDL2main
        GLEW::GLEW
        OpenGL::GL
)
