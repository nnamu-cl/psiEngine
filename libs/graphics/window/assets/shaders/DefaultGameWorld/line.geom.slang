// Geometry Shader — dispatches to per-object-type quad emitters
import common;
import line_ops;
import circle_ops;
import orbital_ops;

[[vk::binding(0, 1)]]
StructuredBuffer<LineObjectData> lineObjects;

[shader("geometry")]
[maxvertexcount(6)]
void main(
    line VertexToGeom input[2],
    inout TriangleStream<FragmentInput> outputStream,
    uint drawID : SV_DrawIndex,
    uniform LinePushConstants pushConstants)
{
    LineObjectData objData = lineObjects[drawID];

    if (objData.objectType == 2)
    {
        emitOrbitalQuad(input[0], objData, pushConstants, outputStream);
    }
    else if (objData.objectType == 1)
    {
        emitCircleQuad(input[0], objData, pushConstants, outputStream);
    }
    else
    {
        emitLineQuad(input[0], input[1], objData, outputStream);
    }
}
