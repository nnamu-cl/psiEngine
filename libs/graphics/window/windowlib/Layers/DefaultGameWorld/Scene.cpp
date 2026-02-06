#include "Scene.h"

void Scene::addObject(GameObject obj)
{
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
