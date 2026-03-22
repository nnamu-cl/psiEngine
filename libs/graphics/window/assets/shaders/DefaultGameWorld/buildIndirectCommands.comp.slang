// Compute shader: builds VkDrawIndirectCommand from LineGPUInfo
// One thread per line object

struct LineGPUInfo
{
    uint64_t vertexOffset;  // byte offset in line vertex buffer
    uint     vertexCount;
};

struct DrawIndirectCommand
{
    uint vertexCount;
    uint instanceCount;
    uint firstVertex;
    uint firstInstance;
};

[[vk::binding(0, 0)]]
StructuredBuffer<LineGPUInfo> lineGPUInfo;

[[vk::binding(1, 0)]]
RWStructuredBuffer<DrawIndirectCommand> indirectCommands;

[shader("compute")]
[numthreads(64, 1, 1)]
void main(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint i = dispatchThreadID.x;

    if (i >= lineGPUInfo.getCount())
        return;

    DrawIndirectCommand cmd;
    cmd.vertexCount   = lineGPUInfo[i].vertexCount;
    cmd.instanceCount = 1;
    cmd.firstVertex   = uint(lineGPUInfo[i].vertexOffset / 36);  // sizeof(LineVertex) == 36
    cmd.firstInstance = 0;

    indirectCommands[i] = cmd;
}
