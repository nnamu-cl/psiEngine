#pragma once

#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>
#include <glm/gtc/quaternion.hpp>
#include <vector>
#include <cstdint>

struct Transform
{
    glm::vec3 position{};
    glm::quat rotation{};
    glm::vec3 scale{ 1.0f, 1.0f, 1.0f };

    glm::mat4 toMatrix() const;
};

struct GameObject
{
    Transform transform;
    uint32_t  meshIndex{     0 };
    uint32_t  materialIndex{ 0 };
};

class Scene
{
public:
    std::vector<GameObject> objects;

    void addObject(const GameObject& obj);
    void clear();
};
