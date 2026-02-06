#pragma once

#include "NodeSystem.h"

// Combine Vec3 node - combines x, y, z into a vec3
class CombineVec3Node : public Node {
public:
    CombineVec3Node();

    void evaluate() override;
    const char* getTypeName() const override { return "Combine Vec3"; }
};

// Separate Vec3 node - splits a vec3 into x, y, z components
class SeparateVec3Node : public Node {
public:
    SeparateVec3Node();

    void evaluate() override;
    const char* getTypeName() const override { return "Separate Vec3"; }
};

// Dot Product node - computes dot product of two vec3s
class DotProductNode : public Node {
public:
    DotProductNode();

    void evaluate() override;
    const char* getTypeName() const override { return "Dot Product"; }
};

// Cross Product node - computes cross product of two vec3s
class CrossProductNode : public Node {
public:
    CrossProductNode();

    void evaluate() override;
    const char* getTypeName() const override { return "Cross Product"; }
};

// Length node - computes magnitude/length of a vector
class LengthNode : public Node {
public:
    LengthNode();

    void evaluate() override;
    const char* getTypeName() const override { return "Length"; }
};
