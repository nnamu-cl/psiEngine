#include "PsiNodeEditorLayer.h"
#include "layers/PsiWorldLayer.h"
#include "drawers/NodeSystemDrawer.h"
#include "nodes/ValueNodes.h"
#include "imgui.h"
#include "imgui_node_editor.h"
#include "implot.h"

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
    // Create ImPlot context
    ImPlot::CreateContext();

    // Create drawers
    if (m_WorldLayer)
    {
        m_PropertyBinding = std::make_unique<NodePropertyBinding>(&m_WorldLayer->data.scene);
    }

    // Create node system drawer (creates its own editor context)
    m_NodeSystemDrawer = std::make_unique<NodeSystemDrawer>(&m_NodeGraph);
}

void PsiNodeEditorLayer::OnDetach()
{
    // Node system drawer will clean up its own context in its destructor

    // Destroy ImPlot context
    ImPlot::DestroyContext();
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
    //m_NodeGraph.markAllDirty();
    m_NodeGraph.evaluateAll();

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

        return;
        static float xs1[1001], ys1[1001];
        for (int i = 0; i < 1001; ++i) {
            xs1[i] = i * 0.001f;
            ys1[i] = 0.5f + 0.5f * sinf(50 * (xs1[i] + (float)ImGui::GetTime() / 10));
        }
        static double xs2[20], ys2[20];
        for (int i = 0; i < 20; ++i) {
            xs2[i] = i * 1/19.0f;
            ys2[i] = xs2[i] * xs2[i];
        }
        ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
        ImGui::Begin("My Window" );
        ImPlotAxisFlags flags = ImPlotAxisFlags_AutoFit;
        if (ImPlot::BeginPlot("Line Plots" )) {
            ImPlot::SetupAxes("x","y", flags, flags);
            ImPlot::PlotLine("f(x)", xs1, ys1, 1001);
            ImPlot::SetNextMarkerStyle(ImPlotMarker_Circle);
            ImPlot::PlotLine("g(x)", xs2, ys2, 20,ImPlotLineFlags_Segments);
            ImPlot::EndPlot();
        }
        ImGui::End();

    }
}

