#include "PsiUILayer.h"
#include "layers/PsiWorldLayer.h"
#include "Application.h"
#include "project/ProjectHub.h"
#include "UI/ControlPanel.h"
#include "UI/StatsPanel.h"
#include "UI/InspectorPanel.h"
#include "UI/ViewManipulatorPanel.h"
#include "UI/GizmoToolbar.h"
#include "UI/ModeToolbar.h"
#include "UI/CreateToolbar.h"
#include "UI/BrandLabel.h"
#include "UI/SaveToolbar.h"
#include "Layers/DefaultGameWorld/Mesh.h"
#include "Layers/DefaultGameWorld/Camera.h"
#include "Components/Transform.h"
#include "imgui.h"
#include "ImGuizmo.h"
#include <glm/gtc/type_ptr.hpp>
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
    m_GizmoToolbar    = std::make_unique<GizmoToolbar>();
    m_ModeToolbar     = std::make_unique<ModeToolbar>(worldLayer, nodeEditorLayer);
    m_CreateToolbar   = std::make_unique<CreateToolbar>(worldLayer, nodeEditorLayer);
    m_BrandLabel      = std::make_unique<BrandLabel>();
    m_SaveToolbar     = std::make_unique<SaveToolbar>();
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
    // Keep all auto-placed windows away from the screen edges
    ImGui::GetStyle().DisplaySafeAreaPadding = ImVec2(8.0f, 8.0f);

    if (m_ShowProjectHub)
    {
        if (m_ProjectHub->Render())
            m_ShowProjectHub = false;
        return;
    }

    // Brand label at top center
    m_BrandLabel->Render();

    // Save button just below the brand label
    m_SaveToolbar->Render();

    // Render the view orientation gizmo first (it calls ImGuizmo::BeginFrame internally)
    m_ViewManipulator->Render();

    const bool isGraphMode = m_ModeToolbar->GetMode() == PsiMode::GraphEditor;

    // Render the transform gizmo for the selected object (not in graph mode)
    if (!isGraphMode)
        RenderSceneGizmo();

    // Toolbar for switching gizmo operation (not in graph mode)
    if (!isGraphMode)
        m_GizmoToolbar->Render();

    // Floating "+" button for adding meshes and nodes
    m_CreateToolbar->Render();

    // Mode switcher toolbar at bottom center
    m_ModeToolbar->Render();

    // Render panels (StatsPanel last so it appears on top)
    m_ControlPanel->Render();
    m_InspectorPanel->Render();

    // Render StatsPanel last so it's on top
    m_StatsPanel->Render();
}


void PsiUILayer::RenderSceneGizmo()
{
    if (m_WorldLayer->selectedObjectIndex < 0)
        return;

    auto& objects = m_WorldLayer->data.scene.objects;
    if (m_WorldLayer->selectedObjectIndex >= static_cast<int>(objects.size()))
        return;

    Transform* transform = objects[m_WorldLayer->selectedObjectIndex].components.get<Transform>();
    if (!transform)
        return;

    Camera* camera = Camera::GetMain();
    if (!camera)
        return;

    ImGuiIO& io = ImGui::GetIO();
    const float aspect = io.DisplaySize.x / io.DisplaySize.y;

    glm::mat4 view = camera->viewMatrix();

    // projectionMatrix() applies the Vulkan Y-flip; ImGuizmo expects OpenGL convention so undo it.
    glm::mat4 proj = camera->projectionMatrix(aspect);
    proj[1][1] *= -1.0f;

    glm::mat4 model = transform->toMatrix();

    // Draw into the background drawlist so the gizmo is not clipped to any panel window.
    ImGuizmo::SetDrawlist(ImGui::GetBackgroundDrawList());
    ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);

    ImGuizmo::Manipulate(
        glm::value_ptr(view),
        glm::value_ptr(proj),
        static_cast<ImGuizmo::OPERATION>(m_GizmoToolbar->GetOperation()),
        ImGuizmo::LOCAL,
        glm::value_ptr(model)
    );

    if (ImGuizmo::IsUsing())
    {
        float t[3], r[3], s[3];
        ImGuizmo::DecomposeMatrixToComponents(glm::value_ptr(model), t, r, s);
        transform->position = { t[0], t[1], t[2] };
        transform->rotation = glm::quat(glm::vec3(glm::radians(r[0]), glm::radians(r[1]), glm::radians(r[2])));
        transform->scale    = { s[0], s[1], s[2] };
    }
}

