#pragma once

#include "NodeSystem.h"

// Float constant node
class FloatConstantNode : public Node {
public:
    FloatConstantNode(float value = 0.0f);

    void evaluate() override;
    void OnDrawNodeUI() override;
    const char* getTypeName() const override { return "Float"; }

    void setValue(float value);
    float getValue() const;

private:
    float m_Value;
};

// Int constant node
class IntConstantNode : public Node {
public:
    IntConstantNode(int value = 0);

    void evaluate() override;
    void OnDrawNodeUI() override;
    const char* getTypeName() const override { return "Int"; }

    void setValue(int value);
    int getValue() const;

private:
    int m_Value;
};

// Vec3 constant node
class Vec3ConstantNode : public Node {
public:
    Vec3ConstantNode(const glm::vec3& value = glm::vec3(0.0f));

    void evaluate() override;
    void OnDrawNodeUI() override;
    const char* getTypeName() const override { return "Vec3"; }

    void setValue(const glm::vec3& value);
    glm::vec3 getValue() const;

private:
    glm::vec3 m_Value;
};

// Time node - provides current time value
class TimeNode : public Node {
public:
    TimeNode();

    void evaluate() override;
    void OnDrawNodeUI() override;
    const char* getTypeName() const override { return "Time"; }

    void setTime(float time);
    float getTime() const;

private:
    float m_Time;
};
