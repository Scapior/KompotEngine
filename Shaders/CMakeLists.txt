cmake_minimum_required(VERSION 3.12)

set(GLSL_SOURCE_FILES
    triangle.vert
    triangle.frag
)

if (WIN32)
    set(GLSL_VALIDATOR "$ENV{VULKAN_SDK}/Bin/glslangValidator.exe")
else()
    set(GLSL_VALIDATOR glslangValidator)
endif()

foreach(GLSL_FILE ${GLSL_SOURCE_FILES})
    get_filename_component(FILE_NAME ${GLSL_FILE} NAME)
    set(SPIRV_FILE "${FILE_NAME}.spv")
    add_custom_command(
        OUTPUT ${SPIRV_FILE}
        COMMAND ${GLSL_VALIDATOR} -V ${CMAKE_SOURCE_DIR}/Shaders/${GLSL_FILE} -o ${CMAKE_SOURCE_DIR}/Shaders/${SPIRV_FILE}
        DEPENDS ${GLSL_FILE}
    )
    list(APPEND SPIRV_BINARY_FILES ${SPIRV_FILE})
endforeach(GLSL_FILE)
add_custom_target(
    Shaders
    DEPENDS ${SPIRV_BINARY_FILES}
)


