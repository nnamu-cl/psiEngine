#pragma once
#include "ecs/IComponent.h"
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

enum class ColorMode
{
    VertexColor,  // Use per-vertex colors from mesh
    ObjectColor   // Use single color for entire object
};

class MeshRenderer : public IComponent
{
public:
    COMPONENT_TYPE_ID(MeshRenderer)

    ColorMode colorMode = ColorMode::VertexColor;
    glm::vec4 objectColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);  // White default

    MeshRenderer() = default;
    MeshRenderer(ColorMode mode, const glm::vec4& color = glm::vec4(1.0f))
        : colorMode(mode), objectColor(color)
    {}

    std::unique_ptr<IComponent> clone() const override
    {
        return std::make_unique<MeshRenderer>(*this);
    }

    // Render inspector UI for this component
    void OnInspectorGUI() override;
};
