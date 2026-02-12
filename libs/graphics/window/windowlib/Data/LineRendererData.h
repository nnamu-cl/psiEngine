#pragma once
#include "LineProperties.h"
#include <glm/vec3.hpp>
#include <vector>

// Line renderer data - holds all line geometry and appearance data
// This struct is owned externally and shared between LineRenderer component and LineRendererNode
struct LineRendererData
{
    // Line points in 3D space
    std::vector<glm::vec3> points;

    // Line appearance properties
    LineProperties properties;

    // Dirty flag - set when data changes and GPU buffer needs refresh
    mutable bool needsGPUUpdate = false;

    LineRendererData() = default;
};
