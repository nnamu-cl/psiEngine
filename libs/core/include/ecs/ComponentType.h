#pragma once
#include <cstdint>

// Component type registry - add new component types here
enum class ComponentType : uint32_t
{
    MeshRenderer = 0,
    // Add future components here:
    // RigidBody,
    // Collider,
    // etc.
};
