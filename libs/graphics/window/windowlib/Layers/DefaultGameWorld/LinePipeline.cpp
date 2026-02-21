#include "LinePipeline.h"
#include "DefaultGameWorld.h"
#include "../../Data/LineProperties.h"
#include <vma/vk_mem_alloc.h>

#include <fstream>
#include <stdexcept>
#include <iostream>
#include <cstring>

#include "Components/LineRenderer.h"
#include "Components/Transform.h"
#include "slang/slang.h"
#include "slang/slang-com-ptr.h"

// Reuse the Slang session from Pipeline
extern Slang::ComPtr<slang::IGlobalSession>& getGlobalSlangSession();

Slang::ComPtr<slang::IGlobalSession>& LinePipeline::getSlangSession()
{
    static Slang::ComPtr<slang::IGlobalSession> globalSession;
    if (!globalSession)
    {
        if (slang::createGlobalSession(globalSession.writeRef()) != SLANG_OK)
        {
            std::cerr << "Failed to create Slang global session for LinePipeline\n";
        }
    }
    return globalSession;
}

void LinePipeline::DoRender(VkCommandBuffer cb, uint32_t frameIndex, float aspectRatio, DefaultGameWorldData &data) {






    // ============================================================
    // LINE RENDERING PASS
    // ============================================================
    // This block handles rendering all "LineRenderer" components in the scene.
    // The pipeline here is a 3-stage pipeline: Vertex → Geometry → Fragment.
    //
    // The overall flow is:
    //   1. CPU uploads line point data to a shared GPU vertex buffer (lineBuffer)
    //      This happens earlier in the frame (e.g. in an "upload" or "prepare" pass).
    //      lineGPUInfo stores per-object metadata: which byte offset in lineBuffer
    //      their vertices start at, and how many vertices they have.
    //
    //   2. We bind the pipeline and global descriptor set once for ALL line objects.
    //
    //   3. For each object we push per-object constants (color, style, transform etc.)
    //      and issue a draw call pointing into the correct region of lineBuffer.
    //
    // The geometry shader is key here: it receives LINE_STRIP segments (2 verts each)
    // and expands them into screen-aligned quads (2 triangles) so lines can have
    // configurable thickness. Without a geom shader, Vulkan lines are always 1px.
    // ============================================================

    if (data.lineBuffer != VK_NULL_HANDLE && !data.lineGPUInfo.empty()) {

        // Debug: log how many line objects exist on the very first frame only.
        // lineGPUInfo.size() == number of LineRenderer components that had valid
        // data when the upload pass ran. If this is 0, nothing was uploaded.
        static bool firstFrame = true;
        if (firstFrame) {
            std::cout << "Rendering lines: " << data.lineGPUInfo.size() << " line objects\n";
            firstFrame = false;
        }

        // --------------------------------------------------------
        // PIPELINE BIND
        // --------------------------------------------------------
        // Switching pipelines is one of the more expensive state changes in Vulkan.
        // We do it once here before looping over all line objects — all lines share
        // the same shader combination (line.vert → line.geom → line.frag).
        //
        // linePipeline.pipeline was created with:
        //   - VK_PRIMITIVE_TOPOLOGY_LINE_STRIP  (input assembly)
        //   - A geometry shader stage           (quad expansion)
        //   - Depth test enabled                (lines respect scene depth)
        //   - Likely alpha blending enabled     (for anti-aliased edges)
        // --------------------------------------------------------
        vkCmdBindPipeline(cb,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            data.linePipeline.pipeline);

        // --------------------------------------------------------
        // DESCRIPTOR SET BIND  (set = 0, the "global" set)
        // --------------------------------------------------------
        // Descriptor sets are how shaders access resources that aren't push constants:
        // textures, UBOs, SSBOs, samplers, etc.
        //
        // Set 0 is the "global" set — shared by ALL objects rendered this frame.
        // It typically contains things like:
        //   - Camera UBO (view/proj matrices, camera position)
        //   - Lighting data
        //   - Time / frame index
        //
        // globalSets[frameIndex] — because we double/triple-buffer descriptor sets
        // in flight to avoid CPU/GPU race conditions. Each in-flight frame has its
        // own copy of the global UBO so the CPU can update frame N+1 while the GPU
        // is still reading frame N.
        //
        // Note: we pass no dynamic offsets (last 2 args are 0, nullptr) meaning
        // all bindings in this set use static offsets baked at set-write time.
        // --------------------------------------------------------
        vkCmdBindDescriptorSets(cb,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            data.linePipeline.layout,
            0,                                          // firstSet = 0 (the global set slot)
            1,                                          // bind 1 set
            &data.resources.globalSets[frameIndex],     // the actual VkDescriptorSet handle
            0,
            nullptr);

        // lineIndex tracks our position in lineGPUInfo[].
        // lineGPUInfo is populated in the upload pass — it must be in the SAME ORDER
        // as we iterate scene.objects here, otherwise we'll match the wrong GPU data
        // to the wrong scene object. This is a common source of subtle rendering bugs.
        size_t lineIndex = 0;

        for (const auto &obj: data.scene.objects) {

            // --------------------------------------------------------
            // COMPONENT FETCH
            // --------------------------------------------------------
            // This engine uses an ECS-style component system. Each scene object
            // can have optional components attached. We need two:
            //   - LineRenderer: holds the point data and visual properties
            //   - Transform:    holds world-space position/rotation/scale
            //
            // get<T>() returns nullptr if the component isn't present.
            // --------------------------------------------------------
            const LineRenderer *lineRenderer = obj.components.get<LineRenderer>();

            // Skip objects that:
            //   - Have no LineRenderer component at all
            //   - Have a LineRenderer but it hasn't been given a data pointer yet
            //   - Have data but no actual points to draw (degenerate or cleared lines)
            // It's important NOT to increment lineIndex here because the upload pass
            // would have also skipped these — so there's no matching entry in lineGPUInfo.
            if (!lineRenderer || !lineRenderer->data || lineRenderer->data->points.empty()) {
                std::cout << "Skipping empty line renderer" << std::endl;
                continue;
            }

            const Transform *transform = obj.components.get<Transform>();
            if (!transform)
                continue;

            // Safety guard: if somehow more objects passed the filter than we uploaded,
            // don't run off the end of lineGPUInfo. This indicates a sync bug between
            // the upload pass and this render pass — they must use identical filtering.
            if (lineIndex >= data.lineGPUInfo.size())
                break;

            // --------------------------------------------------------
            // LINE GPU INFO
            // --------------------------------------------------------
            // LineGPUInfo was populated during the upload pass and contains:
            //   - vertexOffset: byte offset into lineBuffer where this object's
            //                   vertices begin (since all objects share one buffer)
            //   - vertexCount:  number of vertices uploaded for this object
            //
            // We consume one entry per valid line object, in lock-step with the
            // upload pass. The post-increment (lineIndex++) advances for next iteration.
            // --------------------------------------------------------
            const LineGPUInfo &lineInfo = data.lineGPUInfo[lineIndex++];

            // A line strip needs at least 2 points to form a single segment.
            // 1 point = nothing to draw. 0 should have been caught above, but
            // this is a safe secondary guard.
            if (lineInfo.vertexCount < 2)
                continue;

            // --------------------------------------------------------
            // PUSH CONSTANTS
            // --------------------------------------------------------
            // Push constants are the fastest way to pass small per-draw data to
            // shaders — they live directly in the command buffer, no descriptor
            // set or buffer binding needed. They're ideal for per-object data that
            // changes every draw call.
            //
            // The layout here must EXACTLY match the push_constant block declared
            // in both line.vert and line.geom (same offsets, same types, same size).
            // Vulkan is very strict about this — a mismatch causes undefined behavior
            // or validation errors.
            //
            // Total size here: 64+16+4+4+4+4+4+4+24 = 128 bytes.
            // Max guaranteed push constant size in Vulkan spec is only 128 bytes —
            // so this is right at the limit! Check maxPushConstantsSize at init time.
            // --------------------------------------------------------
            struct {
                glm::mat4 viewProj;      // 64 bytes — combined View*Projection matrix.
                                         // Applied in the vertex shader to transform
                                         // world-space points → clip space.
                                         // Note: no model matrix here — either lines are
                                         // in world space already, or it's baked into vertices.

                glm::vec4 globalColor;   // 16 bytes — RGBA base color for this line object.
                                         // The frag shader may multiply this with per-vertex
                                         // color if vertex colors are also in the buffer.

                float globalThickness;   // 4 bytes — set to 1.0f here (per-vertex thickness
                                         // takes precedence). The geometry shader uses this
                                         // to know how wide to expand each line segment into
                                         // a screen-space quad.

                float dashLength;        // 4 bytes — for dashed/dotted line styles.
                                         // Length (in some unit, likely world or screen space)
                                         // of the "on" portion of a dash pattern.

                float gapLength;         // 4 bytes — length of the "off" (gap) portion.
                                         // The frag shader uses dashLength + gapLength to
                                         // compute a repeating pattern along the line.

                uint32_t lineStyle;      // 4 bytes — enum: solid, dashed, dotted, etc.
                                         // Frag shader switches behaviour based on this value.

                uint32_t antiAlias;      // 4 bytes — 0 or 1. When enabled, the frag shader
                                         // fades alpha near the edges of the quad to create
                                         // soft anti-aliased line edges (requires blending).

                float smoothness;        // 4 bytes — controls the width of the AA feather
                                         // region. Higher = softer edges. Used in the
                                         // frag shader's smoothstep() call.

                uint32_t padding[6];     // 24 bytes — explicit padding to reach 128 bytes
                                         // (or to satisfy std430 alignment rules).
                                         // Must match padding in the shader's push_constant block.
            } linePushData;

            // Build the View-Projection matrix for this frame.
            // projectionMatrix(aspectRatio): builds a perspective or ortho matrix
            //   using the viewport's current aspect ratio to avoid stretching.
            // viewMatrix(): builds the camera's look-at / inverse-transform matrix.
            // Combined as Proj * View (NOT View * Proj) — standard column-major convention.
            linePushData.viewProj = data.camera.projectionMatrix(aspectRatio) * data.camera.viewMatrix();

            linePushData.globalColor     = lineRenderer->data->properties.color;
            linePushData.globalThickness = 1.0f; // Override: thickness driven per-vertex in buffer
            linePushData.dashLength      = lineRenderer->data->properties.dashLength;
            linePushData.gapLength       = lineRenderer->data->properties.gapLength;
            linePushData.lineStyle       = static_cast<uint32_t>(lineRenderer->data->properties.style);
            linePushData.antiAlias       = lineRenderer->data->properties.antiAlias ? 1u : 0u;
            linePushData.smoothness      = lineRenderer->data->properties.smoothness;
            std::memset(linePushData.padding, 0, sizeof(linePushData.padding));

            // Upload push constants into the command buffer.
            // VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_GEOMETRY_BIT means BOTH
            // stages can read from offset 0 with this same block.
            // The frag shader doesn't appear in this mask — if it needs color/style
            // data, either it gets it via the geom shader's output varyings, or a
            // separate push constant range was declared for it at pipeline layout creation.
            vkCmdPushConstants(cb,
                data.linePipeline.layout,
                VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_GEOMETRY_BIT,
                0,                   // offset into push constant range
                sizeof(linePushData),
                &linePushData);

            // --------------------------------------------------------
            // VERTEX BUFFER BIND
            // --------------------------------------------------------
            // All line objects share ONE large VkBuffer (lineBuffer), allocated
            // during the upload pass. Each object occupies a sub-range of it.
            //
            // lineInfo.vertexOffset is the BYTE offset where this object's
            // vertex data begins inside lineBuffer.
            //
            // Binding at binding slot 0 tells Vulkan: "when the vertex shader
            // reads from location=0 attributes, fetch from this buffer+offset."
            // The vertex attribute layout (stride, format, offset within vertex)
            // is defined at pipeline creation time in VkVertexInputAttributeDescription.
            // --------------------------------------------------------
            VkDeviceSize offsets[] = {lineInfo.vertexOffset};
            vkCmdBindVertexBuffers(cb,
                0,                   // first binding slot
                1,                   // number of buffers
                &data.lineBuffer,
                offsets);

            // --------------------------------------------------------
            // DRAW CALL
            // --------------------------------------------------------
            // Non-indexed draw: vertices are read sequentially from the bound
            // vertex buffer starting at the offset we just set.
            //
            // vertexCount: how many vertices to consume (lineInfo.vertexCount).
            //   With LINE_STRIP topology, the GPU emits (vertexCount - 1) line
            //   segments. Each segment = 2 adjacent vertices (v[i], v[i+1]).
            //   The geometry shader receives each segment and expands it into
            //   a quad (triangle strip of 4 verts, or 2 triangles = 6 verts).
            //
            // instanceCount = 1: no instancing — we draw one copy.
            //   (Could use instancing to draw the same line path multiple times
            //   with different transforms, but we're using push constants instead.)
            //
            // firstVertex = 0: start reading from the beginning of the bound
            //   buffer region (the offset already accounts for where we start).
            //
            // firstInstance = 0: irrelevant since instanceCount = 1.
            // --------------------------------------------------------
            vkCmdDraw(cb,
                lineInfo.vertexCount,  // vertices to process
                1,                     // instance count
                0,                     // firstVertex
                0);                    // firstInstance
        }
    }
    
}

