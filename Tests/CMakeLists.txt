cmake_minimum_required(VERSION 3.12)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
find_package(GTest REQUIRED) # requires defined GTEST_ROOT in system variables


set(TESTS_SOURCES
    main_tests.cpp
    Vector_tests.cpp
    Misc/StringUtils/StringUtils_tests.cpp
)

enable_testing()

set(LINK_LIST Math)
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    list(APPEND LINK_LIST Threads::Threads)
endif()

add_executable(Tests ${TESTS_SOURCES})
target_include_directories(Tests PUBLIC ${GTEST_INCLUDE_DIRS} "../Source")
target_link_libraries(Tests PRIVATE ${LINK_LIST} ${GTEST_BOTH_LIBRARIES})

message("Libs " + ${GTEST_BOTH_LIBRARIES})
gtest_discover_tests(Tests)
