#include "Mesh.hpp"

using namespace KompotEngine::Renderer;

Mesh::Mesh(const std::vector<glm::vec3> &vertices,
             const std::vector<uint32_t>  &verticesIndicies,
             const std::vector<glm::vec3> &verticesNormals,
             const std::vector<glm::vec2> &uv)
    : m_verticesIndices(verticesIndicies), m_verticesBuffer(nullptr), m_indecesBuffer(nullptr)
{
    auto i = 0_u64t;
    while (i < vertices.size() && i < verticesNormals.size() &&  i < uv.size())
    {
        Vertex vertex{};
        vertex.position = vertices[i];
        vertex.normal = verticesNormals[i];
        vertex.textureCoordinates.s = uv[i].s;
        vertex.textureCoordinates.t = 1.0f - uv[i].t;
        i++;
        m_vertices.push_back(vertex);
    }
    m_ModelMatrix = glm::mat4(1.0f);
}

VkDeviceSize Mesh::getVerticiesSizeForBuffer() const
{
    return sizeof(m_vertices[0]) * m_vertices.size();
}

VkDeviceSize Mesh::getVerticiesIndecesSizeForBuffer() const
{
    return sizeof(m_verticesIndices[0]) * m_verticesIndices.size();
}

Vertex *Mesh::getVerticesData()
{
    return m_vertices.data();
}

uint32_t *Mesh::getVerticiesIndicesData()
{
    return m_verticesIndices.data();
}

uint32_t Mesh::getIndicesCount() const
{
    return  static_cast<uint32_t>(m_verticesIndices.size());
}

const VkBuffer &Mesh::getVertexBuffer() const
{
    return m_verticesBuffer->getBuffer();
}

const VkBuffer &Mesh::getIndecesBuffer() const
{
    return m_indecesBuffer->getBuffer();
}

void Mesh::setBuffer(const std::shared_ptr<Buffer> &verticesBuffer, const std::shared_ptr<Buffer> &indecesBuffer)
{
    m_verticesBuffer = verticesBuffer;
    m_indecesBuffer  = indecesBuffer;
}

void Mesh::setPosition(const glm::vec3 &position)
{
    m_ModelMatrix = glm::translate(m_ModelMatrix, position);
}

void Mesh::setRotation(float angle, const glm::vec3 &axes)
{
    m_ModelMatrix = glm::rotate(m_ModelMatrix, angle, axes);
}

void Mesh::setScale(const glm::vec3 &scale)
{
    m_ModelMatrix = glm::scale(m_ModelMatrix, scale);
}

glm::mat4 Mesh::getModelMatrix() const
{
    return m_ModelMatrix;
}
