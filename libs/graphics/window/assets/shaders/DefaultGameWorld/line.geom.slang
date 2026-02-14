// Line Geometry Shader
// Expands line vertices into thick quads with proper caps and joins

struct VertexInput
{
    float4 position : SV_Position;
    float4 color : COLOR;
    float thickness : THICKNESS;
    float distance : DISTANCE;
};

struct FragmentInput
{
    float4 position : SV_Position;
    float4 color : COLOR;
    float2 uv : TEXCOORD0;  // UV for anti-aliasing (0=center, 1=edge)
    float distance : DISTANCE;
    uint lineStyle : LINESTYLE;
    float dashLength : DASHLENGTH;
    float gapLength : GAPLENGTH;
    uint antiAlias : ANTIALIAS;
    float smoothness : SMOOTHNESS;
};

struct LinePushConstants
{
    float4x4 viewProj;
    float4 globalColor;
    float globalThickness;
    float dashLength;
    float gapLength;
    uint lineStyle;
    uint antiAlias;
    float smoothness;
};

[shader("geometry")]
[maxvertexcount(6)]
void main(
    line VertexInput input[2],
    inout TriangleStream<FragmentInput> outputStream,
    uniform LinePushConstants pushConstants)
{
    // Get the two line endpoints in clip space
    float4 p0 = input[0].position;
    float4 p1 = input[1].position;

    // Convert to NDC for screen-space calculations
    float2 ndc0 = p0.xy / p0.w;
    float2 ndc1 = p1.xy / p1.w;

    // Calculate line direction in screen space
    float2 dir = normalize(ndc1 - ndc0);

    // Calculate perpendicular vector (for thickness)
    float2 perp = float2(-dir.y, dir.x);

    // Average thickness of the two vertices
    float thickness = (input[0].thickness + input[1].thickness) * 0.5;

    // Convert thickness from pixels to NDC
    // For 1920x1080: 1 pixel = 2.0 / 1080 = 0.00185 NDC (height)
    // Using a larger multiplier for better visibility
    float thicknessNDC = thickness * 0.001;  // Pixel-to-NDC conversion

    // Offset for thickness (perpendicular to line)
    float2 offset = perp * thicknessNDC;

    // Extend the line segment along its direction to close gaps at joins
    // Extend by the thickness to ensure overlap
    float2 extension = dir * thicknessNDC;
    ndc0 -= extension;  // Extend start backwards
    ndc1 += extension;  // Extend end forwards

    // Generate quad (two triangles)
    FragmentInput v;

    // Pass through push constant values to all vertices
    v.lineStyle = pushConstants.lineStyle;
    v.dashLength = pushConstants.dashLength;
    v.gapLength = pushConstants.gapLength;
    v.antiAlias = pushConstants.antiAlias;
    v.smoothness = pushConstants.smoothness;

    // Vertex 0: Bottom-left
    v.position = float4((ndc0 - offset) * p0.w, p0.z, p0.w);
    v.color = input[0].color;
    v.uv = float2(0.0, 0.0);
    v.distance = input[0].distance;
    outputStream.Append(v);

    // Vertex 1: Top-left
    v.position = float4((ndc0 + offset) * p0.w, p0.z, p0.w);
    v.color = input[0].color;
    v.uv = float2(0.0, 1.0);
    v.distance = input[0].distance;
    outputStream.Append(v);

    // Vertex 2: Bottom-right
    v.position = float4((ndc1 - offset) * p1.w, p1.z, p1.w);
    v.color = input[1].color;
    v.uv = float2(1.0, 0.0);
    v.distance = input[1].distance;
    outputStream.Append(v);

    outputStream.RestartStrip();

    // Second triangle
    // Vertex 0: Top-left
    v.position = float4((ndc0 + offset) * p0.w, p0.z, p0.w);
    v.color = input[0].color;
    v.uv = float2(0.0, 1.0);
    v.distance = input[0].distance;
    outputStream.Append(v);

    // Vertex 1: Top-right
    v.position = float4((ndc1 + offset) * p1.w, p1.z, p1.w);
    v.color = input[1].color;
    v.uv = float2(1.0, 1.0);
    v.distance = input[1].distance;
    outputStream.Append(v);

    // Vertex 2: Bottom-right
    v.position = float4((ndc1 - offset) * p1.w, p1.z, p1.w);
    v.color = input[1].color;
    v.uv = float2(1.0, 0.0);
    v.distance = input[1].distance;
    outputStream.Append(v);

    outputStream.RestartStrip();
}
