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

void LineGraphNode::evaluate() {
   // never didge evaluation cause of not being dirty

    // if we have a transform to track
    if (trackTransform != nullptr) {
        // Get the position output socket
        if (OutputSocket* posOutput = trackTransform->getOutput("Pos")) {
            // Get the Vec3 value from the output
            glm::vec3 pos3d = std::get<glm::vec3>(posOutput->getValue());

            // Extract the selected axis component
            float value = 0.0f;
            switch (selectedAxis) {
                case Axis::X: value = pos3d.x; break;
                case Axis::Y: value = pos3d.y; break;
                case Axis::Z: value = pos3d.z; break;
            }

            // Push the single dimension value to buffer
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
    const char* currentSelection = trackTransform ? trackTransform->getName().c_str() : "None";

    if (ImGui::BeginCombo("##TrackTransform", currentSelection))
    {
        // Option to clear selection
        bool isNoneSelected = (trackTransform == nullptr);
        if (ImGui::Selectable("None", isNoneSelected))
        {
            trackTransform = nullptr;
        }

        // List all TransformNode instances in the graph
        if (graph)
        {
            for (const auto& node : graph->getNodes())
            {
                // Check if this node is a TransformNode
                TransformNode* transformNode = dynamic_cast<TransformNode*>(node.get());
                if (transformNode)
                {
                    // Display node name with ID
                    std::string label = transformNode->getName() + " [ID: " + std::to_string(transformNode->getId()) + "]";
                    bool isSelected = (trackTransform == transformNode);

                    if (ImGui::Selectable(label.c_str(), isSelected))
                    {
                        trackTransform = transformNode;
                    }

                    if (isSelected)
                    {
                        ImGui::SetItemDefaultFocus();
                    }
                }
            }
        }

        ImGui::EndCombo();
    }

    // Axis selection (only show if tracking a transform)
    if (trackTransform)
    {
        ImGui::Text("Track Axis");
        int axisIndex = static_cast<int>(selectedAxis);
        if (ImGui::RadioButton("X", &axisIndex, 0)) selectedAxis = Axis::X;
        ImGui::SameLine();
        if (ImGui::RadioButton("Y", &axisIndex, 1)) selectedAxis = Axis::Y;
        ImGui::SameLine();
        if (ImGui::RadioButton("Z", &axisIndex, 2)) selectedAxis = Axis::Z;

        // Show tracking info
        ImGui::Text("Tracking: %d points", static_cast<int>(points.buffer.size()));
    }

    ImGui::PopItemWidth();
    ImGui::PopID();
}
