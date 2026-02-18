#pragma once

#include "INodeDrawer.h"
#include "../nodes/NodeSystem.h"
#include <unordered_map>
#include <string>

#include "imgui_node_editor.h"

/**
 * NodeSystemDrawer - Draws node system nodes (TimeNode, AddNode, etc.) in the node editor
 *
 * Visualizes computational nodes with their inputs and outputs, allowing
 * visual programming through connections.
 */
class NodeSystemDrawer : public INodeDrawer {
public:
    explicit NodeSystemDrawer(NodeGraph* nodeGraph);
    ~NodeSystemDrawer();

    ax::NodeEditor::EditorContext* m_NodeEditorContext = nullptr;
    bool Open = true;
    void DrawNode(int nodeId) override;
    void DrawAllNodes();  // Draw all nodes in the node graph
    void DrawLinks() override;
    void HandleInteractions() override;
    int GetNodeCount() const override;

    // High-level method to draw the entire node graph (nodes, links, and handle interactions)
    void DrawNodeGraph();

    void Save(const std::string& filePath);
    void Load(const std::string& filePath);

private:
    NodeGraph* m_NodeGraph;
    ax::NodeEditor::Config m_Config;
    std::string m_SettingsFilePath;
    bool m_NeedsRestore = false;

    // Track positioned nodes (node ID -> positioned flag)
    std::unordered_map<uint64_t, bool> m_PositionedNodes;

    // Track node name buffers (node ID -> name buffer)
    std::unordered_map<uint64_t, std::string> m_NodeNameBuffers;

    // Helper to draw a single node
    void DrawNodeInternal(Node* node, int arrayIndex);

    // Helper to draw node inputs and outputs
    void DrawInputSockets(Node* node);
    void DrawOutputSockets(Node* node);

    // Helper to generate unique pin IDs
    uint64_t GetPinId(uint64_t nodeId, const std::string& socketName, bool isInput) const;

    // Track connections for rendering
    struct ConnectionInfo {
        uint64_t inputPinId;
        uint64_t outputPinId;
    };
    std::vector<ConnectionInfo> m_Connections;
};
