#pragma once
#include "ecs/IComponent.h"
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>
#include <glm/gtc/quaternion.hpp>

// Forward declarations
class TransformNode;
class NodeGraph;

class Transform : public IComponent
{
public:
    COMPONENT_TYPE_ID(Transform)

    glm::vec3 position{0.0f, 0.0f, 0.0f};
    glm::quat rotation{1.0f, 0.0f, 0.0f, 0.0f};  // Identity quaternion
    glm::vec3 scale{1.0f, 1.0f, 1.0f};

    Transform() = default;
    Transform(const glm::vec3& pos, const glm::quat& rot = glm::quat(1.0f, 0.0f, 0.0f, 0.0f), const glm::vec3& scl = glm::vec3(1.0f))
        : position(pos), rotation(rot), scale(scl)
    {}

    // Convert to model matrix
    glm::mat4 toMatrix() const;

    // Getters that return node values if linked, otherwise component values
    glm::vec3 getPos() const;
    glm::vec3 getRot() const;  // Returns euler angles in degrees
    glm::vec3 getScale() const;

    // Node linking
    void linkToNode(TransformNode* node);
    void clearNodeLink();
    TransformNode* getLinkedNode() const { return m_LinkedNode; }

    std::unique_ptr<IComponent> clone() const override
    {
        return std::make_unique<Transform>(*this);
    }

    // Render inspector UI for this component
    void OnInspectorGUI() override;
    void OnInspectorGUI(NodeGraph* nodeGraph);  // Overload with NodeGraph access

private:
    TransformNode* m_LinkedNode = nullptr;
};
