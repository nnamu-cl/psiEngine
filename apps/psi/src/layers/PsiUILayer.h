#pragma once

#include "Layers/Layer.h"
#include <memory>

// Forward declarations
class PsiWorldLayer;
class PsiNodeEditorLayer;
class ControlPanel;
class StatsPanel;
class InspectorPanel;
class ProjectHub;
class ViewManipulatorPanel;
class GizmoToolbar;
class ModeToolbar;
class CreateToolbar;
class BrandLabel;
namespace Application { class Application; }

/**
 * PsiUILayer - Custom UI layer for PSI application
 *
 * Provides the user interface for physics simulation control,
 * parameter adjustment, and results visualization.
 *
 * Inherits from Application::Layer to integrate with the
 * application's layer stack system.
 */
class PsiUILayer : public Application::Layer
{
public:
    PsiUILayer(PsiWorldLayer* worldLayer, PsiNodeEditorLayer* nodeEditorLayer, Application::Application* app);
    ~PsiUILayer() override;

    // Layer lifecycle methods
    void OnAttach() override;
    void OnDetach() override;
    void OnUpdate(float ts) override;
    void OnUIRender() override;

private:
    PsiWorldLayer* m_WorldLayer;
    PsiNodeEditorLayer* m_NodeEditorLayer;
    Application::Application* m_Application;

    // UI Panels
    std::unique_ptr<ControlPanel> m_ControlPanel;
    std::unique_ptr<StatsPanel> m_StatsPanel;
    std::unique_ptr<InspectorPanel> m_InspectorPanel;
    std::unique_ptr<ViewManipulatorPanel> m_ViewManipulator;
    std::unique_ptr<GizmoToolbar>  m_GizmoToolbar;
    std::unique_ptr<ModeToolbar>   m_ModeToolbar;
    std::unique_ptr<CreateToolbar> m_CreateToolbar;
    std::unique_ptr<BrandLabel>    m_BrandLabel;

    // UI state
    bool m_ShowDemoWindow = true;

    // Simulation parameters
    float m_TimeScale = 1.0f;
    bool m_SimulationPaused = false;

    // Project hub (shown on startup)
    bool m_ShowProjectHub = true;
    std::unique_ptr<ProjectHub> m_ProjectHub;

    // Render methods
    void RenderMenuBar();
    void RenderSceneGizmo();
};
