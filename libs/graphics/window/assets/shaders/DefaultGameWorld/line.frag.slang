// Fragment Shader — dispatches to per-object-type shading
import common;
import line_ops;
import circle_ops;
import orbital_ops;

[shader("fragment")]
float4 main(FragmentInput input, uniform LinePushConstants pushConstants) : SV_Target
{
    if (input.objectType == 2)
    {
        return shadeOrbital(input, pushConstants);
    }
    else if (input.objectType == 1)
    {
        return shadeCircle(input);
    }
    else
    {
        return shadeLine(input);
    }
}
