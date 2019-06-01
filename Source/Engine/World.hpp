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
    void deleteObject(uint64_t);

    const std::map<uint64_t, std::shared_ptr<MeshObject>>& getMeshObjects();
    std::shared_ptr<MeshObject> getObjectById(uint64_t);

    void lock() const;
    void unlock() const;

private:

    void lockObjectToLoad() const;
    void unlockObjectToLoad() const;

    void lockObjectToDelete() const;
    void unlockObjectToDelete() const;

    mutable std::atomic_flag m_objectsFlag;
    mutable std::atomic_flag m_objectsToLoadFlag;
    mutable std::atomic_flag m_objectsToDeleteFlag;

    std::map<uint64_t, std::shared_ptr<MeshObject>> m_meshObjects;

    mutable std::map<std::string, std::shared_ptr<Renderer::Mesh>>  m_meshesCache;
    mutable std::map<std::string, std::shared_ptr<Renderer::Image>> m_imagesCache;
    mutable std::map<uint64_t, std::shared_ptr<MeshObject>> m_objectClassesToLoad;

    static uint64_t m_LastFreeId;

    std::vector<uint64_t> m_objectsToDelete;
};


} // namespace KompotEngine
