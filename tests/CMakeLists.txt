find_package(GTest CONFIG REQUIRED)

add_executable(pikselite_tests
    Vector_test.cpp
)

target_link_libraries(pikselite_tests
    PRIVATE
    GTest::gtest
    GTest::gtest_main
)

include(GoogleTest)

gtest_discover_tests(pikselite_tests)
