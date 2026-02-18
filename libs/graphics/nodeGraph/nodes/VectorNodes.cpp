#include "VectorNodes.h"

// CombineVec3Node implementation
CombineVec3Node::CombineVec3Node() {
    addInput("X", SocketType::Float, 0.0f);
    addInput("Y", SocketType::Float, 0.0f);
    addInput("Z", SocketType::Float, 0.0f);
    addOutput("Vector", SocketType::Vec3, glm::vec3(0.0f));
}

void CombineVec3Node::evaluate() {

    float x = std::get<float>(getInput("X")->getValue());
    float y = std::get<float>(getInput("Y")->getValue());
    float z = std::get<float>(getInput("Z")->getValue());

    glm::vec3 result(x, y, z);
    getOutput("Vector")->setValue(result);
}

// SeparateVec3Node implementation
SeparateVec3Node::SeparateVec3Node() {
    addInput("Vector", SocketType::Vec3, glm::vec3(0.0f));
    addOutput("X", SocketType::Float, 0.0f);
    addOutput("Y", SocketType::Float, 0.0f);
    addOutput("Z", SocketType::Float, 0.0f);
}

void SeparateVec3Node::evaluate() {

    glm::vec3 vec = std::get<glm::vec3>(getInput("Vector")->getValue());

    getOutput("X")->setValue(vec.x);
    getOutput("Y")->setValue(vec.y);
    getOutput("Z")->setValue(vec.z);
}

// DotProductNode implementation
DotProductNode::DotProductNode() {
    addInput("A", SocketType::Vec3, glm::vec3(0.0f));
    addInput("B", SocketType::Vec3, glm::vec3(0.0f));
    addOutput("Result", SocketType::Float, 0.0f);
}

void DotProductNode::evaluate() {

    glm::vec3 a = std::get<glm::vec3>(getInput("A")->getValue());
    glm::vec3 b = std::get<glm::vec3>(getInput("B")->getValue());

    float result = glm::dot(a, b);
    getOutput("Result")->setValue(result);
}

// CrossProductNode implementation
CrossProductNode::CrossProductNode() {
    addInput("A", SocketType::Vec3, glm::vec3(0.0f));
    addInput("B", SocketType::Vec3, glm::vec3(0.0f));
    addOutput("Result", SocketType::Vec3, glm::vec3(0.0f));
}

void CrossProductNode::evaluate() {

    glm::vec3 a = std::get<glm::vec3>(getInput("A")->getValue());
    glm::vec3 b = std::get<glm::vec3>(getInput("B")->getValue());

    glm::vec3 result = glm::cross(a, b);
    getOutput("Result")->setValue(result);
}

// LengthNode implementation
LengthNode::LengthNode() {
    addInput("Vector", SocketType::Vec3, glm::vec3(0.0f));
    addOutput("Length", SocketType::Float, 0.0f);
}

void LengthNode::evaluate() {

    glm::vec3 vec = std::get<glm::vec3>(getInput("Vector")->getValue());

    float result = glm::length(vec);
    getOutput("Length")->setValue(result);
}

namespace {
    const bool s_vectorNodes_registered = []() {
        NodeGraph::RegisterNodeType("Combine Vec3",  []() { return std::make_unique<CombineVec3Node>(); });
        NodeGraph::RegisterNodeType("Separate Vec3", []() { return std::make_unique<SeparateVec3Node>(); });
        NodeGraph::RegisterNodeType("Dot Product",   []() { return std::make_unique<DotProductNode>(); });
        NodeGraph::RegisterNodeType("Cross Product",  []() { return std::make_unique<CrossProductNode>(); });
        NodeGraph::RegisterNodeType("Length",        []() { return std::make_unique<LengthNode>(); });
        return true;
    }();
}
