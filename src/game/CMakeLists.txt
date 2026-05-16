add_library(game
    Game.cpp
)

target_include_directories(game
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}
)

target_link_libraries(game
    PUBLIC
    pikselite_interface
    ${SDL2_LIBRARIES}
    ${GLEW_LIBRARIES}
    OpenGL::GL
    box2d::box2d
    ${LUA_LIBRARIES}
    box2d::box2d
    ZLIB::ZLIB
)

target_include_directories(game
    PUBLIC
        ${LUA_INCLUDE_DIR}
        ${SOL2_INCLUDE_DIRS}
)
