#pragma once

#include "Layers/Layer.h"

// Forward declarations
class PsiWorldLayer;
class PsiNodeEditorLayer;
namespace Application { class Application; }

/**
 * ControlPanel - Settings and control UI panel
 *
 * Positioned at middle left of the screen.
 * Provides settings controls like background color adjustment.
 */
class ControlPanel
{
public:
    ControlPanel(PsiWorldLayer* worldLayer, PsiNodeEditorLayer* nodeEditorLayer, Application::Application* app);

    void Render();

    bool IsVisible() const { return m_Visible; }
    void SetVisible(bool visible) { m_Visible = visible; }

private:
    PsiWorldLayer* m_WorldLayer;
    PsiNodeEditorLayer* m_NodeEditorLayer;
    Application::Application* m_Application;
    bool m_Visible = true;
};
