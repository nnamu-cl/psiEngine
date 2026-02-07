#include "PsiNodeEditorLayer.h"
#include "layers/PsiWorldLayer.h"
#include "drawers/GameObjectNodeDrawer.h"
#include "drawers/NodeSystemDrawer.h"
#include "nodes/ValueNodes.h"
#include "imgui.h"
#include "imgui_node_editor.h"

namespace ed = ax::NodeEditor;

PsiNodeEditorLayer::PsiNodeEditorLayer(PsiWorldLayer* worldLayer)
    : m_WorldLayer(worldLayer)
{
}

PsiNodeEditorLayer::~PsiNodeEditorLayer()
{
    // Context will be destroyed in OnDetach
}

void PsiNodeEditorLayer::OnAttach()
{
    // Create drawers
    if (m_WorldLayer)
    {
        m_GameObjectDrawer = std::make_unique<GameObjectNodeDrawer>(&m_WorldLayer->data.scene);
        m_PropertyBinding = std::make_unique<NodePropertyBinding>(&m_WorldLayer->data.scene);
    }

    // Create node system drawer (creates its own editor context)
    m_NodeSystemDrawer = std::make_unique<NodeSystemDrawer>(&m_NodeGraph);
}

void PsiNodeEditorLayer::OnDetach()
{
    // Node system drawer will clean up its own context in its destructor
}

void PsiNodeEditorLayer::OnUpdate(float ts)
{
    // Update elapsed time
    m_ElapsedTime += ts;

    // Update all TimeNodes with current elapsed time
    for (const auto& node : m_NodeGraph.getNodes())
    {
        if (TimeNode* timeNode = dynamic_cast<TimeNode*>(node.get()))
        {
            timeNode->setTime(m_ElapsedTime);
        }
    }

    // Mark all nodes dirty each frame (lazy evaluation system)
    m_NodeGraph.markAllDirty();

    // Update all GameObject properties bound to node outputs
    if (m_PropertyBinding)
    {
        m_PropertyBinding->updateAll();
    }
}

void PsiNodeEditorLayer::OnUIRender()
{
    if (m_ShowNodeEditor)
    {
        RenderNodeEditor();
    }
}

void PsiNodeEditorLayer::RenderNodeEditor()
{
    if (m_NodeSystemDrawer)
    {
        m_NodeSystemDrawer->DrawNodeGraph();
    }
}

