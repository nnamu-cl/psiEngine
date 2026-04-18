#pragma once
#include <vector>
#include <slang-com-ptr.h>
#include <slang.h>

#include "VulkanChecks.h"
#include <volk/volk.h>
#include "glm/fwd.hpp"


class ComputePipeline
{
public:
    /*
     * TODO: Here at the point of creation we can allow the user to assign certain things better
     * Device,
     *
     *
     *
     */
    ComputePipeline() = default;

    void Init(VkDevice device, VkDescriptorPool descPool, const char* pCSFilename)
    {
        m_device = device;
        m_descriptorPool = descPool;
        m_descriptorSetLayout = CreateDescriptorSetLayout(device);

        CreatePipelineLayout();

        m_cs = loadShaderModule(m_device, pCSFilename);

        CreatePipeline(m_cs);
    }

    void Destroy()
    {
        vkDestroyShaderModule(m_device, m_cs, NULL);
        vkDestroyDescriptorSetLayout(m_device, m_descriptorSetLayout, NULL);
        vkDestroyPipelineLayout(m_device, m_pipelineLayout, NULL);
        vkDestroyPipeline(m_device, m_pipeline, NULL);
    }

    void RecordCommandBuffer(VkDescriptorSet DescSet, VkCommandBuffer CmdBuf, glm::u32 GroupCountX,
                             glm::u32 GroupCountY, glm::u32 GroupCountZ)
    {
        vkCmdBindPipeline(CmdBuf, VK_PIPELINE_BIND_POINT_COMPUTE, m_pipeline);

        vkCmdBindDescriptorSets(CmdBuf, VK_PIPELINE_BIND_POINT_COMPUTE, m_pipelineLayout, 0, 1, &DescSet, 0, NULL);

        vkCmdDispatch(CmdBuf, GroupCountX, GroupCountY, GroupCountZ);
    }

    void AllocDescSets(std::vector<VkDescriptorSet>& DescriptorSets)
    {
        assert(DescriptorSets.empty());

        VkDescriptorSetAllocateInfo allocInfo = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
            .pNext = NULL,
            .descriptorPool = m_descriptorPool,
            .descriptorSetCount = 1,
            .pSetLayouts = &m_descriptorSetLayout
        };

        DescriptorSets.resize(1);
        VkResult res = vkAllocateDescriptorSets(m_device, &allocInfo, DescriptorSets.data());
        CHECK_VULKAN_RESULT(res);
    }

protected:
    virtual VkDescriptorSetLayout CreateDescriptorSetLayout(VkDevice device) = 0;

    VkDevice m_device = VK_NULL_HANDLE;
    VkDescriptorPool m_descriptorPool = VK_NULL_HANDLE;

private:
    void CreatePipelineLayout()
    {
        VkPipelineLayoutCreateInfo PipelineLayoutCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
            .pNext = NULL,
            .flags = 0,
            .setLayoutCount = 1,
            .pSetLayouts = &m_descriptorSetLayout,
            .pushConstantRangeCount = 0,
            .pPushConstantRanges = NULL,
        };

        VkResult res = vkCreatePipelineLayout(m_device, &PipelineLayoutCreateInfo, NULL, &m_pipelineLayout);
        CHECK_VULKAN_RESULT(res);
    }

    void CreatePipeline(VkShaderModule cs)
    {
        VkPipelineShaderStageCreateInfo ShaderStageCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .pNext = NULL,
            .flags = 0,
            .stage = VK_SHADER_STAGE_COMPUTE_BIT,
            .module = cs,
            .pName = "main",
            .pSpecializationInfo = NULL,
        };

        const VkComputePipelineCreateInfo ci = {
            .sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
            .flags = 0,
            .stage = ShaderStageCreateInfo,
            .layout = m_pipelineLayout,
            .basePipelineHandle = VK_NULL_HANDLE,
            .basePipelineIndex = -1
        };

        VkResult res = vkCreateComputePipelines(m_device, NULL, 1, &ci, NULL, &m_pipeline);
        CHECK_VULKAN_RESULT(res);
    }

    static Slang::ComPtr<slang::IGlobalSession>& getSlangSession()
    {
        static Slang::ComPtr<slang::IGlobalSession> globalSession;
        if (!globalSession)
        {
            if (slang::createGlobalSession(globalSession.writeRef()) != SLANG_OK)
            {
                std::cerr << "Failed to create Slang global session for ComputePipeline\n";
            }
        }
        return globalSession;
    }

    static VkShaderModule loadShaderModule(VkDevice device, const std::string& path)
    {
        std::cout << "Loading compute shader: " << path << std::endl;

        auto& globalSession = getSlangSession();
        if (!globalSession)
        {
            std::cerr << "No global Slang session available\n";
            return VK_NULL_HANDLE;
        }

        slang::TargetDesc targetDesc{
            .format = SLANG_SPIRV,
            .profile = globalSession->findProfile("spirv_1_4")
        };

        slang::CompilerOptionEntry options[] = {
            {slang::CompilerOptionName::EmitSpirvDirectly, {slang::CompilerOptionValueKind::Int, 1}}
        };

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

        Slang::ComPtr<slang::IBlob> diagnostics;
        Slang::ComPtr<slang::IModule> module{
            session->loadModuleFromSource(
                "compute_shader_module",
                path.c_str(),
                nullptr,
                diagnostics.writeRef()
            )
        };

        if (diagnostics && diagnostics->getBufferSize() > 0)
        {
            std::cout << "Slang diagnostics for " << path << ":\n"
                << (const char*)diagnostics->getBufferPointer() << std::endl;
        }

        if (!module)
        {
            std::cerr << "Failed to load compute shader module: " << path << "\n";
            return VK_NULL_HANDLE;
        }

        Slang::ComPtr<slang::IBlob> spirvCode;
        if (module->getTargetCode(0, spirvCode.writeRef()) != SLANG_OK || !spirvCode)
        {
            std::cerr << "Failed to get SPIR-V from compute module: " << path << "\n";
            return VK_NULL_HANDLE;
        }

        VkShaderModuleCreateInfo moduleCI{
            .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
            .codeSize = spirvCode->getBufferSize(),
            .pCode = static_cast<const uint32_t*>(spirvCode->getBufferPointer())
        };

        VkShaderModule shaderModule = VK_NULL_HANDLE;
        if (vkCreateShaderModule(device, &moduleCI, nullptr, &shaderModule) != VK_SUCCESS)
        {
            std::cerr << "Failed to create VkShaderModule for: " << path << "\n";
            return VK_NULL_HANDLE;
        }

        return shaderModule;
    }

    VkShaderModule m_cs = VK_NULL_HANDLE;
    VkPipeline m_pipeline = VK_NULL_HANDLE;
    VkPipelineLayout m_pipelineLayout = VK_NULL_HANDLE;
    VkDescriptorSetLayout m_descriptorSetLayout = VK_NULL_HANDLE;
};
