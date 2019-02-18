#version 450

layout(binding = 0) uniform MVPMatrices {
    mat4 world;
    mat4 view;
    mat4 projection;
} mvpMatrices;

layout(location = 0) in vec2 position;
layout(location = 1) in vec3 color;
layout(location = 2) in vec2 textureCoordinates;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 outTextureCoordinates;

void main() {
    gl_Position = mvpMatrices.projection * mvpMatrices.view * mvpMatrices.world * vec4(position, 0.0, 1.0);
    fragColor = vec3(color);
    outTextureCoordinates = textureCoordinates;
}
