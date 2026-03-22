#include "LinePipeline.h"
#include "DefaultGameWorld.h"
#include "../../Data/LineProperties.h"
#include <vma/vk_mem_alloc.h>

#include <fstream>
#include <stdexcept>
#include <iostream>
#include <cstring>

#include "ApplicationWindow.h"
#include "Components/LineRenderer.h"
#include "Components/Transform.h"
#include "slang/slang.h"
#include "slang/slang-com-ptr.h"

// Reuse the Slang session from Pipeline
extern Slang::ComPtr<slang::IGlobalSession> &getGlobalSlangSession();

Slang::ComPtr<slang::IGlobalSession> &LinePipeline::getSlangSession() {
    static Slang::ComPtr<slang::IGlobalSession> globalSession;
    if (!globalSession) {
        if (slang::createGlobalSession(globalSession.writeRef()) != SLANG_OK) {
            std::cerr << "Failed to create Slang global session for LinePipeline\n";
        }
    }
    return globalSession;
}

bool LinePipeline::createSSBODescriptorInfrastructure(VkDevice device) {
    std::cout << "[SSBO] Creating descriptor set layout...\n";
    VkDescriptorSetLayoutBinding ssboBinding{
        .binding = 0,
        .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
        .descriptorCount = 1,
        .stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_GEOMETRY_BIT
    };
    VkDescriptorSetLayoutCreateInfo layoutCI{
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .bindingCount = 1,
        .pBindings = &ssboBinding
    };

    VkResult r = vkCreateDescriptorSetLayout(device, &layoutCI, nullptr, &ssboSetLayout);
    if (r != VK_SUCCESS) { std::cerr << "[SSBO] vkCreateDescriptorSetLayout failed: " << r << "\n"; return false; }
    std::cout << "[SSBO] Layout created.\n";

    VkDescriptorPoolSize poolSize{
        .type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
        .descriptorCount = 1
    };
    VkDescriptorPoolCreateInfo poolCI{
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .maxSets = 1,
        .poolSizeCount = 1,
        .pPoolSizes = &poolSize
    };

    r = vkCreateDescriptorPool(device, &poolCI, nullptr, &ssboPool);
    if (r != VK_SUCCESS) { std::cerr << "[SSBO] vkCreateDescriptorPool failed: " << r << "\n"; return false; }
    std::cout << "[SSBO] Pool created.\n";

    VkDescriptorSetAllocateInfo allocInfo{
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .descriptorPool = ssboPool,
        .descriptorSetCount = 1,
        .pSetLayouts = &ssboSetLayout
    };

    r = vkAllocateDescriptorSets(device, &allocInfo, &ssboSet);
    if (r != VK_SUCCESS) { std::cerr << "[SSBO] vkAllocateDescriptorSets failed: " << r << "\n"; return false; }
    std::cout << "[SSBO] Descriptor set allocated. ssboSet=" << ssboSet << "\n";

    return true;
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
                                0, // firstSet = 0 (the global set slot)
                                1, // bind 1 set
                                &data.resources.globalSets[frameIndex], // the actual VkDescriptorSet handle
                                0,
                                nullptr);

        // Bind set 1 — the line SSBO (per-object color, style, thickness etc.)
        vkCmdBindDescriptorSets(cb,
                                VK_PIPELINE_BIND_POINT_GRAPHICS,
                                data.linePipeline.layout,
                                1, // firstSet = 1
                                1,
                                &ssboSet,
                                0, nullptr);


        // Dispatch compute shader to build indirect draw commands on the GPU
        // The compute shader reads LineGPUInfo[] and writes VkDrawIndirectCommand[]
        {
            uint32_t lineCount = static_cast<uint32_t>(data.lineGPUInfo.size());
            uint32_t groupCount = (lineCount + 63) / 64; // 64 threads per workgroup
            indirectCompute.RecordCommandBuffer(computeDescSet, cb, groupCount, 1, 1);

            // Memory barrier: compute shader writes → indirect draw reads
            VkMemoryBarrier barrier{
                .sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER,
                .srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT,
                .dstAccessMask = VK_ACCESS_INDIRECT_COMMAND_READ_BIT
            };
            vkCmdPipelineBarrier(cb,
                                 VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                                 VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT,
                                 0, 1, &barrier, 0, nullptr, 0, nullptr);
        }


        // Push viewProj once — the only remaining push constant
        struct {
            glm::mat4 viewProj;
        } pushData;
        pushData.viewProj = data.camera.projectionMatrix(aspectRatio) * data.camera.viewMatrix();
        vkCmdPushConstants(cb,
                           data.linePipeline.layout,
                           VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_GEOMETRY_BIT,
                           0, sizeof(pushData), &pushData);

        // ============================================================
        // STEP 4: BIND VERTEX BUFFER ONCE + INDIRECT DRAW LOOP
        // ============================================================
        // OLD approach: vkCmdBindVertexBuffers called per-object with a byte offset,
        //   meaning the GPU saw the buffer starting at that object's data each time.
        //
        // NEW approach: bind the shared vertex buffer ONCE at offset 0.
        //   The firstVertex field inside each VkDrawIndirectCommand tells the GPU
        //   which vertex INDEX to start reading from inside that single binding.
        //   firstVertex = byte_offset / sizeof(LineVertex) -- set when we populated
        //   indirectLineDrawCommands above.
        //
        // This is already a meaningful improvement: N vertex buffer binds -> 1.
        // ============================================================
        VkDeviceSize zero = 0;
        vkCmdBindVertexBuffers(cb, 0, 1, &data.lineBuffer, &zero);

        // Per-object data (color, style, thickness etc.) is now in the SSBO bound at set 1.
        // gl_DrawID in the shader indexes into it automatically per draw command.
        // One call handles all objects.
        vkCmdDrawIndirect(cb,
                          indirectBuffer,
                          0, // start from the beginning
                          data.lineGPUInfo.size(), // all objects in one call
                          sizeof(VkDrawIndirectCommand)); // stride between commands
    }
}

