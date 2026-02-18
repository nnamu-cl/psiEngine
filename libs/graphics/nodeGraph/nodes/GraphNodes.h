#pragma once

#include "NodeSystem.h"
#include "ObjectNodes.h"
#include "../../../tools/CircularBuffer.h"


class LineGraphNode : public Node {
public:
    enum class Axis { X = 0, Y = 1, Z = 2 };

    size_t maxCount = 100;
    CircularBuffer<float> points;
    bool drawGraph = false;
    uint64_t m_TrackTransformId = 0;
    Axis selectedAxis = Axis::X;

    LineGraphNode();

    void evaluate() override;
    const char* getTypeName() const override { return "Line Graph"; }
    void OnDrawNodeUI() override;
    void SaveProperties(std::unordered_map<std::string, std::string>& props) override;
    void LoadProperties(const std::unordered_map<std::string, std::string>& props) override;
    void PostNodeLoad() override;

    TransformNode* getTrackedTransform() const;

private:
    TransformNode* resolveTrackedTransform() const;
};
