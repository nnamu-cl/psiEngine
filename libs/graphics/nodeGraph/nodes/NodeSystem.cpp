#include "NodeSystem.h"
#include <cmath>
#include <algorithm>
#include <iostream>
#include <filesystem>
#include <fstream>
#include <glaze/glaze.hpp>

namespace fs = std::filesystem;

// ---------------------------------------------------------------------------
// Serialization data structures (used only by NodeGraph::Save / Load)
// ---------------------------------------------------------------------------

struct NodeSaveData {
    uint64_t id = 0;
    std::string type;
    std::string name;
    std::unordered_map<std::string, std::string> props;
};

struct ConnectionSaveData {
    uint64_t fromNode   = 0;
    std::string fromSocket;
    uint64_t toNode     = 0;
    std::string toSocket;
};

struct GraphSaveData {
    std::vector<NodeSaveData>       nodes;
    std::vector<ConnectionSaveData> connections;
};

namespace {

// Meyers-singleton factory map — guaranteed to be initialised before first use,
// regardless of static initialisation order across translation units.
auto& GetFactories() {
    static std::unordered_map<std::string, std::function<std::unique_ptr<Node>()>> s;
    return s;
}

} // namespace

// TypeConverter implementation
std::optional<NodeValue> TypeConverter::convert(const NodeValue& from, SocketType toType) {
    SocketType fromType = getSocketType(from);

    // No conversion needed
    if (fromType == toType) {
        return from;
    }

    // Float to Vec3 (splat)
    if (std::holds_alternative<float>(from) && toType == SocketType::Vec3) {
        float f = std::get<float>(from);
        return glm::vec3(f, f, f);
    }

    // Float to Vec2 (splat)
    if (std::holds_alternative<float>(from) && toType == SocketType::Vec2) {
        float f = std::get<float>(from);
        return glm::vec2(f, f);
    }

    // Int to Float
    if (std::holds_alternative<int>(from) && toType == SocketType::Float) {
        return static_cast<float>(std::get<int>(from));
    }

    // Vec3 to Float (magnitude)
    if (std::holds_alternative<glm::vec3>(from) && toType == SocketType::Float) {
        return glm::length(std::get<glm::vec3>(from));
    }

    // Vec2 to Vec3 (add z=0)
    if (std::holds_alternative<glm::vec2>(from) && toType == SocketType::Vec3) {
        glm::vec2 v = std::get<glm::vec2>(from);
        return glm::vec3(v.x, v.y, 0.0f);
    }

    // Vec3 to Vec2 (drop z)
    if (std::holds_alternative<glm::vec3>(from) && toType == SocketType::Vec2) {
        glm::vec3 v = std::get<glm::vec3>(from);
        return glm::vec2(v.x, v.y);
    }

    // No conversion available
    return std::nullopt;
}

bool TypeConverter::isCompatible(SocketType from, SocketType to) {
    if (from == to || to == SocketType::Any || from == SocketType::Any) return true;

    // Define compatible type pairs
    static const std::unordered_map<SocketType, std::unordered_set<SocketType>> compatibilityMap = {
        {SocketType::Int, {SocketType::Float}},
        {SocketType::Float, {SocketType::Vec2, SocketType::Vec3, SocketType::Vec4}},
        {SocketType::Vec2, {SocketType::Vec3, SocketType::Float}},
        {SocketType::Vec3, {SocketType::Vec2, SocketType::Float}},
    };

    auto it = compatibilityMap.find(from);
    if (it != compatibilityMap.end()) {
        return it->second.count(to) > 0;
    }
    return false;
}

// OutputSocket implementation
OutputSocket::OutputSocket(Node* owner, const std::string& name, SocketType type, const NodeValue& defaultValue)
    : owner(owner), name(name), type(type), cachedValue(defaultValue)
{
}

void OutputSocket::setValue(const NodeValue& value) {
    cachedValue = value;
}

