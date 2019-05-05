#pragma once

#include "global.hpp"
#include <vector>
#include <glm/glm.hpp>
#include <vulkan/vulkan.hpp>

namespace KompotEngine
{

namespace Renderer
{

struct Vertex
{
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 textureCoordinates;

    static VkVertexInputBindingDescription getBindingDescription()
    {
        VkVertexInputBindingDescription vertexInputBindingDescritpion = {};
        vertexInputBindingDescritpion.binding = 0_u32t;
        vertexInputBindingDescritpion.stride = sizeof(Vertex);
        vertexInputBindingDescritpion.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        return vertexInputBindingDescritpion;
    }

    static std::array<VkVertexInputAttributeDescription, 3_u64t> getAttributeDescritptions()
    {
        std::array<VkVertexInputAttributeDescription, 3_u64t> vertexInputAttributeDescription;
        vertexInputAttributeDescription[0].binding = 0_u32t;
        vertexInputAttributeDescription[0].location = 0_u32t;
        vertexInputAttributeDescription[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        vertexInputAttributeDescription[0].offset = offsetof(Vertex, position);

        vertexInputAttributeDescription[1].binding = 0_u32t;
        vertexInputAttributeDescription[1].location = 1_u32t;
        vertexInputAttributeDescription[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        vertexInputAttributeDescription[1].offset = offsetof(Vertex, normal);

        vertexInputAttributeDescription[2].binding = 0_u32t;
        vertexInputAttributeDescription[2].location = 2_u32t;
        vertexInputAttributeDescription[2].format = VK_FORMAT_R32G32_SFLOAT;
        vertexInputAttributeDescription[2].offset = offsetof(Vertex, textureCoordinates);

        return vertexInputAttributeDescription;
    }
};

//const std::vector<Vertex> vertices = {
//    {{-1.0f, -1.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
//    {{1.0f, -1.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
//    {{-1.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
//    {{1.0f, 1.0f, 0.0f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}}
//};

//const std::vector<uint32_t> verticesIndices = {
//    0, 1, 2, 1, 3, 2
//};

class Model
{
public:
    Model(const std::vector<glm::vec3>&, const std::vector<uint32_t>&,
                 const std::vector<glm::vec3>&, const std::vector<glm::vec2>&);

    VkDeviceSize getVerticiesSizeForBuffer() const;
    VkDeviceSize getVerticiesIndexesSizeForBuffer() const;

    Vertex   *getVerticesData();
    uint32_t *getVerticiesIndicesData();
    uint32_t  getIndicesCount() const;

private:

    std::vector<Vertex>   m_vertices;
    std::vector<uint32_t> m_verticesIndices;
};

} // namespace Renderer

} // namespace KompotEngine
