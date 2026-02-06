#pragma once

#include "ecs/ComponentStore.h"
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>
#include <glm/gtc/quaternion.hpp>
#include <vector>
#include <cstdint>
#include <string>

struct Transform
{
    glm::vec3 position{};
    glm::quat rotation{};
    glm::vec3 scale{ 1.0f, 1.0f, 1.0f };

    glm::mat4 toMatrix() const;
};

struct GameObject
{
    std::string name;
    Transform transform;
    uint32_t  meshIndex{     0 };
    uint32_t  materialIndex{ 0 };
    ComponentStore components;
};

class Scene
{
public:
    std::vector<GameObject> objects;
    void addObject(GameObject obj);  // Take by value for move semantics
    void deleteObject(int index);
    void clear();
};
