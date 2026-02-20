#pragma once

#include "NodeSystem.h"

// Combine Vec3 node - combines x, y, z into a vec3
class CombineVec3Node : public Node {
public:
    static constexpr NodeIcon kIcon = NodeIcon::Layers;
    CombineVec3Node();

    void evaluate() override;
    const char* getTypeName() const override { return "Combine Vec3"; }
    NodeIcon getIcon() const override { return kIcon; }
};

// Separate Vec3 node - splits a vec3 into x, y, z components
class SeparateVec3Node : public Node {
public:
    static constexpr NodeIcon kIcon = NodeIcon::Layers2;
    SeparateVec3Node();

    void evaluate() override;
    const char* getTypeName() const override { return "Separate Vec3"; }
    NodeIcon getIcon() const override { return kIcon; }
};

// Dot Product node - computes dot product of two vec3s
class DotProductNode : public Node {
public:
    static constexpr NodeIcon kIcon = NodeIcon::Dot;
    DotProductNode();

    void evaluate() override;
    const char* getTypeName() const override { return "Dot Product"; }
    NodeIcon getIcon() const override { return kIcon; }
};

// Cross Product node - computes cross product of two vec3s
class CrossProductNode : public Node {
public:
    static constexpr NodeIcon kIcon = NodeIcon::X;
    CrossProductNode();

    void evaluate() override;
    const char* getTypeName() const override { return "Cross Product"; }
    NodeIcon getIcon() const override { return kIcon; }
};

// Length node - computes magnitude/length of a vector
class LengthNode : public Node {
public:
    static constexpr NodeIcon kIcon = NodeIcon::Ruler;
    LengthNode();

    void evaluate() override;
    const char* getTypeName() const override { return "Length"; }
    NodeIcon getIcon() const override { return kIcon; }
};
