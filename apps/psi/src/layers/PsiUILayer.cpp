#include "PsiUILayer.h"
#include "layers/PsiWorldLayer.h"
#include "Application.h"
#include "UI/ControlPanel.h"
#include "UI/StatsPanel.h"
#include "UI/InspectorPanel.h"
#include "imgui.h"
#include <string>

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
}

void PsiUILayer::RenderMenuBar()
{
    if (ImGui::BeginMainMenuBar())
    {

        ImGui::EndMainMenuBar();
    }
}