std::vector<std::vector<LineVertex> > collectLineVertexBatches(const DefaultGameWorldData &data) {
    std::vector<std::vector<LineVertex> > allLineData;
    for (const auto &obj: data.scene.objects) {
        const LineRenderer *lineRenderer = obj.components.get<LineRenderer>();
        if (!lineRenderer || !lineRenderer->data)
            continue;
        std::cout << "Found LineRenderer on object: " << obj.name << "\n";
        allLineData.push_back(lineRenderer->buildVertexData());
    }
    std::cout << "Found " << allLineData.size() << " lines to upload\n";
    return allLineData;
}


bool LinePipeline::UploadLinesToGPU(DefaultGameWorldData &data, VmaAllocator allocator) {
    if (ssboSet == VK_NULL_HANDLE) {
        // Pipeline not initialized yet — data is in the scene and will be uploaded at end of OnAttach
        return true;
    }
    std::cout << "[Upload] ssboSet=" << ssboSet << " lineSSBO=" << lineSSBO << "\n";
    // Collect vertex data from all LineRenderer components in the scene
    std::vector<std::vector<LineVertex> > allLineData = collectLineVertexBatches(data);
    if (allLineData.empty())
        return true;

    // Calculate total buffer size
    VkDeviceSize totalSize = 0;
    for (const std::vector<LineVertex> &lineData: allLineData) // loop through all line objects
        totalSize += lineData.size() * sizeof(LineVertex);

    std::cout << "[Upload] totalSize=" << totalSize << "\n";
    if (totalSize == 0)
        return true;


    // VERTEX LINE BUFFER =====================================================

    VkBufferCreateInfo bufferCI{
        //prepare to create a vertex buffer of the size we need
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = totalSize,
        .usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT
    };

    VmaAllocationCreateInfo allocCI{
        //prepare the allocation for buffer
        .flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT |
                 VMA_ALLOCATION_CREATE_MAPPED_BIT,
        .usage = VMA_MEMORY_USAGE_AUTO
    };

    std::cout << "[Upload] Creating vertex buffer...\n";
    VmaAllocationInfo allocInfo;
    if (vmaCreateBuffer(allocator, &bufferCI, &allocCI,
                        &data.lineBuffer, &data.lineBufferAllocation,
                        &allocInfo) != VK_SUCCESS) //allocate memory to store the buffer
    {
        std::cerr << "Failed to create line buffer\n";
        return false;
    }
    std::cout << "[Upload] Vertex buffer created. pMappedData=" << allocInfo.pMappedData << "\n";





    char *bufferPtr = static_cast<char *>(allocInfo.pMappedData); //get a pointer to the start of the memory
    VkDeviceSize offset = 0; // this the offset per line data

    for (const std::vector<LineVertex> &lineData: allLineData) {
        VkDeviceSize dataSize = lineData.size() * sizeof(LineVertex);
        // fine out how much we need for this specific line
        if (dataSize > 0) // copy this data, starting at the next available location, into the memory
            std::memcpy(bufferPtr + offset, lineData.data(), dataSize);

        data.lineGPUInfo.push_back({
            //save this line data for use later on when rendering
            .vertexOffset = offset, // this offset shows where in the line data buffer this line starts from
            .vertexCount = static_cast<uint32_t>(lineData.size())
        });

        offset += dataSize;
    }



    //========================= Create indirect buffer
    // Destroy old indirect buffer if it exists from a previous upload
    if (indirectBuffer != VK_NULL_HANDLE) {
        vmaDestroyBuffer(allocator, indirectBuffer, indirectBufferAllocation);
        indirectBuffer = VK_NULL_HANDLE;
        indirectBufferAllocation = VK_NULL_HANDLE;
    }

    //Size the indirect buffer for the exact objects as we are planning to render
    // Usage includes STORAGE_BUFFER_BIT so the compute shader can write to it
    VkBufferCreateInfo indirectCI{
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = sizeof(VkDrawIndirectCommand) * allLineData.size(),
        .usage = VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT
    };
    VmaAllocationCreateInfo indirectAllocCI{
        .usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE
    };


    std::cout << "[Upload] Creating indirect buffer...\n";
    if (vmaCreateBuffer(allocator, &indirectCI, &indirectAllocCI, &indirectBuffer, &indirectBufferAllocation, nullptr)
        != VK_SUCCESS) {
        std::cerr << "Failed to create indirect allocation \n";
        return false;
        }
    std::cout << "[Upload] Indirect buffer created.\n";


    //========================= Create LineGPUInfo GPU buffer (input to compute shader)
    if (lineGPUInfoBuffer != VK_NULL_HANDLE) {
        vmaDestroyBuffer(allocator, lineGPUInfoBuffer, lineGPUInfoBufferAllocation);
        lineGPUInfoBuffer = VK_NULL_HANDLE;
        lineGPUInfoBufferAllocation = VK_NULL_HANDLE;
    }

    VkBufferCreateInfo gpuInfoCI{
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = sizeof(LineGPUInfo) * data.lineGPUInfo.size(),
        .usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT
    };
    VmaAllocationCreateInfo gpuInfoAllocCI{
        .flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT,
        .usage = VMA_MEMORY_USAGE_AUTO
    };
    VmaAllocationInfo gpuInfoAllocInfo;
    std::cout << "[Upload] Creating LineGPUInfo GPU buffer...\n";
    if (vmaCreateBuffer(allocator, &gpuInfoCI, &gpuInfoAllocCI, &lineGPUInfoBuffer, &lineGPUInfoBufferAllocation, &gpuInfoAllocInfo)
        != VK_SUCCESS) {
        std::cerr << "Failed to create LineGPUInfo buffer\n";
        return false;
    }
    // Copy the CPU lineGPUInfo into the GPU buffer
    std::memcpy(gpuInfoAllocInfo.pMappedData, data.lineGPUInfo.data(), sizeof(LineGPUInfo) * data.lineGPUInfo.size());
    std::cout << "[Upload] LineGPUInfo buffer created with " << data.lineGPUInfo.size() << " entries.\n";

    // Initialize compute pipeline if not done yet
    if (!computeInitialized) {
        if (!initComputePipeline(data.windowData->device)) {
            std::cerr << "Failed to initialize compute pipeline\n";
            return false;
        }
    }

    // Update compute descriptor sets to point at the new buffers
    indirectCompute.UpdateDescSets(computeDescSet,
                                   lineGPUInfoBuffer, sizeof(LineGPUInfo) * data.lineGPUInfo.size(),
                                   indirectBuffer, sizeof(VkDrawIndirectCommand) * allLineData.size());



    // INDIRECT LINE SSBO =====================================================

    //Clean up any prior ssbo
    if (lineSSBO != VK_NULL_HANDLE) {
        vmaDestroyBuffer(allocator, lineSSBO, lineSSBOAllocation);
        lineSSBO = VK_NULL_HANDLE;
        lineSSBOAllocation = VK_NULL_HANDLE;
    }


    //One LineObjectData per line object
    VkBufferCreateInfo ssboCI{
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = sizeof(LineObjectData) * allLineData.size(),
        .usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT
    };

    VmaAllocationCreateInfo ssboAllocCI{
        .flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT,
        .usage = VMA_MEMORY_USAGE_AUTO
    };
    std::cout << "[Upload] Creating SSBO buffer...\n";
    VmaAllocationInfo ssboAllocInfo;
    if (vmaCreateBuffer(allocator, &ssboCI, &ssboAllocCI, &lineSSBO, &lineSSBOAllocation, &ssboAllocInfo) != VK_SUCCESS) {
        std::cerr << "[Upload] Failed to create SSBO buffer\n";
        return false;
    }
    std::cout << "[Upload] SSBO created. pMappedData=" << ssboAllocInfo.pMappedData << "\n";


    // Populate — one entry per line object, same iteration order as vertex upload
    // so index i here matches gl_DrawID == i in the shader
    LineObjectData *ssboPtr = static_cast<LineObjectData *>(ssboAllocInfo.pMappedData);
    uint32_t ssboIndex = 0;
    for (const auto &obj: data.scene.objects) {
        const LineRenderer *lr = obj.components.get<LineRenderer>();
        if (!lr || !lr->data || lr->data->points.empty()) continue;

        ssboPtr[ssboIndex++] = LineObjectData{
            .color = lr->data->properties.color,
            .thickness = 1.0f,
            .dashLength = lr->data->properties.dashLength,
            .gapLength = lr->data->properties.gapLength,
            .lineStyle = static_cast<uint32_t>(lr->data->properties.style),
            .antiAlias = lr->data->properties.antiAlias ? 1u : 0u,
            .smoothness = lr->data->properties.smoothness
        };
    }

    // Point the descriptor set at the new SSBO buffer
    VkDescriptorBufferInfo ssboBufferInfo{
        .buffer = lineSSBO,
        .offset = 0,
        .range = sizeof(LineObjectData) * allLineData.size()
    };

    VkWriteDescriptorSet ssboWrite{
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .dstSet = ssboSet,
        .dstBinding = 0,
        .descriptorCount = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
        .pBufferInfo = &ssboBufferInfo
    };

    std::cout << "[Upload] Calling vkUpdateDescriptorSets. ssboSet=" << ssboSet << "\n";
    vkUpdateDescriptorSets(data.windowData->device, 1, &ssboWrite, 0, nullptr);
    std::cout << "[Upload] Done.\n";

    return true;
}

