#include "LinePipeline.h"
#include "../../Data/LineProperties.h"

#include <fstream>
#include <stdexcept>
#include <iostream>
#include <cstring>
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
