#include "LineRenderer.h"
#include "imgui.h"
#include <glm/gtc/type_ptr.hpp>

// Catmull-Rom spline interpolation for smooth curves
static glm::vec3 catmullRomInterpolate(const glm::vec3& p0, const glm::vec3& p1,
                                        const glm::vec3& p2, const glm::vec3& p3,
                                        float t)
{
    float t2 = t * t;
    float t3 = t2 * t;

    // Catmull-Rom basis matrix coefficients
    glm::vec3 result = 0.5f * (
        (2.0f * p1) +
        (-p0 + p2) * t +
        (2.0f * p0 - 5.0f * p1 + 4.0f * p2 - p3) * t2 +
        (-p0 + 3.0f * p1 - 3.0f * p2 + p3) * t3
    );

    return result;
}

// Subdivide points using Catmull-Rom spline for smooth curves
static std::vector<glm::vec3> smoothCurve(const std::vector<glm::vec3>& controlPoints, int subdivisions)
{
    std::vector<glm::vec3> smoothed;

    if (controlPoints.size() < 2)
        return controlPoints;

    if (controlPoints.size() == 2)
    {
        // Just a line segment - no smoothing needed
        return controlPoints;
    }

    // For each segment between control points
    for (size_t i = 0; i < controlPoints.size() - 1; ++i)
    {
        // Get the 4 points needed for Catmull-Rom
        glm::vec3 p0 = (i == 0) ? controlPoints[i] : controlPoints[i - 1];
        glm::vec3 p1 = controlPoints[i];
        glm::vec3 p2 = controlPoints[i + 1];
        glm::vec3 p3 = (i + 2 < controlPoints.size()) ? controlPoints[i + 2] : controlPoints[i + 1];

        // Interpolate between p1 and p2
        for (int j = 0; j < subdivisions; ++j)
        {
            float t = static_cast<float>(j) / static_cast<float>(subdivisions);
            smoothed.push_back(catmullRomInterpolate(p0, p1, p2, p3, t));
        }
    }

    // Add the last point
    smoothed.push_back(controlPoints.back());

    return smoothed;
}

std::vector<LineVertex> LineRenderer::buildVertexData() const
{
    std::vector<LineVertex> vertices;

    if (points.empty())
        return vertices;

    // Apply curve smoothing if enabled
    std::vector<glm::vec3> processedPoints = points;
    if (properties.curveSmoothing && points.size() >= 3)
    {
        processedPoints = smoothCurve(points, properties.smoothingSubdivisions);
    }

    // Calculate total distance along line for dashing
    float totalDistance = 0.0f;
    std::vector<float> distances;
    distances.push_back(0.0f);

    for (size_t i = 1; i < processedPoints.size(); ++i)
    {
        float segmentLength = glm::length(processedPoints[i] - processedPoints[i - 1]);
        totalDistance += segmentLength;
        distances.push_back(totalDistance);
    }

    // Build vertices
    for (size_t i = 0; i < processedPoints.size(); ++i)
    {
        LineVertex vertex;
        vertex.position = processedPoints[i];
        vertex.color = properties.color;
        vertex.thickness = properties.thickness;
        vertex.distanceAlongLine = distances[i];
        vertices.push_back(vertex);
    }

    return vertices;
}

