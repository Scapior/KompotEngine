#include "World.hpp"

using namespace KompotEngine;

uint64_t World::m_LastFreeId = 0;

World::World()
{
    m_objectsFlag.clear();
    m_objectsToLoadFlag.clear();
    m_objectsToDeleteFlag.clear();
}

void World::clear()
{
    lock();
    m_meshObjects.clear();
    m_meshesCache.clear();
    m_imagesCache.clear();
    unlock();
}
std::shared_ptr<MeshObject> World::createObject(const std::string &className)
{
    lockObjectToLoad();
    auto objectId = m_LastFreeId++;
    auto object = std::make_shared<MeshObject>(objectId, className);
    m_objectClassesToLoad[objectId] = object;
    unlockObjectToLoad();
    return object;
}

void World::loadObjects(Renderer::ResourcesMaker &resourceMaker)
{
    lock();
    lockObjectToDelete();
    for (const auto& objectId : m_objectsToDelete)
    {
        m_meshObjects.erase(objectId);
    }
    unlock();
    m_objectsToDelete.clear();
    unlockObjectToDelete();

    lockObjectToLoad();
    std::vector<std::shared_ptr<MeshObject>> newLoadedObjects;

    for (auto&& [objectId, objectToLoad] : m_objectClassesToLoad)
    {
        const auto &className = objectToLoad->getClass();
        std::shared_ptr<Renderer::Mesh>  meshPointer{};
        std::shared_ptr<Renderer::Image> texturePointer{};

        const auto meshSearchResult = m_meshesCache.find(className);
        if (meshSearchResult != m_meshesCache.end())
        {
            meshPointer = meshSearchResult->second;
        }

        const auto textureSearchResult = m_imagesCache.find(className);
        if (textureSearchResult != m_imagesCache.end())
        {
            texturePointer = textureSearchResult->second;
        }

        auto newObject = resourceMaker.createMeshObject(objectToLoad->getObjectId(), className, meshPointer, texturePointer);
        newObject->setModelMatrix(objectToLoad->getModelMatrix());

        newLoadedObjects.push_back(newObject);
        if (!meshPointer)
        {
            m_meshesCache.insert(std::pair<std::string, std::shared_ptr<Renderer::Mesh>>(
                                     className, newObject->getMesh()));
        }

        if (!texturePointer)
        {
            m_imagesCache.insert(std::pair<std::string, std::shared_ptr<Renderer::Image>>(
                                     className, newObject->getTextureImage()));
        }
    }
    m_objectClassesToLoad.clear();

    unlockObjectToLoad();

    lock();

    for (auto &newObject : newLoadedObjects )
    {
        m_meshObjects[newObject->getObjectId()] = newObject;
    }

    newLoadedObjects.clear();

    unlock();

}

void World::deleteObject(uint64_t objectId)
{
    lockObjectToDelete();
    m_objectsToDelete.push_back(objectId);
    unlockObjectToDelete();
}

const std::map<uint64_t, std::shared_ptr<MeshObject>>& World::getMeshObjects()
{
    return m_meshObjects;
}

std::shared_ptr<MeshObject> World::getObjectById(uint64_t objectId)
{
    if (m_meshObjects.find(objectId) != m_meshObjects.end())
    {
        return m_meshObjects[objectId];
    }

    if (m_objectClassesToLoad.find(objectId) != m_objectClassesToLoad.end())
    {
        return m_objectClassesToLoad[objectId];
    }

    return nullptr;
}

void World::lock() const
{
    while (m_objectsFlag.test_and_set()){}
}

void World::unlock() const
{
    m_objectsFlag.clear();
}

void World::lockObjectToLoad() const
{
    while (m_objectsToLoadFlag.test_and_set()){}
}

void World::unlockObjectToLoad() const
{
    m_objectsToLoadFlag.clear();
}

void World::lockObjectToDelete() const
{
    while (m_objectsToDeleteFlag.test_and_set()){}
}

void World::unlockObjectToDelete() const
{
    m_objectsToDeleteFlag.clear();
}
