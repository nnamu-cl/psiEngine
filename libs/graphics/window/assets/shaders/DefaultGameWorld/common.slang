// Common types shared across all pipeline shaders
// Imported by vertex, geometry, and fragment shaders

// Push constants — shared by vertex, geometry, and fragment stages
struct LinePushConstants
{
    float4x4 viewProj;
    float    aspectRatio;
    float    projScale;   // projection[1][1] = 1/tan(fov/2), for perspective-correct sizing
    float    time;        // elapsed time in seconds (for orbital animation)
    float    _pad2;
    float3   cameraPos;   // world-space camera position (needed for orbital ray marching)
    float    _pad3;
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
    uint     objectType;   // 0 = line, 1 = circle, 2 = orbital
    float    radius;       // used when objectType != 0
    uint     lit;          // 0 = unlit, 1 = lit
    uint     orbN;         // principal quantum number
    uint     orbL;         // angular momentum quantum number
    int      orbM;         // magnetic quantum number
    float4   positiveColor;// orbital positive-lobe color
    float4   negativeColor;// orbital negative-lobe color
    float    bohrScale;    // visual scaling factor
    float    densityScale; // density-to-opacity multiplier
    uint     stepCount;    // grid resolution
    float    animationSpeed; // probability current animation speed
};

// Vertex shader output / Geometry shader input
struct VertexToGeom
{
    float4 position : SV_Position;
    float4 color : COLOR;
    float thickness : THICKNESS;
    float distance : DISTANCE;
    float3 worldPos : WORLDPOS;    // original world-space position (for orbital ray marching)
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

    // Orbital ray marching data (valid when objectType == 2)
    nointerpolation float3 worldCenter : WORLDCENTER;
    float3 worldPos : WORLDPOS;
    nointerpolation uint orbN : ORBN;
    nointerpolation uint orbL : ORBL;
    nointerpolation int orbM : ORBM;
    nointerpolation float bohrScale : BOHRSCALE;
    nointerpolation float densityScale : DENSITYSCALE;
    nointerpolation uint orbStepCount : ORBSTEPCOUNT;
    nointerpolation float4 positiveColor : POSCOLOR;
    nointerpolation float4 negativeColor : NEGCOLOR;
    nointerpolation float animationSpeed : ANIMSPEED;
};
