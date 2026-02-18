//
// Created by namus on 2/8/2026.
//

#include "GraphNodes.h"

#include <complex>
#include <glm/gtc/type_ptr.hpp>

#include "imgui.h"
#include "imgui_internal.h"


LineGraphNode::LineGraphNode()
    : points(ref_capacity, maxCount)  // Use ref_capacity tag to bind points.capacity to maxCount
{
}

TransformNode* LineGraphNode::getTrackedTransform() const {
    return resolveTrackedTransform();
}

TransformNode* LineGraphNode::resolveTrackedTransform() const {
    if (m_TrackTransformId == 0 || !graph) return nullptr;
    for (const auto& node : graph->getNodes()) {
        if (node->getId() == m_TrackTransformId)
            return dynamic_cast<TransformNode*>(node.get());
    }
    return nullptr;
}

void LineGraphNode::SaveProperties(std::unordered_map<std::string, std::string>& props) {
    props["maxCount"]  = std::to_string(maxCount);
    props["axis"]      = std::to_string(static_cast<int>(selectedAxis));
    props["drawGraph"] = std::to_string(drawGraph);
    if (m_TrackTransformId != 0)
        props["trackId"] = std::to_string(m_TrackTransformId);
}

void LineGraphNode::LoadProperties(const std::unordered_map<std::string, std::string>& props) {
    if (auto it = props.find("maxCount");  it != props.end()) maxCount         = std::stoull(it->second);
    if (auto it = props.find("axis");      it != props.end()) selectedAxis     = static_cast<Axis>(std::stoi(it->second));
    if (auto it = props.find("drawGraph"); it != props.end()) drawGraph        = std::stoi(it->second);
    if (auto it = props.find("trackId");   it != props.end()) m_TrackTransformId = std::stoull(it->second);
}

void LineGraphNode::PostNodeLoad() {
    // Clear the ID if the referenced node no longer exists in the graph
    if (m_TrackTransformId != 0 && resolveTrackedTransform() == nullptr)
        m_TrackTransformId = 0;
}

void LineGraphNode::evaluate() {
    if (TransformNode* tracked = resolveTrackedTransform()) {
        if (OutputSocket* posOutput = tracked->getOutput("Pos")) {
            glm::vec3 pos3d = std::get<glm::vec3>(posOutput->getValue());

            float value = 0.0f;
            switch (selectedAxis) {
                case Axis::X: value = pos3d.x; break;
                case Axis::Y: value = pos3d.y; break;
                case Axis::Z: value = pos3d.z; break;
            }

            points.Push(value);
        }
    }
}

void LineGraphNode::OnDrawNodeUI() {
    ImGui::PushItemWidth(100.0f);
    ImGui::PushID(this);

    // Max Points setting
    int value = maxCount;
    ImGui::Text("Max Points");
    ImGui::SameLine();
    ImGui::DragInt("##value", &value, 1.0f, 10, 1000);
    maxCount = std::abs(value);

    // Display checkbox
    ImGui::Text("Display");
    ImGui::SameLine();
    ImGui::Checkbox("##display", &drawGraph);


    // Track Transform dropdown (inside node)
    ImGui::Text("Track Transform");
    TransformNode* tracked = resolveTrackedTransform();
    const char* currentSelection = tracked ? tracked->getName().c_str() : "None";

    if (ImGui::BeginCombo("##TrackTransform", currentSelection))
    {
        if (ImGui::Selectable("None", m_TrackTransformId == 0))
            m_TrackTransformId = 0;

        if (graph)
        {
            for (const auto& node : graph->getNodes())
            {
                TransformNode* transformNode = dynamic_cast<TransformNode*>(node.get());
                if (transformNode)
                {
                    std::string label = transformNode->getName() + " [ID: " + std::to_string(transformNode->getId()) + "]";
                    bool isSelected = (m_TrackTransformId == transformNode->getId());

                    if (ImGui::Selectable(label.c_str(), isSelected))
                        m_TrackTransformId = transformNode->getId();

                    if (isSelected)
                        ImGui::SetItemDefaultFocus();
                }
            }
        }

        ImGui::EndCombo();
    }

    // Axis selection (only show if tracking a transform)
    if (m_TrackTransformId != 0)
    {
        ImGui::Text("Track Axis");
        int axisIndex = static_cast<int>(selectedAxis);
        if (ImGui::RadioButton("X", &axisIndex, 0)) selectedAxis = Axis::X;
        ImGui::SameLine();
        if (ImGui::RadioButton("Y", &axisIndex, 1)) selectedAxis = Axis::Y;
        ImGui::SameLine();
        if (ImGui::RadioButton("Z", &axisIndex, 2)) selectedAxis = Axis::Z;

        ImGui::Text("Tracking: %d points", static_cast<int>(points.buffer.size()));
    }

    ImGui::PopItemWidth();
    ImGui::PopID();
}

namespace {
    const bool s_graphNodes_registered = []() {
        NodeGraph::RegisterNodeType("Line Graph", []() { return std::make_unique<LineGraphNode>(); });
        return true;
    }();
}
