#pragma once

#include "global.hpp"
#include "Buffer.hpp"
#include <vector>
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vulkan/vulkan.hpp>
#include <memory>
#include <array>
#include <vector>

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

class Mesh
{
public:
    Mesh(const std::vector<glm::vec3>&, const std::vector<uint32_t>&,
                 const std::vector<glm::vec3>&, const std::vector<glm::vec2>&);

    VkDeviceSize getVerticiesSizeForBuffer() const;
    VkDeviceSize getVerticiesIndecesSizeForBuffer() const;

    Vertex   *getVerticesData();
    uint32_t *getVerticiesIndicesData();
    uint32_t  getIndicesCount() const;

    const VkBuffer &getVertexBuffer() const;
    const VkBuffer &getIndecesBuffer() const;

    void setBuffer(const std::shared_ptr<Buffer>&, const std::shared_ptr<Buffer>&);

    void setPosition(const glm::vec3&);
    void setRotation(float, const glm::vec3&);
    void setScale(const glm::vec3&);

    glm::mat4 getModelMatrix() const;

private:
    std::vector<Vertex>   m_vertices;
    std::vector<uint32_t> m_verticesIndices;

    std::shared_ptr<Buffer> m_verticesBuffer;
    std::shared_ptr<Buffer> m_indecesBuffer;

    glm::mat4 m_ModelMatrix;
};

} // namespace Renderer

} // namespace KompotEngine
