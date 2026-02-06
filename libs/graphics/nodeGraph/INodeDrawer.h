#pragma once

#include <cstdint>

namespace ax::NodeEditor { struct EditorContext; }

/**
 * INodeDrawer - Interface for drawing nodes in the node editor
 *
 * Provides a contract for rendering objects as nodes in the ImGui node editor.
 * Implementations should handle the visual representation and pin connections
 * for their specific object types.
 */
class INodeDrawer
{
public:
    virtual ~INodeDrawer() = default;

    /**
     * Draw a single node in the node editor
     * @param nodeId Unique identifier for this node
     */
    virtual void DrawNode(int nodeId) = 0;

    /**
     * Draw connections/links between nodes
     * Called after all nodes have been drawn
     */
    virtual void DrawLinks() {}

    /**
     * Handle node interactions (selection, deletion, etc.)
     * Called after nodes and links are drawn
     */
    virtual void HandleInteractions() {}

    /**
     * Get the total number of nodes this drawer manages
     */
    virtual int GetNodeCount() const = 0;
};
