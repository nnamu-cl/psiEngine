#include "ObjectNodes.h"
#include "imgui.h"

// ObjectNode implementation
TransformNode::TransformNode() {
    addInput("Pos", SocketType::Vec3, glm::vec3(1.0f));
    addInput("Rot", SocketType::Vec3, glm::vec3(1.0f));
    addInput("Scale", SocketType::Vec3, glm::vec3(1.0f));

    addOutput("Pos", SocketType::Vec3, glm::vec3(1.0f));
    addOutput("Rot", SocketType::Vec3, glm::vec3(1.0f));
    addOutput("Scale", SocketType::Vec3, glm::vec3(1.0f));
}

void TransformNode::evaluate() {

    glm::vec3 pos = std::get<glm::vec3>(getInput("Pos")->getValue());
    glm::vec3 rot = std::get<glm::vec3>(getInput("Rot")->getValue());
    glm::vec3 scale = std::get<glm::vec3>(getInput("Scale")->getValue());

    getOutput("Pos")->setValue(pos);
    getOutput("Rot")->setValue(rot);
    getOutput("Scale")->setValue(scale);
}

void TransformNode::OnDrawNodeUI() {
    ImGui::PushID(this);

    glm::vec3 pos = std::get<glm::vec3>(getOutput("Pos")->getValue());
    glm::vec3 rot = std::get<glm::vec3>(getOutput("Rot")->getValue());
    glm::vec3 scale = std::get<glm::vec3>(getOutput("Scale")->getValue());

    ImGui::Text("Pos: %.2f, %.2f, %.2f", pos.x, pos.y, pos.z);
    ImGui::Text("Rot: %.2f, %.2f, %.2f", rot.x, rot.y, rot.z);
    ImGui::Text("Scale: %.2f, %.2f, %.2f", scale.x, scale.y, scale.z);

    ImGui::PopID();
}