bool LinePipeline::UploadLinesToGPU(DefaultGameWorldData& data, VmaAllocator allocator)
{
    std::cout << "uploadLinesToGPU called\n";

    // Collect vertex data from all LineRenderer components in the scene
    std::vector<std::vector<LineVertex>> allLineData;
    data.lineGPUInfo.clear();

    for (const auto& obj : data.scene.objects) {
        const LineRenderer* lineRenderer = obj.components.get<LineRenderer>();
        if (!lineRenderer || !lineRenderer->data)
            continue;

        std::cout << "Found LineRenderer on object: " << obj.name << "\n";
        allLineData.push_back(lineRenderer->buildVertexData());
    }

    std::cout << "Found " << allLineData.size() << " lines to upload\n";

    if (allLineData.empty())
        return true;

    // Calculate total buffer size
    VkDeviceSize totalSize = 0;
    for (const auto& lineData : allLineData)
        totalSize += lineData.size() * sizeof(LineVertex);

    if (totalSize == 0)
        return true;

    VkBufferCreateInfo bufferCI{
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size  = totalSize,
        .usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT
    };

    VmaAllocationCreateInfo allocCI{
        .flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT |
                 VMA_ALLOCATION_CREATE_MAPPED_BIT,
        .usage = VMA_MEMORY_USAGE_AUTO
    };

    VmaAllocationInfo allocInfo;
    if (vmaCreateBuffer(allocator, &bufferCI, &allocCI,
                        &data.lineBuffer, &data.lineBufferAllocation, &allocInfo) != VK_SUCCESS) {
        std::cerr << "Failed to create line buffer\n";
        return false;
    }

    char* bufferPtr = static_cast<char*>(allocInfo.pMappedData);
    VkDeviceSize offset = 0;

    for (const auto& lineData : allLineData) {
        VkDeviceSize dataSize = lineData.size() * sizeof(LineVertex);
        if (dataSize > 0)
            std::memcpy(bufferPtr + offset, lineData.data(), dataSize);

        data.lineGPUInfo.push_back({
            .vertexOffset = offset,
            .vertexCount  = static_cast<uint32_t>(lineData.size())
        });

        offset += dataSize;
    }

    std::cout << "Uploaded " << allLineData.size() << " lines to GPU ("
              << totalSize << " bytes)\n";

    return true;
}

