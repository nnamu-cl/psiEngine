#include "InspectorPanel.h"
#include "layers/PsiWorldLayer.h"
#include "imgui.h"
#include <glm/gtc/type_ptr.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/euler_angles.hpp>

InspectorPanel::InspectorPanel(PsiWorldLayer* worldLayer)
    : m_WorldLayer(worldLayer)
{
}

void InspectorPanel::Render()
{
    if (!m_Visible)
        return;

    // Only show if an object is selected
    if (m_WorldLayer->selectedObjectIndex < 0 ||
        m_WorldLayer->selectedObjectIndex >= static_cast<int>(m_WorldLayer->data.scene.objects.size()))
    {
        return;
    }

    // Position below stats panel at top right
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImVec2 windowPos = ImVec2(viewport->WorkPos.x + viewport->WorkSize.x - 10.0f, viewport->WorkPos.y + 120.0f);
    ImGui::SetNextWindowPos(windowPos, ImGuiCond_Always, ImVec2(1.0f, 0.0f));
    ImGui::SetNextWindowSize(ImVec2(300, 0), ImGuiCond_FirstUseEver);

    ImGui::Begin("Inspector", &m_Visible);

    GameObject& selectedObject = m_WorldLayer->data.scene.objects[m_WorldLayer->selectedObjectIndex];

    // Object name and delete button
    ImGui::Text("%s", selectedObject.name.c_str());
    ImGui::SameLine();
    ImGui::SetCursorPosX(ImGui::GetWindowWidth() - 70.0f);
    if (ImGui::Button("Delete", ImVec2(60, 0)))
    {
        int indexToDelete = m_WorldLayer->selectedObjectIndex;
        m_WorldLayer->selectedObjectIndex = -1; // Deselect before deleting
        m_WorldLayer->data.scene.deleteObject(indexToDelete);
        ImGui::End(); // Close window before returning
        return; // Exit early since we've deleted the object
    }
    ImGui::Separator();

    // Transform section
    if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
    {
        ImGui::Indent();

        // Position
        ImGui::Text("Position");
        ImGui::DragFloat3("##Position", glm::value_ptr(selectedObject.transform.position), 0.1f);

        ImGui::Spacing();

        // Rotation (convert quaternion to Euler angles for editing)
        glm::vec3 eulerAngles = glm::degrees(glm::eulerAngles(selectedObject.transform.rotation));
        ImGui::Text("Rotation");
        if (ImGui::DragFloat3("##Rotation", glm::value_ptr(eulerAngles), 1.0f))
        {
            // Convert back to quaternion
            selectedObject.transform.rotation = glm::quat(glm::radians(eulerAngles));
        }

        ImGui::Spacing();

        // Scale
        ImGui::Text("Scale");
        ImGui::DragFloat3("##Scale", glm::value_ptr(selectedObject.transform.scale), 0.01f, 0.001f, 100.0f);

        ImGui::Unindent();
    }

    // Mesh and Material info (read-only for now)
    if (ImGui::CollapsingHeader("Rendering"))
    {
        ImGui::Indent();
        ImGui::Text("Mesh Index: %u", selectedObject.meshIndex);
        ImGui::Text("Material Index: %u", selectedObject.materialIndex);
        ImGui::Unindent();
    }

    ImGui::End();
}
