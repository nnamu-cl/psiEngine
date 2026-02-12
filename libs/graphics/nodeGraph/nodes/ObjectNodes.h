#pragma once

#include "NodeSystem.h"
#include <functional>

// Forward declaration
struct LineRendererData;

// Object Node - represents a 3D object with position, rotation, and scale
class TransformNode : public Node {
public:
    TransformNode();

    void evaluate() override;
    void OnDrawNodeUI() override;
    const char* getTypeName() const override { return "Transform"; }
};

// Line Renderer Node - generates line geometry based on position input
class LineRendererNode : public Node {
public:
    // Static callback to create line renderer data
    // Set this before creating nodes (typically in DefaultGameWorld::OnAttach)
    static std::function<LineRendererData*()> s_CreateLineCallback;

    //the type of line rendere we are using
    enum  LineRendererMode {
        Tracker,
        PointConnector
    };

    LineRendererNode();

    void SwitchMode(LineRendererMode mode);

    void evaluate() override;
    void OnDrawNodeUI() override;
    const char* getTypeName() const override { return "Line Renderer"; }

private:
    LineRendererData* m_LineData = nullptr;

    // UI-editable settings (not input sockets)
    float m_MinDistance = 0.1f;
    int m_MaxPoints = 1000;  // Maximum number of points (0 = unlimited)
    glm::vec4 m_Color = glm::vec4(1.0f);
    float m_Thickness = 20.0f;
    float m_DashLength = 10.0f;
    float m_GapLength = 5.0f;
    int m_LineStyle = 0;  // 0=Solid, 1=Dashed, 2=Dotted
    bool m_AntiAlias = true;
    float m_Smoothness = 1.0f;
    bool m_CurveSmoothing = false;
    int m_Subdivisions = 4;
};
