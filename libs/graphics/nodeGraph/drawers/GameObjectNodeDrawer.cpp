#include "GameObjectNodeDrawer.h"
#include "Layers/DefaultGameWorld/Scene.h"
#include "Components/Transform.h"
#include "Components/MeshRenderer.h"
#include "imgui.h"
#include "imgui_node_editor.h"
#include <glm/gtc/quaternion.hpp>
#include <unordered_map>

namespace ed = ax::NodeEditor;

GameObjectNodeDrawer::GameObjectNodeDrawer(Scene* scene)
    : m_Scene(scene)
{
}

int GameObjectNodeDrawer::GetNodeCount() const
{
    return m_Scene ? m_Scene->objects.size() : 0;
}



void GameObjectNodeDrawer::DrawNode(int nodeId)
{
    if (!m_Scene || nodeId >= m_Scene->objects.size())
        return;

    DrawGameObjectNode(m_Scene->objects[nodeId], nodeId);
}

void GameObjectNodeDrawer::DrawGameObjectNode(GameObject& obj, int nodeId)
{
    // Use the GameObject's unique ID for the node, not the array index
    uint64_t uniqueNodeId = obj.id;

    // Set initial position for this node if not already positioned
    // Arrange nodes in a grid layout based on array index
    static std::unordered_map<uint64_t, bool> positionedNodes;

    if (positionedNodes.find(uniqueNodeId) == positionedNodes.end())
    {
        // Arrange nodes in a grid: 3 columns, spacing 300x200
        int col = nodeId % 3;
        int row = nodeId / 3;
        ed::SetNodePosition(uniqueNodeId, ImVec2(100.0f + col * 300.0f, 100.0f + row * 200.0f));
        positionedNodes[uniqueNodeId] = true;
    }

    // BeginNode handles all ID scoping internally - no need for PushID!
    // Using GameObject's unique ID guarantees no conflicts
    ed::BeginNode(uniqueNodeId);

    // Node header with object name
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
    ImGui::TextUnformatted(obj.name.c_str());
    ImGui::PopStyleColor();

    ed::EndNode();
}

void GameObjectNodeDrawer::DrawLinks()
{
    // Future implementation: Draw connections between GameObjects
    // For now, no links are drawn
}

void GameObjectNodeDrawer::HandleInteractions()
{
    // Future implementation: Handle node selection, deletion, etc.
    // For now, no interactions are handled
}
