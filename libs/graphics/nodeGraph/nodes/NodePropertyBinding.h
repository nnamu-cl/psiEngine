#pragma once

#include "NodeSystem.h"
#include "Layers/DefaultGameWorld/Scene.h"
#include "Components/Transform.h"
#include <unordered_map>

/**
 * NodePropertyBinding - System for binding GameObject properties to node outputs
 *
 * Allows GameObjects to have their properties (position, rotation, scale) controlled
 * by node graph outputs. Each frame, bound properties are automatically updated
 * from their connected node outputs.
 */
class NodePropertyBinding {
public:
    enum class PropertyType {
        Position,
        Rotation,
        Scale,
        // Add more as needed
    };

    struct Binding {
        uint64_t objectId;      // GameObject ID
        PropertyType property;  // Which property to bind
        OutputSocket* output;   // Node output to read from
    };

    NodePropertyBinding(Scene* scene) : m_Scene(scene) {}

    // Bind a GameObject property to a node output
    void bind(uint64_t objectId, PropertyType property, OutputSocket* output);

    // Unbind a GameObject property
    void unbind(uint64_t objectId, PropertyType property);

    // Update all bound properties from their connected nodes
    void updateAll();

    // Check if a property is bound
    bool isBound(uint64_t objectId, PropertyType property) const;

    // Get the binding for a property
    OutputSocket* getBinding(uint64_t objectId, PropertyType property) const;

private:
    Scene* m_Scene;
    std::vector<Binding> m_Bindings;

    // Helper to find a GameObject by ID
    GameObject* findGameObject(uint64_t objectId);

    // Helper to get a unique key for an object property
    uint64_t getBindingKey(uint64_t objectId, PropertyType property) const;
};

/**
 * Example usage:
 *
 * // Create nodes
 * auto* timeNode = nodeGraph.createNode<TimeNode>();
 * auto* sinNode = nodeGraph.createNode<SinNode>();
 * auto* vec3Node = nodeGraph.createNode<CombineVec3Node>();
 *
 * // Connect: Time -> Sin -> Vec3.Y
 * nodeGraph.connect(timeNode->getOutput("Time"), sinNode->getInput("Value"));
 * nodeGraph.connect(sinNode->getOutput("Result"), vec3Node->getInput("Y"));
 *
 * // Bind GameObject position to vec3 output
 * binding.bind(gameObject.id, NodePropertyBinding::PropertyType::Position, vec3Node->getOutput("Vector"));
 *
 * // Each frame, update bindings to apply node values to GameObjects
 * binding.updateAll();
 */
