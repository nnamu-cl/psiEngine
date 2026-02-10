// Line Vertex Shader
// Passes vertex data to geometry shader for line expansion

struct VertexInput
{
    float3 position : POSITION;
    float4 color : COLOR;
    float thickness : THICKNESS;
    float distance : DISTANCE;
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
    float4 globalColor;
    float globalThickness;
    float dashLength;
    float gapLength;
    uint lineStyle;        // 0=solid, 1=dashed, 2=dotted
    uint antiAlias;
    float smoothness;
};

[shader("vertex")]
VertexOutput main(
    VertexInput input,
    uniform LinePushConstants pushConstants)
{
    VertexOutput output;

    // Transform position to clip space
    output.position = mul(pushConstants.viewProj, float4(input.position, 1.0));

    // Pass through color and thickness
    output.color = input.color * pushConstants.globalColor;
    output.thickness = input.thickness * pushConstants.globalThickness;
    output.distance = input.distance;

    return output;
}
