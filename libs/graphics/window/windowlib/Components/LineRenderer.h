#pragma once
#include "ecs/IComponent.h"
#include "../Data/LineRendererData.h"
#include <glm/vec4.hpp>

class LineRenderer : public IComponent
{
public:
    COMPONENT_TYPE_ID(LineRenderer)

    // Pointer to line data (owned externally, typically by a node or scene)
    LineRendererData* data = nullptr;

    LineRenderer() = default;

    // Constructor with data pointer
    explicit LineRenderer(LineRendererData* lineData)
        : data(lineData)
    {}

    // Constructor with initial points (for backward compatibility)
    LineRenderer(const std::vector<glm::vec3>& initialPoints,
                 const glm::vec4& color = glm::vec4(1.0f))
    {
        // Note: This constructor creates unowned data - caller must manage lifetime
        static LineRendererData tempData;
        tempData.points = initialPoints;
        tempData.properties.color = color;
        data = &tempData;
    }

    std::unique_ptr<IComponent> clone() const override
    {
        auto cloned = std::make_unique<LineRenderer>();
        cloned->data = data;  // Share the same data pointer
        return cloned;
    }

    // Build vertex buffer data from points
    std::vector<LineVertex> buildVertexData() const;

    // Render inspector UI for this component
    void OnInspectorGUI() override;
};
