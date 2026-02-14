#pragma once
#include "ecs/IComponent.h"
#include "../Data/Material.h"
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

class MeshRenderer : public IComponent
{
public:
    COMPONENT_TYPE_ID(MeshRenderer)

    // Material properties
    MaterialProperties material;

    MeshRenderer() = default;
    MeshRenderer(ColorMode mode, const glm::vec4& color = glm::vec4(1.0f))
    {
        material.colorMode = mode;
        material.objectColor = color;
    }

    std::unique_ptr<IComponent> clone() const override
    {
        return std::make_unique<MeshRenderer>(*this);
    }

    // Render inspector UI for this component
    void OnInspectorGUI() override;
};
