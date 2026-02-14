// Unlit fragment shader - Slang native format

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

[[vk::push_constant]]
 ConstantBuffer<PushConstants> pc;

struct FragmentInput {
    float4 vertexColor : COLOR0;
};

[shader("fragment")]
float4 main(FragmentInput input) : SV_Target
{
    float4 finalColor;

    // Determine base color based on color mode
    if (pc.colorMode == 0)  // Vertex color mode
    {
        finalColor = input.vertexColor;
    }
    else  // Object color mode
    {
        finalColor = pc.objectColor;
    }

    // Apply tint color
    finalColor.rgb *= pc.tintColor;

    // Apply emission intensity (for unlit, this makes colors brighter)
    finalColor.rgb *= pc.emissionIntensity;

    // Alpha cutoff (for alpha testing)
    if (finalColor.a < pc.alphaCutoff && pc.alphaCutoff > 0.0)
    {
        discard;
    }

    return finalColor;
}
