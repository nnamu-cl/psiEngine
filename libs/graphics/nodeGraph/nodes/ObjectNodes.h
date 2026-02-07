#pragma once

#include "NodeSystem.h"

// Object Node - represents a 3D object with position, rotation, and scale
class TransformNode : public Node {
public:
    TransformNode();

    void evaluate() override;
    void OnDrawNodeUI() override;
    const char* getTypeName() const override { return "Transform"; }
};
