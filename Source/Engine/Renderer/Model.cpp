#include "Model.hpp"

using namespace KompotEngine::Renderer;

Model::Model(const std::vector<glm::vec3> &vertices,
             const std::vector<uint32_t>  &verticesIndicies,
             const std::vector<glm::vec3> &verticesNormals,
             const std::vector<glm::vec2> &uv)
    : m_verticesIndices(verticesIndicies)
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
}

VkDeviceSize Model::getVerticiesSizeForBuffer() const
{
    return sizeof(m_vertices[0]) * m_vertices.size();
}

VkDeviceSize Model::getVerticiesIndexesSizeForBuffer() const
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
