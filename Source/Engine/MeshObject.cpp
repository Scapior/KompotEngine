#include "MeshObject.hpp"

using namespace KompotEngine;

MeshObject::MeshObject()
    : m_vkDevice(nullptr),
      m_vkDescriptorPool(nullptr)
{
    m_modelMatrix = glm::mat4({1.0f, 0.0f, 0.0f, 0.0f},
                              {0.0f, 1.0f, 0.0f, 0.0f},
                              {0.0f, 0.0f, 1.0f, 0.0f},
                              {0.0f, 0.0f, 0.0f, 1.0f} );
}

MeshObject::MeshObject(const VkDevice &vkDevice,
                       const VkDescriptorPool &vkDescriptorPool,
                       const std::string &className,
                       const std::vector<VkDescriptorSet> &vkDescriptorSets,
                       const std::shared_ptr<Renderer::Mesh> &mesh,
                       const std::shared_ptr<Renderer::Image> &texture,
                       const std::vector<std::shared_ptr<Renderer::Buffer>> &vkUniformMatricesBuffers)
    : MeshObject()
{
    m_vkDevice = vkDevice;
    m_vkDescriptorPool = vkDescriptorPool;
    m_className = className;
    m_vkDescriptorSets = vkDescriptorSets;
    m_mesh = mesh;
    m_texture = texture;
    m_vkUniformMatricesBuffers = vkUniformMatricesBuffers;
    m_currentDescriptorIndex = 0;
}

MeshObject::~MeshObject()
{
    if (m_vkDevice && m_vkDescriptorPool)
    {
        vkFreeDescriptorSets(m_vkDevice, m_vkDescriptorPool, static_cast<uint32_t>(m_vkDescriptorSets.size()), m_vkDescriptorSets.data());
    }
}

std::string MeshObject::getClass() const
{
    return m_className;
}

void MeshObject::setMesh(const std::shared_ptr<Renderer::Mesh> &mesh)
{
    m_mesh = mesh;
}

void MeshObject::setTexture(const std::shared_ptr<Renderer::Image> &image)
{
    m_texture = image;
}

 std::shared_ptr<Renderer::Mesh> MeshObject::getMesh() const
{
    return m_mesh;
}


std::shared_ptr<Renderer::Image> MeshObject::getTextureImage() const
{
    return m_texture;
}

const VkDescriptorSet *MeshObject::getDescriptorSet() const
{
    auto descriptor = m_vkDescriptorSets.data() + m_currentDescriptorIndex;
    m_currentDescriptorIndex = ++m_currentDescriptorIndex % m_vkDescriptorSets.size();
    return descriptor;
}

void MeshObject::moveTo(const glm::vec3 &position)
{
    m_modelMatrix = glm::translate(m_modelMatrix, position);
    updateOrientationData();
}

void MeshObject::rotate(float angleInRadians, const glm::vec3 &axes)
{
    m_modelMatrix = glm::rotate(m_modelMatrix, angleInRadians, axes);
    updateOrientationData();
}

void MeshObject::scale(const glm::vec3 &scale)
{
    m_modelMatrix = glm::scale(m_modelMatrix, scale);
    updateOrientationData();
}

void MeshObject::setModelMatrix(const glm::mat4 &modelMatrix)
{
    m_modelMatrix = modelMatrix;
    updateOrientationData();
}

glm::vec3 MeshObject::getPosition() const
{
    return m_position;
}

glm::quat MeshObject::getRotation() const
{
    return m_rotation;
}

glm::vec3 MeshObject::getScale() const
{
    return m_scale;
}

glm::mat4 MeshObject::getModelMatrix() const
{
    return m_modelMatrix;
}

std::shared_ptr<Renderer::Buffer> MeshObject::getUboBuffer()
{
    return m_vkUniformMatricesBuffers[m_currentDescriptorIndex];
}

void MeshObject::updateOrientationData()
{
    glm::vec3 skew;
    glm::vec4 perspective;
    glm::decompose(m_modelMatrix, m_scale, m_rotation, m_position, skew, perspective);
    m_rotation = glm::conjugate(m_rotation);
}