const NodeValue& OutputSocket::getValue() const {
    return cachedValue;
}

// InputSocket implementation
InputSocket::InputSocket(Node* owner, const std::string& name, SocketType type, const NodeValue& defaultValue)
    : owner(owner), name(name), type(type), defaultValue(defaultValue)
{
    switch (type)
    {
        case SocketType::Any: editMode = SocketEditMode::Float; break;
        case SocketType::Float: editMode = SocketEditMode::Float; break;
        case SocketType::Int:   editMode = SocketEditMode::Int;   break;
        case SocketType::Vec2:  editMode = SocketEditMode::Vec2;  break;
        case SocketType::Vec3:  editMode = SocketEditMode::Vec3;  break;
        case SocketType::Vec4:  editMode = SocketEditMode::Vec4;  break;
        default:                editMode = SocketEditMode::None;  break; // Mat4, Any
    }
}

NodeValue InputSocket::getValue() {
    if (!isConnected()) {
        return defaultValue;
    }

    // Evaluate the connected node if it's dirty
    connectedOutput->owner->evaluate();

    // Get the value from the connected output
    NodeValue outputValue = connectedOutput->getValue();

    // Try to convert if types don't match
    if (getSocketType(outputValue) != type && type != SocketType::Any) {
        auto converted = TypeConverter::convert(outputValue, type);
        if (converted.has_value()) {
            return converted.value();
        }
    }

    return outputValue;
}

bool InputSocket::isConnected() const {
    return connectedOutput != nullptr;
}

void InputSocket::connectTo(OutputSocket* output) {
    if (connectedOutput != nullptr) {
        disconnect();
    }
    connectedOutput = output;
    if (output) {
        output->connections.push_back(this);
    }
}

void InputSocket::disconnect() {
    if (connectedOutput) {
        auto& connections = connectedOutput->connections;
        connections.erase(
            std::remove(connections.begin(), connections.end(), this),
            connections.end()
        );
        connectedOutput = nullptr;
    }
}


InputSocket* Node::getInput(const std::string& name) {
    for (auto& input : m_Inputs) {
        if (input.name == name) {
            return &input;
        }
    }
    return nullptr;
}

OutputSocket* Node::getOutput(const std::string& name) {
    for (auto& output : m_Outputs) {
        if (output.name == name) {
            return &output;
        }
    }
    return nullptr;
}

InputSocket& Node::addInput(const std::string& name, SocketType type, const NodeValue& defaultValue) {
    m_Inputs.emplace_back(this, name, type, defaultValue);
    return m_Inputs.back();
}

OutputSocket& Node::addOutput(const std::string& name, SocketType type, const NodeValue& defaultValue) {
    m_Outputs.emplace_back(this, name, type, defaultValue);
    return m_Outputs.back();
}

// ---------------------------------------------------------------------------
// NodeGraph — factory registry
// ---------------------------------------------------------------------------

void NodeGraph::RegisterNodeType(const std::string& typeName,
                                  std::function<std::unique_ptr<Node>()> factory) {
    GetFactories()[typeName] = std::move(factory);
}

std::unique_ptr<Node> NodeGraph::CreateNode(const std::string& typeName) {
    auto& factories = GetFactories();
    auto  it        = factories.find(typeName);
    if (it != factories.end()) return it->second();
    return nullptr;
}

// NodeGraph implementation
bool NodeGraph::connect(OutputSocket* output, InputSocket* input) {
    if (!output || !input) return false;

    // Check type compatibility
    if (!TypeConverter::isCompatible(output->type, input->type)) {
        return false;
    }

    input->connectTo(output);

    // Mark downstream nodes dirty
    if (input->owner) {
        // Notify the node about the new connection
        input->owner->OnInputConnected(input, output);
    }

    return true;
}

void NodeGraph::disconnect(InputSocket* input) {
    if (input) {
        // Notify before disconnecting
        if (input->owner) {
            input->owner->OnInputDisconnected(input);
        }

        input->disconnect();


    }
}

