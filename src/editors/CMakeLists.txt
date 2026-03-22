add_library(editor
    sprite/spriteEditor.cpp
    project/projectEditor.cpp
    aEditor.cpp
)

target_include_directories(editor
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${SDL2_INCLUDE_DIRS}
        ${GLEW_INCLUDE_DIRS}
)

target_link_libraries(editor
    PUBLIC
    ${SDL2_LIBRARIES}
    ${GLEW_LIBRARIES}
    OpenGL::GL
    pikselite_interface
)