VkShaderModule LinePipeline::loadShaderModule(VkDevice device, const std::string& path)
{
    std::cout << "Loading shader: " << path << std::endl;

    auto& globalSession = getSlangSession();
    if (!globalSession)
    {
        std::cerr << "No global session available\n";
        return VK_NULL_HANDLE;
    }

    // Configure target (SPIR-V 1.4)
    slang::TargetDesc targetDesc{
        .format = SLANG_SPIRV,
        .profile = globalSession->findProfile("spirv_1_4")
    };

    // Configure compiler options
    slang::CompilerOptionEntry options[] = {
        { slang::CompilerOptionName::EmitSpirvDirectly, {slang::CompilerOptionValueKind::Int, 1} }
    };

    // Create session descriptor
    slang::SessionDesc sessionDesc{
        .targets = &targetDesc,
        .targetCount = 1,
        .defaultMatrixLayoutMode = SLANG_MATRIX_LAYOUT_COLUMN_MAJOR,
        .compilerOptionEntries = options,
        .compilerOptionEntryCount = 1
    };

    Slang::ComPtr<slang::ISession> session;
    if (globalSession->createSession(sessionDesc, session.writeRef()) != SLANG_OK)
    {
        std::cerr << "Failed to create Slang session\n";
        return VK_NULL_HANDLE;
    }

    // Load shader module from source file
    Slang::ComPtr<slang::IBlob> diagnostics;
    Slang::ComPtr<slang::IModule> module{ session->loadModuleFromSource(
        "shader_module",
        path.c_str(),
        nullptr,
        diagnostics.writeRef()
    ) };

    // Print diagnostics if any
    if (diagnostics && diagnostics->getBufferSize() > 0)
    {
        std::cout << "Slang diagnostics for " << path << ":\n"
                  << (const char*)diagnostics->getBufferPointer() << std::endl;
    }

    if (!module)
    {
        std::cerr << "Failed to load shader module: " << path << "\n";
        return VK_NULL_HANDLE;
    }

    // Get SPIR-V code from module
    Slang::ComPtr<slang::IBlob> spirvCode;
    if (module->getTargetCode(0, spirvCode.writeRef()) != SLANG_OK || !spirvCode)
    {
        std::cerr << "Failed to get SPIR-V from module: " << path << "\n";
        return VK_NULL_HANDLE;
    }

    // Create VkShaderModule from SPIR-V binary
    VkShaderModuleCreateInfo moduleCI{
        .sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .codeSize = spirvCode->getBufferSize(),
        .pCode    = static_cast<const uint32_t*>(spirvCode->getBufferPointer())
    };

    VkShaderModule shaderModule = VK_NULL_HANDLE;
    if (vkCreateShaderModule(device, &moduleCI, nullptr, &shaderModule) != VK_SUCCESS)
    {
        std::cerr << "Failed to create VkShaderModule for: " << path << "\n";
        return VK_NULL_HANDLE;
    }

    return shaderModule;
}

