// Vertex Shader — passes vertex data through to geometry shader
import common;

struct VertexInput
{
    float3 position : POSITION;
    float4 color : COLOR;
    float thickness : THICKNESS;
    float distance : DISTANCE;
    uint   drawID    : SV_DrawIndex;
};

[[vk::binding(0, 1)]]
StructuredBuffer<LineObjectData> lineObjects;

[shader("vertex")]
VertexToGeom main(
    VertexInput input,
    uniform LinePushConstants pushConstants)
{
    VertexToGeom output;

    LineObjectData objData = lineObjects[input.drawID];

    output.position  = mul(pushConstants.viewProj, float4(input.position, 1.0));
    output.color     = input.color * objData.color;
    output.thickness = input.thickness * objData.thickness;
    output.distance  = input.distance;

    return output;
}
