#include "Scene.h"

// Initialize static counter starting from a high number to avoid conflicts with low IDs
uint64_t Scene::s_NextObjectID = 10000;

uint64_t Scene::GenerateObjectID()
{
    return s_NextObjectID++;
}

void Scene::addObject(GameObject obj)
{
    // Assign unique ID if not already set
    if (obj.id == 0)
    {
        obj.id = GenerateObjectID();
    }
    objects.push_back(std::move(obj));
}

void Scene::deleteObject(int index)
{
    if (index >= 0 && index < static_cast<int>(objects.size()))
    {
        objects.erase(objects.begin() + index);
    }
}

void Scene::clear()
{
    objects.clear();
}
