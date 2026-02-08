#pragma once

#include "../nodes/NodeSystem.h"

/**
 * GraphDrawer - Renders full-screen graphs for LineGraphNode instances
 *
 * Displays data from LineGraphNode instances with drawGraph=true in a full-screen
 * transparent overlay using ImPlot. Multiple graphs can be shown simultaneously.
 */
class GraphDrawer {
public:
    explicit GraphDrawer(NodeGraph* nodeGraph);
    ~GraphDrawer() = default;

    // Draw all graphs from LineGraphNode instances with drawGraph=true
    void DrawGraphs();

private:
    NodeGraph* m_NodeGraph;
};
