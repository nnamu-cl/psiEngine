#pragma once

#include "Layers/Layer.h"
#include <string>
#include <memory>

// Forward declarations
class PsiWorldLayer;
class ControlPanel;
class StatsPanel;
class InspectorPanel;
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
    PsiUILayer(PsiWorldLayer* worldLayer, Application::Application* app);
    ~PsiUILayer() override;

    // Layer lifecycle methods
    void OnAttach() override;
    void OnDetach() override;
    void OnUpdate(float ts) override;
    void OnUIRender() override;

private:
    PsiWorldLayer* m_WorldLayer;
    Application::Application* m_Application;

    // UI Panels
    std::unique_ptr<ControlPanel> m_ControlPanel;
    std::unique_ptr<StatsPanel> m_StatsPanel;
    std::unique_ptr<InspectorPanel> m_InspectorPanel;

    // UI state
    bool m_ShowDemoWindow = true;

    // Simulation parameters
    float m_TimeScale = 1.0f;
    bool m_SimulationPaused = false;

    // Render menu bar
    void RenderMenuBar();

    // Helper: Add a primitive mesh to the scene
    void AddPrimitiveMesh(const std::string& primitiveType);
};
