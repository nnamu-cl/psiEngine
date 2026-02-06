#pragma once

#include "ecs/ComponentStore.h"
#include <vector>
#include <cstdint>
#include <string>

struct GameObject
{
    uint64_t id{ 0 };  // Unique identifier for this GameObject
    std::string name;
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

private:
    static uint64_t s_NextObjectID;  // Static counter for unique IDs
    uint64_t GenerateObjectID();     // Generate next unique ID
};
