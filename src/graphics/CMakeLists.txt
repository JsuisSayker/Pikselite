add_library(graphics
    interface/interface.cpp
    renderer/renderer.cpp
    renderer/camera.cpp
    imgui/imguiInterface.cpp
    imgui/components/bars.cpp
    imgui/components/buttons.cpp
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
    box2d::box2d
    imgui::imgui
    pikselite_interface
)
