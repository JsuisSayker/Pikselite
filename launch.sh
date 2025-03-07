#!/bin/env bash

binary="sdl2_poc"
buildDirectory="build"

function clean {
    rm -rf $binary
}

function fclean {
    clean
    rm -rf $buildDirectory
}

function compile {
    cmake -B $buildDirectory -S .
    cmake --build $buildDirectory -j $(nproc)
}

args=("$@")

if [ "$#" -gt 1 ]; then
    echo "To much arguments"
    exit 1
fi

if [ "${args[0]}" == "help" ] || [ "${args[0]}" == "h" ]; then
    echo -e "Usage: $0 [Options]\n"
    echo "Options:"
    echo "  none      Compile the project."
    echo "  re        Recompile the project from scratch. Use the fclean option."
    echo "  clean     Remove all post-compilation files (binaries...)."
    echo "  fclean    Remove all post-compilation and compilation files (binaries, build dir, .so files,...)."
    exit 0
fi

cmakePath=$( which cmake )
if [ ! $? -eq 0 ]; then
    echo -e "\nPlease install CMake first.\n"
    exit 1
fi

if [[ "${args[0]}" == "clean" ]] ; then
    clean
elif [[ "${args[0]}" == "fclean" ]] ; then
    fclean
elif [[ "${args[0]}" == "re" ]] ; then
    fclean
    compile
else
    compile
fi