void LineRenderer::OnInspectorGUI()
{
    ImGui::Indent();

    // Color
    ImGui::Text("Color");
    ImGui::SameLine();
    ImGui::SetCursorPosX(120);
    if (ImGui::ColorEdit4("##LineColor", glm::value_ptr(properties.color)))
    {
        needsGPUUpdate = true;
    }

    // Thickness
    ImGui::Text("Thickness");
    ImGui::SameLine();
    ImGui::SetCursorPosX(120);
    ImGui::SetNextItemWidth(150);
    if (ImGui::SliderFloat("##Thickness", &properties.thickness, 0.1f, 100.0f, "%.1f"))
    {
        needsGPUUpdate = true;
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // Line Style
    ImGui::Text("Line Style");
    ImGui::SameLine();
    ImGui::SetCursorPosX(120);
    const char* lineStyles[] = { "Solid", "Dashed", "Dotted" };
    int lineStyle = static_cast<int>(properties.style);
    if (ImGui::Combo("##LineStyle", &lineStyle, lineStyles, IM_ARRAYSIZE(lineStyles)))
    {
        properties.style = static_cast<LineStyle>(lineStyle);
    }

    // Dash/Dot settings (only for Dashed/Dotted)
    if (properties.style != LineStyle::Solid)
    {
        ImGui::Text("Dash Length");
        ImGui::SameLine();
        ImGui::SetCursorPosX(120);
        ImGui::SetNextItemWidth(150);
        ImGui::SliderFloat("##DashLength", &properties.dashLength, 1.0f, 50.0f, "%.1f");

        ImGui::Text("Gap Length");
        ImGui::SameLine();
        ImGui::SetCursorPosX(120);
        ImGui::SetNextItemWidth(150);
        ImGui::SliderFloat("##GapLength", &properties.gapLength, 1.0f, 50.0f, "%.1f");
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // Topology
    ImGui::Text("Topology");
    ImGui::SameLine();
    ImGui::SetCursorPosX(120);
    const char* topologies[] = { "Line List", "Line Strip", "Line Loop" };
    int topology = static_cast<int>(properties.topology);
    if (ImGui::Combo("##Topology", &topology, topologies, IM_ARRAYSIZE(topologies)))
    {
        properties.topology = static_cast<LineTopology>(topology);
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // Anti-aliasing
    ImGui::Text("Anti-alias");
    ImGui::SameLine();
    ImGui::SetCursorPosX(120);
    ImGui::Checkbox("##AntiAlias", &properties.antiAlias);

    if (properties.antiAlias)
    {
        ImGui::Text("Smoothness");
        ImGui::SameLine();
        ImGui::SetCursorPosX(120);
        ImGui::SetNextItemWidth(150);
        ImGui::SliderFloat("##Smoothness", &properties.smoothness, 0.0f, 2.0f, "%.2f");
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // Curve Smoothing (Bezier-like)
    ImGui::Text("Curve Smooth");
    ImGui::SameLine();
    ImGui::SetCursorPosX(120);
    if (ImGui::Checkbox("##CurveSmoothing", &properties.curveSmoothing))
    {
        needsGPUUpdate = true;
    }

    if (properties.curveSmoothing)
    {
        ImGui::Text("Subdivisions");
        ImGui::SameLine();
        ImGui::SetCursorPosX(120);
        ImGui::SetNextItemWidth(150);
        if (ImGui::SliderInt("##Subdivisions", &properties.smoothingSubdivisions, 1, 16))
        {
            needsGPUUpdate = true;
        }
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // Cap Style
    ImGui::Text("Cap Style");
    ImGui::SameLine();
    ImGui::SetCursorPosX(120);
    const char* capStyles[] = { "Butt", "Round", "Square" };
    int capStyle = static_cast<int>(properties.capStyle);
    if (ImGui::Combo("##CapStyle", &capStyle, capStyles, IM_ARRAYSIZE(capStyles)))
    {
        properties.capStyle = static_cast<LineCapStyle>(capStyle);
    }

    // Join Style
    ImGui::Text("Join Style");
    ImGui::SameLine();
    ImGui::SetCursorPosX(120);
    const char* joinStyles[] = { "Miter", "Round", "Bevel" };
    int joinStyle = static_cast<int>(properties.joinStyle);
    if (ImGui::Combo("##JoinStyle", &joinStyle, joinStyles, IM_ARRAYSIZE(joinStyles)))
    {
        properties.joinStyle = static_cast<LineJoinStyle>(joinStyle);
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // Blend Mode
    ImGui::Text("Blend Mode");
    ImGui::SameLine();
    ImGui::SetCursorPosX(120);
    const char* blendModes[] = { "Opaque", "Transparent", "Additive" };
    int blendMode = static_cast<int>(properties.blendMode);
    if (ImGui::Combo("##BlendMode", &blendMode, blendModes, IM_ARRAYSIZE(blendModes)))
    {
        properties.blendMode = static_cast<LineProperties::LineBlendMode>(blendMode);
    }

    // Depth Test
    ImGui::Text("Depth Test");
    ImGui::SameLine();
    ImGui::SetCursorPosX(120);
    ImGui::Checkbox("##DepthTest", &properties.depthTest);

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // Points editor
    ImGui::Text("Points: %zu", points.size());
    if (ImGui::Button("Add Point"))
    {
        if (!points.empty())
            points.push_back(points.back() + glm::vec3(1.0f, 0.0f, 0.0f));
        else
            points.push_back(glm::vec3(0.0f));
        needsGPUUpdate = true;
    }
    ImGui::SameLine();
    if (ImGui::Button("Remove Point") && !points.empty())
    {
        points.pop_back();
        needsGPUUpdate = true;
    }

    // Edit individual points
    for (size_t i = 0; i < points.size(); ++i)
    {
        ImGui::PushID(static_cast<int>(i));
        std::string label = "Point " + std::to_string(i);
        ImGui::Text("%s", label.c_str());
        ImGui::SameLine();
        ImGui::SetCursorPosX(120);
        ImGui::SetNextItemWidth(200);
        if (ImGui::DragFloat3("##Point", glm::value_ptr(points[i]), 0.1f))
        {
            needsGPUUpdate = true;
        }
        ImGui::PopID();
    }

    ImGui::Unindent();
}
