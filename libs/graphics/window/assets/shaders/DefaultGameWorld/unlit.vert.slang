// Unlit vertex shader - Slang native format

struct CameraUBO {
    float4x4 view;
    float4x4 proj;
};

struct PushConstants {
    float4x4 model;
    float4 objectColor;       // RGB + alpha
    float emissionIntensity;  // Brightness multiplier
    float3 tintColor;         // Color tint multiplier
    float alphaCutoff;        // Alpha test threshold
    uint colorMode;           // 0 = vertex color, 1 = object color
    uint shadingMode;         // 0 = lit, 1 = unlit
    uint padding[2];          // Align to 16 bytes
};

[[vk::binding(0, 0)]]
 ConstantBuffer<CameraUBO> camera;

[[vk::push_constant]]
 ConstantBuffer<PushConstants> pc;

struct VertexOutput {
    float4 position : SV_Position;
    float4 vertexColor : COLOR0;
};

[shader("vertex")]
 VertexOutput main(
    float3 inPosition : POSITION,
    float3 inNormal : NORMAL,
    float2 inTexCoord : TEXCOORD,
    float4 inColor : COLOR)
{
    VertexOutput output;

    // Transform to clip space: proj * view * model * position
    float4 worldPos = mul(pc.model, float4(inPosition, 1.0));
    float4 viewPos = mul(camera.view, worldPos);
    output.position = mul(camera.proj, viewPos);

    // Pass through vertex color for fragment shader
    output.vertexColor = inColor;

    return output;
}
