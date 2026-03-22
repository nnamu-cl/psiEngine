// Line-specific operations for geometry expansion and fragment shading
import common;

// Expand a line segment (2 vertices) into a screen-aligned quad (2 triangles)
void emitLineQuad(
    VertexToGeom p0In,
    VertexToGeom p1In,
    LineObjectData objData,
    inout TriangleStream<FragmentInput> outputStream)
{
    float4 p0 = p0In.position;
    float4 p1 = p1In.position;

    float2 ndc0 = p0.xy / p0.w;
    float2 ndc1 = p1.xy / p1.w;

    float2 dir = normalize(ndc1 - ndc0);
    float2 perp = float2(-dir.y, dir.x);

    float thickness = (p0In.thickness + p1In.thickness) * 0.5;
    float thicknessNDC = thickness * 0.001;

    float2 offset = perp * thicknessNDC;
    float2 extension = dir * thicknessNDC;
    ndc0 -= extension;
    ndc1 += extension;

    FragmentInput v;
    v.objectType = 0;
    v.lineStyle  = objData.lineStyle;
    v.dashLength = objData.dashLength;
    v.gapLength  = objData.gapLength;
    v.antiAlias  = objData.antiAlias;
    v.smoothness = objData.smoothness;
    v.lit        = 0;

    // Triangle 1: bottom-left, top-left, bottom-right
    v.position = float4((ndc0 - offset) * p0.w, p0.z, p0.w);
    v.color = p0In.color;
    v.uv = float2(0.0, 0.0);
    v.distance = p0In.distance;
    outputStream.Append(v);

    v.position = float4((ndc0 + offset) * p0.w, p0.z, p0.w);
    v.color = p0In.color;
    v.uv = float2(0.0, 1.0);
    v.distance = p0In.distance;
    outputStream.Append(v);

    v.position = float4((ndc1 - offset) * p1.w, p1.z, p1.w);
    v.color = p1In.color;
    v.uv = float2(1.0, 0.0);
    v.distance = p1In.distance;
    outputStream.Append(v);

    outputStream.RestartStrip();

    // Triangle 2: top-left, top-right, bottom-right
    v.position = float4((ndc0 + offset) * p0.w, p0.z, p0.w);
    v.color = p0In.color;
    v.uv = float2(0.0, 1.0);
    v.distance = p0In.distance;
    outputStream.Append(v);

    v.position = float4((ndc1 + offset) * p1.w, p1.z, p1.w);
    v.color = p1In.color;
    v.uv = float2(1.0, 1.0);
    v.distance = p1In.distance;
    outputStream.Append(v);

    v.position = float4((ndc1 - offset) * p1.w, p1.z, p1.w);
    v.color = p1In.color;
    v.uv = float2(1.0, 0.0);
    v.distance = p1In.distance;
    outputStream.Append(v);

    outputStream.RestartStrip();
}

// Fragment shading for lines: dashing, dotting, and edge anti-aliasing
float4 shadeLine(FragmentInput input)
{
    float4 color = input.color;

    // Apply dashed/dotted pattern
    if (input.lineStyle == 1) // Dashed
    {
        float pattern = fmod(input.distance, input.dashLength + input.gapLength);
        if (pattern > input.dashLength)
            discard;
    }
    else if (input.lineStyle == 2) // Dotted
    {
        float dotSize = input.dashLength * 0.3;
        float pattern = fmod(input.distance, input.dashLength + input.gapLength);
        if (pattern > dotSize)
            discard;
    }

    // Anti-aliasing at edges
    if (input.antiAlias != 0)
    {
        float edgeDist = abs(input.uv.y - 0.5) * 2.0;
        float edgeFactor = 1.0 - smoothstep(1.0 - input.smoothness * 0.2, 1.0, edgeDist);
        color.a *= edgeFactor;

        if (color.a < 0.01)
            discard;
    }

    return color;
}
