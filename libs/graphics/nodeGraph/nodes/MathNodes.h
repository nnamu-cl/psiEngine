#pragma once

#include "NodeSystem.h"

// Add node - adds two values (works with float and vectors)
class AddNode : public Node {
public:
    AddNode();

    void evaluate() override;
    const char* getTypeName() const override { return "Add"; }
};

// Subtract node - subtracts B from A
class SubtractNode : public Node {
public:
    SubtractNode();

    void evaluate() override;
    const char* getTypeName() const override { return "Subtract"; }
};

// Multiply node - multiplies two values (works with float and vectors)
class MultiplyNode : public Node {
public:
    MultiplyNode();

    void evaluate() override;
    const char* getTypeName() const override { return "Multiply"; }
};

// Divide node - divides A by B
class DivideNode : public Node {
public:
    DivideNode();

    void evaluate() override;
    const char* getTypeName() const override { return "Divide"; }
};

// Sin node - applies sine function to input
class SinNode : public Node {
public:
    SinNode();

    void evaluate() override;
    const char* getTypeName() const override { return "Sin"; }
};

// Cos node - applies cosine function to input
class CosNode : public Node {
public:
    CosNode();

    void evaluate() override;
    const char* getTypeName() const override { return "Cos"; }
};

// Tan node - applies tangent function to input
class TanNode : public Node {
public:
    TanNode();

    void evaluate() override;
    const char* getTypeName() const override { return "Tan"; }
};

// Arctan node - applies arctangent function to input
class ArctanNode : public Node {
public:
    ArctanNode();

    void evaluate() override;
    const char* getTypeName() const override { return "Arctan"; }
};

// Arcsin node - applies arcsine function to input
class ArcsinNode : public Node {
public:
    ArcsinNode();

    void evaluate() override;
    const char* getTypeName() const override { return "Arcsin"; }
};

// Arccos node - applies arccosine function to input
class ArccosNode : public Node {
public:
    ArccosNode();

    void evaluate() override;
    const char* getTypeName() const override { return "Arccos"; }
};

// Pow node - raises base to power
class PowNode : public Node {
public:
    PowNode();

    void evaluate() override;
    const char* getTypeName() const override { return "Pow"; }
};

// Root node - calculates square root of input
class RootNode : public Node {
public:
    RootNode();

    void evaluate() override;
    const char* getTypeName() const override { return "Root"; }
};

// PI constant node - provides the value of PI
class PINode : public Node {
public:
    PINode();

    void evaluate() override;
    void OnDrawNodeUI() override;
    const char* getTypeName() const override { return "PI"; }
};
