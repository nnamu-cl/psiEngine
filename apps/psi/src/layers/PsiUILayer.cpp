#include "PsiUILayer.h"
#include "layers/PsiWorldLayer.h"
#include "Application.h"
#include "project/ProjectHub.h"
#include "UI/ControlPanel.h"
#include "UI/StatsPanel.h"
#include "UI/InspectorPanel.h"
#include "UI/ViewManipulatorPanel.h"
#include "Layers/DefaultGameWorld/Mesh.h"
#include "imgui.h"
#include <string>
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

PsiUILayer::PsiUILayer(PsiWorldLayer* worldLayer, PsiNodeEditorLayer* nodeEditorLayer, Application::Application* app)
    : m_WorldLayer(worldLayer)
    , m_NodeEditorLayer(nodeEditorLayer)
    , m_Application(app)
{
    m_ControlPanel  = std::make_unique<ControlPanel>(worldLayer, nodeEditorLayer, app);
    m_StatsPanel    = std::make_unique<StatsPanel>(worldLayer);
    m_InspectorPanel = std::make_unique<InspectorPanel>(worldLayer, nodeEditorLayer);
    m_ProjectHub    = std::make_unique<ProjectHub>();
    m_ViewManipulator = std::make_unique<ViewManipulatorPanel>();
}

PsiUILayer::~PsiUILayer()
{
    // Panels will be automatically destroyed via unique_ptr
}

void PsiUILayer::OnAttach()
{
    // Called when the layer is attached to the application
    // Initialize UI resources, load settings, etc.
}

void PsiUILayer::OnDetach()
{
    // Called when the layer is detached from the application
    // Cleanup UI resources, save settings, etc.
}

void PsiUILayer::OnUpdate(float ts)
{
    // Update logic called every frame
    // ts = time step (delta time) in seconds
    // Update animations, time-based UI effects, etc.
}

void PsiUILayer::OnUIRender()
{
    if (m_ShowProjectHub)
    {
        if (m_ProjectHub->Render())
            m_ShowProjectHub = false;
        return;
    }

    // Render the view orientation gizmo first (it calls ImGuizmo::BeginFrame internally)
    m_ViewManipulator->Render();

    // Render panels (StatsPanel last so it appears on top)
    m_ControlPanel->Render();
    m_InspectorPanel->Render();

    // Render StatsPanel last so it's on top
    m_StatsPanel->Render();
}


