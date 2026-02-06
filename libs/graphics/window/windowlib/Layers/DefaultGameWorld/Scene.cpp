#include "Scene.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

glm::mat4 Transform::toMatrix() const
{
    // TRS order: scale first, then rotate, then translate.
    // This matches the conventional column-major composition
    // T * R * S applied right-to-left to a vertex.
    glm::mat4 mat = glm::translate(glm::mat4(1.0f), position);
    mat            = mat * glm::mat4_cast(rotation);
    mat            = glm::scale(mat, scale);
    return mat;
}

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
