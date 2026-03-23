#pragma once

#include "NodeSystem.h"
#include <functional>

// Forward declarations
struct LineRendererData;
struct VolumeRendererData;
struct AtomData;
struct AtomVisualizerData;

// Object Node - represents a 3D object with position, rotation, and scale
class TransformNode : public Node {
public:
    static constexpr NodeIcon kIcon = NodeIcon::Move3D;
    TransformNode();

    void evaluate() override;
    void OnDrawNodeUI() override;
    const char* getTypeName() const override { return "Transform"; }
    NodeIcon getIcon() const override { return kIcon; }
};

// Line Renderer Node - generates line geometry based on position input
class LineRendererNode : public Node {
public:
    // Static callback to create line renderer data
    // Set this before creating nodes (typically in DefaultGameWorld::OnAttach)
    static std::function<LineRendererData*()> s_CreateLineCallback;

    //the type of line render we are using
    enum LineRendererMode {
        Tracker,
        PointConnector
    };


    //Record
    bool record =  true;

    //Does the line renderer need a gpu update
    bool needsGPUUpdate = false;

    static constexpr NodeIcon kIcon = NodeIcon::Spline;
    LineRendererNode();


    void evaluate() override;
    void OnDrawNodeUI() override;
    const char* getTypeName() const override { return "Line Renderer"; }
    NodeIcon getIcon() const override { return kIcon; }
    void SaveProperties(std::unordered_map<std::string, std::string>& props) override;
    void LoadProperties(const std::unordered_map<std::string, std::string>& props) override;

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

// Volume Renderer Node - UI-only node to control a VolumeRenderer's properties
class VolumeRendererNode : public Node {
public:
    static constexpr NodeIcon kIcon = NodeIcon::Volume;

    explicit VolumeRendererNode(VolumeRendererData* volumeData = nullptr);

    void evaluate() override;
    void OnDrawNodeUI() override;
    const char* getTypeName() const override { return "Volume Renderer"; }
    NodeIcon getIcon() const override { return kIcon; }

private:
    VolumeRendererData* m_VolumeData = nullptr;
};

// Atom Node — UI node to control both Atom (physics) and AtomVisualizer (rendering) properties
class AtomNode : public Node {
public:
    static constexpr NodeIcon kIcon = NodeIcon::Atom;

    explicit AtomNode(AtomData* atomData = nullptr, AtomVisualizerData* visData = nullptr);

    void evaluate() override;
    void OnDrawNodeUI() override;
    const char* getTypeName() const override { return "Atom"; }
    NodeIcon getIcon() const override { return kIcon; }

private:
    AtomData* m_AtomData = nullptr;
    AtomVisualizerData* m_VisData = nullptr;
};
