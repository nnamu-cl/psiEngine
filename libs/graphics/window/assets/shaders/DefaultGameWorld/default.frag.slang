// DefaultGameWorld fragment shader - Slang native format

struct PushConstants {
    float4x4 model;
    float4 objectColor;     // RGB + alpha
    uint colorMode;         // 0 = vertex color, 1 = object color
    uint padding[3];        // Align to 16 bytes
};

[[vk::push_constant]]
ConstantBuffer<PushConstants> pc;

struct FragmentInput {
    float3 worldNormal : NORMAL;
    float4 vertexColor : COLOR0;
};

[shader("fragment")]
float4 main(FragmentInput input) : SV_Target
{
    float4 finalColor;

    if (pc.colorMode == 0)  // Vertex color mode
    {
        finalColor = input.vertexColor;
    }
    else  // Object color mode
    {
        finalColor = pc.objectColor;
    }

    // Simple lighting with normal
    float3 lightDir = normalize(float3(0.5, 1.0, 0.3));
    float ndotl = max(dot(normalize(input.worldNormal), lightDir), 0.3);

    return float4(finalColor.rgb * ndotl, finalColor.a);
}
