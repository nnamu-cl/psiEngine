#pragma once

#include "NodeSystem.h"

// Add node - adds two values (works with float and vectors)
class AddNode : public Node {
public:
    static constexpr NodeIcon kIcon = NodeIcon::Plus;
    AddNode();

    void evaluate() override;
    const char* getTypeName() const override { return "Add"; }
    NodeIcon getIcon() const override { return kIcon; }
};

// Subtract node - subtracts B from A
class SubtractNode : public Node {
public:
    static constexpr NodeIcon kIcon = NodeIcon::Minus;
    SubtractNode();

    void evaluate() override;
    const char* getTypeName() const override { return "Subtract"; }
    NodeIcon getIcon() const override { return kIcon; }
};

// Multiply node - multiplies two values (works with float and vectors)
class MultiplyNode : public Node {
public:
    static constexpr NodeIcon kIcon = NodeIcon::Asterisk;
    MultiplyNode();

    void evaluate() override;
    const char* getTypeName() const override { return "Multiply"; }
    NodeIcon getIcon() const override { return kIcon; }
};

// Divide node - divides A by B
class DivideNode : public Node {
public:
    static constexpr NodeIcon kIcon = NodeIcon::Divide;
    DivideNode();

    void evaluate() override;
    const char* getTypeName() const override { return "Divide"; }
    NodeIcon getIcon() const override { return kIcon; }
};

// Sin node - applies sine function to input
class SinNode : public Node {
public:
    static constexpr NodeIcon kIcon = NodeIcon::Trig;
    SinNode();

    void evaluate() override;
    const char* getTypeName() const override { return "Sin"; }
    NodeIcon getIcon() const override { return kIcon; }
};

// Cos node - applies cosine function to input
class CosNode : public Node {
public:
    static constexpr NodeIcon kIcon = NodeIcon::Trig;
    CosNode();

    void evaluate() override;
    const char* getTypeName() const override { return "Cos"; }
    NodeIcon getIcon() const override { return kIcon; }
};

// Tan node - applies tangent function to input
class TanNode : public Node {
public:
    static constexpr NodeIcon kIcon = NodeIcon::Trig;
    TanNode();

    void evaluate() override;
    const char* getTypeName() const override { return "Tan"; }
    NodeIcon getIcon() const override { return kIcon; }
};

// Arctan node - applies arctangent function to input
class ArctanNode : public Node {
public:
    static constexpr NodeIcon kIcon = NodeIcon::Trig;
    ArctanNode();

    void evaluate() override;
    const char* getTypeName() const override { return "Arctan"; }
    NodeIcon getIcon() const override { return kIcon; }
};

// Arcsin node - applies arcsine function to input
class ArcsinNode : public Node {
public:
    static constexpr NodeIcon kIcon = NodeIcon::Trig;
    ArcsinNode();

    void evaluate() override;
    const char* getTypeName() const override { return "Arcsin"; }
    NodeIcon getIcon() const override { return kIcon; }
};

// Arccos node - applies arccosine function to input
class ArccosNode : public Node {
public:
    static constexpr NodeIcon kIcon = NodeIcon::Trig;
    ArccosNode();

    void evaluate() override;
    const char* getTypeName() const override { return "Arccos"; }
    NodeIcon getIcon() const override { return kIcon; }
};

// Pow node - raises base to power
class PowNode : public Node {
public:
    static constexpr NodeIcon kIcon = NodeIcon::SquareFunction;
    PowNode();

    void evaluate() override;
    const char* getTypeName() const override { return "Pow"; }
    NodeIcon getIcon() const override { return kIcon; }
};

// Root node - calculates square root of input
class RootNode : public Node {
public:
    static constexpr NodeIcon kIcon = NodeIcon::Radical;
    RootNode();

    void evaluate() override;
    const char* getTypeName() const override { return "Root"; }
    NodeIcon getIcon() const override { return kIcon; }
};

// PI constant node - provides the value of PI
class PINode : public Node {
public:
    static constexpr NodeIcon kIcon = NodeIcon::Pi;
    PINode();

    void evaluate() override;
    void OnDrawNodeUI() override;
    const char* getTypeName() const override { return "PI"; }
    NodeIcon getIcon() const override { return kIcon; }
};
