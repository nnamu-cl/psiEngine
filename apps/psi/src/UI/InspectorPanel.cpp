#include "InspectorPanel.h"
#include "layers/PsiWorldLayer.h"
#include "Components/Transform.h"
#include "Components/MeshRenderer.h"
#include "imgui.h"

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

    // Render all components generically
    // Transform component
    Transform* transform = selectedObject.components.get<Transform>();
    if (transform && ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
    {
        transform->OnInspectorGUI();
    }

    // MeshRenderer component
    MeshRenderer* renderer = selectedObject.components.get<MeshRenderer>();
    if (renderer && ImGui::CollapsingHeader("MeshRenderer", ImGuiTreeNodeFlags_DefaultOpen))
    {
        // Mesh info (read-only)
        ImGui::Indent();
        ImGui::Text("Mesh Index: %u", selectedObject.meshIndex);
        ImGui::Unindent();
        ImGui::Spacing();

        // Let the component render its own UI
        renderer->OnInspectorGUI();
    }

    ImGui::End();
}
