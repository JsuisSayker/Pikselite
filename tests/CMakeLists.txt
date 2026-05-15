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

if(PIKSELITE_ENABLE_PROFILING)
    message(STATUS "JE SUIS LA GROS COCHON")
    find_package(tracy CONFIG REQUIRED)
    add_custom_command(TARGET pikselite_tests POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "$<TARGET_FILE:Tracy::TracyClient>"
            $<TARGET_FILE_DIR:pikselite_tests>
        COMMENT "Copying TracyClient.dll to test output directory"
    )
endif()

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

include(GoogleTest)

gtest_discover_tests(pikselite_tests)
