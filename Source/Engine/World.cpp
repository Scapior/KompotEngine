#include "World.hpp"

using namespace KompotEngine;

World::World()
    : isNeedToLoadResource(false)
{
    m_atomicFlag.clear();
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
    lock();
    auto fakeObject = std::make_shared<MeshObject>();
    m_objectClassesToLoad.emplace_back(className, fakeObject);
    unlock();
    return fakeObject;
}

void World::loadObjects(Renderer::ResourcesMaker &resourceMaker)
{
    lock();

    for (auto& objectToLoad : m_objectClassesToLoad)
    {
        const auto & className = objectToLoad.first;
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

        auto newObject = resourceMaker.createMeshObject(className, meshPointer, texturePointer);

        newObject->setModelMatrix(objectToLoad.second->getModelMatrix());

        objectToLoad.second.swap(newObject);
        auto &object = objectToLoad.second;

        m_meshObjects.push_back(object);
        if (!meshPointer)
        {
            m_meshesCache.insert(std::pair<std::string, std::shared_ptr<Renderer::Mesh>>(
                                     className, object->getMesh()));
        }

        if (!texturePointer)
        {
            m_imagesCache.insert(std::pair<std::string, std::shared_ptr<Renderer::Image>>(
                                     className, object->getTextureImage()));
        }

    }
    m_objectClassesToLoad.clear();
    unlock();
}

const std::vector<std::shared_ptr<MeshObject>>& World::getMeshObjects()
{
    return m_meshObjects;
}

void World::lock() const
{
    while (m_atomicFlag.test_and_set()){}
}

void World::unlock() const
{
    m_atomicFlag.clear();
}
