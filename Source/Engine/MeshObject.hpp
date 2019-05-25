#pragma once

#include "Renderer/Mesh.hpp"
#include "Renderer/Image.hpp"
#include "Renderer/Buffer.hpp"
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <string>
#include <memory>

namespace KompotEngine
{

class MeshObject
{
public:
    MeshObject();
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

    void moveTo(const glm::vec3&);
    void rotate(float, const glm::vec3&);
    void scale(const glm::vec3&);

    void setModelMatrix(const glm::mat4&);

    glm::vec3 getPosition() const;
    glm::quat getRotation() const;
    glm::vec3 getScale() const;

    glm::mat4 getModelMatrix() const;
    std::shared_ptr<Renderer::Buffer> getUboBuffer();
private:
    VkDevice m_vkDevice;
    VkDescriptorPool m_vkDescriptorPool;
    std::string m_className;
    std::vector<VkDescriptorSet> m_vkDescriptorSets;

    std::shared_ptr<Renderer::Mesh>  m_mesh;
    std::shared_ptr<Renderer::Image> m_texture;

    std::vector<std::shared_ptr<Renderer::Buffer>> m_vkUniformMatricesBuffers;
    mutable uint64_t m_currentDescriptorIndex;

    glm::vec3 m_position;
    glm::quat m_rotation;
    glm::vec3 m_scale;

    glm::mat4 m_modelMatrix;

    void updateOrientationData();
};

} // namespace KompotEngine
