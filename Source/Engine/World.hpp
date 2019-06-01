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
    std::shared_ptr<MeshObject> createObject(const std::string&);
    void loadObjects(Renderer::ResourcesMaker&);

    const std::vector<std::shared_ptr<MeshObject>>& getMeshObjects();

    void lock() const;
    void unlock() const;

private:

    void lockObjectToLoad() const;
    void unlockObjectToLoad() const;

    mutable std::atomic_flag m_objectsFlag;
    mutable std::atomic_flag m_objectsToLoadFlag;

    std::vector<std::shared_ptr<MeshObject>> m_meshObjects;

    mutable std::map<std::string, std::shared_ptr<Renderer::Mesh>>  m_meshesCache;
    mutable std::map<std::string, std::shared_ptr<Renderer::Image>> m_imagesCache;
    mutable std::vector<std::shared_ptr<MeshObject>> m_objectClassesToLoad;

    static uint64_t m_LastFreeId;
};


} // namespace KompotEngine
