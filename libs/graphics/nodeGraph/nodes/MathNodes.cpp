#include "MathNodes.h"
#include <cmath>

// Helper template for binary operations that work with both float and vec3
template<typename Op>
NodeValue binaryOp(const NodeValue& a, const NodeValue& b, Op op) {
    // Both int
    if (std::holds_alternative<int>(a) && std::holds_alternative<int>(b)) {
        return op(std::get<int>(a), std::get<int>(b));
    }
    // Int and float
    else if (std::holds_alternative<int>(a) && std::holds_alternative<float>(b)) {
        return op(static_cast<float>(std::get<int>(a)), std::get<float>(b));
    }
    // Float and int
    else if (std::holds_alternative<float>(a) && std::holds_alternative<int>(b)) {
        return op(std::get<float>(a), static_cast<float>(std::get<int>(b)));
    }
    // Both float
    else if (std::holds_alternative<float>(a) && std::holds_alternative<float>(b)) {
        return op(std::get<float>(a), std::get<float>(b));
    }
    // Both vec3
    else if (std::holds_alternative<glm::vec3>(a) && std::holds_alternative<glm::vec3>(b)) {
        return op(std::get<glm::vec3>(a), std::get<glm::vec3>(b));
    }
    // Vec3 and float (scalar operation)
    else if (std::holds_alternative<glm::vec3>(a) && std::holds_alternative<float>(b)) {
        glm::vec3 vec = std::get<glm::vec3>(a);
        float scalar = std::get<float>(b);
        return glm::vec3(op(vec.x, scalar), op(vec.y, scalar), op(vec.z, scalar));
    }
    // Float and vec3 (scalar operation)
    else if (std::holds_alternative<float>(a) && std::holds_alternative<glm::vec3>(b)) {
        float scalar = std::get<float>(a);
        glm::vec3 vec = std::get<glm::vec3>(b);
        return glm::vec3(op(scalar, vec.x), op(scalar, vec.y), op(scalar, vec.z));
    }
    // Vec3 and int (scalar operation)
    else if (std::holds_alternative<glm::vec3>(a) && std::holds_alternative<int>(b)) {
        glm::vec3 vec = std::get<glm::vec3>(a);
        float scalar = static_cast<float>(std::get<int>(b));
        return glm::vec3(op(vec.x, scalar), op(vec.y, scalar), op(vec.z, scalar));
    }
    // Int and vec3 (scalar operation)
    else if (std::holds_alternative<int>(a) && std::holds_alternative<glm::vec3>(b)) {
        float scalar = static_cast<float>(std::get<int>(a));
        glm::vec3 vec = std::get<glm::vec3>(b);
        return glm::vec3(op(scalar, vec.x), op(scalar, vec.y), op(scalar, vec.z));
    }

    // Default: return 0
    return 0.0f;
}

// AddNode implementation
AddNode::AddNode() {
    addInput("A", SocketType::Any, 0.0f);
    addInput("B", SocketType::Any, 0.0f);
    addOutput("Result", SocketType::Any, 0.0f);
}

void AddNode::evaluate() {
    if (!isDirty()) return;

    NodeValue a = getInput("A")->getValue();
    NodeValue b = getInput("B")->getValue();

    NodeValue result = binaryOp(a, b, [](auto x, auto y) { return x + y; });

    getOutput("Result")->setValue(result);
    markClean();
}

// SubtractNode implementation
SubtractNode::SubtractNode() {
    addInput("A", SocketType::Any, 0.0f);
    addInput("B", SocketType::Any, 0.0f);
    addOutput("Result", SocketType::Any, 0.0f);
}

void SubtractNode::evaluate() {
    if (!isDirty()) return;

    NodeValue a = getInput("A")->getValue();
    NodeValue b = getInput("B")->getValue();

    NodeValue result = binaryOp(a, b, [](auto x, auto y) { return x - y; });

    getOutput("Result")->setValue(result);
    markClean();
}

// MultiplyNode implementation
MultiplyNode::MultiplyNode() {
    addInput("A", SocketType::Any, 1.0f);
    addInput("B", SocketType::Any, 1.0f);
    addOutput("Result", SocketType::Any, 1.0f);
}

void MultiplyNode::evaluate() {
    if (!isDirty()) return;

    NodeValue a = getInput("A")->getValue();
    NodeValue b = getInput("B")->getValue();

    NodeValue result = binaryOp(a, b, [](auto x, auto y) { return x * y; });

    getOutput("Result")->setValue(result);
    markClean();
}

// DivideNode implementation
DivideNode::DivideNode() {
    addInput("A", SocketType::Any, 1.0f);
    addInput("B", SocketType::Any, 1.0f);
    addOutput("Result", SocketType::Any, 1.0f);
}

void DivideNode::evaluate() {
    if (!isDirty()) return;

    NodeValue a = getInput("A")->getValue();
    NodeValue b = getInput("B")->getValue();

    NodeValue result = binaryOp(a, b, [](auto x, auto y) {
        // Avoid division by zero
        if constexpr (std::is_same_v<decltype(y), float>) {
            return (y != 0.0f) ? x / y : 0.0f;
        } else {
            return x / y;
        }
    });

    getOutput("Result")->setValue(result);
    markClean();
}

// SinNode implementation
SinNode::SinNode() {
    addInput("Value", SocketType::Float, 0.0f);
    addOutput("Result", SocketType::Float, 0.0f);
}

void SinNode::evaluate() {
    if (!isDirty()) return;

    float value = std::get<float>(getInput("Value")->getValue());
    float result = std::sin(value);

    getOutput("Result")->setValue(result);
    markClean();
}

// CosNode implementation
CosNode::CosNode() {
    addInput("Value", SocketType::Float, 0.0f);
    addOutput("Result", SocketType::Float, 0.0f);
}

void CosNode::evaluate() {
    if (!isDirty()) return;

    float value = std::get<float>(getInput("Value")->getValue());
    float result = std::cos(value);

    getOutput("Result")->setValue(result);
    markClean();
}
