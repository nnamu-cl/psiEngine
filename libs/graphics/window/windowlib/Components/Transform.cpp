#include "Transform.h"
#include "imgui.h"
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/euler_angles.hpp>

glm::mat4 Transform::toMatrix() const
{
    // TRS order: scale first, then rotate, then translate.
    // This matches the conventional column-major composition
    // T * R * S applied right-to-left to a vertex.
    glm::mat4 mat = glm::translate(glm::mat4(1.0f), position);
    mat            = mat * glm::mat4_cast(rotation);
    mat            = glm::scale(mat, scale);
    return mat;
}

void Transform::OnInspectorGUI()
{
    ImGui::Indent();

    // Position
    ImGui::Text("Position");
    ImGui::DragFloat3("##Position", glm::value_ptr(position), 0.1f);

    ImGui::Spacing();

    // Rotation (convert quaternion to Euler angles for editing)
    glm::vec3 eulerAngles = glm::degrees(glm::eulerAngles(rotation));
    ImGui::Text("Rotation");
    if (ImGui::DragFloat3("##Rotation", glm::value_ptr(eulerAngles), 1.0f))
    {
        // Convert back to quaternion
        rotation = glm::quat(glm::radians(eulerAngles));
    }

    ImGui::Spacing();

    // Scale
    ImGui::Text("Scale");
    ImGui::DragFloat3("##Scale", glm::value_ptr(scale), 0.01f, 0.001f, 100.0f);

    ImGui::Unindent();
}
