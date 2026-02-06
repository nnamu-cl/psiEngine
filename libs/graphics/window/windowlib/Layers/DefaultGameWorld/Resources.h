#pragma once

#include <volk/volk.h>
#include <vma/vk_mem_alloc.h>
#include <vector>

// Descriptor set 0 (global)  -- camera uniform buffer
// Descriptor set 1 (object)  -- model-matrix uniform buffer + material sampler
//
// All per-frame resources (buffers, descriptor sets) are sized by
// framesInFlight at create() time so this file has no hard dependency
// on the maxFramesInFlight constant from ApplicationWindow.

class Resources
{
public:
    bool create(VkDevice device, VmaAllocator allocator, uint32_t framesInFlight);
    void destroy(VkDevice device, VmaAllocator allocator);

    // --- Descriptor set layouts (created here, referenced by Pipeline) ---
    VkDescriptorSetLayout globalSetLayout{ VK_NULL_HANDLE };
    VkDescriptorSetLayout objectSetLayout{ VK_NULL_HANDLE };

    // --- Descriptor pool ---
    VkDescriptorPool descriptorPool{ VK_NULL_HANDLE };

    // --- Per-frame globals: one descriptor set + one uniform buffer per frame ---
    std::vector<VkDescriptorSet> globalSets;
    std::vector<VkBuffer>        cameraBuffers;
    std::vector<VmaAllocation>   cameraAllocations;
    std::vector<void*>           cameraMapped; // Persistently mapped pointers

    // Update camera uniform buffer for a given frame
    void updateCameraBuffer(uint32_t frameIndex, const class Camera& camera, float aspectRatio);

private:
    bool createDescriptorLayouts(VkDevice device);
    bool createDescriptorPool(VkDevice device, uint32_t framesInFlight);
    bool createGlobalResources(VkDevice device, VmaAllocator allocator, uint32_t framesInFlight);
};
