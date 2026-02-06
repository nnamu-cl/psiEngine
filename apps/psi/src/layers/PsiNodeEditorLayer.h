#pragma once

#include "Layers/Layer.h"
#include <memory>

// Forward declarations
class PsiWorldLayer;
class GameObjectNodeDrawer;
namespace ax::NodeEditor { struct EditorContext; }

/**
 * PsiNodeEditorLayer - Node editor layer for PSI application
 *
 * Provides a fullscreen node editor canvas that renders behind the UI layer.
 * Used for visual programming and node-based workflows.
 */
class PsiNodeEditorLayer : public Application::Layer
{
public:
    PsiNodeEditorLayer(PsiWorldLayer* worldLayer);
    ~PsiNodeEditorLayer() override;

    // Layer lifecycle methods
    void OnAttach() override;
    void OnDetach() override;
    void OnUpdate(float ts) override;
    void OnUIRender() override;

    // Style control methods
    void UpdateNodeEditorStyle();

    // Public style properties
    float nodeEditorBgColor[4] = {0.1f, 0.1f, 0.1f, 0.0f};
    float nodeEditorGridColor[4] = {1.0f, 1.0f, 1.0f, 0.0f};
    float nodeEditorBorderColor[4] = {0.0f, 0.0f, 0.0f, 0.0f}; // Fully transparent border

private:
    PsiWorldLayer* m_WorldLayer;

    // Node Editor
    ax::NodeEditor::EditorContext* m_NodeEditorContext = nullptr;
    std::unique_ptr<GameObjectNodeDrawer> m_NodeDrawer;

    // UI state
    bool m_ShowNodeEditor = true;
    bool m_NodeEditorFullscreen = true;

    // Render methods
    void RenderNodeEditor();
};
