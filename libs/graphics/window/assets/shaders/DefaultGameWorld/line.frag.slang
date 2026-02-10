// Line Fragment Shader
// Handles line color, dashing, and anti-aliasing

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

[shader("fragment")]
float4 main(FragmentInput input) : SV_Target
{
    float4 color = input.color;

    // Apply dashed/dotted pattern
    if (input.lineStyle == 1) // Dashed
    {
        float pattern = fmod(input.distance, input.dashLength + input.gapLength);
        if (pattern > input.dashLength)
        {
            discard;
        }
    }
    else if (input.lineStyle == 2) // Dotted
    {
        float dotSize = input.dashLength * 0.3;
        float pattern = fmod(input.distance, input.dashLength + input.gapLength);
        if (pattern > dotSize)
        {
            discard;
        }
    }

    // Apply anti-aliasing at edges
    if (input.antiAlias != 0)
    {
        // Calculate distance from line center (using UV)
        float edgeDist = abs(input.uv.y - 0.5) * 2.0;  // 0 at center, 1 at edge

        // Smooth falloff at edges
        float edgeFactor = 1.0 - smoothstep(1.0 - input.smoothness * 0.2, 1.0, edgeDist);
        color.a *= edgeFactor;

        // Discard fully transparent pixels
        if (color.a < 0.01)
            discard;
    }

    return color;
}
