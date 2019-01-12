cmake_minimum_required(VERSION 3.12)
list(APPEND CMAKE_CXX_FLAGS "-pthread -std=c++17 ${CMAKE_CXX_FLAGS} -fno-exceptions")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/../ )
project(KompotEngine)
find_package(OpenGL REQUIRED)
find_package(PkgConfig REQUIRED)
find_package(GLEW REQUIRED)
find_package(Boost COMPONENTS program_options REQUIRED)
pkg_check_modules(GLFW REQUIRED glfw3)
include_directories(
        ${OPENGL_INCLUDE_DIR}
        ${GLFW_INCLUDE_DIRS}
        ${GLEW_INCLUDE_DIRS}
        ${Boost_INCLUDE_DIRS}
)
link_directories(${Boost_LIBRARY_DIR})
FILE(GLOB_RECURSE SOURCES "Source/*.cpp")
FILE(GLOB_RECURSE HEADERS "Source/*.hpp")
add_executable(
    KompotEngine
    ${SOURCES}
    ${HEADERS}
)
target_include_directories(
    KompotEngine
    PUBLIC "${CMAKE_SOURCE_DIR}/Source"
)
target_link_libraries(KompotEngine
        ${OPENGL_gl_LIBRARY}
        ${GLFW_LIBRARIES}
        ${Boost_LIBRARIES}
)
