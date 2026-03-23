#include "AtomVisualizer.h"
#include "imgui.h"
#include <glm/gtc/type_ptr.hpp>

std::vector<LineVertex> AtomVisualizer::buildVertexData(const glm::vec3& worldPos) const
{
    std::vector<LineVertex> vertices;
    if (!atomData || !visData)
        return vertices;

    // Same degenerate-line-segment trick as VolumeRenderer.
    // thickness carries the bounding radius so the geometry shader
    // can emit a correctly-sized billboard quad.
    LineVertex v;
    v.position = worldPos;
    v.color = visData->positiveColor;  // base color (shader overrides per-fragment)
    v.thickness = atomData->boundingRadius();
    v.distanceAlongLine = 0.0f;

    vertices.push_back(v);
    vertices.push_back(v);

    return vertices;
}

void AtomVisualizer::OnInspectorGUI()
{
    if (!visData)
    {
        ImGui::Text("No visualizer data attached");
        return;
    }

    ImGui::Indent();
    DrawVisualizerUI(visData);
    ImGui::Unindent();
}

void AtomVisualizer::DrawVisualizerUI(AtomVisualizerData* data, bool compact)
{
    if (!data)
    {
        ImGui::Text("No visualizer data attached");
        return;
    }

    bool changed = false;
    float itemWidth = compact ? 150.0f : 150.0f;
    float labelX = compact ? 0.0f : 120.0f;

    auto label = [&](const char* text) {
        ImGui::Text("%s", text);
        if (!compact) { ImGui::SameLine(); ImGui::SetCursorPosX(labelX); }
        ImGui::SetNextItemWidth(itemWidth);
    };

    // --- Particle Cloud Settings ---
    if (!compact) { ImGui::TextDisabled("Particle Cloud"); ImGui::Spacing(); }

    label("Density");
    changed |= ImGui::SliderFloat("##DensityScale", &data->densityScale, 0.05f, 3.0f, "%.2f");

    label("Resolution");
    changed |= ImGui::SliderInt("##StepCount", &data->stepCount, 64, 400);
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("Grid resolution — more = denser, smaller dots");

    if (!compact) { ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing(); }

    // --- Animation ---
    if (!compact) { ImGui::TextDisabled("Animation"); ImGui::Spacing(); }

    label("Anim Speed");
    changed |= ImGui::SliderFloat("##AnimSpeed", &data->animationSpeed, 0.0f, 5.0f, "%.2f");
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("Probability current rotation (visible for m != 0)");

    if (changed)
        data->needsGPUUpdate = true;
}
