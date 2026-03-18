find_package(GTest CONFIG REQUIRED)

add_executable(pikselite_tests
    engine_editor_tests.cpp
    imguiComponents/testBarComponent.cpp
    imguiComponents/testGetDesiredPos.cpp
    renderer/testSpriteEditor.cpp
    renderer/testProjectEditor.cpp
    engine/testChunk.cpp
    renderer/testDrawOnEditor.cpp
)

target_compile_definitions(pikselite_tests PRIVATE UNIT_TEST)

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
