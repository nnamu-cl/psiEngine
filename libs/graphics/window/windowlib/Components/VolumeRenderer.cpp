#include "VolumeRenderer.h"
#include "imgui.h"
#include <glm/gtc/type_ptr.hpp>

std::vector<LineVertex> VolumeRenderer::buildVertexData(const glm::vec3& worldPos) const
{
    std::vector<LineVertex> vertices;

    if (!data)
        return vertices;

    // Emit 2 identical vertices at the Transform's world position — a degenerate line segment.
    // The geometry shader recognises objectType == 1 and emits a billboard quad instead.
    LineVertex v;
    v.position = worldPos;
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
    DrawVolumeUI(data);
    ImGui::Unindent();
}

void VolumeRenderer::DrawVolumeUI(VolumeRendererData* data, bool compact)
{
    if (!data)
    {
        ImGui::Text("No volume data attached");
        return;
    }

    if (compact)
    {
        // Compact layout for node UI — labels above controls, no SameLine offsets
        ImGui::Text("Type: Circle");

        ImGui::Text("Color");
        ImGui::SetNextItemWidth(150);
        if (ImGui::ColorEdit4("##VolumeColor", glm::value_ptr(data->color),
                              ImGuiColorEditFlags_NoInputs))
            data->needsGPUUpdate = true;

        ImGui::Text("Radius");
        ImGui::SetNextItemWidth(150);
        if (ImGui::SliderFloat("##Radius", &data->radius, 0.01f, 10.0f, "%.2f"))
            data->needsGPUUpdate = true;

        ImGui::Text("Lit");
        ImGui::Checkbox("##VolumeLit", &data->lit);

        ImGui::Text("Anti-alias");
        ImGui::Checkbox("##VolumeAntiAlias", &data->antiAlias);

        if (data->antiAlias)
        {
            ImGui::Text("Smoothness");
            ImGui::SetNextItemWidth(150);
            ImGui::SliderFloat("##VolumeSmoothness", &data->smoothness, 0.0f, 2.0f, "%.2f");
        }
    }
    else
    {
        // Inspector layout — label + control on the same row
        ImGui::Text("Type");
        ImGui::SameLine();
        ImGui::SetCursorPosX(120);
        ImGui::Text("Circle");

        ImGui::Text("Color");
        ImGui::SameLine();
        ImGui::SetCursorPosX(120);
        if (ImGui::ColorEdit4("##VolumeColor", glm::value_ptr(data->color)))
            data->needsGPUUpdate = true;

        ImGui::Text("Radius");
        ImGui::SameLine();
        ImGui::SetCursorPosX(120);
        ImGui::SetNextItemWidth(150);
        if (ImGui::SliderFloat("##Radius", &data->radius, 0.01f, 10.0f, "%.2f"))
            data->needsGPUUpdate = true;

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        ImGui::Text("Lit");
        ImGui::SameLine();
        ImGui::SetCursorPosX(120);
        ImGui::Checkbox("##VolumeLit", &data->lit);

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
    }
}
