#include "MeshObject.hpp"

using namespace KompotEngine;

MeshObject::MeshObject(uint64_t id, const std::string &className)
    : m_objectId(id),
      m_vkDevice(nullptr),
      m_vkDescriptorPool(nullptr),
      m_className(className)
{
    m_modelMatrix = glm::mat4({1.0f, 0.0f, 0.0f, 0.0f},
                              {0.0f, 1.0f, 0.0f, 0.0f},
                              {0.0f, 0.0f, 1.0f, 0.0f},
                              {0.0f, 0.0f, 0.0f, 1.0f} );
    m_scale = glm::vec3(1.0f, 1.0f, 1.0f);
}

MeshObject::MeshObject(uint64_t id,
                       const VkDevice &vkDevice,
                       const VkDescriptorPool &vkDescriptorPool,
                       const std::string &className,
                       const std::vector<VkDescriptorSet> &vkDescriptorSets,
                       const std::shared_ptr<Renderer::Mesh> &mesh,
                       const std::shared_ptr<Renderer::Image> &texture,
                       const std::vector<std::shared_ptr<Renderer::Buffer>> &vkUniformMatricesBuffers,
                       const std::string &scriptFileName)
    : MeshObject(id, className)
{
    m_vkDevice = vkDevice;
    m_vkDescriptorPool = vkDescriptorPool;
    m_vkDescriptorSets = vkDescriptorSets;
    m_mesh = mesh;
    m_texture = texture;
    m_vkUniformMatricesBuffers = vkUniformMatricesBuffers;
    m_currentDescriptorIndex = 0;
    if (!scriptFileName.empty())
    {
        if (const auto rfindResult = scriptFileName.rfind(".py");
            scriptFileName.rfind(".py") == scriptFileName.size() - 3)
        {
            m_scriptFileName = scriptFileName.substr(0, rfindResult);
        }
        else
        {
            m_scriptFileName = scriptFileName;
        }
    }
}

MeshObject::~MeshObject()
{
    if (m_vkDevice && m_vkDescriptorPool)
    {
        vkFreeDescriptorSets(m_vkDevice, m_vkDescriptorPool, static_cast<uint32_t>(m_vkDescriptorSets.size()), m_vkDescriptorSets.data());
    }
}

uint64_t MeshObject::getObjectId() const
{
    return m_objectId;
}

std::string MeshObject::getClass() const
{
    return m_className;
}

std::string MeshObject::getScriptModuleName() const
{
    return m_scriptFileName;
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
    m_position = position;
    updateOrientationData();
}

void MeshObject::rotate(const glm::vec3 &axes)
{
    m_rotation = axes;
    updateOrientationData();
}

void MeshObject::scale(const glm::vec3 &scale)
{
    m_scale = scale;
    updateOrientationData();
}

void MeshObject::setModelMatrix(const glm::mat4 &modelMatrix)
{
    m_modelMatrix = modelMatrix;

    glm::quat rot;
    glm::vec3 skew;
    glm::vec4 persp;

    glm::decompose(m_modelMatrix, m_scale, rot, m_position, skew, persp);
}

glm::vec3 MeshObject::getPosition() const
{
    return m_position;
}

glm::vec3 MeshObject::getRotation() const
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
    m_modelMatrix = glm::mat4(1.0f);
    m_modelMatrix = glm::translate(m_modelMatrix, m_position);

//    m_modelMatrix = glm::rotate(m_modelMatrix, glm::radians(m_rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
//    m_modelMatrix = glm::rotate(m_modelMatrix, glm::radians(m_rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
//    m_modelMatrix = glm::rotate(m_modelMatrix, glm::radians(m_rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));

    m_modelMatrix = glm::scale(m_modelMatrix, m_scale);
}
