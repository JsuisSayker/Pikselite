add_library(engine
    core.cpp
    events/eventBus.cpp
    time.cpp
    managers/entityManager.cpp
    managers/systemManager.cpp
    managers/luaManager.cpp
    pixels/simulation.cpp
    ecs/systems/spriteSystem.cpp
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
    ${LUA_LIBRARIES}
)

target_include_directories(engine
    PUBLIC
        ${LUA_INCLUDE_DIR}
        ${SOL2_INCLUDE_DIRS}
)
