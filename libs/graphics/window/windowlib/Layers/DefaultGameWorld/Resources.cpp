#include "Resources.h"
#include "Camera.h"
#include <cstring>

bool Resources::create(VkDevice device, VmaAllocator allocator, uint32_t framesInFlight)
{
    if (!createDescriptorLayouts(device))                          return false;
    if (!createDescriptorPool(device, framesInFlight))             return false;
    if (!createGlobalResources(device, allocator, framesInFlight)) return false;
    return true;
}

bool Resources::createDescriptorLayouts(VkDevice device)
{
    // Set 0 (Global): Camera uniform buffer
    VkDescriptorSetLayoutBinding globalBinding{
        .binding         = 0,
        .descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .descriptorCount = 1,
        .stageFlags      = VK_SHADER_STAGE_VERTEX_BIT,
        .pImmutableSamplers = nullptr
    };

    VkDescriptorSetLayoutCreateInfo globalLayoutCI{
        .sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .bindingCount = 1,
        .pBindings    = &globalBinding
    };

    if (vkCreateDescriptorSetLayout(device, &globalLayoutCI, nullptr, &globalSetLayout) != VK_SUCCESS)
        return false;

    // Set 1 (Per-Object): Defer for now - we'll use push constants for model matrix
    // objectSetLayout remains VK_NULL_HANDLE

    return true;
}

bool Resources::createDescriptorPool(VkDevice device, uint32_t framesInFlight)
{
    // Pool sized for global sets (camera UBO)
    // Later we can expand for per-object sets
    VkDescriptorPoolSize poolSize{
        .type            = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .descriptorCount = framesInFlight  // One camera UBO per frame
    };

    VkDescriptorPoolCreateInfo poolCI{
        .sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .maxSets       = framesInFlight,  // One global set per frame
        .poolSizeCount = 1,
        .pPoolSizes    = &poolSize
    };

    if (vkCreateDescriptorPool(device, &poolCI, nullptr, &descriptorPool) != VK_SUCCESS)
        return false;

    return true;
}

bool Resources::createGlobalResources(VkDevice device, VmaAllocator allocator, uint32_t framesInFlight)
{
    // CameraUBO structure: 2 × mat4 = 128 bytes
    constexpr VkDeviceSize cameraUBOSize = sizeof(float) * 16 * 2;  // view + proj

    cameraBuffers.resize(framesInFlight);
    cameraAllocations.resize(framesInFlight);
    cameraMapped.resize(framesInFlight);
    globalSets.resize(framesInFlight);

    // Create uniform buffers (one per frame)
    for (uint32_t i = 0; i < framesInFlight; i++)
    {
        VkBufferCreateInfo bufferCI{
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .size  = cameraUBOSize,
            .usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT
        };

        VmaAllocationCreateInfo allocCI{
            .flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT |
                     VMA_ALLOCATION_CREATE_MAPPED_BIT,
            .usage = VMA_MEMORY_USAGE_AUTO
        };

        VmaAllocationInfo allocInfo;
        if (vmaCreateBuffer(allocator, &bufferCI, &allocCI, &cameraBuffers[i],
                           &cameraAllocations[i], &allocInfo) != VK_SUCCESS)
            return false;

        cameraMapped[i] = allocInfo.pMappedData;
    }

    // Allocate descriptor sets
    std::vector<VkDescriptorSetLayout> layouts(framesInFlight, globalSetLayout);
    VkDescriptorSetAllocateInfo allocInfo{
        .sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .descriptorPool     = descriptorPool,
        .descriptorSetCount = framesInFlight,
        .pSetLayouts        = layouts.data()
    };

    if (vkAllocateDescriptorSets(device, &allocInfo, globalSets.data()) != VK_SUCCESS)
        return false;

    // Bind buffers to descriptor sets
    for (uint32_t i = 0; i < framesInFlight; i++)
    {
        VkDescriptorBufferInfo bufferInfo{
            .buffer = cameraBuffers[i],
            .offset = 0,
            .range  = cameraUBOSize
        };

        VkWriteDescriptorSet write{
            .sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .dstSet          = globalSets[i],
            .dstBinding      = 0,
            .dstArrayElement = 0,
            .descriptorCount = 1,
            .descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .pBufferInfo     = &bufferInfo
        };

        vkUpdateDescriptorSets(device, 1, &write, 0, nullptr);
    }

    return true;
}

void Resources::updateCameraBuffer(uint32_t frameIndex, const Camera& camera, float aspectRatio)
{
    if (frameIndex >= cameraMapped.size())
        return;

    // Build camera matrices
    glm::mat4 view = camera.viewMatrix();
    glm::mat4 proj = camera.projectionMatrix(aspectRatio);

    // Write to mapped buffer: view then proj
    float* dst = static_cast<float*>(cameraMapped[frameIndex]);
    std::memcpy(dst,      &view, sizeof(glm::mat4));
    std::memcpy(dst + 16, &proj, sizeof(glm::mat4));
}

void Resources::destroy(VkDevice device, VmaAllocator allocator)
{
    for (size_t i = 0; i < cameraBuffers.size(); i++)
        vmaDestroyBuffer(allocator, cameraBuffers[i], cameraAllocations[i]);

    cameraBuffers.clear();
    cameraAllocations.clear();
    cameraMapped.clear();
    globalSets.clear();

    // Destroying the pool implicitly frees all sets allocated from it
    if (descriptorPool != VK_NULL_HANDLE)
        vkDestroyDescriptorPool(device, descriptorPool, nullptr);

    if (globalSetLayout != VK_NULL_HANDLE)
        vkDestroyDescriptorSetLayout(device, globalSetLayout, nullptr);
    if (objectSetLayout != VK_NULL_HANDLE)
        vkDestroyDescriptorSetLayout(device, objectSetLayout, nullptr);

    descriptorPool  = VK_NULL_HANDLE;
    globalSetLayout = VK_NULL_HANDLE;
    objectSetLayout = VK_NULL_HANDLE;
}
