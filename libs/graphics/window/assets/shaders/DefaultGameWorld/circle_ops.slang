// Circle volume operations for geometry expansion and fragment shading
import common;

// Emit a billboard quad for a circle volume, aspect-ratio corrected
void emitCircleQuad(
    VertexToGeom centerIn,
    LineObjectData objData,
    LinePushConstants pushConstants,
    inout TriangleStream<FragmentInput> outputStream)
{
    float4 center = centerIn.position;
    float2 ndcCenter = center.xy / center.w;

    // Perspective-correct radius: project world-space radius to NDC
    // projScale = proj[1][1] = 1/tan(fov/2), center.w = view-space depth
    float radiusNDC = objData.radius * pushConstants.projScale / center.w;

    // Aspect correction: NDC X spans [-1,1] mapped to a wider pixel range on
    // widescreen displays, so we shrink the X extent to keep the circle round.
    float radiusX = radiusNDC / pushConstants.aspectRatio;
    float radiusY = radiusNDC;

    FragmentInput v;
    v.objectType = 1;
    v.color      = centerIn.color;
    v.lineStyle  = 0;
    v.dashLength = 0;
    v.gapLength  = 0;
    v.antiAlias  = objData.antiAlias;
    v.smoothness = objData.smoothness;
    v.lit        = objData.lit;
    v.distance   = 0;

    // Triangle 1: bottom-left, top-left, bottom-right
    v.position = float4((ndcCenter + float2(-radiusX, -radiusY)) * center.w, center.z, center.w);
    v.uv = float2(-1.0, -1.0);
    outputStream.Append(v);

    v.position = float4((ndcCenter + float2(-radiusX, radiusY)) * center.w, center.z, center.w);
    v.uv = float2(-1.0, 1.0);
    outputStream.Append(v);

    v.position = float4((ndcCenter + float2(radiusX, -radiusY)) * center.w, center.z, center.w);
    v.uv = float2(1.0, -1.0);
    outputStream.Append(v);

    outputStream.RestartStrip();

    // Triangle 2: top-left, top-right, bottom-right
    v.position = float4((ndcCenter + float2(-radiusX, radiusY)) * center.w, center.z, center.w);
    v.uv = float2(-1.0, 1.0);
    outputStream.Append(v);

    v.position = float4((ndcCenter + float2(radiusX, radiusY)) * center.w, center.z, center.w);
    v.uv = float2(1.0, 1.0);
    outputStream.Append(v);

    v.position = float4((ndcCenter + float2(radiusX, -radiusY)) * center.w, center.z, center.w);
    v.uv = float2(1.0, -1.0);
    outputStream.Append(v);

    outputStream.RestartStrip();
}

// Fragment shading for sphere volumes: SDF with optional 3D lighting
float4 shadeCircle(FragmentInput input)
{
    float4 baseColor = input.color;

    float2 uv = input.uv;
    float r2 = dot(uv, uv);

    // Discard fragments outside the sphere silhouette
    if (r2 > 1.0)
        discard;

    float4 color;

    if (input.lit != 0)
    {
        // Reconstruct hemisphere normal pointing toward camera
        float z = sqrt(1.0 - r2);
        float3 normal = float3(uv.x, uv.y, z);

        // Fixed directional light (upper-right-front in view space)
        float3 lightDir = normalize(float3(0.4, 0.7, 0.6));

        // Ambient + Lambertian diffuse
        float ambient = 0.15;
        float diffuse = max(dot(normal, lightDir), 0.0);

        // Blinn-Phong specular (view direction is always +Z for a billboard)
        float3 viewDir = float3(0.0, 0.0, 1.0);
        float3 halfVec = normalize(lightDir + viewDir);
        float specular = pow(max(dot(normal, halfVec), 0.0), 32.0) * 0.5;

        float lighting = ambient + diffuse * 0.75 + specular;
        color = float4(baseColor.rgb * lighting, baseColor.a);
    }
    else
    {
        // Unlit: flat color
        color = baseColor;
    }

    // Anti-aliased edge
    float dist = sqrt(r2);
    if (input.antiAlias != 0)
    {
        float edgeSoftness = input.smoothness * 0.1 + 0.01;
        color.a *= 1.0 - smoothstep(1.0 - edgeSoftness, 1.0, dist);
    }

    if (color.a < 0.01)
        discard;

    return color;
}
