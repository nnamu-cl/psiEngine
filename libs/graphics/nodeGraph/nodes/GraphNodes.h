#pragma once

#include "NodeSystem.h"
#include "ObjectNodes.h"
#include "../../../tools/CircularBuffer.h"


class LineGraphNode : public Node {
public:
    enum class Axis { X = 0, Y = 1, Z = 2 };

    size_t maxCount = 100; // the max number allowed
    CircularBuffer<float> points; // list of tracked values
    bool drawGraph = false;
    TransformNode *trackTransform = nullptr;
    Axis selectedAxis = Axis::X; // which axis to track

    LineGraphNode();

    void evaluate() override;
    const char* getTypeName() const override { return "Line Graph"; }
    void OnDrawNodeUI() override;
};
