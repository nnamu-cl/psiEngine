#include "NodePropertyBinding.h"
#include <algorithm>

void NodePropertyBinding::bind(uint64_t objectId, PropertyType property, OutputSocket* output) {
    if (!output) return;

    // Check if binding already exists, update it
    for (auto& binding : m_Bindings) {
        if (binding.objectId == objectId && binding.property == property) {
            binding.output = output;
            return;
        }
    }

    // Create new binding
    m_Bindings.push_back({objectId, property, output});
}

void NodePropertyBinding::unbind(uint64_t objectId, PropertyType property) {
    m_Bindings.erase(
        std::remove_if(m_Bindings.begin(), m_Bindings.end(),
            [objectId, property](const Binding& b) {
                return b.objectId == objectId && b.property == property;
            }),
        m_Bindings.end()
    );
}

void NodePropertyBinding::updateAll() {
    if (!m_Scene) return;

    for (const auto& binding : m_Bindings) {
        GameObject* obj = findGameObject(binding.objectId);
        if (!obj || !binding.output) continue;

        // Get Transform component
        Transform* transform = obj->components.get<Transform>();
        if (!transform) continue;

        // Evaluate the node output
        NodeValue value = binding.output->owner->isDirty()
            ? (binding.output->owner->evaluate(), binding.output->getValue())
            : binding.output->getValue();

        // Apply value based on property type
        switch (binding.property) {
            case PropertyType::Position:
                if (std::holds_alternative<glm::vec3>(value)) {
                    transform->position = std::get<glm::vec3>(value);
                }
                break;

            case PropertyType::Rotation:
                if (std::holds_alternative<glm::vec3>(value)) {
                    // Assuming euler angles in radians
                    transform->rotation = std::get<glm::vec3>(value);
                }
                break;

            case PropertyType::Scale:
                if (std::holds_alternative<glm::vec3>(value)) {
                    transform->scale = std::get<glm::vec3>(value);
                }
                break;
        }
    }
}

bool NodePropertyBinding::isBound(uint64_t objectId, PropertyType property) const {
    for (const auto& binding : m_Bindings) {
        if (binding.objectId == objectId && binding.property == property) {
            return true;
        }
    }
    return false;
}

OutputSocket* NodePropertyBinding::getBinding(uint64_t objectId, PropertyType property) const {
    for (const auto& binding : m_Bindings) {
        if (binding.objectId == objectId && binding.property == property) {
            return binding.output;
        }
    }
    return nullptr;
}

GameObject* NodePropertyBinding::findGameObject(uint64_t objectId) {
    for (auto& obj : m_Scene->objects) {
        if (obj.id == objectId) {
            return &obj;
        }
    }
    return nullptr;
}

uint64_t NodePropertyBinding::getBindingKey(uint64_t objectId, PropertyType property) const {
    // Combine objectId and property into a unique key
    return (objectId << 8) | static_cast<uint64_t>(property);
}
