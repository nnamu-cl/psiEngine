#pragma once

#include "NodeSystem.h"
#include <array>




// Float constant node
class PhysicsConstantNode : public Node {
public:
    static constexpr NodeIcon kIcon = NodeIcon::Atom;

    enum PhysicsConstantMode:uint8_t {
        Gravity = 0
    };

    std::array<const char*, 1> modeNames = {
        "Gravity"
    };


    PhysicsConstantNode();

    void evaluate() override;
    void OnDrawNodeUI() override;
    const char* getTypeName() const override { return "Physics Constant"; }
    NodeIcon getIcon() const override { return kIcon; }
    void SaveProperties(std::unordered_map<std::string, std::string>& props) override;
    void LoadProperties(const std::unordered_map<std::string, std::string>& props) override;

    PhysicsConstantMode mode = PhysicsConstantMode::Gravity;

    float Value;

};


// Float constant node
class FloatConstantNode : public Node {
public:
    static constexpr NodeIcon kIcon = NodeIcon::Variable;
    FloatConstantNode(float value = 0.0f);

    void evaluate() override;
    void OnDrawNodeUI() override;
    const char* getTypeName() const override { return "Float"; }
    NodeIcon getIcon() const override { return kIcon; }
    void SaveProperties(std::unordered_map<std::string, std::string>& props) override;
    void LoadProperties(const std::unordered_map<std::string, std::string>& props) override;

    void setValue(float value);
    float getValue() const;

private:
    float m_Value;
};

// Int constant node
class IntConstantNode : public Node {
public:
    static constexpr NodeIcon kIcon = NodeIcon::Variable;
    IntConstantNode(int value = 0);

    void evaluate() override;
    void OnDrawNodeUI() override;
    const char* getTypeName() const override { return "Int"; }
    NodeIcon getIcon() const override { return kIcon; }
    void SaveProperties(std::unordered_map<std::string, std::string>& props) override;
    void LoadProperties(const std::unordered_map<std::string, std::string>& props) override;

    void setValue(int value);
    int getValue() const;

private:
    int m_Value;
};

// Vec3 constant node
class Vec3ConstantNode : public Node {
public:
    static constexpr NodeIcon kIcon = NodeIcon::VectorSquare;
    Vec3ConstantNode(const glm::vec3& value = glm::vec3(0.0f));

    void evaluate() override;
    void OnDrawNodeUI() override;
    const char* getTypeName() const override { return "Vec3"; }
    NodeIcon getIcon() const override { return kIcon; }
    void SaveProperties(std::unordered_map<std::string, std::string>& props) override;
    void LoadProperties(const std::unordered_map<std::string, std::string>& props) override;

    void setValue(const glm::vec3& value);
    glm::vec3 getValue() const;

private:
    glm::vec3 m_Value;
};

// Time node - provides current time value
class TimeNode : public Node {
public:
    static constexpr NodeIcon kIcon = NodeIcon::Time;
    TimeNode();

    void evaluate() override;
    void OnDrawNodeUI() override;
    const char* getTypeName() const override { return "Time"; }
    NodeIcon getIcon() const override { return kIcon; }
    void SaveProperties(std::unordered_map<std::string, std::string>& props) override;
    void LoadProperties(const std::unordered_map<std::string, std::string>& props) override;

    float rangedTime;
    std::vector <float> min_max = {0,1};
    bool useMinMax = false;


private:
    float m_Time;
};
