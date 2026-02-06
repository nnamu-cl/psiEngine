// DefaultGameWorld vertex shader - Slang native format

struct CameraUBO {
    float4x4 view;
    float4x4 proj;
};

struct PushConstants {
    float4x4 model;
};

[[vk::binding(0, 0)]]
ConstantBuffer<CameraUBO> camera;

[[vk::push_constant]]
ConstantBuffer<PushConstants> pc;

struct VertexOutput {
    float4 position : SV_Position;
    float3 color : COLOR0;
};

[shader("vertex")]
VertexOutput main(
    float3 inPosition : POSITION,
    float3 inNormal : NORMAL,
    float2 inTexCoord : TEXCOORD)
{
    VertexOutput output;

    // Transform to clip space: proj * view * model * position
    float4 worldPos = mul(pc.model, float4(inPosition, 1.0));
    float4 viewPos = mul(camera.view, worldPos);
    output.position = mul(camera.proj, viewPos);

    // Use normal as color (map [-1,1] to [0,1])
    output.color = inNormal * 0.5 + 0.5;

    return output;
}
