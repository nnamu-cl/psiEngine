#pragma once

#include "Layers/DefaultGameWorld/DefaultGameWorld.h"
#include "PsiColors.h"
#include <glm/vec3.hpp>

/**
 * PsiWorldLayer - Custom world/rendering layer for PSI application
 *
 * Inherits from DefaultGameWorld to leverage the existing 3D rendering
 * infrastructure. This layer can be extended to add PSI-specific
 * rendering features, physics visualization, or custom scene management.
 */
class PsiWorldLayer : public DefaultGameWorld
{
public:
    // Constructor - passes windowData to parent DefaultGameWorld
    explicit PsiWorldLayer(ApplicationWindowData* windowData);

    ~PsiWorldLayer() override = default;

    // Background color for the world/scene (secondary background - furthest back layer)
    glm::vec3 backgroundColor;

    // Currently selected object index (-1 means no selection)
    int selectedObjectIndex = -1;

    // No overrides for now - using DefaultGameWorld implementation
    // Future overrides could include:
    // - void OnAttach() override;
    // - void OnDetach() override;
    // - void OnUpdate(float ts) override;
    // - void OnRender(VkCommandBuffer cb, const glm::ivec2& windowSize, uint32_t frameIndex) override;
};
