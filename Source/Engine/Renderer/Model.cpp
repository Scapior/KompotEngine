#include "Model.hpp"

using namespace KompotEngine::Renderer;

Model::Model(const std::vector<glm::vec3> &vertices,
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
    m_modelMatrix = glm::mat4(1.0f);
}

VkDeviceSize Model::getVerticiesSizeForBuffer() const
{
    return sizeof(m_vertices[0]) * m_vertices.size();
}

VkDeviceSize Model::getVerticiesIndecesSizeForBuffer() const
{
    return sizeof(m_verticesIndices[0]) * m_verticesIndices.size();
}

Vertex *Model::getVerticesData()
{
    return m_vertices.data();
}

uint32_t *Model::getVerticiesIndicesData()
{
    return m_verticesIndices.data();
}

uint32_t Model::getIndicesCount() const
{
    return  static_cast<uint32_t>(m_verticesIndices.size());
}

const VkBuffer &Model::getVertexBuffer() const
{
    return m_verticesBuffer->getBuffer();
}

const VkBuffer &Model::getIndecesBuffer() const
{
    return m_indecesBuffer->getBuffer();
}

void Model::setBuffer(const std::shared_ptr<Buffer> &verticesBuffer, const std::shared_ptr<Buffer> &indecesBuffer)
{
    m_verticesBuffer = verticesBuffer;
    m_indecesBuffer  = indecesBuffer;
}

void Model::setPosition(const glm::vec3 &position)
{
    m_modelMatrix = glm::translate(m_modelMatrix, position);
}

void Model::setRotation(float angle, const glm::vec3 &axes)
{
    m_modelMatrix = glm::rotate(m_modelMatrix, angle, axes);
}

void Model::setScale(const glm::vec3 &scale)
{
    m_modelMatrix = glm::scale(m_modelMatrix, scale);
}

glm::mat4 Model::getModelMatrix() const
{
    return m_modelMatrix;
}
