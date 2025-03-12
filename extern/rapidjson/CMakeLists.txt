cmake_minimum_required(VERSION 3.10)
project(rapidjson_interface)

add_library(rapidjson_interface INTERFACE)

target_include_directories(rapidjson_interface INTERFACE 
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)
