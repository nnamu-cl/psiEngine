#include "ControlPanel.h"
#include "UISettings.h"
#include "layers/PsiWorldLayer.h"
#include "layers/PsiNodeEditorLayer.h"
#include "Application.h"
#include "Layers/DefaultGameWorld/Mesh.h"
#include "imgui.h"

// Node system headers
#include "nodes/NodeSystem.h"

namespace {
    // Helper function to convert generator mesh to our Mesh format
    template<typename GeneratorMesh>
    Mesh convertFromGenerator(GeneratorMesh&& genMesh)
    {
        Mesh mesh;

        // Extract vertices
        auto vertexGen = genMesh.vertices();
        while (!vertexGen.done()) {
            auto v = vertexGen.generate();
            mesh.vertices.push_back({
                glm::vec3(static_cast<float>(v.position[0]),
                         static_cast<float>(v.position[1]),
                         static_cast<float>(v.position[2])),
                glm::vec3(static_cast<float>(v.normal[0]),
                         static_cast<float>(v.normal[1]),
                         static_cast<float>(v.normal[2])),
                glm::vec2(static_cast<float>(v.texCoord[0]),
                         static_cast<float>(v.texCoord[1])),
                glm::vec4(1.0f)
            });
            vertexGen.next();
        }

        // Extract indices
        auto triangleGen = genMesh.triangles();
        while (!triangleGen.done()) {
            auto tri = triangleGen.generate();
            mesh.indices.push_back(static_cast<uint32_t>(tri.vertices[0]));
            mesh.indices.push_back(static_cast<uint32_t>(tri.vertices[1]));
            mesh.indices.push_back(static_cast<uint32_t>(tri.vertices[2]));
            triangleGen.next();
        }

        return mesh;
    }
}

ControlPanel::ControlPanel(PsiWorldLayer* worldLayer, PsiNodeEditorLayer* nodeEditorLayer, Application::Application* app)
    : m_WorldLayer(worldLayer)
    , m_NodeEditorLayer(nodeEditorLayer)
    , m_Application(app)
{
}

void ControlPanel::Render()
{
    if (!m_Visible)
        return;

    ImGui::SetNextWindowSize(ImVec2(300, 0), ImGuiCond_FirstUseEver);

    ImGui::Begin("Control Panel", &m_Visible);

    if (ImGui::BeginTabBar("ControlPanelTabs"))
    {

        // Objects Tab
        if (ImGui::BeginTabItem("Objects"))
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

            ImGui::EndTabItem();
        }




        // UI Layout Tab
        if (ImGui::BeginTabItem("UI Layout"))
        {
            UISettings& s = g_UISettings;

            ImGui::Text("Create Toolbar");
            ImGui::Separator();
            ImGui::Spacing();

            ImGui::SliderFloat("Create Top Margin", &s.createTopMargin, 0.0f, 800.0f);

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            ImGui::Text("Gizmo Toolbar");
            ImGui::Separator();
            ImGui::Spacing();

            ImGui::SliderFloat("Left Margin",    &s.gizmoLeftMargin,   0.0f,  400.0f);
            ImGui::SliderFloat("Top Margin",     &s.gizmoTopMargin,    0.0f,  800.0f);
            ImGui::Spacing();
            ImGui::SliderFloat("Frame Pad X",    &s.gizmoFramePadX,    0.0f,  30.0f);
            ImGui::SliderFloat("Frame Pad Y",    &s.gizmoFramePadY,    0.0f,  30.0f);
            ImGui::SliderFloat("Item Spacing X", &s.gizmoItemSpacingX, 0.0f,  30.0f);
            ImGui::SliderFloat("Item Spacing Y", &s.gizmoItemSpacingY, 0.0f,  30.0f);
            ImGui::Spacing();
            ImGui::SliderFloat("Font Scale",     &s.gizmoFontScale,    0.5f,  4.0f);
            ImGui::SliderFloat("Hover Alpha",    &s.gizmoHoverAlpha,   0.0f,  1.0f);
            ImGui::SliderFloat("Active Alpha",   &s.gizmoActiveAlpha,  0.0f,  1.0f);

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            ImGui::Text("Icon + Text Centering");
            ImGui::Separator();
            ImGui::Spacing();

            ImGui::SliderFloat("Icon Height",    &s.iconTextHeight,    10.0f, 60.0f);

            ImGui::Spacing();
            if (ImGui::Button("Reset Defaults", ImVec2(-1, 0)))
                s = UISettings{};

            ImGui::EndTabItem();
        }


        // Settings Tab
        if (ImGui::BeginTabItem("Settings"))
        {
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

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            // Node Editor Appearance
            if (m_NodeEditorLayer)
            {
                ImGui::Text("Node Editor Appearance");
                ImGui::Spacing();

            }

            ImGui::EndTabItem();
        }



        ImGui::EndTabBar();
    }

    ImGui::End();
}
