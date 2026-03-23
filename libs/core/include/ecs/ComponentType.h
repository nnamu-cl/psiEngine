#pragma once
#include <cstdint>

// Component type registry - add new component types here
enum class ComponentType : uint32_t
{
    Transform = 0,
    MeshRenderer = 1,
    LineRenderer = 2,
    VolumeRenderer = 3,
    Atom = 4,
    AtomVisualizer = 5,
    // Add future components here:
    // RigidBody,
    // Collider,
    // etc.
};
