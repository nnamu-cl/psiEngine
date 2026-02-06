#include "MeshRenderer.h"
#include "imgui.h"
#include <glm/gtc/type_ptr.hpp>

void MeshRenderer::OnInspectorGUI()
{
    ImGui::Indent();

    // Color mode selection
    const char* colorModes[] = { "Vertex Color", "Object Color" };
    int currentMode = (colorMode == ColorMode::VertexColor) ? 0 : 1;
    if (ImGui::Combo("Color Mode", &currentMode, colorModes, 2))
    {
        colorMode = (currentMode == 0) ? ColorMode::VertexColor : ColorMode::ObjectColor;
    }

    // Object color picker (only shown in ObjectColor mode)
    if (colorMode == ColorMode::ObjectColor)
    {
        ImGui::Spacing();
        ImGui::ColorEdit4("Object Color", glm::value_ptr(objectColor));
    }

    ImGui::Unindent();
}
