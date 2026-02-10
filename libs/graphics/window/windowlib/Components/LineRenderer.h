#pragma once
#include "ecs/IComponent.h"
#include "../Data/LineProperties.h"
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <vector>

class LineRenderer : public IComponent
{
public:
    COMPONENT_TYPE_ID(LineRenderer)

    // Line points
    std::vector<glm::vec3> points;

    // Line properties
    LineProperties properties;

    // Dirty flag - set when properties change and GPU data needs refresh
    mutable bool needsGPUUpdate = false;

    LineRenderer() = default;

    // Constructor with initial points
    LineRenderer(const std::vector<glm::vec3>& initialPoints,
                 const glm::vec4& color = glm::vec4(1.0f))
        : points(initialPoints)
    {
        properties.color = color;
    }

    std::unique_ptr<IComponent> clone() const override
    {
        return std::make_unique<LineRenderer>(*this);
    }

    // Build vertex buffer data from points
    std::vector<LineVertex> buildVertexData() const;

    // Render inspector UI for this component
    void OnInspectorGUI() override;
};
