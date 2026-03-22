#include "VolumeRenderer.h"
#include "imgui.h"
#include <glm/gtc/type_ptr.hpp>

std::vector<LineVertex> VolumeRenderer::buildVertexData() const
{
    std::vector<LineVertex> vertices;

    if (!data)
        return vertices;

    // Emit 2 identical vertices at the center — a degenerate line segment.
    // The geometry shader recognises objectType == 1 and emits a billboard quad instead.
    LineVertex v;
    v.position = data->center;
    v.color = data->color;
    v.thickness = data->radius;
    v.distanceAlongLine = 0.0f;

    vertices.push_back(v);
    vertices.push_back(v);

    return vertices;
}

void VolumeRenderer::OnInspectorGUI()
{
    if (!data)
    {
        ImGui::Text("No volume data attached");
        return;
    }

    ImGui::Indent();

    // Volume Type (read-only for now)
    ImGui::Text("Type");
    ImGui::SameLine();
    ImGui::SetCursorPosX(120);
    ImGui::Text("Circle");

    // Color
    ImGui::Text("Color");
    ImGui::SameLine();
    ImGui::SetCursorPosX(120);
    if (ImGui::ColorEdit4("##VolumeColor", glm::value_ptr(data->color)))
    {
        data->needsGPUUpdate = true;
    }

    // Radius
    ImGui::Text("Radius");
    ImGui::SameLine();
    ImGui::SetCursorPosX(120);
    ImGui::SetNextItemWidth(150);
    if (ImGui::SliderFloat("##Radius", &data->radius, 0.01f, 10.0f, "%.2f"))
    {
        data->needsGPUUpdate = true;
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // Lit / Unlit
    ImGui::Text("Lit");
    ImGui::SameLine();
    ImGui::SetCursorPosX(120);
    ImGui::Checkbox("##VolumeLit", &data->lit);

    // Anti-aliasing
    ImGui::Text("Anti-alias");
    ImGui::SameLine();
    ImGui::SetCursorPosX(120);
    ImGui::Checkbox("##VolumeAntiAlias", &data->antiAlias);

    if (data->antiAlias)
    {
        ImGui::Text("Smoothness");
        ImGui::SameLine();
        ImGui::SetCursorPosX(120);
        ImGui::SetNextItemWidth(150);
        ImGui::SliderFloat("##VolumeSmoothness", &data->smoothness, 0.0f, 2.0f, "%.2f");
    }

    ImGui::Unindent();
}
