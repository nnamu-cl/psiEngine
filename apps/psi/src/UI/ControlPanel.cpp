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
#include <generator/DiskMesh.hpp>

// Node system headers
#include "nodes/GraphNodes.h"
#include "nodes/NodeSystem.h"
#include "nodes/ValueNodes.h"
#include "nodes/MathNodes.h"
#include "nodes/VectorNodes.h"
#include "nodes/ObjectNodes.h"

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
                auto mesh = convertFromGenerator(generator::TorusMesh(0.25, 1.0, 32, 16));
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

            if (ImGui::Button("Circle", ImVec2(-1, 0)))
            {
                auto mesh = convertFromGenerator(generator::DiskMesh(1.0, 0.0, 32, 4));
                m_WorldLayer->addMeshPrimitive("Circle", std::move(mesh),
                    glm::vec3(0.0f, 0.0f, 0.0f),
                    glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
            }

            ImGui::EndTabItem();
        }

        // Nodes Tab
        if (ImGui::BeginTabItem("Nodes"))
        {
            // Special color for node buttons (cyan/blue theme with better contrast)
            ImVec4 nodeButtonColor = ImVec4(0.15f, 0.35f, 0.55f, 1.0f);     // Darker for readability
            ImVec4 nodeButtonHovered = ImVec4(0.2f, 0.45f, 0.65f, 1.0f);    // Hovered
            ImVec4 nodeButtonActive = ImVec4(0.1f, 0.3f, 0.5f, 1.0f);       // Active

            if (!m_NodeEditorLayer)
            {
                ImGui::TextWrapped("Node Editor not available");
                ImGui::EndTabItem();
            }
            else
            {
                NodeGraph& graph = m_NodeEditorLayer->getNodeGraph();

                ImGui::Text("Graph Nodes");
                ImGui::Separator();

                ImGui::PushStyleColor(ImGuiCol_Button, nodeButtonColor);
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, nodeButtonHovered);
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, nodeButtonActive);

                if (ImGui::Button("Graph Node", ImVec2(-1, 0)))
                {
                    graph.createNode<LineGraphNode>();
                }

                ImGui::PopStyleColor(3);

                ImGui::Text("Value Nodes");
                ImGui::Separator();

                ImGui::PushStyleColor(ImGuiCol_Button, nodeButtonColor);
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, nodeButtonHovered);
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, nodeButtonActive);

                if (ImGui::Button("Float Constant", ImVec2(-1, 0)))
                {
                    graph.createNode<FloatConstantNode>(0.0f);
                }

                if (ImGui::Button("Int Constant", ImVec2(-1, 0)))
                {
                    graph.createNode<IntConstantNode>(0);
                }

                if (ImGui::Button("Vec3 Constant", ImVec2(-1, 0)))
                {
                    graph.createNode<Vec3ConstantNode>(glm::vec3(0.0f));
                }

                if (ImGui::Button("Time", ImVec2(-1, 0)))
                {
                    graph.createNode<TimeNode>();
                }


                if (ImGui::Button("Physics Constant", ImVec2(-1, 0)))
                {
                    graph.createNode<PhysicsConstantNode>();
                }

                ImGui::PopStyleColor(3);

                ImGui::Spacing();
                ImGui::Text("Math Nodes");
                ImGui::Separator();

                ImGui::PushStyleColor(ImGuiCol_Button, nodeButtonColor);
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, nodeButtonHovered);
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, nodeButtonActive);

                if (ImGui::Button("Add", ImVec2(-1, 0)))
                {
                    graph.createNode<AddNode>();
                }

                if (ImGui::Button("Subtract", ImVec2(-1, 0)))
                {
                    graph.createNode<SubtractNode>();
                }

                if (ImGui::Button("Multiply", ImVec2(-1, 0)))
                {
                    graph.createNode<MultiplyNode>();
                }

                if (ImGui::Button("Divide", ImVec2(-1, 0)))
                {
                    graph.createNode<DivideNode>();
                }

                if (ImGui::Button("Sin", ImVec2(-1, 0)))
                {
                    graph.createNode<SinNode>();
                }

                if (ImGui::Button("Cos", ImVec2(-1, 0)))
                {
                    graph.createNode<CosNode>();
                }

                if (ImGui::Button("Tan", ImVec2(-1, 0)))
                {
                    graph.createNode<TanNode>();
                }

                if (ImGui::Button("Arcsin", ImVec2(-1, 0)))
                {
                    graph.createNode<ArcsinNode>();
                }

                if (ImGui::Button("Arccos", ImVec2(-1, 0)))
                {
                    graph.createNode<ArccosNode>();
                }

                if (ImGui::Button("Arctan", ImVec2(-1, 0)))
                {
                    graph.createNode<ArctanNode>();
                }

                if (ImGui::Button("Pow", ImVec2(-1, 0)))
                {
                    graph.createNode<PowNode>();
                }

                if (ImGui::Button("Root", ImVec2(-1, 0)))
                {
                    graph.createNode<RootNode>();
                }

                ImGui::PopStyleColor(3);

                ImGui::Spacing();
                ImGui::Text("Constants");
                ImGui::Separator();

                ImGui::PushStyleColor(ImGuiCol_Button, nodeButtonColor);
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, nodeButtonHovered);
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, nodeButtonActive);

                if (ImGui::Button("PI", ImVec2(-1, 0)))
                {
                    graph.createNode<PINode>();
                }

                ImGui::PopStyleColor(3);

                ImGui::Spacing();
                ImGui::Text("Vector Nodes");
                ImGui::Separator();

                ImGui::PushStyleColor(ImGuiCol_Button, nodeButtonColor);
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, nodeButtonHovered);
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, nodeButtonActive);

                if (ImGui::Button("Combine Vec3", ImVec2(-1, 0)))
                {
                    graph.createNode<CombineVec3Node>();
                }

                if (ImGui::Button("Separate Vec3", ImVec2(-1, 0)))
                {
                    graph.createNode<SeparateVec3Node>();
                }

                if (ImGui::Button("Dot Product", ImVec2(-1, 0)))
                {
                    graph.createNode<DotProductNode>();
                }

                if (ImGui::Button("Cross Product", ImVec2(-1, 0)))
                {
                    graph.createNode<CrossProductNode>();
                }

                if (ImGui::Button("Length", ImVec2(-1, 0)))
                {
                    graph.createNode<LengthNode>();
                }

                ImGui::PopStyleColor(3);

                ImGui::Spacing();
                ImGui::Text("Object Nodes");
                ImGui::Separator();

                ImGui::PushStyleColor(ImGuiCol_Button, nodeButtonColor);
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, nodeButtonHovered);
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, nodeButtonActive);

                if (ImGui::Button("Transform", ImVec2(-1, 0)))
                {
                    graph.createNode<TransformNode>();
                }

                if (ImGui::Button("Line Renderer", ImVec2(-1, 0)))
                {
                    graph.createNode<LineRendererNode>();
                }

                ImGui::PopStyleColor(3);

                ImGui::EndTabItem();
            }
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
