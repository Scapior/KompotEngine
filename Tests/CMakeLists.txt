cmake_minimum_required(VERSION 3.14)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads)
find_package(GTest) # requires defined GTEST_ROOT in system variables
# if you use win and "build shared libs" flag then add
# GTEST_ROOT/bin to PATH system variable (i.g. C:\Program Files (x86)\googletest\bin)

if (GTest_FOUND AND Threads_FOUND)
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
    target_link_libraries(Tests PRIVATE ${LINK_LIST} GTest::Main)

    gtest_discover_tests(Tests)
endif()
