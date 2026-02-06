#pragma once
#include "ComponentType.h"
#include <memory>

// Base interface for all components
class IComponent
{
public:
    virtual ~IComponent() = default;

    // Get component type ID
    virtual ComponentType getTypeID() const = 0;

    // Clone for copying game objects
    virtual std::unique_ptr<IComponent> clone() const = 0;

    // Optional: Render inspector UI for this component
    // Override in derived components to show editable properties
    virtual void OnInspectorGUI() {}
};

// Helper macro to define component type ID
#define COMPONENT_TYPE_ID(TypeEnum) \
    static constexpr ComponentType TypeID = ComponentType::TypeEnum; \
    ComponentType getTypeID() const override { return TypeID; }
