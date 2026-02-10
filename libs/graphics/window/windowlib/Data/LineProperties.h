#pragma once
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

// Line style selection
enum class LineStyle
{
    Solid,    // Continuous line
    Dashed,   // Dashed pattern
    Dotted    // Dotted pattern
};

// Line topology
enum class LineTopology
{
    LineList,   // Each pair of vertices forms a separate line
    LineStrip,  // Connected line through all vertices
    LineLoop    // Connected line with last vertex connecting to first
};

// Cap style for line ends
enum class LineCapStyle
{
    Butt,   // Flat end at vertex
    Round,  // Rounded end
    Square  // Extended flat end
};

// Join style for line segments
enum class LineJoinStyle
{
    Miter,  // Sharp corner
    Round,  // Rounded corner
    Bevel   // Beveled corner
};

// Vertex structure for line rendering
struct LineVertex
{
    glm::vec3 position;
    glm::vec4 color;
    float thickness;
    float distanceAlongLine;
};

// Line rendering properties
struct LineProperties
{
    // Basic appearance
    glm::vec4 color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    float thickness = 20.0f;  // Increased default for better visibility

    // Line style
    LineStyle style = LineStyle::Solid;
    float dashLength = 10.0f;
    float gapLength = 5.0f;

    // Smoothing
    bool antiAlias = true;
    float smoothness = 1.0f;

    // Curve smoothing (Bezier-like interpolation)
    bool curveSmoothing = false;
    int smoothingSubdivisions = 4;  // Number of segments per original segment

    // Topology
    LineTopology topology = LineTopology::LineStrip;

    // Join/Cap styles
    LineCapStyle capStyle = LineCapStyle::Round;
    LineJoinStyle joinStyle = LineJoinStyle::Round;

    // Rendering options
    bool depthTest = true;
    bool billboard = false;  // Always face camera (not implemented yet)

    // Blending
    enum class LineBlendMode
    {
        Opaque,
        Transparent,
        Additive
    };
    LineBlendMode blendMode = LineBlendMode::Transparent;
};
