#include "PsiUILayer.h"
#include "layers/PsiWorldLayer.h"
#include "Application.h"
#include "UI/ControlPanel.h"
#include "UI/StatsPanel.h"
#include "UI/InspectorPanel.h"
#include "Layers/DefaultGameWorld/Mesh.h"
#include "imgui.h"
#include "imgui_node_editor.h"
#include <string>
#include <generator/BoxMesh.hpp>
#include <generator/TriangleMesh.hpp>
#include <generator/SphereMesh.hpp>
#include <generator/CylinderMesh.hpp>
#include <generator/TorusMesh.hpp>
#include <generator/ConeMesh.hpp>

namespace ed = ax::NodeEditor;

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

PsiUILayer::PsiUILayer(PsiWorldLayer* worldLayer, Application::Application* app)
    : m_WorldLayer(worldLayer)
    , m_Application(app)
{
    // Create UI panels
    m_ControlPanel = std::make_unique<ControlPanel>(worldLayer, app);
    m_StatsPanel = std::make_unique<StatsPanel>(worldLayer);
    m_InspectorPanel = std::make_unique<InspectorPanel>(worldLayer);
}

PsiUILayer::~PsiUILayer()
{
    // Panels will be automatically destroyed via unique_ptr
}

void PsiUILayer::OnAttach()
{
    // Called when the layer is attached to the application
    // Initialize UI resources, load settings, etc.

    // Create node editor context
    ed::Config config;
    m_NodeEditorContext = ed::CreateEditor(&config);
}

void PsiUILayer::OnDetach()
{
    // Called when the layer is detached from the application
    // Cleanup UI resources, save settings, etc.

    // Destroy node editor context
    if (m_NodeEditorContext)
    {
        ed::DestroyEditor(m_NodeEditorContext);
        m_NodeEditorContext = nullptr;
    }
}

void PsiUILayer::OnUpdate(float ts)
{
    // Update logic called every frame
    // ts = time step (delta time) in seconds
    // Update animations, time-based UI effects, etc.
}

void PsiUILayer::OnUIRender()
{
    // Render the UI using ImGui
    RenderMenuBar();

    // Render all panels
    m_ControlPanel->Render();
    m_StatsPanel->Render();
    m_InspectorPanel->Render();

    if (m_ShowDemoWindow)
    {
        ImGui::ShowDemoWindow(&m_ShowDemoWindow);
    }

    if (m_ShowNodeEditor)
    {
        RenderNodeEditor();
    }
}

void PsiUILayer::RenderMenuBar()
{
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("Add"))
        {
            if (ImGui::MenuItem("Cube"))
            {
                auto mesh = MeshTable::unitCube();
                m_WorldLayer->addMeshPrimitive("Cube", std::move(mesh),
                    glm::vec3(0.0f, 0.0f, 0.0f),
                    glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
            }

            if (ImGui::MenuItem("Triangle"))
            {
                auto mesh = MeshTable::unitTriangle();
                m_WorldLayer->addMeshPrimitive("Triangle", std::move(mesh),
                    glm::vec3(0.0f, 0.0f, 0.0f),
                    glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));
            }

            if (ImGui::MenuItem("Sphere"))
            {
                auto mesh = convertFromGenerator(generator::SphereMesh(1.0, 32, 16));
                m_WorldLayer->addMeshPrimitive("Sphere", std::move(mesh),
                    glm::vec3(0.0f, 0.0f, 0.0f),
                    glm::vec4(0.0f, 0.0f, 1.0f, 1.0f));
            }

            if (ImGui::MenuItem("Cylinder"))
            {
                auto mesh = convertFromGenerator(generator::CylinderMesh(1.0, 1.0, 2.0, 32, 8));
                m_WorldLayer->addMeshPrimitive("Cylinder", std::move(mesh),
                    glm::vec3(0.0f, 0.0f, 0.0f),
                    glm::vec4(1.0f, 1.0f, 0.0f, 1.0f));
            }

            if (ImGui::MenuItem("Torus"))
            {
                auto mesh = convertFromGenerator(generator::TorusMesh(1.0, 0.25, 32, 16));
                m_WorldLayer->addMeshPrimitive("Torus", std::move(mesh),
                    glm::vec3(0.0f, 0.0f, 0.0f),
                    glm::vec4(1.0f, 0.0f, 1.0f, 1.0f));
            }

            if (ImGui::MenuItem("Cone"))
            {
                auto mesh = convertFromGenerator(generator::ConeMesh(1.0, 2.0, 32, 8));
                m_WorldLayer->addMeshPrimitive("Cone", std::move(mesh),
                    glm::vec3(0.0f, 0.0f, 0.0f),
                    glm::vec4(0.0f, 1.0f, 1.0f, 1.0f));
            }

            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }
}

void PsiUILayer::RenderNodeEditor()
{
    ImGui::Begin("Node Editor", &m_ShowNodeEditor, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

    ed::SetCurrentEditor(m_NodeEditorContext);
    ed::Begin("My Editor");

    // Create a simple node
    int uniqueId = 1;
    ed::BeginNode(uniqueId++);
        ImGui::Text("Basic Node");
        ed::BeginPin(uniqueId++, ed::PinKind::Input);
            ImGui::Text("-> Input");
        ed::EndPin();
        ImGui::SameLine();
        ed::BeginPin(uniqueId++, ed::PinKind::Output);
            ImGui::Text("Output ->");
        ed::EndPin();
    ed::EndNode();

    ed::End();
    ed::SetCurrentEditor(nullptr);

    ImGui::End();
}