bool LinePipeline::initComputePipeline(VkDevice device) {
    // Create descriptor pool for the compute pipeline (2 storage buffers, 1 set)
    VkDescriptorPoolSize poolSize{
        .type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
        .descriptorCount = 2
    };
    VkDescriptorPoolCreateInfo poolCI{
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .maxSets = 1,
        .poolSizeCount = 1,
        .pPoolSizes = &poolSize
    };
    if (vkCreateDescriptorPool(device, &poolCI, nullptr, &computeDescPool) != VK_SUCCESS) {
        std::cerr << "Failed to create compute descriptor pool\n";
        return false;
    }

    // Init the compute pipeline (creates layout, compiles shader, creates pipeline)
    indirectCompute.Init(device, computeDescPool,
                         "libs/graphics/window/assets/shaders/DefaultGameWorld/buildIndirectCommands.comp.slang");

    // Allocate a single descriptor set for the compute pipeline
    std::vector<VkDescriptorSet> sets;
    indirectCompute.AllocDescSets(sets);
    computeDescSet = sets[0];

    computeInitialized = true;
    std::cout << "[Compute] Indirect command compute pipeline initialized.\n";
    return true;
}

VkShaderModule LinePipeline::loadShaderModule(VkDevice device, const std::string &path) {
    std::cout << "Loading shader: " << path << std::endl;

    auto &globalSession = getSlangSession();
    if (!globalSession) {
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
        {slang::CompilerOptionName::EmitSpirvDirectly, {slang::CompilerOptionValueKind::Int, 1}}
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
    if (globalSession->createSession(sessionDesc, session.writeRef()) != SLANG_OK) {
        std::cerr << "Failed to create Slang session\n";
        return VK_NULL_HANDLE;
    }

    // Load shader module from source file
    Slang::ComPtr<slang::IBlob> diagnostics;
    Slang::ComPtr<slang::IModule> module{
        session->loadModuleFromSource(
            "shader_module",
            path.c_str(),
            nullptr,
            diagnostics.writeRef()
        )
    };

    // Print diagnostics if any
    if (diagnostics && diagnostics->getBufferSize() > 0) {
        std::cout << "Slang diagnostics for " << path << ":\n"
                << (const char *) diagnostics->getBufferPointer() << std::endl;
    }

    if (!module) {
        std::cerr << "Failed to load shader module: " << path << "\n";
        return VK_NULL_HANDLE;
    }

    // Get SPIR-V code from module
    Slang::ComPtr<slang::IBlob> spirvCode;
    if (module->getTargetCode(0, spirvCode.writeRef()) != SLANG_OK || !spirvCode) {
        std::cerr << "Failed to get SPIR-V from module: " << path << "\n";
        return VK_NULL_HANDLE;
    }

    // Create VkShaderModule from SPIR-V binary
    VkShaderModuleCreateInfo moduleCI{
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .codeSize = spirvCode->getBufferSize(),
        .pCode = static_cast<const uint32_t *>(spirvCode->getBufferPointer())
    };

    VkShaderModule shaderModule = VK_NULL_HANDLE;
    if (vkCreateShaderModule(device, &moduleCI, nullptr, &shaderModule) != VK_SUCCESS) {
        std::cerr << "Failed to create VkShaderModule for: " << path << "\n";
        return VK_NULL_HANDLE;
    }

    return shaderModule;
}

bool LinePipeline::create(VkDevice device,
                          const LinePipelineDesc &desc,
                          const std::vector<VkDescriptorSetLayout> &setLayouts,
                          VkFormat colorFormat,
                          VkFormat depthFormat) {
    // Load shader modules
    VkShaderModule vertModule = loadShaderModule(device, desc.vertexShaderPath);
    VkShaderModule geomModule = loadShaderModule(device, desc.geometryShaderPath);
    VkShaderModule fragModule = loadShaderModule(device, desc.fragmentShaderPath);

    if (vertModule == VK_NULL_HANDLE || geomModule == VK_NULL_HANDLE || fragModule == VK_NULL_HANDLE) {
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
        .offset = 0,
        .size = 128
    };

    //Make sure we include our own line pipeline set
    if (!createSSBODescriptorInfrastructure(device))
        return false;

    //Build the full layout list: set 0 =  global (camera), set 1  = line SSBO
    std::vector<VkDescriptorSetLayout> fullLayouts = setLayouts;
    fullLayouts.push_back(ssboSetLayout);


    // Create pipeline layout
    VkPipelineLayoutCreateInfo layoutCI{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .setLayoutCount = static_cast<uint32_t>(fullLayouts.size()),
        .pSetLayouts = fullLayouts.data(),
        .pushConstantRangeCount = 1,
        .pPushConstantRanges = &pushConstantRange
    };


    if (vkCreatePipelineLayout(device, &layoutCI, nullptr, &layout) != VK_SUCCESS) {
        vkDestroyShaderModule(device, vertModule, nullptr);
        vkDestroyShaderModule(device, geomModule, nullptr);
        vkDestroyShaderModule(device, fragModule, nullptr);
        return false;
    }

    // Shader stages (vertex, geometry, fragment)
    VkPipelineShaderStageCreateInfo shaderStages[] = {
        {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage = VK_SHADER_STAGE_VERTEX_BIT,
            .module = vertModule,
            .pName = "main"
        },
        {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage = VK_SHADER_STAGE_GEOMETRY_BIT,
            .module = geomModule,
            .pName = "main"
        },
        {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
            .module = fragModule,
            .pName = "main"
        }
    };

    // Vertex input: LineVertex (vec3 pos, vec4 color, float thickness, float distance)
    VkVertexInputBindingDescription vertexBinding{
        .binding = 0,
        .stride = 32, // 12 + 16 + 4 + 4 bytes
        .inputRate = VK_VERTEX_INPUT_RATE_VERTEX
    };

    VkVertexInputAttributeDescription vertexAttributes[] = {
        {.location = 0, .binding = 0, .format = VK_FORMAT_R32G32B32_SFLOAT, .offset = 0}, // position
        {.location = 1, .binding = 0, .format = VK_FORMAT_R32G32B32A32_SFLOAT, .offset = 12}, // color
        {.location = 2, .binding = 0, .format = VK_FORMAT_R32_SFLOAT, .offset = 28}, // thickness
        {.location = 3, .binding = 0, .format = VK_FORMAT_R32_SFLOAT, .offset = 32}
        // distance (actually offset 28+4=32, but size is 32 total, this is wrong)
    };


    vertexBinding.stride = 36;
    vertexAttributes[3].offset = 32;

    VkPipelineVertexInputStateCreateInfo vertexInputState{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .vertexBindingDescriptionCount = 1,
        .pVertexBindingDescriptions = &vertexBinding,
        .vertexAttributeDescriptionCount = 4,
        .pVertexAttributeDescriptions = vertexAttributes
    };

    // Input assembly (line topology)
    VkPipelineInputAssemblyStateCreateInfo inputAssembly{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .topology = desc.topology
    };

    // Dynamic state
    VkDynamicState dynamicStates[] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
    VkPipelineDynamicStateCreateInfo dynamicState{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
        .dynamicStateCount = 2,
        .pDynamicStates = dynamicStates
    };

    VkPipelineViewportStateCreateInfo viewportState{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .viewportCount = 1,
        .scissorCount = 1
    };

    // Rasterization (no culling for lines)
    VkPipelineRasterizationStateCreateInfo rasterization{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        .polygonMode = VK_POLYGON_MODE_FILL,
        .cullMode = VK_CULL_MODE_NONE,
        .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
        .lineWidth = 1.0f
    };

    // Multisampling (none)
    VkPipelineMultisampleStateCreateInfo multisampling{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT
    };

    // Depth/stencil
    VkPipelineDepthStencilStateCreateInfo depthStencil{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
        .depthTestEnable = desc.depthTest ? VK_TRUE : VK_FALSE,
        .depthWriteEnable = desc.depthTest ? VK_TRUE : VK_FALSE,
        .depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL
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
        .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .attachmentCount = 1,
        .pAttachments = &colorBlendAttachment
    };

    // Dynamic rendering
    VkPipelineRenderingCreateInfo renderingCI{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
        .colorAttachmentCount = 1,
        .pColorAttachmentFormats = &colorFormat,
        .depthAttachmentFormat = depthFormat
    };

    // Create graphics pipeline
    VkGraphicsPipelineCreateInfo pipelineCI{
        .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .pNext = &renderingCI,
        .stageCount = 3, // vertex, geometry, fragment
        .pStages = shaderStages,
        .pVertexInputState = &vertexInputState,
        .pInputAssemblyState = &inputAssembly,
        .pViewportState = &viewportState,
        .pRasterizationState = &rasterization,
        .pMultisampleState = &multisampling,
        .pDepthStencilState = &depthStencil,
        .pColorBlendState = &colorBlending,
        .pDynamicState = &dynamicState,
        .layout = layout
    };

    VkResult result = vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineCI, nullptr, &pipeline);

    // Destroy shader modules
    vkDestroyShaderModule(device, vertModule, nullptr);
    vkDestroyShaderModule(device, geomModule, nullptr);
    vkDestroyShaderModule(device, fragModule, nullptr);

    return result == VK_SUCCESS;
}

