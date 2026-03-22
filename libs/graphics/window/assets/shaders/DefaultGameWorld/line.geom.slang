// Line Geometry Shader
// Expands line vertices into thick quads with proper caps and joins

struct VertexInput
{
    float4 position : SV_Position;
    float4 color : COLOR;
    float thickness : THICKNESS;
    float distance : DISTANCE;
};

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
};

// Only viewProj remains here — everything else moved to SSBO
struct LinePushConstants
{
    float4x4 viewProj;
};

// Per-object data — must match LineObjectData in LinePipeline.h exactly
struct LineObjectData
{
    float4   color;
    float    thickness;
    float    dashLength;
    float    gapLength;
    uint     lineStyle;
    uint     antiAlias;
    float    smoothness;
    uint     padding[2];
};

// Set 1, binding 0 — the line SSBO owned by LinePipeline
// gl_DrawID (SV_DrawIndex) indexes into this to get the current object's data
[[vk::binding(0, 1)]]
StructuredBuffer<LineObjectData> lineObjects;

[shader("geometry")]
[maxvertexcount(6)]
void main(
    line VertexInput input[2],
    inout TriangleStream<FragmentInput> outputStream,
    uint drawID : SV_DrawIndex,                  // which object are we drawing
    uniform LinePushConstants pushConstants)
{
    // Fetch this object's data from the SSBO
    LineObjectData objData = lineObjects[drawID];

    float4 p0 = input[0].position;
    float4 p1 = input[1].position;

    float2 ndc0 = p0.xy / p0.w;
    float2 ndc1 = p1.xy / p1.w;

    float2 dir = normalize(ndc1 - ndc0);
    float2 perp = float2(-dir.y, dir.x);

    float thickness = (input[0].thickness + input[1].thickness) * 0.5;
    float thicknessNDC = thickness * 0.001;

    float2 offset = perp * thicknessNDC;
    float2 extension = dir * thicknessNDC;
    ndc0 -= extension;
    ndc1 += extension;

    FragmentInput v;

    // Read style data from SSBO instead of push constants
    v.lineStyle  = objData.lineStyle;
    v.dashLength = objData.dashLength;
    v.gapLength  = objData.gapLength;
    v.antiAlias  = objData.antiAlias;
    v.smoothness = objData.smoothness;

    // Vertex 0: Bottom-left
    v.position = float4((ndc0 - offset) * p0.w, p0.z, p0.w);
    v.color = input[0].color;
    v.uv = float2(0.0, 0.0);
    v.distance = input[0].distance;
    outputStream.Append(v);

    // Vertex 1: Top-left
    v.position = float4((ndc0 + offset) * p0.w, p0.z, p0.w);
    v.color = input[0].color;
    v.uv = float2(0.0, 1.0);
    v.distance = input[0].distance;
    outputStream.Append(v);

    // Vertex 2: Bottom-right
    v.position = float4((ndc1 - offset) * p1.w, p1.z, p1.w);
    v.color = input[1].color;
    v.uv = float2(1.0, 0.0);
    v.distance = input[1].distance;
    outputStream.Append(v);

    outputStream.RestartStrip();

    // Vertex 0: Top-left
    v.position = float4((ndc0 + offset) * p0.w, p0.z, p0.w);
    v.color = input[0].color;
    v.uv = float2(0.0, 1.0);
    v.distance = input[0].distance;
    outputStream.Append(v);

    // Vertex 1: Top-right
    v.position = float4((ndc1 + offset) * p1.w, p1.z, p1.w);
    v.color = input[1].color;
    v.uv = float2(1.0, 1.0);
    v.distance = input[1].distance;
    outputStream.Append(v);

    // Vertex 2: Bottom-right
    v.position = float4((ndc1 - offset) * p1.w, p1.z, p1.w);
    v.color = input[1].color;
    v.uv = float2(1.0, 0.0);
    v.distance = input[1].distance;
    outputStream.Append(v);

    outputStream.RestartStrip();
}