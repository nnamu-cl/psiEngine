#pragma once
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

enum class VolumeType : uint32_t
{
    Circle = 1
};

struct VolumeRendererData
{
    VolumeType type = VolumeType::Circle;
    glm::vec3 center = glm::vec3(0.0f);
    float radius = 1.0f;
    glm::vec4 color = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
    bool antiAlias = true;
    float smoothness = 1.0f;
    bool lit = false;           // unlit by default

    mutable bool needsGPUUpdate = false;

    VolumeRendererData() = default;
};
