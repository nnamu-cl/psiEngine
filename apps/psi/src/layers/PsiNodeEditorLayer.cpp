#include "PsiNodeEditorLayer.h"
#include "layers/PsiWorldLayer.h"
#include "GameObjectNodeDrawer.h"
#include "NodeSystemDrawer.h"
#include "ValueNodes.h"
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
    // Create node editor context
    ed::Config config;
    m_NodeEditorContext = ed::CreateEditor(&config);

    // Apply initial style
    UpdateNodeEditorStyle();

    // Create drawers
    if (m_WorldLayer)
    {
        m_GameObjectDrawer = std::make_unique<GameObjectNodeDrawer>(&m_WorldLayer->data.scene);
        m_PropertyBinding = std::make_unique<NodePropertyBinding>(&m_WorldLayer->data.scene);
    }

    // Create node system drawer
    m_NodeSystemDrawer = std::make_unique<NodeSystemDrawer>(&m_NodeGraph);
}

void PsiNodeEditorLayer::UpdateNodeEditorStyle()
{
    if (!m_NodeEditorContext)
        return;

    ed::SetCurrentEditor(m_NodeEditorContext);
    auto& style = ed::GetStyle();
    style.Colors[ed::StyleColor_Bg] = ImVec4(nodeEditorBgColor[0], nodeEditorBgColor[1], nodeEditorBgColor[2], nodeEditorBgColor[3]);
    style.Colors[ed::StyleColor_Grid] = ImVec4(nodeEditorGridColor[0], nodeEditorGridColor[1], nodeEditorGridColor[2], nodeEditorGridColor[3]);

    // Set all border-related colors to transparent
    ImVec4 transparentBorder = ImVec4(nodeEditorBorderColor[0], nodeEditorBorderColor[1], nodeEditorBorderColor[2], nodeEditorBorderColor[3]);

    ed::SetCurrentEditor(nullptr);
}

void PsiNodeEditorLayer::OnDetach()
{
    // Destroy node editor context
    if (m_NodeEditorContext)
    {
        ed::DestroyEditor(m_NodeEditorContext);
        m_NodeEditorContext = nullptr;
    }
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
    ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;

    if (m_NodeEditorFullscreen)
    {
        // Get main viewport
        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(viewport->WorkSize);

        windowFlags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
                       ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                       ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
    }

    ImGui::SetNextWindowBgAlpha(0.0f); // Fully transparent window background
    ImGui::Begin("Node Editor", &m_ShowNodeEditor, windowFlags);

    ed::SetCurrentEditor(m_NodeEditorContext);
    ed::Begin("My Editor");

    // PHASE 1: Draw ALL nodes first (both GameObjects and NodeSystem)
    if (m_GameObjectDrawer)
    {
        int nodeCount = m_GameObjectDrawer->GetNodeCount();
        for (int i = 0; i < nodeCount; ++i)
        {
            m_GameObjectDrawer->DrawNode(i);
        }
    }

    if (m_NodeSystemDrawer)
    {
        int nodeCount = m_NodeSystemDrawer->GetNodeCount();
        for (int i = 0; i < nodeCount; ++i)
        {
            m_NodeSystemDrawer->DrawNode(i);
        }
    }

    // PHASE 2: Draw ALL links (after all nodes are drawn)
    if (m_GameObjectDrawer)
    {
        m_GameObjectDrawer->DrawLinks();
    }

    if (m_NodeSystemDrawer)
    {
        m_NodeSystemDrawer->DrawLinks();
    }

    // PHASE 3: Handle ALL interactions in ONE unified block
    // NOTE: ed::BeginCreate() and ed::BeginDelete() can only be called ONCE per frame
    HandleAllInteractions();

    // Navigate to show all content on first frame
    static bool firstFrame = true;
    if (firstFrame)
    {
        ed::NavigateToContent();
        firstFrame = false;
    }

    ed::End();
    ed::SetCurrentEditor(nullptr);

    ImGui::End();
}

void PsiNodeEditorLayer::HandleAllInteractions()
{
    // Unified interaction handling for all node types
    // This method can only be called once per frame (single BeginCreate/BeginDelete)

    // For now, delegate to NodeSystemDrawer which has the pin ID logic
    if (m_NodeSystemDrawer)
    {
        m_NodeSystemDrawer->HandleInteractions();
    }
}
