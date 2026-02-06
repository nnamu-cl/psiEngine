#include "ControlPanel.h"
#include "layers/PsiWorldLayer.h"
#include "layers/PsiNodeEditorLayer.h"
#include "Application.h"
#include "Layers/DefaultGameWorld/Mesh.h"
#include "imgui.h"
#include <generator/BoxMesh.hpp>
#include <generator/TriangleMesh.hpp>
#include <generator/SphereMesh.hpp>
#include <generator/CylinderMesh.hpp>
#include <generator/TorusMesh.hpp>
#include <generator/ConeMesh.hpp>

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

    // Position at middle left of the screen
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImVec2 windowPos = ImVec2(.0f, viewport->WorkPos.y + viewport->WorkSize.y * 0.5f);
    ImGui::SetNextWindowPos(windowPos, ImGuiCond_FirstUseEver, ImVec2(0.0f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(300, 0), ImGuiCond_FirstUseEver);

    ImGui::Begin("Control Panel", &m_Visible);

    if (ImGui::BeginTabBar("ControlPanelTabs"))
    {
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

                // Background Color
                ImGui::Text("Background Color");
                if (ImGui::ColorEdit4("##NodeEditorBgColor", m_NodeEditorLayer->nodeEditorBgColor))
                {
                    m_NodeEditorLayer->UpdateNodeEditorStyle();
                }

                ImGui::Spacing();

                // Grid Color
                ImGui::Text("Grid Color");
                if (ImGui::ColorEdit4("##NodeEditorGridColor", m_NodeEditorLayer->nodeEditorGridColor))
                {
                    m_NodeEditorLayer->UpdateNodeEditorStyle();
                }
            }

            ImGui::EndTabItem();
        }

        // Add Tab
        if (ImGui::BeginTabItem("Add"))
        {
            if (ImGui::Button("Cube", ImVec2(-1, 0)))
            {
                auto mesh = MeshTable::unitCube();
                m_WorldLayer->addMeshPrimitive("Cube", std::move(mesh),
                    glm::vec3(0.0f, 0.0f, 0.0f),
                    glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
            }

            if (ImGui::Button("Triangle", ImVec2(-1, 0)))
            {
                auto mesh = MeshTable::unitTriangle();
                m_WorldLayer->addMeshPrimitive("Triangle", std::move(mesh),
                    glm::vec3(0.0f, 0.0f, 0.0f),
                    glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));
            }

            if (ImGui::Button("Sphere", ImVec2(-1, 0)))
            {
                auto mesh = convertFromGenerator(generator::SphereMesh(1.0, 32, 16));
                m_WorldLayer->addMeshPrimitive("Sphere", std::move(mesh),
                    glm::vec3(0.0f, 0.0f, 0.0f),
                    glm::vec4(0.0f, 0.0f, 1.0f, 1.0f));
            }

            if (ImGui::Button("Cylinder", ImVec2(-1, 0)))
            {
                auto mesh = convertFromGenerator(generator::CylinderMesh(1.0, 1.0, 2.0, 32, 8));
                m_WorldLayer->addMeshPrimitive("Cylinder", std::move(mesh),
                    glm::vec3(0.0f, 0.0f, 0.0f),
                    glm::vec4(1.0f, 1.0f, 0.0f, 1.0f));
            }

            if (ImGui::Button("Torus", ImVec2(-1, 0)))
            {
                auto mesh = convertFromGenerator(generator::TorusMesh(1.0, 0.25, 32, 16));
                m_WorldLayer->addMeshPrimitive("Torus", std::move(mesh),
                    glm::vec3(0.0f, 0.0f, 0.0f),
                    glm::vec4(1.0f, 0.0f, 1.0f, 1.0f));
            }

            if (ImGui::Button("Cone", ImVec2(-1, 0)))
            {
                auto mesh = convertFromGenerator(generator::ConeMesh(1.0, 2.0, 32, 8));
                m_WorldLayer->addMeshPrimitive("Cone", std::move(mesh),
                    glm::vec3(0.0f, 0.0f, 0.0f),
                    glm::vec4(0.0f, 1.0f, 1.0f, 1.0f));
            }

            ImGui::EndTabItem();
        }

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

        ImGui::EndTabBar();
    }

    ImGui::End();
}
