add_library(engine
    core.cpp
    event.cpp
    eventBus.cpp
    time.cpp
)

target_include_directories(engine
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}
)

target_link_libraries(engine
    PUBLIC
    pikselite_interface
    ${SDL2_LIBRARIES}
    ${GLEW_LIBRARIES}
    OpenGL::GL
)