bool NodeGraph::deleteNode(uint64_t nodeId) {
    // Find the node
    auto it = std::find_if(m_Nodes.begin(), m_Nodes.end(),
        [nodeId](const std::unique_ptr<Node>& node) {
            return node->getId() == nodeId;
        });

    if (it == m_Nodes.end()) {
        return false; // Node not found
    }

    Node* nodeToDelete = it->get();

    // Disconnect all input sockets
    for (auto& input : nodeToDelete->getInputs()) {
        if (input.isConnected()) {
            disconnect(&input);
        }
    }

    // Disconnect all output sockets from their connected inputs
    for (auto& output : nodeToDelete->getOutputs()) {
        // Make a copy of connections vector since we'll be modifying it
        auto connectionsCopy = output.connections;
        for (auto* inputSocket : connectionsCopy) {
            disconnect(inputSocket);
        }
    }

    // Remove the node from the vector
    m_Nodes.erase(it);

    // Evaluate all remaining nodes in the graph after deletion
    evaluateAll();

    return true;
}


void NodeGraph::evaluateAll() {
    for (auto& node : m_Nodes) {
        node->evaluate();
    }
}

NodeValue NodeGraph::evaluate(OutputSocket* output) {
    if (!output || !output->owner) {
        return 0.0f;
    }

    output->owner->evaluate();

    return output->getValue();
}


void NodeGraph::Save(std::string directory) {
    GraphSaveData data;

    for (const auto& nodePtr : m_Nodes) {
        Node* node = nodePtr.get();

        NodeSaveData nd;
        nd.id   = node->getId();
        nd.type = node->getTypeName();
        nd.name = node->getName();
        node->SaveProperties(nd.props);
        data.nodes.push_back(std::move(nd));

        // Each connected input socket represents one directed edge in the graph
        for (const auto& input : node->getInputs()) {
            if (input.isConnected()) {
                data.connections.push_back({
                    input.connectedOutput->owner->getId(),
                    input.connectedOutput->name,
                    node->getId(),
                    input.name
                });
            }
        }
    }

    std::string buffer;
    auto err = glz::write<glz::opts{.prettify = true}>(data, buffer);
    if (err) return;

    fs::create_directories(directory);
    std::ofstream file(fs::path(directory) / "graph.json", std::ios::trunc);
    file << buffer;
}

void NodeGraph::Load(std::string filename) {
    std::ifstream file(filename);
    if (!file.is_open()) return;

    std::string buffer((std::istreambuf_iterator<char>(file)),
                        std::istreambuf_iterator<char>());

    GraphSaveData data;
    auto err = glz::read_json(data, buffer);
    if (err) return;

    m_Nodes.clear();
    m_NextNodeId = 1;

    // Phase 1 — rebuild nodes from their saved type + properties
    for (const auto& nd : data.nodes) {
        auto node = CreateNode(nd.type);
        if (!node) {
            std::cerr << "[NodeGraph::Load] Unknown node type: " << nd.type << "\n";
            continue;
        }

        node->setId(nd.id);
        node->setName(nd.name);
        node->graph = this;
        node->LoadProperties(nd.props);

        if (nd.id >= m_NextNodeId)
            m_NextNodeId = nd.id + 1;

        m_Nodes.push_back(std::move(node));
    }

    // Phase 2 — rewire connections using stable socket names
    for (const auto& conn : data.connections) {
        OutputSocket* from = nullptr;
        InputSocket*  to   = nullptr;

        for (const auto& n : m_Nodes) {
            if (n->getId() == conn.fromNode) from = n->getOutput(conn.fromSocket);
            if (n->getId() == conn.toNode)   to   = n->getInput(conn.toSocket);
        }

        if (from && to) connect(from, to);
    }

    // Phase 3 — let nodes do any post-load wiring/validation
    for (const auto& n : m_Nodes)
        n->PostNodeLoad();
}
