#pragma once

#include <variant>
#include <vector>
#include <optional>
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <memory>
#include <glm/glm.hpp>

// Forward declarations
class Node;
struct InputSocket;
struct OutputSocket;

// All possible data types in the node system
using NodeValue = std::variant<
    float,
    int,
    glm::vec2,
    glm::vec3,
    glm::vec4,
    glm::mat4
>;

enum class SocketType {
    Float,
    Int,
    Vec2,
    Vec3,
    Vec4,
    Mat4,
    Any  // Can accept any type
};

// Helper to get SocketType from variant index
inline SocketType getSocketType(const NodeValue& value) {
    return static_cast<SocketType>(value.index());
}

// Type conversion utilities
class TypeConverter {
public:
    // Try to convert between compatible types
    static std::optional<NodeValue> convert(const NodeValue& from, SocketType toType);

    // Check if two types are compatible (can be converted)
    static bool isCompatible(SocketType from, SocketType to);
};

// Output socket - one output can feed multiple inputs
struct OutputSocket {
    Node* owner = nullptr;
    std::string name;
    SocketType type;
    NodeValue cachedValue;
    std::vector<InputSocket*> connections;

    OutputSocket(Node* owner, const std::string& name, SocketType type, const NodeValue& defaultValue = 0.0f);

    void setValue(const NodeValue& value);
    const NodeValue& getValue() const;
};

// Input socket - can connect to one output
struct InputSocket {
    Node* owner = nullptr;
    std::string name;
    SocketType type;
    NodeValue defaultValue;
    OutputSocket* connectedOutput = nullptr;

    InputSocket(Node* owner, const std::string& name, SocketType type, const NodeValue& defaultValue);

    NodeValue getValue();
    bool isConnected() const;
    void connectTo(OutputSocket* output);
    void disconnect();
};

// Base node class
class Node {
public:
    virtual ~Node() = default;

    // Each node computes its outputs based on inputs
    virtual void evaluate() = 0;

    // Node UI rendering - override to customize node appearance
    virtual void OnDrawNodeUI() {}

    // Connection callbacks - override to handle graph changes
    virtual void OnInputConnected(InputSocket* input, OutputSocket* output) {}
    virtual void OnInputDisconnected(InputSocket* input) {}

    // Dirty flag management
    bool isDirty() const { return m_Dirty; }
    void markDirty();
    void markClean() { m_Dirty = false; }

    // Socket access
    InputSocket* getInput(const std::string& name);
    OutputSocket* getOutput(const std::string& name);

    const std::vector<InputSocket>& getInputs() const { return m_Inputs; }
    const std::vector<OutputSocket>& getOutputs() const { return m_Outputs; }

    std::vector<InputSocket>& getInputs() { return m_Inputs; }
    std::vector<OutputSocket>& getOutputs() { return m_Outputs; }

    // Unique ID for this node
    uint64_t getId() const { return m_Id; }
    void setId(uint64_t id) { m_Id = id; }

    // Node name/type
    virtual const char* getTypeName() const = 0;

protected:
    // Helper to add sockets during construction
    InputSocket& addInput(const std::string& name, SocketType type, const NodeValue& defaultValue = 0.0f);
    OutputSocket& addOutput(const std::string& name, SocketType type, const NodeValue& defaultValue = 0.0f);

private:
    std::vector<InputSocket> m_Inputs;
    std::vector<OutputSocket> m_Outputs;
    bool m_Dirty = true;
    uint64_t m_Id = 0;
};

// Node graph manager
class NodeGraph {
public:
    NodeGraph() = default;
    ~NodeGraph() = default;

    // Factory method to create nodes
    template<typename T, typename... Args>
    T* createNode(Args&&... args) {
        auto node = std::make_unique<T>(std::forward<Args>(args)...);
        T* ptr = node.get();
        ptr->setId(generateNodeId());
        m_Nodes.push_back(std::move(node));
        return ptr;
    }

    // Connect two sockets
    bool connect(OutputSocket* output, InputSocket* input);
    void disconnect(InputSocket* input);

    // Mark all nodes dirty (call this each frame)
    void markAllDirty();

    // Evaluate a specific output (will recursively evaluate dependencies)
    NodeValue evaluate(OutputSocket* output);

    // Access to all nodes
    const std::vector<std::unique_ptr<Node>>& getNodes() const { return m_Nodes; }

private:
    std::vector<std::unique_ptr<Node>> m_Nodes;
    uint64_t m_NextNodeId = 1;

    uint64_t generateNodeId() { return m_NextNodeId++; }
};
