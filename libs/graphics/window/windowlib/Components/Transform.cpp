#include "Transform.h"
#include "imgui.h"
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/euler_angles.hpp>
#include "nodes/ObjectNodes.h"
#include "nodes/NodeSystem.h"

glm::mat4 Transform::toMatrix() const
{
    // TRS order: scale first, then rotate, then translate.
    // This matches the conventional column-major composition
    // T * R * S applied right-to-left to a vertex.
    glm::vec3 pos = getPos();
    glm::vec3 scl = getScale();

    glm::mat4 mat = glm::translate(glm::mat4(1.0f), pos);

    // Handle rotation: use quaternion directly if not linked to avoid lossy conversion
    if (m_LinkedNode != nullptr)
    {
        OutputSocket* rotOutput = m_LinkedNode->getOutput("Rot");
        if (rotOutput)
        {
            // Node outputs Euler angles in degrees, convert to quaternion
            glm::vec3 rot = std::get<glm::vec3>(rotOutput->getValue());
            mat = mat * glm::mat4_cast(glm::quat(glm::radians(rot)));
        }
        else
        {
            // No rotation output, use stored quaternion directly
            mat = mat * glm::mat4_cast(rotation);
        }
    }
    else
    {
        // Not linked: use stored quaternion directly to avoid quat->euler->quat conversion
        mat = mat * glm::mat4_cast(rotation);
    }

    mat = glm::scale(mat, scl);
    return mat;
}

glm::vec3 Transform::getPos() const
{
    if (m_LinkedNode != nullptr)
    {
        OutputSocket* posOutput = m_LinkedNode->getOutput("Pos");
        if (posOutput)
        {
            return std::get<glm::vec3>(posOutput->getValue());
        }
    }
    return position;
}

glm::vec3 Transform::getRot() const
{
    if (m_LinkedNode != nullptr)
    {
        OutputSocket* rotOutput = m_LinkedNode->getOutput("Rot");
        if (rotOutput)
        {
            return std::get<glm::vec3>(rotOutput->getValue());
        }
    }
    return glm::degrees(glm::eulerAngles(rotation));
}

glm::vec3 Transform::getScale() const
{
    if (m_LinkedNode != nullptr)
    {
        OutputSocket* scaleOutput = m_LinkedNode->getOutput("Scale");
        if (scaleOutput)
        {
            return std::get<glm::vec3>(scaleOutput->getValue());
        }
    }
    return scale;
}

void Transform::linkToNode(TransformNode* node)
{
    m_LinkedNode = node;
}

void Transform::clearNodeLink()
{
    m_LinkedNode = nullptr;
}

void Transform::OnInspectorGUI()
{
    OnInspectorGUI(nullptr);
}

void Transform::OnInspectorGUI(NodeGraph* nodeGraph)
{
    ImGui::Indent();

    // Show linked node status
    if (m_LinkedNode != nullptr)
    {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.4f, 1.0f, 0.4f, 1.0f));
        ImGui::Text("Linked to Transform Node (ID: %llu)", m_LinkedNode->getId());
        ImGui::PopStyleColor();

        // Right-click to show context menu
        if (ImGui::IsItemClicked(ImGuiMouseButton_Right))
        {
            ImGui::OpenPopup("TransformLinkContextMenu");
        }
    }
    else
    {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.7f, 0.7f, 0.7f, 1.0f));
        ImGui::Text("Not linked to node");
        ImGui::PopStyleColor();

        // Right-click to show context menu
        if (ImGui::IsItemClicked(ImGuiMouseButton_Right))
        {
            ImGui::OpenPopup("TransformLinkContextMenu");
        }
    }

    // Context menu for linking
    if (ImGui::BeginPopup("TransformLinkContextMenu"))
    {
        if (nodeGraph != nullptr)
        {
            if (ImGui::BeginMenu("Link to Transform Node"))
            {
                // Find all TransformNodes in the graph
                bool foundAny = false;
                for (const auto& nodePtr : nodeGraph->getNodes())
                {
                    TransformNode* transformNode = dynamic_cast<TransformNode*>(nodePtr.get());
                    if (transformNode)
                    {
                        foundAny = true;
                        char label[64];
                        snprintf(label, sizeof(label), "Transform Node (ID: %llu)", transformNode->getId());
                        if (ImGui::MenuItem(label))
                        {
                            linkToNode(transformNode);
                        }
                    }
                }

                if (!foundAny)
                {
                    ImGui::TextDisabled("No Transform Nodes available");
                }

                ImGui::EndMenu();
            }

            if (m_LinkedNode != nullptr)
            {
                if (ImGui::MenuItem("Clear Link"))
                {
                    clearNodeLink();
                }
            }
        }
        else
        {
            ImGui::TextDisabled("Node graph not available");
        }

        ImGui::EndPopup();
    }

    ImGui::Spacing();

    // Position
    ImGui::Text("Position");
    if (m_LinkedNode != nullptr)
    {

    }
    else
    {
        ImGui::DragFloat3("##Position", glm::value_ptr(position), 0.1f);
    }

    ImGui::Spacing();

    // Rotation (convert quaternion to Euler angles for editing)
    ImGui::Text("Rotation");
    if (m_LinkedNode != nullptr)
    {

    }
    else
    {
        glm::vec3 eulerAngles = glm::degrees(glm::eulerAngles(rotation));
        if (ImGui::DragFloat3("##Rotation", glm::value_ptr(eulerAngles), 1.0f))
        {
            // Convert back to quaternion
            rotation = glm::quat(glm::radians(eulerAngles));
        }
    }

    ImGui::Spacing();

    // Scale
    ImGui::Text("Scale");
    if (m_LinkedNode != nullptr)
    {

    }
    else
    {
        ImGui::DragFloat3("##Scale", glm::value_ptr(scale), 0.01f, 0.001f, 100.0f);
    }

    ImGui::Unindent();
}
