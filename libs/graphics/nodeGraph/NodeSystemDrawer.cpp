#include "NodeSystemDrawer.h"
#include "imgui.h"
#include "imgui_node_editor.h"

namespace ed = ax::NodeEditor;

NodeSystemDrawer::NodeSystemDrawer(NodeGraph* nodeGraph)
    : m_NodeGraph(nodeGraph)
{
}

int NodeSystemDrawer::GetNodeCount() const
{
    return m_NodeGraph ? m_NodeGraph->getNodes().size() : 0;
}

void NodeSystemDrawer::DrawNode(int nodeId)
{
    if (!m_NodeGraph || nodeId >= m_NodeGraph->getNodes().size())
        return;

    DrawNodeInternal(m_NodeGraph->getNodes()[nodeId].get(), nodeId);
}

uint64_t NodeSystemDrawer::GetPinId(uint64_t nodeId, const std::string& socketName, bool isInput) const
{
    // Generate unique pin ID by combining node ID, socket name hash, and input/output flag
    // Use high bits for node ID, middle bits for socket hash, low bit for input/output
    std::hash<std::string> hasher;
    uint64_t socketHash = hasher(socketName) & 0xFFFFFF;  // 24 bits for socket name
    uint64_t inputFlag = isInput ? 1 : 0;

    return (nodeId << 32) | (socketHash << 8) | inputFlag;
}

void NodeSystemDrawer::DrawNodeInternal(Node* node, int arrayIndex)
{
    if (!node) return;

    uint64_t nodeId = node->getId();

    // Set initial position for this node if not already positioned
    if (m_PositionedNodes.find(nodeId) == m_PositionedNodes.end())
    {
        // Arrange nodes in a grid: 3 columns, spacing 300x200
        // Offset from GameObject nodes by starting further right
        int col = arrayIndex % 3;
        int row = arrayIndex / 3;
        ed::SetNodePosition(nodeId, ImVec2(600.0f + col * 300.0f, 100.0f + row * 200.0f));
        m_PositionedNodes[nodeId] = true;
    }

    // Begin node
    ed::BeginNode(nodeId);

    // Node header with type name
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.3f, 0.7f, 1.0f, 1.0f));  // Cyan for node system nodes
    ImGui::TextUnformatted(node->getTypeName());
    ImGui::PopStyleColor();

    ImGui::Spacing();

    // Layout: Input pins on left, custom UI in middle, output pins on right
    ImGui::BeginGroup();
    DrawInputSockets(node);
    ImGui::EndGroup();

    ImGui::SameLine();

    // Custom node UI (sliders, inputs, etc.)
    ImGui::BeginGroup();
    node->OnDrawNodeUI();
    ImGui::EndGroup();

    ImGui::SameLine();

    ImGui::BeginGroup();
    DrawOutputSockets(node);
    ImGui::EndGroup();

    ed::EndNode();
}

void NodeSystemDrawer::DrawInputSockets(Node* node)
{
    for (const auto& input : node->getInputs())
    {
        uint64_t pinId = GetPinId(node->getId(), input.name, true);
        ed::BeginPin(pinId, ed::PinKind::Input);
        ImGui::Text(">");  // Simple input arrow
        ed::EndPin();

        if (node->getInputs().size() > 1)
        {
            ImGui::SameLine();
            ImGui::TextDisabled("%s", input.name.c_str());
        }
    }
}

void NodeSystemDrawer::DrawOutputSockets(Node* node)
{
    for (const auto& output : node->getOutputs())
    {
        uint64_t pinId = GetPinId(node->getId(), output.name, false);

        if (node->getOutputs().size() > 1)
        {
            ImGui::TextDisabled("%s", output.name.c_str());
            ImGui::SameLine();
        }

        ed::BeginPin(pinId, ed::PinKind::Output);
        ImGui::Text(">");  // Simple output arrow
        ed::EndPin();
    }
}

void NodeSystemDrawer::DrawLinks()
{
    if (!m_NodeGraph) return;

    // Collect all connections from the node graph
    m_Connections.clear();

    for (const auto& node : m_NodeGraph->getNodes())
    {
        // For each input socket, check if it's connected
        for (const auto& input : node->getInputs())
        {
            if (input.connectedOutput)
            {
                // Generate pin IDs
                uint64_t inputPinId = GetPinId(node->getId(), input.name, true);
                uint64_t outputPinId = GetPinId(input.connectedOutput->owner->getId(),
                                                 input.connectedOutput->name, false);

                m_Connections.push_back({inputPinId, outputPinId});
            }
        }
    }

    // Draw all links
    for (size_t i = 0; i < m_Connections.size(); ++i)
    {
        const auto& conn = m_Connections[i];
        // Use connection index as link ID (offset by a large number to avoid conflicts)
        ed::Link(100000 + i, conn.outputPinId, conn.inputPinId);
    }
}

void NodeSystemDrawer::HandleInteractions()
{
    if (!m_NodeGraph) return;

    // Handle new connections - MUST call EndCreate() regardless of BeginCreate() result
    ed::BeginCreate();
    {
        ed::PinId inputPinId, outputPinId;
        if (ed::QueryNewLink(&inputPinId, &outputPinId))
        {
            // Find the sockets from pin IDs
            OutputSocket* outputSocket = nullptr;
            InputSocket* inputSocket = nullptr;

            // Search through all nodes to find matching sockets
            for (const auto& node : m_NodeGraph->getNodes())
            {
                // Check outputs
                for (auto& output : node->getOutputs())
                {
                    if (GetPinId(node->getId(), output.name, false) == outputPinId.Get())
                    {
                        outputSocket = &output;
                        break;
                    }
                }

                // Check inputs
                for (auto& input : node->getInputs())
                {
                    if (GetPinId(node->getId(), input.name, true) == inputPinId.Get())
                    {
                        inputSocket = &input;
                        break;
                    }
                }

                if (outputSocket && inputSocket) break;
            }

            // Accept connection if both sockets found
            if (outputSocket && inputSocket)
            {
                if (ed::AcceptNewItem())
                {
                    m_NodeGraph->connect(outputSocket, inputSocket);
                }
            }
        }
    }
    ed::EndCreate();

    // Handle deletions - MUST call EndDelete() regardless of BeginDelete() result
    ed::BeginDelete();
    {
        ed::LinkId deletedLinkId;
        while (ed::QueryDeletedLink(&deletedLinkId))
        {
            if (ed::AcceptDeletedItem())
            {
                // Find and disconnect the link
                // The link ID is offset by 100000, so get the connection index
                size_t connIndex = deletedLinkId.Get() - 100000;
                if (connIndex < m_Connections.size())
                {
                    // Find the input socket and disconnect it
                    uint64_t inputPinId = m_Connections[connIndex].inputPinId;

                    for (const auto& node : m_NodeGraph->getNodes())
                    {
                        for (auto& input : node->getInputs())
                        {
                            if (GetPinId(node->getId(), input.name, true) == inputPinId)
                            {
                                m_NodeGraph->disconnect(&input);
                                break;
                            }
                        }
                    }
                }
            }
        }
    }
    ed::EndDelete();
}
