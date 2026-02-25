// Line Vertex Shader
// Passes vertex data to geometry shader for line expansion

struct VertexInput
{
    float3 position : POSITION;
    float4 color : COLOR;
    float thickness : THICKNESS;
    float distance : DISTANCE;
    uint   drawID    : SV_DrawIndex;  // gl_DrawID equivalent in Slang/HLSL
};

struct VertexOutput
{
    float4 position : SV_Position;
    float4 color : COLOR;
    float thickness : THICKNESS;
    float distance : DISTANCE;
};

struct LinePushConstants
{
    float4x4 viewProj;
};


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

[[vk::binding(0, 1)]]
StructuredBuffer<LineObjectData> lineObjects;

[shader("vertex")]
VertexOutput main(
    VertexInput input,
    uniform LinePushConstants pushConstants)
{
    VertexOutput output;

    LineObjectData objData = lineObjects[input.drawID];

    output.position  = mul(pushConstants.viewProj, float4(input.position, 1.0));
    output.color     = input.color * objData.color;
    output.thickness = input.thickness * objData.thickness;
    output.distance  = input.distance;

    return output;
}
