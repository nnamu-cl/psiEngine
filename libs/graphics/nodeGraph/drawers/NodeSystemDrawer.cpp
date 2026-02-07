#include "NodeSystemDrawer.h"
#include "NodeEditorIcons.h"
#include "NodeEditorPinTypes.h"
#include "NodeEditorPinDrawing.h"
#include "imgui.h"
#include "imgui_node_editor.h"
#include "imgui_node_editor_internal.h"

namespace ed = ax::NodeEditor;


NodeSystemDrawer::NodeSystemDrawer(NodeGraph* nodeGraph)
    : m_NodeGraph(nodeGraph)
{
    // Create node editor context
    ed::Config config;
    m_NodeEditorContext = ed::CreateEditor(&config);
}

NodeSystemDrawer::~NodeSystemDrawer()
{
    // Destroy node editor context
    if (m_NodeEditorContext)
    {
        ed::DestroyEditor(m_NodeEditorContext);
        m_NodeEditorContext = nullptr;
    }
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

void NodeSystemDrawer::DrawAllNodes()
{
    if (!m_NodeGraph) return;

    int nodeCount = m_NodeGraph->getNodes().size();
    for (int i = 0; i < nodeCount; ++i)
    {
        DrawNode(i);
    }
}

void NodeSystemDrawer::DrawNodeGraph()
{
    ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;

    // Get main viewport
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);

    windowFlags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
                   ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                   ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

    ImGui::SetNextWindowBgAlpha(0.0f); // Fully transparent window background
    ImGui::Begin("Node Editor", &Open, windowFlags);

    ed::SetCurrentEditor(m_NodeEditorContext);
    ed::Begin("My Editor");

    DrawAllNodes();
    DrawLinks();
    HandleInteractions();

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
        ed::PinPivotAlignment(ImVec2(1.0f, 0.5f));
        ed::PinPivotSize(ImVec2(0, 0));
        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 1);

        ImGui::Text(input.name.c_str());  // Value name
        Pin pin {static_cast<int> (pinId), input.name.c_str(), PinType::Object};
        DrawPinIcon(pin, false, (int)(1 * 255));
        ImGui::PopStyleVar();
        ed::EndPin();


    }
}

void NodeSystemDrawer::DrawOutputSockets(Node* node)
{
    for (const auto& output : node->getOutputs())
    {
        uint64_t pinId = GetPinId(node->getId(), output.name, false);



        ed::BeginPin(pinId, ed::PinKind::Output);
            ed::PinPivotAlignment(ImVec2(1.0f, 0.5f));
            ed::PinPivotSize(ImVec2(0, 0));
            ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 1);

            ImGui::Text(output.name.c_str());  // Value name
            Pin pin {static_cast<int> (pinId), output.name.c_str(), PinType::Float};
            DrawPinIcon(pin, false, (int)(1 * 255));
            ImGui::PopStyleVar();
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
