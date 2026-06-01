# -------------------------------------------------
# Game project auto-discovery
# Each subdirectory (except __template__) with a
# CMakeLists.txt is added as a build target.
# -------------------------------------------------
file(GLOB game_cmake_lists */CMakeLists.txt)
foreach(game_cmake ${game_cmake_lists})
    get_filename_component(game_dir ${game_cmake} DIRECTORY)
    get_filename_component(dir_name ${game_dir} NAME)
    if(NOT dir_name STREQUAL "__template__")
        add_subdirectory(${game_dir} ${game_dir})
    endif()
endforeach()
