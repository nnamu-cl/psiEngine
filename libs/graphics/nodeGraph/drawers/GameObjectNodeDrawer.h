#pragma once

#include "INodeDrawer.h"
#include <vector>

// Forward declarations
class Scene;
struct GameObject;

/**
 * GameObjectNodeDrawer - Draws GameObject instances as nodes in the node editor
 *
 * Visualizes each GameObject in the scene as a node, displaying its name
 * and delegating component-specific UI to each component's DrawNodeUI method.
 */
class GameObjectNodeDrawer : public INodeDrawer
{
public:
    explicit GameObjectNodeDrawer(Scene* scene);

    void DrawNode(int nodeId) override;
    void DrawLinks() override;
    void HandleInteractions() override;
    int GetNodeCount() const override;

private:
    Scene* m_Scene;

    // Helper to draw a single GameObject as a node
    void DrawGameObjectNode(GameObject& obj, int nodeId);

};
