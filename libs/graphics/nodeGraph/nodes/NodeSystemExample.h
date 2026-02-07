#pragma once

/**
 * Node System Usage Examples
 *
 * This file demonstrates how to use the node-based evaluation system
 * to control GameObject properties through visual node graphs.
 */

#include "NodeSystem.h"
#include "NodePropertyBinding.h"
#include "ValueNodes.h"
#include "MathNodes.h"
#include "VectorNodes.h"

/**
 * Example 1: Oscillating Y Position
 *
 * Creates a simple sine wave animation for a GameObject's Y position.
 *
 * Node Graph:
 *   [Time] -> [Multiply x5] -> [Sin] -> [CombineVec3(Y)] -> GameObject.Position
 */
inline void ExampleOscillatingPosition(NodeGraph& graph, NodePropertyBinding& binding, uint64_t objectId) {
    // Create nodes
    auto* timeNode = graph.createNode<TimeNode>();
    auto* multiplyNode = graph.createNode<MultiplyNode>();
    auto* sinNode = graph.createNode<SinNode>();
    auto* combineVec3 = graph.createNode<CombineVec3Node>();
    auto* speedConstant = graph.createNode<FloatConstantNode>(5.0f);  // Speed multiplier

    // Connect nodes: Time -> Multiply(speed=5) -> Sin -> Vec3.Y
    graph.connect(timeNode->getOutput("Time"), multiplyNode->getInput("A"));
    graph.connect(speedConstant->getOutput("Value"), multiplyNode->getInput("B"));
    graph.connect(multiplyNode->getOutput("Result"), sinNode->getInput("Value"));
    graph.connect(sinNode->getOutput("Result"), combineVec3->getInput("Y"));

    // Bind GameObject position to the vec3 output
    binding.bind(objectId, NodePropertyBinding::PropertyType::Position, combineVec3->getOutput("Vector"));
}

/**
 * Example 2: Circular Motion
 *
 * Creates circular motion in the XZ plane using sin/cos.
 *
 * Node Graph:
 *   [Time] -> [Multiply x2] ----+--> [Sin] -> [CombineVec3(X)]
 *                                |                    |
 *                                +--> [Cos] ----------+-> GameObject.Position
 */
inline void ExampleCircularMotion(NodeGraph& graph, NodePropertyBinding& binding, uint64_t objectId, float radius = 2.0f) {
    // Create nodes
    auto* timeNode = graph.createNode<TimeNode>();
    auto* speedNode = graph.createNode<FloatConstantNode>(2.0f);
    auto* radiusNode = graph.createNode<FloatConstantNode>(radius);
    auto* multiplyTime = graph.createNode<MultiplyNode>();
    auto* sinNode = graph.createNode<SinNode>();
    auto* cosNode = graph.createNode<CosNode>();
    auto* multiplyX = graph.createNode<MultiplyNode>();
    auto* multiplyZ = graph.createNode<MultiplyNode>();
    auto* combineVec3 = graph.createNode<CombineVec3Node>();

    // Connect: Time * Speed -> Sin/Cos
    graph.connect(timeNode->getOutput("Time"), multiplyTime->getInput("A"));
    graph.connect(speedNode->getOutput("Value"), multiplyTime->getInput("B"));
    graph.connect(multiplyTime->getOutput("Result"), sinNode->getInput("Value"));
    graph.connect(multiplyTime->getOutput("Result"), cosNode->getInput("Value"));

    // Scale by radius
    graph.connect(sinNode->getOutput("Result"), multiplyX->getInput("A"));
    graph.connect(radiusNode->getOutput("Value"), multiplyX->getInput("B"));
    graph.connect(cosNode->getOutput("Result"), multiplyZ->getInput("A"));
    graph.connect(radiusNode->getOutput("Value"), multiplyZ->getInput("B"));

    // Combine into vec3
    graph.connect(multiplyX->getOutput("Result"), combineVec3->getInput("X"));
    graph.connect(multiplyZ->getOutput("Result"), combineVec3->getInput("Z"));

    // Bind to position
    binding.bind(objectId, NodePropertyBinding::PropertyType::Position, combineVec3->getOutput("Vector"));
}

/**
 * Example 3: Pulsating Scale
 *
 * Creates a pulsating scale effect using abs(sin(time)).
 *
 * Node Graph:
 *   [Time] -> [Multiply x4] -> [Sin] -> [Add +1.5] -> [Float to Vec3] -> GameObject.Scale
 */
