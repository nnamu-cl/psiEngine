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
