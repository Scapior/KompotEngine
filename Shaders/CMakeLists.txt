cmake_minimum_required(VERSION 3.14)

set(GLSL_SOURCE_FILES
        triangle.vert
        triangle.frag
        )

find_program(GLSL_VALIDATOR glslangValidator HINTS /usr/bin /usr/local/bin $ENV{VULKAN_SDK}/Bin/ $ENV{VULKAN_SDK}/Bin32/)
file(GLOB_RECURSE GLSL_SOURCE_FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/*.vert"
        "${CMAKE_CURRENT_SOURCE_DIR}/*.frag"
        )

message(STATUS "Building shaders via ${GLSL_VALIDATOR}")
foreach(GLSL_FILE ${GLSL_SOURCE_FILES})
    get_filename_component(FILE_NAME ${GLSL_FILE} NAME)
    set(SPIRV_FILE "${CMAKE_CURRENT_SOURCE_DIR}/${FILE_NAME}.spv")
    message(STATUS "${FILE_NAME}...")
    add_custom_command(
            OUTPUT ${SPIRV_FILE}
            COMMAND ${GLSL_VALIDATOR} -C -g -V "${GLSL_FILE}" -o "${SPIRV_FILE}"
            DEPENDS ${GLSL_FILE}
    )
    list(APPEND SPIRV_BINARY_FILES ${SPIRV_FILE})
endforeach(GLSL_FILE)

add_custom_target(
        Shaders
        DEPENDS ${SPIRV_BINARY_FILES}
)
