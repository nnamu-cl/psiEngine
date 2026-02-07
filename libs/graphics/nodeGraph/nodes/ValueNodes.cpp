#include "ValueNodes.h"

#include <glm/gtc/type_ptr.hpp>

#include "imgui.h"

// FloatConstantNode implementation
FloatConstantNode::FloatConstantNode(float value)
    : m_Value(value)
{
    addOutput("Value", SocketType::Float, value);
}

void FloatConstantNode::evaluate() {
    if (!isDirty()) return;

    getOutput("Value")->setValue(m_Value);
    markClean();
}

void FloatConstantNode::OnDrawNodeUI() {
    ImGui::PushID(this);
    ImGui::PushItemWidth(120.0f);
    if (ImGui::DragFloat("##value", &m_Value, 0.01f)) {
        markDirty();
    }
    ImGui::PopItemWidth();
    ImGui::PopID();
}

void FloatConstantNode::setValue(float value) {
    if (m_Value != value) {
        m_Value = value;
        markDirty();
    }
}

float FloatConstantNode::getValue() const {
    return m_Value;
}

// IntConstantNode implementation
IntConstantNode::IntConstantNode(int value)
    : m_Value(value)
{
    addOutput("Value", SocketType::Int, value);
}

void IntConstantNode::evaluate() {
    if (!isDirty()) return;

    getOutput("Value")->setValue(m_Value);
    markClean();
}

void IntConstantNode::OnDrawNodeUI() {
    ImGui::PushID(this);
    ImGui::PushItemWidth(120.0f);
    if (ImGui::DragInt("##value", &m_Value, 0.1f)) {
        markDirty();
    }
    ImGui::PopItemWidth();
    ImGui::PopID();
}

void IntConstantNode::setValue(int value) {
    if (m_Value != value) {
        m_Value = value;
        markDirty();
    }
}

int IntConstantNode::getValue() const {
    return m_Value;
}

// Vec3ConstantNode implementation
Vec3ConstantNode::Vec3ConstantNode(const glm::vec3& value)
    : m_Value(value)
{
    addOutput("Value", SocketType::Vec3, value);
}

void Vec3ConstantNode::evaluate() {
    if (!isDirty()) return;

    getOutput("Value")->setValue(m_Value);
    markClean();
}

void Vec3ConstantNode::OnDrawNodeUI() {
    ImGui::PushID(this);
    ImGui::PushItemWidth(150.0f);
    if (ImGui::DragFloat3("##value", glm::value_ptr(m_Value), 0.01f)) {
        markDirty();
    }
    ImGui::PopItemWidth();
    ImGui::PopID();
}

void Vec3ConstantNode::setValue(const glm::vec3& value) {
    if (m_Value != value) {
        m_Value = value;
        markDirty();
    }
}

glm::vec3 Vec3ConstantNode::getValue() const {
    return m_Value;
}

// TimeNode implementation
TimeNode::TimeNode()
    : m_Time(0.0f)
{
    addOutput("Time", SocketType::Float, 0.0f);
}

void TimeNode::evaluate() {
    if (!isDirty()) return;

    getOutput("Time")->setValue(m_Time);
    markClean();
}

void TimeNode::OnDrawNodeUI() {
    ImGui::PushID(this);
    ImGui::Text("%.2fs", m_Time);
    ImGui::PopID();
}

void TimeNode::setTime(float time) {
    if (m_Time != time) {
        m_Time = time;
        markDirty();
    }
}

float TimeNode::getTime() const {
    return m_Time;
}
