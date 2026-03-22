#pragma once
#include "ComputePipelineTools/ComputePipeline.h"


class LineRendererCompute : public ComputePipeline
{
public:
    LineRendererCompute() = default;

    void UpdateDescSets(VkDescriptorSet descSet, VkBuffer lineGPUInfoBuffer, VkDeviceSize lineGPUInfoSize,
                        VkBuffer indirectBuffer, VkDeviceSize indirectSize)
    {
        // Binding 0: input LineGPUInfo SSBO
        VkDescriptorBufferInfo inputInfo{
            .buffer = lineGPUInfoBuffer,
            .offset = 0,
            .range = lineGPUInfoSize
        };

        // Binding 1: output VkDrawIndirectCommand SSBO
        VkDescriptorBufferInfo outputInfo{
            .buffer = indirectBuffer,
            .offset = 0,
            .range = indirectSize
        };

        VkWriteDescriptorSet writes[2] = {
            {
                .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .dstSet = descSet,
                .dstBinding = 0,
                .descriptorCount = 1,
                .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                .pBufferInfo = &inputInfo
            },
            {
                .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .dstSet = descSet,
                .dstBinding = 1,
                .descriptorCount = 1,
                .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                .pBufferInfo = &outputInfo
            }
        };

        vkUpdateDescriptorSets(m_device, 2, writes, 0, nullptr);
    }

protected:
    virtual VkDescriptorSetLayout CreateDescriptorSetLayout(VkDevice device) override
    {
        VkDescriptorSetLayoutBinding bindings[2] = {
            {
                // Binding 0: input LineGPUInfo[]
                .binding = 0,
                .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                .descriptorCount = 1,
                .stageFlags = VK_SHADER_STAGE_COMPUTE_BIT,
                .pImmutableSamplers = NULL
            },
            {
                // Binding 1: output VkDrawIndirectCommand[]
                .binding = 1,
                .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                .descriptorCount = 1,
                .stageFlags = VK_SHADER_STAGE_COMPUTE_BIT,
                .pImmutableSamplers = NULL
            }
        };

        VkDescriptorSetLayoutCreateInfo layoutCI = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
            .pNext = NULL,
            .bindingCount = 2,
            .pBindings = bindings
        };

        VkDescriptorSetLayout descSetLayout = VK_NULL_HANDLE;
        const VkResult res = vkCreateDescriptorSetLayout(device, &layoutCI, NULL, &descSetLayout);
        CHECK_VULKAN_RESULT(res);

        return descSetLayout;
    }
};