void LinePipeline::destroy(VkDevice device, VmaAllocator allocator) {
    // Destroy compute pipeline resources
    if (computeInitialized) {
        indirectCompute.Destroy();
        computeInitialized = false;
    }
    if (computeDescPool != VK_NULL_HANDLE) {
        vkDestroyDescriptorPool(device, computeDescPool, nullptr);
        computeDescPool = VK_NULL_HANDLE;
        computeDescSet = VK_NULL_HANDLE;
    }

    // Destroy LineGPUInfo GPU buffer
    if (lineGPUInfoBuffer != VK_NULL_HANDLE) {
        vmaDestroyBuffer(allocator, lineGPUInfoBuffer, lineGPUInfoBufferAllocation);
        lineGPUInfoBuffer = VK_NULL_HANDLE;
        lineGPUInfoBufferAllocation = VK_NULL_HANDLE;
    }

    if (indirectBuffer != VK_NULL_HANDLE) {
        vmaDestroyBuffer(allocator, indirectBuffer, indirectBufferAllocation);
        indirectBuffer = VK_NULL_HANDLE;
        indirectBufferAllocation = VK_NULL_HANDLE;
    }

    if (lineSSBO != VK_NULL_HANDLE) {
        vmaDestroyBuffer(allocator, lineSSBO, lineSSBOAllocation);
        lineSSBO = VK_NULL_HANDLE;
        lineSSBOAllocation = VK_NULL_HANDLE;
    }

    if (pipeline != VK_NULL_HANDLE)
        vkDestroyPipeline(device, pipeline, nullptr);
    if (layout != VK_NULL_HANDLE)
        vkDestroyPipelineLayout(device, layout, nullptr);

    if (ssboPool != VK_NULL_HANDLE)
        vkDestroyDescriptorPool(device, ssboPool, nullptr); // also frees ssboSet
    if (ssboSetLayout != VK_NULL_HANDLE)
        vkDestroyDescriptorSetLayout(device, ssboSetLayout, nullptr);

    pipeline = VK_NULL_HANDLE;
    layout = VK_NULL_HANDLE;
    ssboPool = VK_NULL_HANDLE;
    ssboSet = VK_NULL_HANDLE;
    ssboSetLayout = VK_NULL_HANDLE;
}
