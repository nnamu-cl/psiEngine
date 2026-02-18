#include "GraphDrawer.h"
#include "../nodes/GraphNodes.h"
#include "imgui.h"
#include "implot.h"

GraphDrawer::GraphDrawer(NodeGraph* nodeGraph)
    : m_NodeGraph(nodeGraph)
{
}

void GraphDrawer::DrawGraphs()
{
    if (!m_NodeGraph) return;

    // Find all LineGraphNode instances with drawGraph=true
    std::vector<LineGraphNode*> activeGraphs;
    for (const auto& node : m_NodeGraph->getNodes())
    {
        if (LineGraphNode* graphNode = dynamic_cast<LineGraphNode*>(node.get()))
        {
            if (graphNode->drawGraph)
            {
                activeGraphs.push_back(graphNode);
            }
        }
    }

    // If no active graphs, return early
    if (activeGraphs.empty()) return;

    // Create a full-screen transparent window
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration |
                                    ImGuiWindowFlags_NoMove |
                                    ImGuiWindowFlags_NoResize |
                                    ImGuiWindowFlags_NoSavedSettings |
                                    ImGuiWindowFlags_NoBackground |
                                    ImGuiWindowFlags_NoBringToFrontOnFocus;

    ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
    ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

    ImGui::Begin("FullscreenPlot", nullptr, window_flags);

    ImPlotAxisFlags flags = ImPlotAxisFlags_AutoFit;
    ImVec2 plotSize = ImGui::GetContentRegionAvail();

    if (ImPlot::BeginPlot("##FullscreenLinePlots", plotSize))
    {
        ImPlot::SetupAxes("Sample Index", "Value", flags, flags);

        // Plot each active graph
        for (LineGraphNode* graphNode : activeGraphs)
        {
            const auto& buffer = graphNode->points.buffer;
            if (!buffer.empty())
            {
                // Prepare data arrays for plotting
                std::vector<double> xs(buffer.size());
                std::vector<double> ys(buffer.size());

                // Fill arrays with indices and values
                for (size_t i = 0; i < buffer.size(); ++i)
                {
                    xs[i] = static_cast<double>(i);
                    ys[i] = static_cast<double>(buffer[i]);
                }

                // Plot line with node's name as label
                std::string label = graphNode->getName();

                // Add axis indicator to label if tracking a transform
                if (graphNode->getTrackedTransform())
                {
                    switch (graphNode->selectedAxis)
                    {
                        case LineGraphNode::Axis::X: label += " [X]"; break;
                        case LineGraphNode::Axis::Y: label += " [Y]"; break;
                        case LineGraphNode::Axis::Z: label += " [Z]"; break;
                    }
                }

                ImPlot::PlotLine(label.c_str(), xs.data(), ys.data(), static_cast<int>(buffer.size()));
            }
        }

        ImPlot::EndPlot();
    }

    ImGui::End();
    ImGui::PopStyleVar();

    // Add close button overlays for each active graph in the top-right corner
    // Stack them vertically
    float buttonY = 10.0f;
    for (LineGraphNode* graphNode : activeGraphs)
    {
        ImGuiWindowFlags button_flags = ImGuiWindowFlags_NoDecoration |
                                        ImGuiWindowFlags_NoMove |
                                        ImGuiWindowFlags_NoResize |
                                        ImGuiWindowFlags_NoSavedSettings |
                                        ImGuiWindowFlags_AlwaysAutoResize |
                                        ImGuiWindowFlags_NoBackground;

        ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x - 210.0f, buttonY));

        std::string windowName = "CloseButton##" + std::to_string(graphNode->getId());
        ImGui::Begin(windowName.c_str(), nullptr, button_flags);

        // Show node name and close button
        ImGui::Text("%s", graphNode->getName().c_str());
        ImGui::SameLine();

        std::string buttonLabel = "X##" + std::to_string(graphNode->getId());
        if (ImGui::Button(buttonLabel.c_str(), ImVec2(30.0f, 30.0f)))
        {
            graphNode->drawGraph = false;
        }

        ImGui::End();

        // Move down for next button
        buttonY += 50.0f;
    }
}
