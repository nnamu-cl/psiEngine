#include "ValueNodes.h"

#include <glm/gtc/type_ptr.hpp>

#include "ApplicationWindow.h"
#include "crude_json.h"
#include "imgui.h"
#include "ImGuiSkins/ShadSkin.h"

PhysicsConstantNode::PhysicsConstantNode() {
    addOutput("Value", SocketType::Float, Value);
}

void PhysicsConstantNode::OnDrawNodeUI() {
    ImGui::SetNextItemWidth(120.0f);
    ImGui::PushID(this);

    static ImGuiComboFlags flags;
    if (ImGui::BeginCombo(nullptr, modeNames[mode], flags)) {
        for (int n = 0; n < modeNames.size(), n++;) {
            const bool is_selected = (mode == n);
            if (ImGui::Selectable(modeNames[static_cast<int>(n)], is_selected))
                mode = static_cast<PhysicsConstantMode>(n);

            // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
            if (is_selected)
                ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }


    ImGui::PopID();
}

void PhysicsConstantNode::evaluate() {
    switch (mode) {
        case PhysicsConstantMode::Gravity:
            Value = 9.81f;
            break;
    }

    getOutput("Value")->setValue(Value);
}


// FloatConstantNode implementation
FloatConstantNode::FloatConstantNode(float value)
    : m_Value(value) {
    addOutput("Value", SocketType::Float, value);
}

void FloatConstantNode::evaluate() {
    getOutput("Value")->setValue(m_Value);
}

void FloatConstantNode::OnDrawNodeUI() {
    ImGui::PushID(this);
    ImGui::PushItemWidth(120.0f);
    ImGui::DragFloat("##value", &m_Value, 0.01f);
    ImGui::PopItemWidth();
    ImGui::PopID();
}

void FloatConstantNode::setValue(float value) {
    if (m_Value != value) {
        m_Value = value;
    }
}

float FloatConstantNode::getValue() const {
    return m_Value;
}

// IntConstantNode implementation
IntConstantNode::IntConstantNode(int value)
    : m_Value(value) {
    addOutput("Value", SocketType::Int, value);
}

void IntConstantNode::evaluate() {
    getOutput("Value")->setValue(m_Value);
}

void IntConstantNode::OnDrawNodeUI() {
    ImGui::PushID(this);
    ImGui::PushItemWidth(120.0f);
    ImGui::DragInt("##value", &m_Value, 0.1f);
    ImGui::PopItemWidth();
    ImGui::PopID();
}

void IntConstantNode::setValue(int value) {
    if (m_Value != value) {
        m_Value = value;
    }
}

int IntConstantNode::getValue() const {
    return m_Value;
}

// Vec3ConstantNode implementation
Vec3ConstantNode::Vec3ConstantNode(const glm::vec3 &value)
    : m_Value(value) {
    addOutput("Value", SocketType::Vec3, value);
}

void Vec3ConstantNode::evaluate() {
    getOutput("Value")->setValue(m_Value);
}

void Vec3ConstantNode::OnDrawNodeUI() {
    ImGui::PushID(this);
    ImGui::PushItemWidth(150.0f);
    ImGui::DragFloat3("##value", glm::value_ptr(m_Value), 0.01f);
    ImGui::PopItemWidth();
    ImGui::PopID();
}

void Vec3ConstantNode::setValue(const glm::vec3 &value) {
    if (m_Value != value) {
        m_Value = value;
    }
}

glm::vec3 Vec3ConstantNode::getValue() const {
    return m_Value;
}

// TimeNode implementation
TimeNode::TimeNode()
    : m_Time(0.0f) {
    addOutput("Time", SocketType::Float, 0.0f);
    addOutput("Delta Time", SocketType::Float, .016f);
}

void TimeNode::evaluate() {
    if (ApplicationWindow::instance != nullptr) {
        if (useMinMax) {
            rangedTime += ApplicationWindow::instance->timestep;
            if (rangedTime < min_max[0]) {
                rangedTime = min_max[0];
            }

            if (rangedTime > min_max[1]) {
                rangedTime = min_max[0];
            }

            getOutput("Time")->setValue(rangedTime);
        } else {
            getOutput("Time")->setValue(ApplicationWindow::instance->currentTime);
        }

        getOutput("Delta Time")->setValue(ApplicationWindow::instance->timestep);
    }
}

void TimeNode::OnDrawNodeUI() {
    ImGui::PushID(this);
    ImGui::SetNextItemWidth(100);
    ImGui::Text(" Time: %.2fs", useMinMax? rangedTime : ApplicationWindow::instance->currentTime  );
    ImGui::Checkbox("Ranged Value", &useMinMax);

    if (useMinMax) {
        ImGui::SetNextItemWidth(50);
        ImGui::DragFloat("Min ", &min_max[0]);
        ImGui::SameLine();
        ImGui::SetNextItemWidth(50);
        ImGui::DragFloat("Max", &min_max[1]);
    }

    ImGui::Text("Delta Time %.5fs", ApplicationWindow::instance->timestep);
    ImGui::PopID();
}