inline void ExamplePulsatingScale(NodeGraph& graph, NodePropertyBinding& binding, uint64_t objectId) {
    // Create nodes
    auto* timeNode = graph.createNode<TimeNode>();
    auto* speedNode = graph.createNode<FloatConstantNode>(4.0f);
    auto* multiplyNode = graph.createNode<MultiplyNode>();
    auto* sinNode = graph.createNode<SinNode>();
    auto* addNode = graph.createNode<AddNode>();
    auto* offsetNode = graph.createNode<FloatConstantNode>(1.5f);  // Base scale
    auto* vec3Node = graph.createNode<Vec3ConstantNode>(glm::vec3(1.0f));

    // Connect: Time * Speed -> Sin -> Add offset
    graph.connect(timeNode->getOutput("Time"), multiplyNode->getInput("A"));
    graph.connect(speedNode->getOutput("Value"), multiplyNode->getInput("B"));
    graph.connect(multiplyNode->getOutput("Result"), sinNode->getInput("Value"));
    graph.connect(sinNode->getOutput("Result"), addNode->getInput("A"));
    graph.connect(offsetNode->getOutput("Value"), addNode->getInput("B"));

    // Note: The Add result (float) will be auto-converted to Vec3 (splatted)
    // when connected to Vec3 scale property via type conversion system

    // For uniform scale, we can use a multiply node with vec3
    auto* scaleMultiply = graph.createNode<MultiplyNode>();
    graph.connect(vec3Node->getOutput("Value"), scaleMultiply->getInput("A"));
    graph.connect(addNode->getOutput("Result"), scaleMultiply->getInput("B"));

    // Bind to scale
    binding.bind(objectId, NodePropertyBinding::PropertyType::Scale, scaleMultiply->getOutput("Result"));
}

/**
 * Example 4: Spiral Motion
 *
 * Combines circular motion with vertical movement.
 */
inline void ExampleSpiralMotion(NodeGraph& graph, NodePropertyBinding& binding, uint64_t objectId) {
    // Create nodes
    auto* timeNode = graph.createNode<TimeNode>();
    auto* speedNode = graph.createNode<FloatConstantNode>(1.5f);
    auto* radiusNode = graph.createNode<FloatConstantNode>(3.0f);
    auto* verticalSpeedNode = graph.createNode<FloatConstantNode>(0.5f);

    auto* multiplyTime = graph.createNode<MultiplyNode>();
    auto* multiplyVertical = graph.createNode<MultiplyNode>();
    auto* sinNode = graph.createNode<SinNode>();
    auto* cosNode = graph.createNode<CosNode>();
    auto* multiplyX = graph.createNode<MultiplyNode>();
    auto* multiplyZ = graph.createNode<MultiplyNode>();
    auto* combineVec3 = graph.createNode<CombineVec3Node>();

    // Horizontal circular motion (X, Z)
    graph.connect(timeNode->getOutput("Time"), multiplyTime->getInput("A"));
    graph.connect(speedNode->getOutput("Value"), multiplyTime->getInput("B"));
    graph.connect(multiplyTime->getOutput("Result"), sinNode->getInput("Value"));
    graph.connect(multiplyTime->getOutput("Result"), cosNode->getInput("Value"));
    graph.connect(sinNode->getOutput("Result"), multiplyX->getInput("A"));
    graph.connect(radiusNode->getOutput("Value"), multiplyX->getInput("B"));
    graph.connect(cosNode->getOutput("Result"), multiplyZ->getInput("A"));
    graph.connect(radiusNode->getOutput("Value"), multiplyZ->getInput("B"));

    // Vertical motion (Y)
    graph.connect(timeNode->getOutput("Time"), multiplyVertical->getInput("A"));
    graph.connect(verticalSpeedNode->getOutput("Value"), multiplyVertical->getInput("B"));

    // Combine
    graph.connect(multiplyX->getOutput("Result"), combineVec3->getInput("X"));
    graph.connect(multiplyVertical->getOutput("Result"), combineVec3->getInput("Y"));
    graph.connect(multiplyZ->getOutput("Result"), combineVec3->getInput("Z"));

    // Bind
    binding.bind(objectId, NodePropertyBinding::PropertyType::Position, combineVec3->getOutput("Vector"));
}

/**
 * Example 5: Manual constant position
 *
 * Simple static position using a Vec3 constant.
 */
inline void ExampleStaticPosition(NodeGraph& graph, NodePropertyBinding& binding, uint64_t objectId, const glm::vec3& position) {
    auto* vec3Node = graph.createNode<Vec3ConstantNode>(position);
    binding.bind(objectId, NodePropertyBinding::PropertyType::Position, vec3Node->getOutput("Value"));
}

/**
 * How to use these examples in your code:
 *
 * // In PsiNodeEditorLayer or similar:
 * NodeGraph& graph = getNodeGraph();
 * NodePropertyBinding& binding = getPropertyBinding();
 *
 * // Get a GameObject ID
 * uint64_t objectId = scene.objects[0].id;
 *
 * // Apply an example
 * ExampleOscillatingPosition(graph, binding, objectId);
 *
 * // Each frame in OnUpdate, the system will:
 * // 1. Mark all nodes dirty
 * // 2. Evaluate bound node outputs
 * // 3. Apply values to GameObject properties
 */
