#include "Pipeline.h"

#include <fstream>
#include <stdexcept>
#include <iostream>
#include <cstring>
#include "slang/slang.h"
#include "slang/slang-com-ptr.h"

Slang::ComPtr<slang::IGlobalSession>& Pipeline::getSlangSession()
{
    static Slang::ComPtr<slang::IGlobalSession> globalSession;
    if (!globalSession)
    {
        std::cout << "Creating Slang global session..." << std::endl;
        if (slang::createGlobalSession(globalSession.writeRef()) != SLANG_OK)
        {
            std::cerr << "Failed to create Slang global session\n";
        }
        else
        {
            std::cout << "Slang global session created successfully" << std::endl;
        }
    }
    return globalSession;
}

VkShaderModule Pipeline::loadShaderModule(VkDevice device, const std::string& path)
{
    std::cout << "Loading shader: " << path << std::endl;

    auto& globalSession = getSlangSession();
    if (!globalSession)
    {
        std::cerr << "No global session available\n";
        return VK_NULL_HANDLE;
    }

    std::cout << "Creating Slang session with target configuration..." << std::endl;

    // Configure target (SPIR-V 1.4)
    slang::TargetDesc targetDesc{
        .format = SLANG_SPIRV,
        .profile = globalSession->findProfile("spirv_1_4")
    };

    // Configure compiler options (emit SPIR-V directly)
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

    std::cout << "Session created successfully" << std::endl;

    // Load shader module from source file
    std::cout << "Loading module from source: " << path << std::endl;
    Slang::ComPtr<slang::IBlob> diagnostics;
    Slang::ComPtr<slang::IModule> module{ session->loadModuleFromSource(
        "shader_module",  // module name
        path.c_str(),     // source path
        nullptr,          // source blob (nullptr = use path instead)
        diagnostics.writeRef()  // output diagnostics
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

    std::cout << "Module loaded successfully" << std::endl;

    // Get SPIR-V code from module
    std::cout << "Getting SPIR-V code from module..." << std::endl;
    Slang::ComPtr<slang::IBlob> spirvCode;
    if (module->getTargetCode(0, spirvCode.writeRef()) != SLANG_OK || !spirvCode)
    {
        std::cerr << "Failed to get SPIR-V from module: " << path << "\n";
        return VK_NULL_HANDLE;
    }

    std::cout << "Got SPIR-V code (" << spirvCode->getBufferSize() << " bytes)" << std::endl;

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

    std::cout << "VkShaderModule created successfully" << std::endl;

    return shaderModule;
}

bool Pipeline::create(VkDevice device,
                      const PipelineDesc& desc,
                      const std::vector<VkDescriptorSetLayout>& setLayouts,
                      VkFormat colorFormat,
                      VkFormat depthFormat)
{
    // Load shader modules
    VkShaderModule vertModule = loadShaderModule(device, desc.vertexShaderPath);
    VkShaderModule fragModule = loadShaderModule(device, desc.fragmentShaderPath);

    if (vertModule == VK_NULL_HANDLE || fragModule == VK_NULL_HANDLE)
    {
        if (vertModule != VK_NULL_HANDLE) vkDestroyShaderModule(device, vertModule, nullptr);
        if (fragModule != VK_NULL_HANDLE) vkDestroyShaderModule(device, fragModule, nullptr);
        return false;
    }

    // Push constant range for model matrix + color data (64 + 16 + 16 = 96 bytes)
    VkPushConstantRange pushConstantRange{
        .stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
        .offset     = 0,
        .size       = 96  // sizeof(glm::mat4) + sizeof(glm::vec4) + sizeof(uint32_t) + 12 bytes padding
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
        vkDestroyShaderModule(device, fragModule, nullptr);
        return false;
    }

    // Shader stages
    VkPipelineShaderStageCreateInfo shaderStages[] = {
        {
            .sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage  = VK_SHADER_STAGE_VERTEX_BIT,
            .module = vertModule,
            .pName  = "main"
        },
        {
            .sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage  = VK_SHADER_STAGE_FRAGMENT_BIT,
            .module = fragModule,
            .pName  = "main"
        }
    };

    // Vertex input: matches Mesh::Vertex (vec3 pos, vec3 normal, vec2 uv, vec4 color)
    VkVertexInputBindingDescription vertexBinding{
        .binding   = 0,
        .stride    = 48,  // 12 + 12 + 8 + 16 bytes
        .inputRate = VK_VERTEX_INPUT_RATE_VERTEX
    };

    VkVertexInputAttributeDescription vertexAttributes[] = {
        { .location = 0, .binding = 0, .format = VK_FORMAT_R32G32B32_SFLOAT,    .offset = 0 },   // position
        { .location = 1, .binding = 0, .format = VK_FORMAT_R32G32B32_SFLOAT,    .offset = 12 },  // normal
        { .location = 2, .binding = 0, .format = VK_FORMAT_R32G32_SFLOAT,       .offset = 24 },  // texCoord
        { .location = 3, .binding = 0, .format = VK_FORMAT_R32G32B32A32_SFLOAT, .offset = 32 }   // color
    };

    VkPipelineVertexInputStateCreateInfo vertexInputState{
        .sType                           = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .vertexBindingDescriptionCount   = 1,
        .pVertexBindingDescriptions      = &vertexBinding,
        .vertexAttributeDescriptionCount = 4,
        .pVertexAttributeDescriptions    = vertexAttributes
    };

    // Input assembly
    VkPipelineInputAssemblyStateCreateInfo inputAssembly{
        .sType    = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST
    };

    // Dynamic state (viewport + scissor set at draw time)
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

    // Rasterization
    VkPipelineRasterizationStateCreateInfo rasterization{
        .sType       = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        .polygonMode = VK_POLYGON_MODE_FILL,
        .cullMode    = VK_CULL_MODE_BACK_BIT,
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
        .depthTestEnable  = VK_TRUE,
        .depthWriteEnable = VK_TRUE,
        .depthCompareOp   = VK_COMPARE_OP_LESS_OR_EQUAL
    };

    // Color blending (opaque, no blending)
    VkPipelineColorBlendAttachmentState colorBlendAttachment{
        .blendEnable    = VK_FALSE,
        .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                          VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT
    };

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
        .stageCount          = 2,
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

    // Destroy shader modules (no longer needed)
    vkDestroyShaderModule(device, vertModule, nullptr);
    vkDestroyShaderModule(device, fragModule, nullptr);

    return result == VK_SUCCESS;
}

void Pipeline::destroy(VkDevice device)
{
    if (pipeline != VK_NULL_HANDLE)
        vkDestroyPipeline(device, pipeline, nullptr);
    if (layout != VK_NULL_HANDLE)
        vkDestroyPipelineLayout(device, layout, nullptr);

    pipeline = VK_NULL_HANDLE;
    layout   = VK_NULL_HANDLE;
}
