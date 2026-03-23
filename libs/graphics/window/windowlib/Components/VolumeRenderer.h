#pragma once
#include "ecs/IComponent.h"
#include "../Data/VolumeRendererData.h"
#include "../Data/LineProperties.h"
#include <vector>

class VolumeRenderer : public IComponent
{
public:
    COMPONENT_TYPE_ID(VolumeRenderer)

    VolumeRendererData* data = nullptr;

    VolumeRenderer() = default;

    explicit VolumeRenderer(VolumeRendererData* volumeData)
        : data(volumeData)
    {}

    std::unique_ptr<IComponent> clone() const override
    {
        auto cloned = std::make_unique<VolumeRenderer>();
        cloned->data = data;
        return cloned;
    }

    // Build 2 degenerate vertices at the given world position (consumed as a LINE_STRIP segment)
    // Position comes from the Transform component on the same object.
    std::vector<LineVertex> buildVertexData(const glm::vec3& worldPos) const;

    void OnInspectorGUI() override;

    // Static UI drawing function — draws the volume renderer controls for any VolumeRendererData*.
    // Used by both the inspector panel and the VolumeRendererNode.
    // When compact=true, uses a tighter layout suitable for rendering inside a node.
    static void DrawVolumeUI(VolumeRendererData* data, bool compact = false);
};
