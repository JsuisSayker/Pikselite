add_library(graphics
    interface/interface.cpp
    renderer/renderer.cpp
    renderer/camera.cpp
)

target_include_directories(graphics
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${SDL2_INCLUDE_DIRS}
        ${GLEW_INCLUDE_DIRS}
)

target_link_libraries(graphics
    PUBLIC
    ${SDL2_LIBRARIES}
    ${GLEW_LIBRARIES}
    OpenGL::GL
    pikselite_interface
)
