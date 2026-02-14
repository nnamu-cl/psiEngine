#include "MeshRenderer.h"
#include "imgui.h"
#include <glm/gtc/type_ptr.hpp>

void MeshRenderer::OnInspectorGUI()
{
    ImGui::Indent();

    // Shading Mode
    ImGui::Text("Shading");
    ImGui::SameLine();
    ImGui::SetCursorPosX(120);
    const char* shadingModes[] = { "Lit", "Unlit" };
    int shadingMode = static_cast<int>(material.shadingMode);
    if (ImGui::Combo("##ShadingMode", &shadingMode, shadingModes, IM_ARRAYSIZE(shadingModes)))
    {
        material.shadingMode = static_cast<ShadingMode>(shadingMode);
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // Color Section
    ImGui::Text("Color Mode");
    ImGui::SameLine();
    ImGui::SetCursorPosX(120);
    const char* colorModes[] = { "Vertex Color", "Object Color" };
    int colorMode = static_cast<int>(material.colorMode);
    if (ImGui::Combo("##ColorMode", &colorMode, colorModes, IM_ARRAYSIZE(colorModes)))
    {
        material.colorMode = static_cast<ColorMode>(colorMode);
    }

    // Show object color picker only when in ObjectColor mode
    if (material.colorMode == ColorMode::ObjectColor)
    {
        ImGui::Text("Object Color");
        ImGui::SameLine();
        ImGui::SetCursorPosX(120);
        ImGui::ColorEdit4("##ObjectColor", glm::value_ptr(material.objectColor));
    }

    // Tint Color (always available)
    ImGui::Text("Tint Color");
    ImGui::SameLine();
    ImGui::SetCursorPosX(120);
    ImGui::ColorEdit3("##TintColor", glm::value_ptr(material.tintColor));

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // Emission (for Unlit mode)
    if (material.shadingMode == ShadingMode::Unlit)
    {
        ImGui::Text("Emission");
        ImGui::SameLine();
        ImGui::SetCursorPosX(120);
        ImGui::SetNextItemWidth(150);
        ImGui::SliderFloat("##EmissionIntensity", &material.emissionIntensity, 0.0f, 10.0f, "%.2f");

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
    }

    // Blend Mode
    ImGui::Text("Blend Mode");
    ImGui::SameLine();
    ImGui::SetCursorPosX(120);
    const char* blendModes[] = { "Opaque", "Transparent", "Additive", "Multiply" };
    int blendMode = static_cast<int>(material.blendMode);
    if (ImGui::Combo("##BlendMode", &blendMode, blendModes, IM_ARRAYSIZE(blendModes)))
    {
        material.blendMode = static_cast<BlendMode>(blendMode);
    }

    // Alpha Cutoff (for transparent blend mode)
    if (material.blendMode == BlendMode::Transparent)
    {
        ImGui::Text("Alpha Cutoff");
        ImGui::SameLine();
        ImGui::SetCursorPosX(120);
        ImGui::SetNextItemWidth(150);
        ImGui::SliderFloat("##AlphaCutoff", &material.alphaCutoff, 0.0f, 1.0f, "%.2f");
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // Rendering Options
    ImGui::Text("Double Sided");
    ImGui::SameLine();
    ImGui::SetCursorPosX(120);
    ImGui::Checkbox("##DoubleSided", &material.doubleSided);

    ImGui::Unindent();
}
