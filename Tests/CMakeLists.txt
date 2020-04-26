cmake_minimum_required(VERSION 3.12)

enable_testing()
find_package(GTest REQUIRED) # requires defined GTEST_ROOT in system variables

set(TESTS_SOURCES
    main_tests.cpp
    Vector_tests.cpp
)

add_executable(Tests ${TESTS_SOURCES})
target_include_directories(Tests PUBLIC ${GTEST_INCLUDE_DIRS} "../Source")
target_link_libraries(Tests Math ${GTEST_BOTH_LIBRARIES})

gtest_discover_tests(Tests)
