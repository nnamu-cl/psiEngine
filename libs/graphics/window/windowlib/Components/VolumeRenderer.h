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

    // Build 2 degenerate vertices at the center position (consumed as a LINE_STRIP segment)
    std::vector<LineVertex> buildVertexData() const;

    void OnInspectorGUI() override;
};
