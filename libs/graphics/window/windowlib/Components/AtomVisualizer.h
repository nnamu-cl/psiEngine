#pragma once
#include "ecs/IComponent.h"
#include "../Data/AtomVisualizerData.h"
#include "../Data/AtomData.h"
#include "../Data/LineProperties.h"
#include <vector>

class AtomVisualizer : public IComponent
{
public:
    COMPONENT_TYPE_ID(AtomVisualizer)

    AtomVisualizerData* visData = nullptr;
    AtomData* atomData = nullptr;   // sibling Atom component's data

    AtomVisualizer() = default;
    AtomVisualizer(AtomVisualizerData* vis, AtomData* atom)
        : visData(vis), atomData(atom) {}

    std::unique_ptr<IComponent> clone() const override
    {
        auto cloned = std::make_unique<AtomVisualizer>();
        cloned->visData = visData;
        cloned->atomData = atomData;
        return cloned;
    }

    // Build 2 degenerate vertices (same pattern as VolumeRenderer)
    std::vector<LineVertex> buildVertexData(const glm::vec3& worldPos) const;

    void OnInspectorGUI() override;

    static void DrawVisualizerUI(AtomVisualizerData* data, bool compact = false);
};
