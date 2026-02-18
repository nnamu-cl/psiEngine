#pragma once

#include "Layers/Layer.h"
#include "nodes/NodeSystem.h"
#include "nodes/NodePropertyBinding.h"
#include <memory>

// Forward declarations
class PsiWorldLayer;
class NodeSystemDrawer;
class GraphDrawer;

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

    // Node system access
    NodeGraph& getNodeGraph() { return m_NodeGraph; }
    NodePropertyBinding& getPropertyBinding() { return *m_PropertyBinding; }
    float getElapsedTime() const { return m_ElapsedTime; }
    NodeSystemDrawer* getNodeSystemDrawer() { return m_NodeSystemDrawer.get(); }

private:
    PsiWorldLayer* m_WorldLayer;

    // Node Editor
    std::unique_ptr<NodeSystemDrawer> m_NodeSystemDrawer;
    std::unique_ptr<GraphDrawer> m_GraphDrawer;

    // Node System
    NodeGraph m_NodeGraph;
    std::unique_ptr<NodePropertyBinding> m_PropertyBinding;
    float m_ElapsedTime = 0.0f;

    // UI state
    bool m_ShowNodeEditor = true;
    bool m_NodeEditorFullscreen = true;

    // Render methods
    void RenderNodeEditor();
};
