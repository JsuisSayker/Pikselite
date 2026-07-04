find_package(GTest CONFIG REQUIRED)

add_executable(pikselite_tests
    imguiComponents/testBarComponent.cpp
    imguiComponents/testGetDesiredPos.cpp
    # renderer/testSpriteEditor.cpp
    # renderer/testProjectEditor.cpp
    renderer/testDrawOnEditor.cpp
    engine/testEcs.cpp
    engine/testEcsLoad.cpp
    engine/testEcsResilience.cpp
    engine/testEcsStress.cpp
    engine/testPhysics.cpp
    engine/testSceneSerialization.cpp
    engine/testSimulationLoad.cpp
    engine/testSimulationResilience.cpp
    engine/testSimulationScenarios.cpp
    # engine/testEcsMigration.cpp
    # engine/testChunk.cpp
    graphics/testWindowSize.cpp
)

target_compile_definitions(pikselite_tests PRIVATE UNIT_TEST)

target_include_directories(pikselite_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(pikselite_tests
    PRIVATE
    GTest::gtest
    GTest::gtest_main
    pikselite_interface
    graphics
    engine
    editor
)

# Copy all runtime DLLs to the test output directory
add_custom_command(TARGET pikselite_tests POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "$<TARGET_RUNTIME_DLLS:pikselite_tests>"
        "$<TARGET_FILE_DIR:pikselite_tests>"
    COMMAND_EXPAND_LISTS
    COMMENT "Copying runtime DLLs to test output directory"
)

include(GoogleTest)

gtest_discover_tests(pikselite_tests)