bool LinePipeline::create(VkDevice device,
                          const LinePipelineDesc& desc,
                          const std::vector<VkDescriptorSetLayout>& setLayouts,
                          VkFormat colorFormat,
                          VkFormat depthFormat)
{
    // Load shader modules
    VkShaderModule vertModule = loadShaderModule(device, desc.vertexShaderPath);
    VkShaderModule geomModule = loadShaderModule(device, desc.geometryShaderPath);
    VkShaderModule fragModule = loadShaderModule(device, desc.fragmentShaderPath);

    if (vertModule == VK_NULL_HANDLE || geomModule == VK_NULL_HANDLE || fragModule == VK_NULL_HANDLE)
    {
        if (vertModule != VK_NULL_HANDLE) vkDestroyShaderModule(device, vertModule, nullptr);
        if (geomModule != VK_NULL_HANDLE) vkDestroyShaderModule(device, geomModule, nullptr);
        if (fragModule != VK_NULL_HANDLE) vkDestroyShaderModule(device, fragModule, nullptr);
        return false;
    }

    // Push constant range for line data
    // Layout: mat4 viewProj (64) + vec4 globalColor (16) + float globalThickness (4) +
    //         float dashLength (4) + float gapLength (4) + uint lineStyle (4) +
    //         uint antiAlias (4) + float smoothness (4) + padding (24) = 128 bytes
    VkPushConstantRange pushConstantRange{
        .stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_GEOMETRY_BIT,
        .offset     = 0,
        .size       = 128
    };

    // Create pipeline layout
    VkPipelineLayoutCreateInfo layoutCI{
        .sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .setLayoutCount         = static_cast<uint32_t>(setLayouts.size()),
        .pSetLayouts            = setLayouts.data(),
        .pushConstantRangeCount = 1,
        .pPushConstantRanges    = &pushConstantRange
    };

    if (vkCreatePipelineLayout(device, &layoutCI, nullptr, &layout) != VK_SUCCESS)
    {
        vkDestroyShaderModule(device, vertModule, nullptr);
        vkDestroyShaderModule(device, geomModule, nullptr);
        vkDestroyShaderModule(device, fragModule, nullptr);
        return false;
    }

    // Shader stages (vertex, geometry, fragment)
    VkPipelineShaderStageCreateInfo shaderStages[] = {
        {
            .sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage  = VK_SHADER_STAGE_VERTEX_BIT,
            .module = vertModule,
            .pName  = "main"
        },
        {
            .sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage  = VK_SHADER_STAGE_GEOMETRY_BIT,
            .module = geomModule,
            .pName  = "main"
        },
        {
            .sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage  = VK_SHADER_STAGE_FRAGMENT_BIT,
            .module = fragModule,
            .pName  = "main"
        }
    };

    // Vertex input: LineVertex (vec3 pos, vec4 color, float thickness, float distance)
    VkVertexInputBindingDescription vertexBinding{
        .binding   = 0,
        .stride    = 32,  // 12 + 16 + 4 + 4 bytes
        .inputRate = VK_VERTEX_INPUT_RATE_VERTEX
    };

    VkVertexInputAttributeDescription vertexAttributes[] = {
        { .location = 0, .binding = 0, .format = VK_FORMAT_R32G32B32_SFLOAT,    .offset = 0 },   // position
        { .location = 1, .binding = 0, .format = VK_FORMAT_R32G32B32A32_SFLOAT, .offset = 12 },  // color
        { .location = 2, .binding = 0, .format = VK_FORMAT_R32_SFLOAT,          .offset = 28 },  // thickness
        { .location = 3, .binding = 0, .format = VK_FORMAT_R32_SFLOAT,          .offset = 32 }   // distance (actually offset 28+4=32, but size is 32 total, this is wrong)
    };

    // Fix: recalculate offsets
    // position: 3 floats = 12 bytes, offset 0
    // color: 4 floats = 16 bytes, offset 12
    // thickness: 1 float = 4 bytes, offset 28
    // distance: 1 float = 4 bytes, offset 32
    // Total: 36 bytes (not 32!)

    vertexBinding.stride = 36;
    vertexAttributes[3].offset = 32;

    VkPipelineVertexInputStateCreateInfo vertexInputState{
        .sType                           = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .vertexBindingDescriptionCount   = 1,
        .pVertexBindingDescriptions      = &vertexBinding,
        .vertexAttributeDescriptionCount = 4,
        .pVertexAttributeDescriptions    = vertexAttributes
    };

    // Input assembly (line topology)
    VkPipelineInputAssemblyStateCreateInfo inputAssembly{
        .sType    = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .topology = desc.topology
    };

    // Dynamic state
    VkDynamicState dynamicStates[] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
    VkPipelineDynamicStateCreateInfo dynamicState{
        .sType             = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
        .dynamicStateCount = 2,
        .pDynamicStates    = dynamicStates
    };

    VkPipelineViewportStateCreateInfo viewportState{
        .sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .viewportCount = 1,
        .scissorCount  = 1
    };

    // Rasterization (no culling for lines)
    VkPipelineRasterizationStateCreateInfo rasterization{
        .sType       = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        .polygonMode = VK_POLYGON_MODE_FILL,
        .cullMode    = VK_CULL_MODE_NONE,
        .frontFace   = VK_FRONT_FACE_COUNTER_CLOCKWISE,
        .lineWidth   = 1.0f
    };

    // Multisampling (none)
    VkPipelineMultisampleStateCreateInfo multisampling{
        .sType                = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT
    };

    // Depth/stencil
    VkPipelineDepthStencilStateCreateInfo depthStencil{
        .sType            = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
        .depthTestEnable  = desc.depthTest ? VK_TRUE : VK_FALSE,
        .depthWriteEnable = desc.depthTest ? VK_TRUE : VK_FALSE,
        .depthCompareOp   = VK_COMPARE_OP_LESS_OR_EQUAL
    };

    // Color blending (alpha blending for smooth lines)
    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                                          VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_TRUE;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

    VkPipelineColorBlendStateCreateInfo colorBlending{
        .sType           = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .attachmentCount = 1,
        .pAttachments    = &colorBlendAttachment
    };

    // Dynamic rendering
    VkPipelineRenderingCreateInfo renderingCI{
        .sType                   = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
        .colorAttachmentCount    = 1,
        .pColorAttachmentFormats = &colorFormat,
        .depthAttachmentFormat   = depthFormat
    };

    // Create graphics pipeline
    VkGraphicsPipelineCreateInfo pipelineCI{
        .sType               = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .pNext               = &renderingCI,
        .stageCount          = 3,  // vertex, geometry, fragment
        .pStages             = shaderStages,
        .pVertexInputState   = &vertexInputState,
        .pInputAssemblyState = &inputAssembly,
        .pViewportState      = &viewportState,
        .pRasterizationState = &rasterization,
        .pMultisampleState   = &multisampling,
        .pDepthStencilState  = &depthStencil,
        .pColorBlendState    = &colorBlending,
        .pDynamicState       = &dynamicState,
        .layout              = layout
    };

    VkResult result = vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineCI, nullptr, &pipeline);

    // Destroy shader modules
    vkDestroyShaderModule(device, vertModule, nullptr);
    vkDestroyShaderModule(device, geomModule, nullptr);
    vkDestroyShaderModule(device, fragModule, nullptr);

    return result == VK_SUCCESS;
}

void LinePipeline::destroy(VkDevice device)
{
    if (pipeline != VK_NULL_HANDLE)
        vkDestroyPipeline(device, pipeline, nullptr);
    if (layout != VK_NULL_HANDLE)
        vkDestroyPipelineLayout(device, layout, nullptr);

    pipeline = VK_NULL_HANDLE;
    layout   = VK_NULL_HANDLE;
}
