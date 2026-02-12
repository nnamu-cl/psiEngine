#include "ObjectNodes.h"
#include "imgui.h"
#include "../../window/windowlib/Data/LineRendererData.h"
#include <iostream>

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

// ===== LineRendererNode Implementation =====

// Static callback initialization
std::function<LineRendererData*()> LineRendererNode::s_CreateLineCallback = nullptr;

LineRendererNode::LineRendererNode() {
    // Only input: Position to potentially add as a point
    addInput("Position", SocketType::Vec3, glm::vec3(0.0f));

    // Call the callback to get line data pointer
    if (s_CreateLineCallback) {
        m_LineData = s_CreateLineCallback();
        if (m_LineData) {
            std::cout << "LineRendererNode created and linked to line data\n";
        } else {
            std::cerr << "LineRendererNode: Callback returned null data pointer\n";
        }
    } else {
        std::cerr << "LineRendererNode: No create callback set! Call LineRendererNode::s_CreateLineCallback first.\n";
    }
}

void LineRendererNode::evaluate() {

    bool isConnected = getInput("Position")->isConnected();

    if (m_LineData &&  isConnected) {

        // Read position input
        glm::vec3 position = std::get<glm::vec3>(getInput("Position")->getValue());

        // Check if we should add this point
        bool shouldAddPoint = false;
        //Only add points if we are connected to an input node
        if (m_LineData->points.empty() &&
            getInput("Position")->isConnected()) {
            // Always add first point
            shouldAddPoint = true;
            } else {
                // Check distance from last point
                glm::vec3 lastPoint = m_LineData->points.back();
                float distance = glm::length(position - lastPoint);

                if (distance >= m_MinDistance) {
                    shouldAddPoint = true;
                }
            }

        // Add point if threshold met
        if (shouldAddPoint) {
            m_LineData->points.push_back(position);
            m_LineData->needsGPUUpdate = true;

            // Remove oldest points if we exceed max points limit
            if (m_MaxPoints > 0 && m_LineData->points.size() > static_cast<size_t>(m_MaxPoints)) {
                m_LineData->points.erase(m_LineData->points.begin());
                m_LineData->needsGPUUpdate = true;
            }
        }
    }

    // Update appearance properties from UI settings
    m_LineData->properties.color = m_Color;
    m_LineData->properties.thickness = m_Thickness;
    m_LineData->properties.dashLength = m_DashLength;
    m_LineData->properties.gapLength = m_GapLength;
    m_LineData->properties.style = static_cast<LineStyle>(m_LineStyle);
    m_LineData->properties.antiAlias = m_AntiAlias;
    m_LineData->properties.smoothness = m_Smoothness;
    m_LineData->properties.curveSmoothing = m_CurveSmoothing;
    m_LineData->properties.smoothingSubdivisions = m_Subdivisions;
}

void LineRendererNode::OnDrawNodeUI() {
    ImGui::PushID(this);

    if (m_LineData) {
        ImGui::Text("Points: %zu", m_LineData->points.size());
        ImGui::SameLine();
        if (ImGui::Button("Clear Points")) {
            m_LineData->points.clear();
            m_LineData->needsGPUUpdate = true;
        }



        // Min Distance
        ImGui::Text("Min Distance");
        ImGui::SetNextItemWidth(150);
        ImGui::DragFloat("##MinDist", &m_MinDistance, 0.01f, 0.01f, 10.0f, "%.2f");

        // Max Points
        ImGui::Text("Max Points");
        ImGui::SetNextItemWidth(150);
        ImGui::DragInt("##MaxPoints", &m_MaxPoints, 1.0f, 0, 10000);


        // Color
        ImGui::Text("Color");
        ImGui::SetNextItemWidth(200);
        ImGui::ColorEdit4("##Color", &m_Color.x);

        // Thickness
        ImGui::Text("Thickness");
        ImGui::SetNextItemWidth(150);
        ImGui::SliderFloat("##Thickness", &m_Thickness, 0.1f, 100.0f, "%.1f");


        // Line Style
        ImGui::Text("Line Style");
        const char* lineStyles[] = { "Solid", "Dashed", "Dotted" };
        ImGui::SetNextItemWidth(150);
        ImGui::Combo("##LineStyle", &m_LineStyle, lineStyles, 3);

        // Dash/Gap settings (only for Dashed/Dotted)
        if (m_LineStyle != 0) {
            ImGui::Text("Dash Length");
            ImGui::SetNextItemWidth(150);
            ImGui::SliderFloat("##DashLength", &m_DashLength, 1.0f, 50.0f, "%.1f");

            ImGui::Text("Gap Length");
            ImGui::SetNextItemWidth(150);
            ImGui::SliderFloat("##GapLength", &m_GapLength, 1.0f, 50.0f, "%.1f");
        }


        // Anti-aliasing
        ImGui::Text("Anti-alias");
        ImGui::Checkbox("##AntiAlias", &m_AntiAlias);

        if (m_AntiAlias) {
            ImGui::Text("Smoothness");
            ImGui::SetNextItemWidth(150);
            ImGui::SliderFloat("##Smoothness", &m_Smoothness, 0.0f, 2.0f, "%.2f");
        }



        // Curve Smoothing
        ImGui::Text("Curve Smooth");
        ImGui::Checkbox("##CurveSmooth", &m_CurveSmoothing);

        if (m_CurveSmoothing) {
            ImGui::Text("Subdivisions");
            ImGui::SetNextItemWidth(150);
            ImGui::SliderInt("##Subdivisions", &m_Subdivisions, 1, 16);
        }
    } else {
        ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "No line data!");
    }

    ImGui::PopID();
}
