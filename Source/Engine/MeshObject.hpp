#pragma once

#include "Renderer/Mesh.hpp"
#include "Renderer/Image.hpp"
#include "Renderer/Buffer.hpp"
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <string>
#include <memory>

namespace KompotEngine
{

class MeshObject
{
public:
    MeshObject(const VkDevice&,
               const VkDescriptorPool&,
               const std::string&,
               const std::vector<VkDescriptorSet>&,
               const std::shared_ptr<Renderer::Mesh>&,
               const std::shared_ptr<Renderer::Image>&,
               const std::vector<std::shared_ptr<Renderer::Buffer>>&);

    ~MeshObject();

    std::string getClass() const;

    void setMesh(const std::shared_ptr<Renderer::Mesh>&);
    void setTexture(const std::shared_ptr<Renderer::Image>&);

    std::shared_ptr<Renderer::Mesh>  getMesh() const;
    std::shared_ptr<Renderer::Image> getTextureImage() const;

    const VkDescriptorSet *getDescriptorSet() const;

    void setPosition(const glm::vec3&);
    void setRotation(float, const glm::vec3&);
    void setScale(const glm::vec3&);

    glm::mat4 getModelMatrix() const;
    std::shared_ptr<Renderer::Buffer> getUboBuffer();
private:
    VkDevice m_vkDevice;
    VkDescriptorPool m_vkDescriptorPool;
    std::string m_className;
    std::vector<VkDescriptorSet> m_vkDescriptorSets;
    mutable uint64_t m_currentDescriptorIndex;

    std::shared_ptr<Renderer::Mesh>  m_mesh;
    std::shared_ptr<Renderer::Image> m_texture;

    std::vector<std::shared_ptr<Renderer::Buffer>> m_vkUniformMatricesBuffers;

    glm::vec3 m_position;
    glm::vec3 m_rotation;
    glm::vec3 m_scale;

    glm::mat4 m_modelMatrix;
};

} // namespace KompotEngine
