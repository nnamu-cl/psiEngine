#pragma once
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <limits>

enum class VolumeType : uint32_t
{
    Circle = 1,
    Orbital = 2
};

// Position comes from the Transform component on the same GameObject.
struct VolumeRendererData
{
    VolumeType type = VolumeType::Circle;
    float radius = 1.0f;
    glm::vec4 color = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
    bool antiAlias = true;
    float smoothness = 1.0f;
    bool lit = false;           // unlit by default

    mutable bool needsGPUUpdate = false;
    mutable glm::vec3 lastUploadedPos = glm::vec3(std::numeric_limits<float>::max());

    VolumeRendererData() = default;
};
