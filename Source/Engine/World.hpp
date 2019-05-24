#pragma once

#include "MeshObject.hpp"
#include <vector>
#include <memory>
#include <string>
#include <map>
#include <atomic>
#include "Renderer/ResourcesMaker.hpp"

namespace KompotEngine
{

class World
{
public:
    World();

    void clear();
    void createObject(const std::string&);
    void loadObjects(Renderer::ResourcesMaker&);

    const std::vector<std::shared_ptr<MeshObject>>& getMeshObjects();

    void lock() const;
    void unlock() const;

private:
    mutable std::atomic_flag m_atomicFlag;
    bool isNeedToLoadResource;

    std::vector<std::shared_ptr<MeshObject>> m_meshObjects;

    mutable std::map<std::string, std::shared_ptr<Renderer::Mesh>>  m_meshesCache;
    mutable std::map<std::string, std::shared_ptr<Renderer::Image>> m_imagesCache;
    mutable std::vector<std::string> m_objectClassesToLoad;
};


} // namespace KompotEngine
