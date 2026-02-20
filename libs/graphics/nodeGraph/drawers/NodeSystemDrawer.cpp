#include "NodeSystemDrawer.h"

#include <iostream>

#include "ApplicationWindow.h"
#include "NodeEditorIcons.h"
#include "NodeEditorPinTypes.h"
#include "NodeEditorPinDrawing.h"
#include "imgui.h"
#include "imgui_internal.h"
#include "imgui_node_editor.h"
#include "IconsLucide.h"

namespace ed = ax::NodeEditor;


NodeSystemDrawer::NodeSystemDrawer(NodeGraph* nodeGraph)
    : m_NodeGraph(nodeGraph)
{
    m_Config.SettingsFile = nullptr;
    m_NodeEditorContext = ed::CreateEditor(&m_Config);
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
                   ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus ;

    ImGui::SetNextWindowBgAlpha(0.0f); // Fully transparent window background
    ImGui::Begin("Node Editor", &Open, windowFlags);

    ed::SetCurrentEditor(m_NodeEditorContext);

    // Set background and grid to fully transparent
    ax::NodeEditor::Style &style = ed::GetStyle();
    style.Colors[ed::StyleColor_Bg] = ImVec4(0, 0, 0, 0);
    style.Colors[ed::StyleColor_Grid] = ImVec4(0, 0, 0, 0);
    style.NodeBorderWidth = 0.0f;
    style.NodeRounding = 2.5f;

    ed::Begin("My Editor");

    DrawAllNodes();
    DrawLinks();
    HandleInteractions();

    // After Begin() has fired LoadSettings() internally, restore all node states from file
    if (m_NeedsRestore && m_NodeGraph)
    {
        for (const auto& node : m_NodeGraph->getNodes())
            ed::RestoreNodeState(node->getId());
        m_NeedsRestore = false;
    }

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


enum class IconPosition { Left, Right };


// Renders an icon with optional text, both vertically centered.
// Uses DrawList so the two fonts never fight over cursor position.
static void IconText(const char* icon, IconPosition pos = IconPosition::Left, const char* text = nullptr) {
    const bool  hasText = text && text[0] != '\0';
    const float iconSz  = ImGui::GetFontSize();
    const float textSz  = ImGui::GetFontSize();
    const float lineH   = ImMax(iconSz, textSz);
    const float spacing = hasText ? ImGui::GetStyle().ItemSpacing.x : 0.0f;
    const float textW   = hasText ? ImGui::CalcTextSize(text).x : 0.0f;
    const float totalW  = iconSz + (hasText ? spacing + textW : 0.0f);

    ImGui::Dummy(ImVec2(totalW, lineH));
    ImVec2      origin = ImGui::GetItemRectMin();
    ImDrawList* dl     = ImGui::GetWindowDrawList();
    ImU32       col    = ImGui::GetColorU32(ImGuiCol_Text);

    const float iconX = (pos == IconPosition::Left) ? origin.x : origin.x + textW + spacing;
    const float textX = (pos == IconPosition::Left) ? origin.x + iconSz + spacing : origin.x;
    const float iconY = origin.y + (lineH - iconSz) * 0.5f;
    const float textY = origin.y + (lineH - textSz) * 0.5f;

    ApplicationWindow::PushIconFont();
    dl->AddText(ApplicationWindow::iconFont, iconSz, ImVec2(iconX, iconY), col, icon);
    ApplicationWindow::PopIconFont();

    if (hasText)
        dl->AddText(ImGui::GetFont(), textSz, ImVec2(textX, textY), col, text);
}




void NodeSystemDrawer::DrawNodeInternal(Node* node, int arrayIndex)
{
    if (!node) return;

    uint64_t nodeId = node->getId();

    // // Set initial position for this node if not already positioned
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

    // Layout: Input pins on left, custom UI in middle, output pins on right
    ImGui::BeginGroup();

    {


        ImGui::PushFont(nullptr, 24.0f);
        IconText(toIconGlyph(node->getIcon()), IconPosition::Left);
        ImGui::PopFont();

        // Node name input field
        ImGui::PushID(node);
        ImGui::SameLine();

        ImGui::PushItemWidth(100.0f);

        // Get or create buffer for this node
        std::string& nameBuffer = m_NodeNameBuffers[nodeId];

        // Sync buffer with node's current name if it differs
        if (nameBuffer != node->getName())
        {
            nameBuffer = node->getName();
        }

        // Resize buffer to accommodate input (256 chars max)
        nameBuffer.resize(256);

        if (ImGui::InputText("##nodename", nameBuffer.data(), nameBuffer.capacity()))
        {
            // Trim to actual string length and update node
            nameBuffer.resize(strlen(nameBuffer.c_str()));
            node->setName(nameBuffer);
        }

        ImGui::PopItemWidth();
        ImGui::PopID();
    }
    ImGui::EndGroup();



    // Layout: Input pins on left, custom UI in middle, output pins on right
    ImGui::BeginGroup();

    {

        DrawInputSockets(node);
    }
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
        ed::PinPivotAlignment(ImVec2(0.0f, 0.5f));
        ed::PinPivotSize(ImVec2(0, 0));
        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 1);

        DrawInputSocketPin(&input, pinId);

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

            DrawOutputSocketPin(&output, pinId);

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


// Loading and saving here doesn't work yet
// We shall fix this to make it work later on
// It has been put off for later for now
//TODO: Make the load and save functions here work
void NodeSystemDrawer::Save(const std::string& filePath)
{
    // Collect current node positions before destroying the context
    std::unordered_map<uint64_t, ImVec2> positions;
    if (m_NodeGraph)
    {
        ed::SetCurrentEditor(m_NodeEditorContext);
        for (const auto& node : m_NodeGraph->getNodes())
        {
            uint64_t id = node->getId();
            positions[id] = ed::GetNodePosition(id);
        }
        ed::SetCurrentEditor(nullptr);
    }

    // Recreate context pointing at the project settings file
    ed::DestroyEditor(m_NodeEditorContext);
    m_SettingsFilePath = filePath;
    m_Config.SettingsFile = m_SettingsFilePath.c_str();
    m_NodeEditorContext = ed::CreateEditor(&m_Config);

    // Push positions into the new context so they're dirty and written on next End()
    ed::SetCurrentEditor(m_NodeEditorContext);
    for (const auto& [id, pos] : positions)
    {
        ed::SetNodePosition(id, pos);
        m_PositionedNodes[id] = true;
    }
    ed::SetCurrentEditor(nullptr);
}

void NodeSystemDrawer::Load(const std::string& filePath)
{
    // Recreate context pointing at the project settings file.
    // The first Begin() call will trigger LoadSettings() internally.
    ed::DestroyEditor(m_NodeEditorContext);
    std::cout << "Loading in file path for node system: " << filePath << std::endl;
    m_SettingsFilePath = filePath;
    m_Config.SettingsFile = m_SettingsFilePath.c_str();
    m_NodeEditorContext = ed::CreateEditor(&m_Config);

    m_PositionedNodes.clear();
    //Mark all loaded nodes as already positioned to avoid position loss
    if (m_NodeGraph) {
        for (const std::unique_ptr<Node> &node: m_NodeGraph->getNodes()) {
            m_PositionedNodes[node->getId()] = true;
        }
    }
    m_NeedsRestore = true;
}

void NodeSystemDrawer::HandleInteractions()
{
    if (!m_NodeGraph) return;

    // Handle new connections - MUST call EndCreate() regardless of BeginCreate() result
    ed::BeginCreate();
    {
        ed::PinId inputPinId, outputPinId;
        if (ed::QueryNewLink(&outputPinId, &inputPinId))
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
        // Handle link deletion
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

        // Handle node deletion
        ed::NodeId deletedNodeId;
        while (ed::QueryDeletedNode(&deletedNodeId))
        {
            if (ed::AcceptDeletedItem())
            {
                uint64_t nodeId = deletedNodeId.Get();

                // Remove node from graph (this also disconnects all connections)
                if (m_NodeGraph->deleteNode(nodeId))
                {
                    // Clean up drawer-specific data
                    m_PositionedNodes.erase(nodeId);
                    m_NodeNameBuffers.erase(nodeId);
                }
            }
        }
    }
    ed::EndDelete();
}
