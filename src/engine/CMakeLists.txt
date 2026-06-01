add_library(engine
    core.cpp
    core_scene.cpp
    core_ecs.cpp
    events/eventBus.cpp
    time.cpp
    managers/entityManager.cpp
    managers/systemManager.cpp
    managers/luaManager.cpp
    pixels/simulation/element.cpp
    ecs/systems/spriteSystem.cpp
    physics/boxWorld.cpp
    scene/sceneSerializer.cpp
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
    box2d::box2d
    ${LUA_LIBRARIES}
    box2d::box2d
    ZLIB::ZLIB
)

target_include_directories(engine
    PUBLIC
        ${LUA_INCLUDE_DIR}
        ${SOL2_INCLUDE_DIRS}
)
