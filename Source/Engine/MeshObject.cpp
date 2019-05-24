#include "MeshObject.hpp"

using namespace KompotEngine;

MeshObject::MeshObject(const VkDevice &vkDevice,
                       const VkDescriptorPool &vkDescriptorPool,
                       const std::string &className,
                       const std::vector<VkDescriptorSet> &vkDescriptorSets,
                       const std::shared_ptr<Renderer::Mesh> &mesh,
                       const std::shared_ptr<Renderer::Image> &texture,
                       const std::vector<std::shared_ptr<Renderer::Buffer>> &vkUniformMatricesBuffers)
    : m_vkDevice(vkDevice),
      m_vkDescriptorPool(vkDescriptorPool),
      m_className(className),
      m_vkDescriptorSets(vkDescriptorSets),
      m_mesh(mesh),
      m_texture(texture),
      m_vkUniformMatricesBuffers(vkUniformMatricesBuffers),
      m_currentDescriptorIndex(0)

{
    m_position = glm::vec3(0.0f);
    m_scale    = glm::vec3(1.0f);
    m_rotation = glm::vec3(0.0f);
    m_modelMatrix = glm::mat4(1.0f);
}

MeshObject::~MeshObject()
{
    vkFreeDescriptorSets(m_vkDevice, m_vkDescriptorPool, m_vkDescriptorSets.size(), m_vkDescriptorSets.data());
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

void MeshObject::setPosition(const glm::vec3 &position)
{
    m_modelMatrix = glm::translate(m_modelMatrix, position);
}

void MeshObject::setRotation(float angle, const glm::vec3 &axes)
{
    m_modelMatrix = glm::rotate(m_modelMatrix, angle, axes);
}

void MeshObject::setScale(const glm::vec3 &scale)
{
    m_modelMatrix = glm::scale(m_modelMatrix, scale);
}

glm::mat4 MeshObject::getModelMatrix() const
{
    return m_modelMatrix;
}

std::shared_ptr<Renderer::Buffer> MeshObject::getUboBuffer()
{
    return m_vkUniformMatricesBuffers[m_currentDescriptorIndex];
}
