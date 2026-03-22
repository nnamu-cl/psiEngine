// Common types shared across all pipeline shaders
// Imported by vertex, geometry, and fragment shaders

// Push constants — shared by vertex and geometry stages
struct LinePushConstants
{
    float4x4 viewProj;
    float    aspectRatio;
    float    projScale;   // projection[1][1] = 1/tan(fov/2), for perspective-correct sizing
    float    _pad1;
    float    _pad2;
};

// Per-object data stored in SSBO — must match LineObjectData in LinePipeline.h exactly
struct LineObjectData
{
    float4   color;
    float    thickness;
    float    dashLength;
    float    gapLength;
    uint     lineStyle;
    uint     antiAlias;
    float    smoothness;
    uint     objectType;   // 0 = line, 1 = circle
    float    radius;       // used when objectType != 0
    uint     lit;          // 0 = unlit, 1 = lit
    float    _pad0;
    float    _pad1;
    float    _pad2;
};

// Vertex shader output / Geometry shader input
struct VertexToGeom
{
    float4 position : SV_Position;
    float4 color : COLOR;
    float thickness : THICKNESS;
    float distance : DISTANCE;
};

// Geometry shader output / Fragment shader input
struct FragmentInput
{
    float4 position : SV_Position;
    float4 color : COLOR;
    float2 uv : TEXCOORD0;
    float distance : DISTANCE;
    uint lineStyle : LINESTYLE;
    float dashLength : DASHLENGTH;
    float gapLength : GAPLENGTH;
    uint antiAlias : ANTIALIAS;
    float smoothness : SMOOTHNESS;
    uint objectType : OBJECTTYPE;
    uint lit : LIT;
};
