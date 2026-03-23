#pragma once

#define IMGUI_DEFINE_MATH_OPERATORS

#include "INodeDrawer.h"
#include "../nodes/NodeSystem.h"
#include <unordered_map>
#include <string>

#include "IconsLucide.h"
#include "imgui_node_editor.h"

/**
 * NodeSystemDrawer - Draws node system nodes (TimeNode, AddNode, etc.) in the node editor
 *
 * Visualizes computational nodes with their inputs and outputs, allowing
 * visual programming through connections.
 */





// Maps a semantic NodeIcon enum value to the corresponding Lucide icon glyph.
// This is the only place in the codebase that couples NodeIcon to a specific icon font.
static const char* toIconGlyph(NodeIcon icon)
{
    switch (icon)
    {
        case NodeIcon::Network:        return ICON_LC_NETWORK;
        case NodeIcon::Variable:       return ICON_LC_VARIABLE;
        case NodeIcon::VectorSquare:   return ICON_LC_VECTOR_SQUARE;
        case NodeIcon::Wave:           return ICON_LC_WAVES;
        case NodeIcon::Trig:           return ICON_LC_TRIANGLE_RIGHT;
        case NodeIcon::Time:           return ICON_LC_CLOCK_8;
        case NodeIcon::Atom:           return ICON_LC_ATOM;
        case NodeIcon::Plus:           return ICON_LC_PLUS;
        case NodeIcon::Minus:          return ICON_LC_MINUS;
        case NodeIcon::Asterisk:       return ICON_LC_ASTERISK;
        case NodeIcon::Divide:         return ICON_LC_DIVIDE;
        case NodeIcon::SquareFunction: return ICON_LC_SQUARE_FUNCTION;
        case NodeIcon::Radical:        return ICON_LC_RADICAL;
        case NodeIcon::Pi:             return ICON_LC_PI;
        case NodeIcon::Layers:         return ICON_LC_LAYERS;
        case NodeIcon::Layers2:        return ICON_LC_LAYERS_2;
        case NodeIcon::Dot:            return ICON_LC_DOT;
        case NodeIcon::X:              return ICON_LC_X;
        case NodeIcon::Ruler:          return ICON_LC_RULER;
        case NodeIcon::Move3D:         return ICON_LC_MOVE_3D;
        case NodeIcon::Spline:         return ICON_LC_SPLINE;
        case NodeIcon::Volume:         return ICON_LC_CIRCLE;
        case NodeIcon::Generic:
        default:                       return ICON_LC_CIRCLE_DOT;
    }
}



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

    void DrawInlineEditor(InputSocket* socket);

    // Helper to draw node inputs and outputs
    void    DrawInputSockets(Node* node);
    void DrawOutputSockets(Node* node);

    // Helper to generate unique pin IDs
    uint64_t GetPinId(uint64_t nodeId, const std::string& socketName, bool isInput) const;

    // Track connections for rendering
    struct ConnectionInfo {
        uint64_t inputPinId;
        uint64_t outputPinId;
        ImU32 fromColor;
        ImU32 toColor;
    };
    std::vector<ConnectionInfo> m_Connections;
};
