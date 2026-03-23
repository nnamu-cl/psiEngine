#pragma once
#include "ecs/IComponent.h"
#include "../Data/AtomData.h"

class Atom : public IComponent
{
public:
    COMPONENT_TYPE_ID(Atom)

    AtomData* data = nullptr;

    Atom() = default;
    explicit Atom(AtomData* atomData) : data(atomData) {}

    std::unique_ptr<IComponent> clone() const override
    {
        auto cloned = std::make_unique<Atom>();
        cloned->data = data;
        return cloned;
    }

    void OnInspectorGUI() override;

    // Static UI drawing — used by both InspectorPanel and AtomNode.
    // compact = true gives a tighter layout for inside a node body.
    static void DrawAtomUI(AtomData* data, bool compact = false);
};
