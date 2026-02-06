#include "PsiNodeEditorLayer.h"
#include "layers/PsiWorldLayer.h"
#include "GameObjectNodeDrawer.h"
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

    // Create node drawer for GameObjects
    if (m_WorldLayer)
    {
        m_NodeDrawer = std::make_unique<GameObjectNodeDrawer>(&m_WorldLayer->data.scene);
    }
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
    style.Colors[ed::StyleColor_NodeBorder] = transparentBorder;
    style.Colors[ed::StyleColor_HovNodeBorder] = transparentBorder;
    style.Colors[ed::StyleColor_SelNodeBorder] = transparentBorder;

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
    // Update logic called every frame
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

    // Draw all GameObject nodes
    if (m_NodeDrawer)
    {
        int nodeCount = m_NodeDrawer->GetNodeCount();
        for (int i = 0; i < nodeCount; ++i)
        {
            m_NodeDrawer->DrawNode(i);
        }

        // Draw connections between nodes
        m_NodeDrawer->DrawLinks();

        // Handle interactions (selection, deletion, etc.)
        m_NodeDrawer->HandleInteractions();
    }

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
