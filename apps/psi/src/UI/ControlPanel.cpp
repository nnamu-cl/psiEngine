#include "ControlPanel.h"
#include "layers/PsiWorldLayer.h"
#include "Application.h"
#include "imgui.h"

ControlPanel::ControlPanel(PsiWorldLayer* worldLayer, Application::Application* app)
    : m_WorldLayer(worldLayer)
    , m_Application(app)
{
}

void ControlPanel::Render()
{
    if (!m_Visible)
        return;

    // Position at middle left of the screen
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImVec2 windowPos = ImVec2(.0f, viewport->WorkPos.y + viewport->WorkSize.y * 0.5f);
    ImGui::SetNextWindowPos(windowPos, ImGuiCond_FirstUseEver, ImVec2(0.0f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(300, 0), ImGuiCond_FirstUseEver);

    ImGui::Begin("Control Panel", &m_Visible);

    // Settings Section
    if (ImGui::CollapsingHeader("Settings"))
    {
        ImGui::Indent();

        // Background Color Picker
        if (m_Application)
        {
            ImGui::Text("World Background Color");
            ImGui::SameLine();
            ImGui::TextDisabled("(?)");
            if (ImGui::IsItemHovered())
            {
                ImGui::SetTooltip("Secondary background - furthest back layer");
            }

            // Color picker with RGB sliders - directly modifies Application's clear color
            ImGui::ColorEdit3("##BackgroundColor",
                            m_Application->clearColorValue.float32,
                            ImGuiColorEditFlags_NoAlpha);

            // Also update the PsiWorldLayer backgroundColor to keep it in sync
            if (m_WorldLayer)
            {
                m_WorldLayer->backgroundColor = glm::vec3(
                    m_Application->clearColorValue.float32[0],
                    m_Application->clearColorValue.float32[1],
                    m_Application->clearColorValue.float32[2]
                );
            }
        }

        ImGui::Unindent();
    }

    // Objects Section
    if (ImGui::CollapsingHeader("Objects", ImGuiTreeNodeFlags_DefaultOpen))
    {
        for (int i = 0; i < m_WorldLayer->data.scene.objects.size(); i++)
        {
            // Selectable object name - clicking selects the object
            bool isSelected = (m_WorldLayer->selectedObjectIndex == i);

            ImGui::PushID(i);

            if (ImGui::Selectable(m_WorldLayer->data.scene.objects[i].name.c_str(), isSelected, ImGuiSelectableFlags_AllowOverlap))
            {
                m_WorldLayer->selectedObjectIndex = i;
            }

            // Right-click context menu for delete
            if (ImGui::BeginPopupContextItem())
            {
                if (ImGui::MenuItem("Delete"))
                {
                    // Deselect if we're deleting the selected object
                    if (m_WorldLayer->selectedObjectIndex == i)
                    {
                        m_WorldLayer->selectedObjectIndex = -1;
                    }
                    // Adjust selection index if deleting before selected object
                    else if (m_WorldLayer->selectedObjectIndex > i)
                    {
                        m_WorldLayer->selectedObjectIndex--;
                    }

                    m_WorldLayer->data.scene.deleteObject(i);
                    ImGui::EndPopup();
                    ImGui::PopID();
                    break; // Exit loop since we modified the vector
                }
                ImGui::EndPopup();
            }

            ImGui::PopID();
        }
    }

    ImGui::End();
}
