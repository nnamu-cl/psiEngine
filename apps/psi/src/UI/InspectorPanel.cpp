#include "InspectorPanel.h"

#include "IconsLucide.h"
#include "ApplicationWindow.h"
#include "utils/TextIcons.h"
#include "layers/PsiWorldLayer.h"
#include "layers/PsiNodeEditorLayer.h"
#include "Components/Transform.h"
#include "Components/MeshRenderer.h"
#include "Components/VolumeRenderer.h"
#include "imgui.h"

InspectorPanel::InspectorPanel(PsiWorldLayer* worldLayer, PsiNodeEditorLayer* nodeEditorLayer)
    : m_WorldLayer(worldLayer)
    , m_NodeEditorLayer(nodeEditorLayer)
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
    ImGui::SetNextWindowPos(windowPos, ImGuiCond_FirstUseEver, ImVec2(1.0f, 0.0f));
    ImGui::SetNextWindowSize(ImVec2(300, 400), ImGuiCond_FirstUseEver);

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
    if (transform)
    {
        UIUtils::IconText(ICON_LC_EXPAND, IconPosition::Left, "Transform");


        // Pass NodeGraph to Transform if available
        if (m_NodeEditorLayer)
        {
            transform->OnInspectorGUI(&m_NodeEditorLayer->getNodeGraph());
        }
        else
        {
            transform->OnInspectorGUI();
        }
    }

    // MeshRenderer component
    MeshRenderer* renderer = selectedObject.components.get<MeshRenderer>();
    if (renderer)
    {
        ImGui::Spacing();
        ImGui::Spacing();
        ImGui::SeparatorText("MeshRenderer");
        ImGui::Spacing();

        // Mesh info (read-only)
        ImGui::Indent();
        ImGui::Text("Mesh Index: %u", selectedObject.meshIndex);
        ImGui::Unindent();
        ImGui::Spacing();

        // Let the component render its own UI
        renderer->OnInspectorGUI();
    }

    // VolumeRenderer component
    VolumeRenderer* volumeRenderer = selectedObject.components.get<VolumeRenderer>();
    if (volumeRenderer)
    {
        ImGui::Spacing();
        ImGui::Spacing();
        ImGui::SeparatorText("VolumeRenderer");
        ImGui::Spacing();

        volumeRenderer->OnInspectorGUI();
    }

    ImGui::End();
}
