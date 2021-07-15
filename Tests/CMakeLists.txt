cmake_minimum_required(VERSION 3.14)

project(Tests)

get_directory_property(HAS_PARENT PARENT_DIRECTORY)
find_package(GTest) # requires defined GTEST_ROOT in system variables
# if you use win and "build shared libs" flag then add
# GTEST_ROOT/bin to PATH system variable (i.g. C:\Program Files (x86)\googletest\bin)

if (GTest_FOUND)
	enable_testing()	
	    
    set(TESTS_SOURCES
        main_tests.cpp
        Vector_tests.cpp
		Misc/StringUtils/StringUtils_tests.cpp
    )
	include(CTest)
	include(GoogleTest)

    set(THREADS_PREFER_PTHREAD_FLAG ON)
    find_package(Threads)    

    set(LINK_LIST Math)
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
        list(APPEND LINK_LIST Threads::Threads)
    endif()

    add_executable(Tests ${TESTS_SOURCES})

    target_include_directories(Tests PUBLIC ${GTEST_INCLUDE_DIRS} "../Source")
    	
    if(NOT HAS_PARENT)
		message(STATUS "Standalone build of tests without engine")
		add_subdirectory(../Source/Math Math)
        target_include_directories(Tests PUBLIC ${googletest_SOURCE_DIR}/googletest/include)
    endif()	
	target_link_libraries(Tests PRIVATE ${LINK_LIST} GTest::Main)
    gtest_discover_tests(Tests)
	
endif()
