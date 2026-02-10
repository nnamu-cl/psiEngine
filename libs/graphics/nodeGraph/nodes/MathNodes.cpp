#include "MathNodes.h"
#include <cmath>
#include "imgui.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

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

    NodeValue a = getInput("A")->getValue();
    NodeValue b = getInput("B")->getValue();

    NodeValue result = binaryOp(a, b, [](auto x, auto y) { return x + y; });

    getOutput("Result")->setValue(result);
}

// SubtractNode implementation
SubtractNode::SubtractNode() {
    addInput("A", SocketType::Any, 0.0f);
    addInput("B", SocketType::Any, 0.0f);
    addOutput("Result", SocketType::Any, 0.0f);
}

void SubtractNode::evaluate() {

    NodeValue a = getInput("A")->getValue();
    NodeValue b = getInput("B")->getValue();

    NodeValue result = binaryOp(a, b, [](auto x, auto y) { return x - y; });

    getOutput("Result")->setValue(result);
}

// MultiplyNode implementation
MultiplyNode::MultiplyNode() {
    addInput("A", SocketType::Any, 1.0f);
    addInput("B", SocketType::Any, 1.0f);
    addOutput("Result", SocketType::Any, 1.0f);
}

void MultiplyNode::evaluate() {

    NodeValue a = getInput("A")->getValue();
    NodeValue b = getInput("B")->getValue();

    NodeValue result = binaryOp(a, b, [](auto x, auto y) { return x * y; });

    getOutput("Result")->setValue(result);
}

// DivideNode implementation
DivideNode::DivideNode() {
    addInput("A", SocketType::Any, 1.0f);
    addInput("B", SocketType::Any, 1.0f);
    addOutput("Result", SocketType::Any, 1.0f);
}

void DivideNode::evaluate() {

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
}

// SinNode implementation
SinNode::SinNode() {
    addInput("Value", SocketType::Float, 0.0f);
    addOutput("Result", SocketType::Float, 0.0f);
}

void SinNode::evaluate() {

    float value = std::get<float>(getInput("Value")->getValue());
    float result = std::sin(value);

    getOutput("Result")->setValue(result);
}

// CosNode implementation
CosNode::CosNode() {
    addInput("Value", SocketType::Float, 0.0f);
    addOutput("Result", SocketType::Float, 0.0f);
}

void CosNode::evaluate() {

    float value = std::get<float>(getInput("Value")->getValue());
    float result = std::cos(value);

    getOutput("Result")->setValue(result);
}

// TanNode implementation
TanNode::TanNode() {
    addInput("Value", SocketType::Float, 0.0f);
    addOutput("Result", SocketType::Float, 0.0f);
}

void TanNode::evaluate() {

    float value = std::get<float>(getInput("Value")->getValue());
    float result = std::tan(value);

    getOutput("Result")->setValue(result);
}

// ArctanNode implementation
ArctanNode::ArctanNode() {
    addInput("Value", SocketType::Float, 0.0f);
    addOutput("Result", SocketType::Float, 0.0f);
}

void ArctanNode::evaluate() {

    float value = std::get<float>(getInput("Value")->getValue());
    float result = std::atan(value);

    getOutput("Result")->setValue(result);
}

// ArcsinNode implementation
ArcsinNode::ArcsinNode() {
    addInput("Value", SocketType::Float, 0.0f);
    addOutput("Result", SocketType::Float, 0.0f);
}

void ArcsinNode::evaluate() {

    float value = std::get<float>(getInput("Value")->getValue());
    // Clamp value to [-1, 1] to avoid NaN
    value = std::max(-1.0f, std::min(1.0f, value));
    float result = std::asin(value);

    getOutput("Result")->setValue(result);
}

// ArccosNode implementation
ArccosNode::ArccosNode() {
    addInput("Value", SocketType::Float, 0.0f);
    addOutput("Result", SocketType::Float, 0.0f);
}

void ArccosNode::evaluate() {

    float value = std::get<float>(getInput("Value")->getValue());
    // Clamp value to [-1, 1] to avoid NaN
    value = std::max(-1.0f, std::min(1.0f, value));
    float result = std::acos(value);

    getOutput("Result")->setValue(result);
}

// PowNode implementation
PowNode::PowNode() {
    addInput("Base", SocketType::Float, 1.0f);
    addInput("Exponent", SocketType::Float, 2.0f);
    addOutput("Result", SocketType::Float, 1.0f);
}

void PowNode::evaluate() {

    float base = std::get<float>(getInput("Base")->getValue());
    float exponent = std::get<float>(getInput("Exponent")->getValue());
    float result = std::pow(base, exponent);

    getOutput("Result")->setValue(result);
}

// RootNode implementation
RootNode::RootNode() {
    addInput("Value", SocketType::Float, 0.0f);
    addOutput("Result", SocketType::Float, 0.0f);
}

void RootNode::evaluate() {

    float value = std::get<float>(getInput("Value")->getValue());
    // Clamp to non-negative to avoid NaN
    value = std::max(0.0f, value);
    float result = std::sqrt(value);

    getOutput("Result")->setValue(result);
}

// PINode implementation
PINode::PINode() {
    addOutput("Value", SocketType::Float, static_cast<float>(M_PI));
}

void PINode::evaluate() {

    getOutput("Value")->setValue(static_cast<float>(M_PI));
}

void PINode::OnDrawNodeUI() {
    ImGui::PushID(this);
    ImGui::Text("3.14159...");
    ImGui::PopID();
}
