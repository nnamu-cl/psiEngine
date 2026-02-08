#pragma once
#include "IComponent.h"
#include <vector>
#include <memory>

// Simple component storage for GameObject
class ComponentStore
{
public:
    // Delete copy operations since unique_ptr is not copyable
    ComponentStore(const ComponentStore&) = delete;
    ComponentStore& operator=(const ComponentStore&) = delete;

    // Default move operations
    ComponentStore(ComponentStore&&) noexcept = default;
    ComponentStore& operator=(ComponentStore&&) noexcept = default;

    // Default constructor
    ComponentStore() = default;

    template<typename T>
    T* add(std::unique_ptr<T> component)
    {
        static_assert(std::is_base_of_v<IComponent, T>, "T must inherit from IComponent");
        T* ptr = component.get();
        m_components.push_back(std::move(component));
        return ptr;
    }

    template<typename T>
    T* get()
    {
        static_assert(std::is_base_of_v<IComponent, T>, "T must inherit from IComponent");
        for (auto& comp : m_components)
        {
            if (comp->getTypeID() == T::TypeID)
                return static_cast<T*>(comp.get());
        }
        return nullptr;
    }

    template<typename T>
    const T* get() const
    {
        static_assert(std::is_base_of_v<IComponent, T>, "T must inherit from IComponent");
        for (const auto& comp : m_components)
        {
            if (comp->getTypeID() == T::TypeID)
                return static_cast<const T*>(comp.get());
        }
        return nullptr;
    }

    template<typename T>
    bool has() const
    {
        return get<T>() != nullptr;
    }

    ComponentStore clone() const
    {
        ComponentStore copy;
        for (const auto& comp : m_components)
            copy.m_components.push_back(comp->clone());
        return copy;
    }

private:
    std::vector<std::unique_ptr<IComponent>> m_components;
};
