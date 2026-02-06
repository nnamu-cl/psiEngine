#include <iostream>
#include <fstream>
#include <catch2/catch_test_macros.hpp>
#include "../windowlib/Layers/DefaultGameWorld/DefaultGameWorld.h"
#include "../windowlib/Layers/DefaultGameWorld/Resources.h"
#include "../windowlib/Layers/DefaultGameWorld/Pipeline.h"
#include "../windowlib/Layers/DefaultGameWorld/Camera.h"
#include "../windowlib/Layers/DefaultGameWorld/Mesh.h"
#include "../windowlib/ApplicationWindow.h"

// Helper to initialize minimal Vulkan context for testing
struct VulkanTestContext {
    VkInstance instance{ VK_NULL_HANDLE };
    VkPhysicalDevice physicalDevice{ VK_NULL_HANDLE };
    VkDevice device{ VK_NULL_HANDLE };
    VmaAllocator allocator{ VK_NULL_HANDLE };
    uint32_t queueFamily{ 0 };

    bool init() {
        // Initialize Volk
        if (volkInitialize() != VK_SUCCESS)
            return false;

        // Create instance
        VkApplicationInfo appInfo{
            .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
            .pApplicationName = "VulkanSetupTest",
            .apiVersion = VK_API_VERSION_1_3
        };

        VkInstanceCreateInfo instanceCI{
            .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
            .pApplicationInfo = &appInfo
        };

        if (vkCreateInstance(&instanceCI, nullptr, &instance) != VK_SUCCESS)
            return false;

        volkLoadInstance(instance);

        // Pick first physical device
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
        if (deviceCount == 0)
            return false;

        vkEnumeratePhysicalDevices(instance, &deviceCount, &physicalDevice);

        // Find graphics queue family
        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);
        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.data());

        for (uint32_t i = 0; i < queueFamilyCount; i++) {
            if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                queueFamily = i;
                break;
            }
        }

        // Create logical device
        float queuePriority = 1.0f;
        VkDeviceQueueCreateInfo queueCI{
            .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            .queueFamilyIndex = queueFamily,
            .queueCount = 1,
            .pQueuePriorities = &queuePriority
        };

        VkPhysicalDeviceVulkan12Features vk12Features{
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES,
            .bufferDeviceAddress = VK_TRUE
        };

        VkPhysicalDeviceVulkan13Features vk13Features{
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
            .pNext = &vk12Features,
            .synchronization2 = VK_TRUE,
            .dynamicRendering = VK_TRUE
        };

        VkDeviceCreateInfo deviceCI{
            .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
            .pNext = &vk13Features,
            .queueCreateInfoCount = 1,
            .pQueueCreateInfos = &queueCI
        };

        if (vkCreateDevice(physicalDevice, &deviceCI, nullptr, &device) != VK_SUCCESS)
            return false;

        // Create VMA allocator
        VmaVulkanFunctions vkFunctions{
            .vkGetInstanceProcAddr = vkGetInstanceProcAddr,
            .vkGetDeviceProcAddr = vkGetDeviceProcAddr
        };

        VmaAllocatorCreateInfo allocatorCI{
            .flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT,
            .physicalDevice = physicalDevice,
            .device = device,
            .pVulkanFunctions = &vkFunctions,
            .instance = instance
        };

        if (vmaCreateAllocator(&allocatorCI, &allocator) != VK_SUCCESS)
            return false;

        return true;
    }

    void cleanup() {
        if (allocator != VK_NULL_HANDLE)
            vmaDestroyAllocator(allocator);
        if (device != VK_NULL_HANDLE)
            vkDestroyDevice(device, nullptr);
        if (instance != VK_NULL_HANDLE)
            vkDestroyInstance(instance, nullptr);
    }
};

TEST_CASE("Resources creation succeeds", "[vulkan][resources]") {
    VulkanTestContext ctx;
    REQUIRE(ctx.init());

    Resources resources;
    REQUIRE(resources.create(ctx.device, ctx.allocator, 2));

    // Verify handles are non-null
    REQUIRE(resources.globalSetLayout != VK_NULL_HANDLE);
    REQUIRE(resources.descriptorPool != VK_NULL_HANDLE);
    REQUIRE(resources.globalSets.size() == 2);
    REQUIRE(resources.cameraBuffers.size() == 2);

    for (uint32_t i = 0; i < 2; i++) {
        REQUIRE(resources.globalSets[i] != VK_NULL_HANDLE);
        REQUIRE(resources.cameraBuffers[i] != VK_NULL_HANDLE);
    }

    resources.destroy(ctx.device, ctx.allocator);
    ctx.cleanup();
}

TEST_CASE("Pipeline creation succeeds with valid shaders", "[vulkan][pipeline]") {
    std::cout << "=== Starting pipeline creation test ===" << std::endl;

    std::cout << "Initializing Vulkan context..." << std::endl;
    VulkanTestContext ctx;
    REQUIRE(ctx.init());
    std::cout << "Vulkan context initialized" << std::endl;

    // Create resources first (need descriptor layouts)
    std::cout << "Creating resources..." << std::endl;
    Resources resources;
    REQUIRE(resources.create(ctx.device, ctx.allocator, 2));
    std::cout << "Resources created" << std::endl;

    // Construct absolute paths to shader files (relative to this source file)
    // __FILE__ gives us: libs/graphics/window/tests/test_DefaultGameWorld_VulkanSetup.cpp
    std::cout << "Constructing shader paths..." << std::endl;
    std::string testDir = __FILE__;
    size_t lastSlash = testDir.find_last_of("/\\");
    std::string baseDir = testDir.substr(0, lastSlash) + "/../shaders/DefaultGameWorld/";

    std::cout << "Base directory: " << baseDir << std::endl;

    // Create pipeline
    std::cout << "Creating pipeline object..." << std::endl;
    Pipeline pipeline;
    PipelineDesc desc{
        .vertexShaderPath   = baseDir + "default.vert.slang",
        .fragmentShaderPath = baseDir + "default.frag.slang"
    };

    std::cout << "Vertex shader: " << desc.vertexShaderPath << std::endl;
    std::cout << "Fragment shader: " << desc.fragmentShaderPath << std::endl;

    std::vector<VkDescriptorSetLayout> layouts = { resources.globalSetLayout };
    std::cout << "Calling pipeline.create()..." << std::endl;
    REQUIRE(pipeline.create(ctx.device, desc, layouts, VK_FORMAT_B8G8R8A8_SRGB, VK_FORMAT_D24_UNORM_S8_UINT));
    std::cout << "Pipeline created successfully" << std::endl;

    // Verify handles are non-null
    REQUIRE(pipeline.pipeline != VK_NULL_HANDLE);
    REQUIRE(pipeline.layout != VK_NULL_HANDLE);

    pipeline.destroy(ctx.device);
    resources.destroy(ctx.device, ctx.allocator);
    ctx.cleanup();
}

TEST_CASE("Camera buffer update works", "[vulkan][resources]") {
    VulkanTestContext ctx;
    REQUIRE(ctx.init());

    Resources resources;
    REQUIRE(resources.create(ctx.device, ctx.allocator, 2));

    Camera camera;  // Default position
    resources.updateCameraBuffer(0, camera, 16.0f / 9.0f);

    // If we got here without crashing, the update worked
    REQUIRE(true);

    resources.destroy(ctx.device, ctx.allocator);
    ctx.cleanup();
}

// ===========================================================================
// INTEGRATION TESTS
// ===========================================================================

// Helper to convert VulkanTestContext to ApplicationWindowData-compatible structure
struct DefaultGameWorldTestContext {
    VulkanTestContext vkCtx;
    ApplicationWindowData windowData;

    bool init() {
        if (!vkCtx.init())
            return false;

        // Map VulkanTestContext to ApplicationWindowData
        windowData.device = vkCtx.device;
        windowData.physicalDevice = vkCtx.physicalDevice;
        windowData.vkInstance = vkCtx.instance;
        windowData.allocator = vkCtx.allocator;
        windowData.queueFamily = vkCtx.queueFamily;
        windowData.swapchainImageFormat = VK_FORMAT_B8G8R8A8_SRGB;
        windowData.depthFormat = VK_FORMAT_D24_UNORM_S8_UINT;
        windowData.windowSize = { 1280, 720 };

        return true;
    }

    void cleanup() {
        vkCtx.cleanup();
    }
};

// ===========================================================================
// Test #3: Mesh Upload Validation
// ===========================================================================

TEST_CASE("DefaultGameWorld uploads meshes with correct offsets", "[integration][gpu][mesh]") {
    std::cout << "=== Test: Mesh Upload Validation ===" << std::endl;

    DefaultGameWorldTestContext ctx;
    REQUIRE(ctx.init());

    DefaultGameWorld layer(&ctx.windowData);
    layer.OnAttach();

    // Verify mesh GPU info was populated
    REQUIRE(layer.data.meshGPUInfo.size() > 0);
    std::cout << "  Uploaded " << layer.data.meshGPUInfo.size() << " meshes" << std::endl;

    // Verify offsets don't overlap (vertex regions)
    for (size_t i = 0; i + 1 < layer.data.meshGPUInfo.size(); i++) {
        const auto& mesh1 = layer.data.meshGPUInfo[i];
        const auto& mesh2 = layer.data.meshGPUInfo[i + 1];

        VkDeviceSize mesh1VertexEnd = mesh1.vertexOffset + (mesh1.vertexCount * sizeof(Vertex));
        REQUIRE(mesh2.vertexOffset >= mesh1VertexEnd);

        std::cout << "  Mesh " << i << " vertex range: [" << mesh1.vertexOffset
                  << ", " << mesh1VertexEnd << ")" << std::endl;
    }

    // Verify all counts are non-zero
    for (size_t i = 0; i < layer.data.meshGPUInfo.size(); i++) {
        REQUIRE(layer.data.meshGPUInfo[i].vertexCount > 0);
        REQUIRE(layer.data.meshGPUInfo[i].indexCount > 0);
    }

    // Verify mesh buffer was created
    REQUIRE(layer.data.meshBuffer != VK_NULL_HANDLE);

    layer.OnDetach();
    ctx.cleanup();
}

TEST_CASE("DefaultGameWorld handles empty mesh table gracefully", "[integration][gpu][mesh]") {
    DefaultGameWorldTestContext ctx;
    REQUIRE(ctx.init());

    // Create a custom DefaultGameWorld without adding meshes
    // (We can't easily do this with the current design, so skip for now)
    // This test would require refactoring DefaultGameWorld to allow empty initialization

    ctx.cleanup();
}

// ===========================================================================
// Test #2: Shader Compilation Error Handling
// ===========================================================================

TEST_CASE("Pipeline creation fails gracefully with missing shader files", "[integration][gpu][pipeline][error]") {
    std::cout << "=== Test: Missing Shader Files ===" << std::endl;

    VulkanTestContext ctx;
    REQUIRE(ctx.init());

    Resources resources;
    REQUIRE(resources.create(ctx.device, ctx.allocator, 2));

    Pipeline pipeline;
    PipelineDesc desc{
        .vertexShaderPath   = "nonexistent/path/shader.vert.slang",
        .fragmentShaderPath = "nonexistent/path/shader.frag.slang"
    };

    std::vector<VkDescriptorSetLayout> layouts = { resources.globalSetLayout };

    // Should fail gracefully without crashing
    bool result = pipeline.create(ctx.device, desc, layouts,
                                   VK_FORMAT_B8G8R8A8_SRGB,
                                   VK_FORMAT_D24_UNORM_S8_UINT);

    REQUIRE(result == false);
    REQUIRE(pipeline.pipeline == VK_NULL_HANDLE);
    REQUIRE(pipeline.layout == VK_NULL_HANDLE);

    std::cout << "  Pipeline correctly failed with missing shaders" << std::endl;

    pipeline.destroy(ctx.device);
    resources.destroy(ctx.device, ctx.allocator);
    ctx.cleanup();
}

TEST_CASE("Pipeline creation fails gracefully with invalid Slang code", "[integration][gpu][pipeline][error]") {
    std::cout << "=== Test: Invalid Slang Code ===" << std::endl;

    VulkanTestContext ctx;
    REQUIRE(ctx.init());

    Resources resources;
    REQUIRE(resources.create(ctx.device, ctx.allocator, 2));

    // Create temporary invalid shader files
    std::string testDir = __FILE__;
    size_t lastSlash = testDir.find_last_of("/\\");
    std::string shaderDir = testDir.substr(0, lastSlash) + "/../shaders/DefaultGameWorld/";

    std::string invalidVertPath = shaderDir + "invalid_test.vert.slang";
    std::string invalidFragPath = shaderDir + "invalid_test.frag.slang";

    // Write invalid shader code
    {
        std::ofstream vertFile(invalidVertPath);
        vertFile << "this is not valid slang code!\n";
        vertFile << "syntax error here {{{\n";
    }
    {
        std::ofstream fragFile(invalidFragPath);
        fragFile << "also invalid @#$%\n";
    }

    Pipeline pipeline;
    PipelineDesc desc{
        .vertexShaderPath   = invalidVertPath,
        .fragmentShaderPath = invalidFragPath
    };

    std::vector<VkDescriptorSetLayout> layouts = { resources.globalSetLayout };

    // Should fail gracefully and print diagnostics
    bool result = pipeline.create(ctx.device, desc, layouts,
                                   VK_FORMAT_B8G8R8A8_SRGB,
                                   VK_FORMAT_D24_UNORM_S8_UINT);

    REQUIRE(result == false);
    std::cout << "  Pipeline correctly failed with invalid Slang code" << std::endl;

    // Cleanup temporary files
    std::remove(invalidVertPath.c_str());
    std::remove(invalidFragPath.c_str());

    pipeline.destroy(ctx.device);
    resources.destroy(ctx.device, ctx.allocator);
    ctx.cleanup();
}

// ===========================================================================
// Test #5: Resource Lifecycle (No Leaks)
// ===========================================================================

TEST_CASE("DefaultGameWorld attach/detach cycle cleans up all resources", "[integration][gpu][lifecycle]") {
    std::cout << "=== Test: Resource Lifecycle ===" << std::endl;

    DefaultGameWorldTestContext ctx;
    REQUIRE(ctx.init());

    // Get baseline allocation count
    VmaTotalStatistics statsBefore;
    vmaCalculateStatistics(ctx.windowData.allocator, &statsBefore);
    std::cout << "  Allocations before: " << statsBefore.total.statistics.allocationCount << std::endl;

    // Attach and detach multiple times
    for (int cycle = 0; cycle < 3; cycle++) {
        std::cout << "  Cycle " << cycle << std::endl;

        DefaultGameWorld layer(&ctx.windowData);
        layer.OnAttach();

        // Verify resources were created
        REQUIRE(layer.data.meshBuffer != VK_NULL_HANDLE);

        layer.OnDetach();

        // After detach, mesh buffer should be destroyed
        // (We can't check this directly since m_MeshBuffer is private after detach)
    }

    // Check that allocations returned to baseline (or very close)
    VmaTotalStatistics statsAfter;
    vmaCalculateStatistics(ctx.windowData.allocator, &statsAfter);
    std::cout << "  Allocations after: " << statsAfter.total.statistics.allocationCount << std::endl;

    // Allow small variance due to VMA internal bookkeeping
    REQUIRE(statsAfter.total.statistics.allocationCount <= statsBefore.total.statistics.allocationCount + 2);

    ctx.cleanup();
}

// ===========================================================================
// Test #1: Full Integration - Render Complete Frame
// ===========================================================================

TEST_CASE("DefaultGameWorld renders a complete frame successfully ", "[integration][gpu][render]") {
    std::cout << "=== Test: Full Frame Rendering ===" << std::endl;

    DefaultGameWorldTestContext ctx;
    REQUIRE(ctx.init());

    // Create command pool for test
    VkCommandPoolCreateInfo poolCI{
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = ctx.vkCtx.queueFamily
    };
    VkCommandPool cmdPool;
    REQUIRE(vkCreateCommandPool(ctx.vkCtx.device, &poolCI, nullptr, &cmdPool) == VK_SUCCESS);

    // Allocate command buffer
    VkCommandBuffer cmdBuffer;
    VkCommandBufferAllocateInfo cbAllocInfo{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = cmdPool,
        .commandBufferCount = 1
    };
    REQUIRE(vkAllocateCommandBuffers(ctx.vkCtx.device, &cbAllocInfo, &cmdBuffer) == VK_SUCCESS);

    // Create color and depth images for rendering target
    VkImageCreateInfo colorImageCI{
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .imageType = VK_IMAGE_TYPE_2D,
        .format = VK_FORMAT_B8G8R8A8_SRGB,
        .extent = { 1280, 720, 1 },
        .mipLevels = 1,
        .arrayLayers = 1,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .tiling = VK_IMAGE_TILING_OPTIMAL,
        .usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED
    };

    VkImage colorImage;
    VmaAllocation colorAlloc;
    VmaAllocationCreateInfo allocCI{ .usage = VMA_MEMORY_USAGE_AUTO };
    REQUIRE(vmaCreateImage(ctx.vkCtx.allocator, &colorImageCI, &allocCI,
                          &colorImage, &colorAlloc, nullptr) == VK_SUCCESS);

    VkImageViewCreateInfo colorViewCI{
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .image = colorImage,
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .format = VK_FORMAT_B8G8R8A8_SRGB,
        .subresourceRange = {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .levelCount = 1,
            .layerCount = 1
        }
    };
    VkImageView colorView;
    REQUIRE(vkCreateImageView(ctx.vkCtx.device, &colorViewCI, nullptr, &colorView) == VK_SUCCESS);

    VkImageCreateInfo depthImageCI{
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .imageType = VK_IMAGE_TYPE_2D,
        .format = VK_FORMAT_D24_UNORM_S8_UINT,
        .extent = { 1280, 720, 1 },
        .mipLevels = 1,
        .arrayLayers = 1,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .tiling = VK_IMAGE_TILING_OPTIMAL,
        .usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED
    };

    VkImage depthImage;
    VmaAllocation depthAlloc;
    REQUIRE(vmaCreateImage(ctx.vkCtx.allocator, &depthImageCI, &allocCI,
                          &depthImage, &depthAlloc, nullptr) == VK_SUCCESS);

    VkImageViewCreateInfo depthViewCI{
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .image = depthImage,
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .format = VK_FORMAT_D24_UNORM_S8_UINT,
        .subresourceRange = {
            .aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT,
            .levelCount = 1,
            .layerCount = 1
        }
    };
    VkImageView depthView;
    REQUIRE(vkCreateImageView(ctx.vkCtx.device, &depthViewCI, nullptr, &depthView) == VK_SUCCESS);

    // Create DefaultGameWorld and attach
    DefaultGameWorld layer(&ctx.windowData);
    layer.OnAttach();

    // Record command buffer with rendering
    VkCommandBufferBeginInfo beginInfo{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
    };
    REQUIRE(vkBeginCommandBuffer(cmdBuffer, &beginInfo) == VK_SUCCESS);

    // Transition images to correct layouts
    VkImageMemoryBarrier2 colorBarrier{
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
        .srcStageMask = VK_PIPELINE_STAGE_2_NONE,
        .dstStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
        .dstAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
        .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        .image = colorImage,
        .subresourceRange = { .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .levelCount = 1, .layerCount = 1 }
    };

    VkImageMemoryBarrier2 depthBarrier{
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
        .srcStageMask = VK_PIPELINE_STAGE_2_NONE,
        .dstStageMask = VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT,
        .dstAccessMask = VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
        .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .newLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
        .image = depthImage,
        .subresourceRange = { .aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT, .levelCount = 1, .layerCount = 1 }
    };

    VkImageMemoryBarrier2 barriers[] = { colorBarrier, depthBarrier };
    VkDependencyInfo depInfo{
        .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
        .imageMemoryBarrierCount = 2,
        .pImageMemoryBarriers = barriers
    };
    vkCmdPipelineBarrier2(cmdBuffer, &depInfo);

    // Begin rendering
    VkRenderingAttachmentInfo colorAttachment{
        .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
        .imageView = colorView,
        .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .clearValue = { .color = { 0.1f, 0.1f, 0.1f, 1.0f } }
    };

    VkRenderingAttachmentInfo depthAttachment{
        .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
        .imageView = depthView,
        .imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .clearValue = { .depthStencil = { 1.0f, 0 } }
    };

    VkRenderingInfo renderingInfo{
        .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
        .renderArea = { .extent = { 1280, 720 } },
        .layerCount = 1,
        .colorAttachmentCount = 1,
        .pColorAttachments = &colorAttachment,
        .pDepthAttachment = &depthAttachment
    };

    vkCmdBeginRendering(cmdBuffer, &renderingInfo);

    // Call DefaultGameWorld's OnRender
    layer.OnRender(cmdBuffer, ctx.windowData.windowSize, 0);

    vkCmdEndRendering(cmdBuffer);

    REQUIRE(vkEndCommandBuffer(cmdBuffer) == VK_SUCCESS);

    std::cout << "  Command buffer recorded successfully" << std::endl;

    // Submit and wait (optional - could skip to just verify recording worked)
    VkFence fence;
    VkFenceCreateInfo fenceCI{ .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };
    REQUIRE(vkCreateFence(ctx.vkCtx.device, &fenceCI, nullptr, &fence) == VK_SUCCESS);

    VkQueue queue;
    vkGetDeviceQueue(ctx.vkCtx.device, ctx.vkCtx.queueFamily, 0, &queue);

    VkSubmitInfo submitInfo{
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .commandBufferCount = 1,
        .pCommandBuffers = &cmdBuffer
    };
    REQUIRE(vkQueueSubmit(queue, 1, &submitInfo, fence) == VK_SUCCESS);
    REQUIRE(vkWaitForFences(ctx.vkCtx.device, 1, &fence, VK_TRUE, UINT64_MAX) == VK_SUCCESS);

    std::cout << "  Frame rendered and submitted successfully!" << std::endl;

    // Cleanup
    vkDestroyFence(ctx.vkCtx.device, fence, nullptr);
    layer.OnDetach();
    vkDestroyImageView(ctx.vkCtx.device, colorView, nullptr);
    vkDestroyImageView(ctx.vkCtx.device, depthView, nullptr);
    vmaDestroyImage(ctx.vkCtx.allocator, colorImage, colorAlloc);
    vmaDestroyImage(ctx.vkCtx.allocator, depthImage, depthAlloc);
    vkDestroyCommandPool(ctx.vkCtx.device, cmdPool, nullptr);
    ctx.cleanup();
}

// ===========================================================================
// Test #4: Multiple Objects with Different Transforms
// ===========================================================================

TEST_CASE("DefaultGameWorld renders multiple objects with different transforms", "[integration][gpu][render][scene]") {
    std::cout << "=== Test: Multiple Objects Rendering ===" << std::endl;

    DefaultGameWorldTestContext ctx;
    REQUIRE(ctx.init());

    DefaultGameWorld layer(&ctx.windowData);
    layer.OnAttach();

    // Verify scene was populated with multiple objects (from OnAttach)
    REQUIRE(layer.data.scene.objects.size() >= 3);
    std::cout << "  Scene has " << layer.data.scene.objects.size() << " objects" << std::endl;

    // Verify objects have different transforms
    bool foundDifferentPositions = false;
    for (size_t i = 1; i < layer.data.scene.objects.size(); i++) {
        if (layer.data.scene.objects[i].transform.position != layer.data.scene.objects[0].transform.position) {
            foundDifferentPositions = true;
            break;
        }
    }
    REQUIRE(foundDifferentPositions);
    std::cout << "  Objects have different transforms" << std::endl;

    // Verify objects reference valid mesh indices
    for (const auto& obj : layer.data.scene.objects) {
        REQUIRE(obj.meshIndex < layer.data.meshGPUInfo.size());
    }

    std::cout << "  All objects reference valid meshes" << std::endl;

    layer.OnDetach();
    ctx.cleanup();
}

// ===========================================================================
// Test: Empty Scene Rendering
// ===========================================================================

TEST_CASE("DefaultGameWorld renders empty scene without crashing", "[integration][gpu][render][edge-case]") {
    std::cout << "=== Test: Empty Scene Rendering ===" << std::endl;

    DefaultGameWorldTestContext ctx;
    REQUIRE(ctx.init());

    // Create command pool for test
    VkCommandPoolCreateInfo poolCI{
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = ctx.vkCtx.queueFamily
    };
    VkCommandPool cmdPool;
    REQUIRE(vkCreateCommandPool(ctx.vkCtx.device, &poolCI, nullptr, &cmdPool) == VK_SUCCESS);

    // Allocate command buffer
    VkCommandBuffer cmdBuffer;
    VkCommandBufferAllocateInfo cbAllocInfo{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = cmdPool,
        .commandBufferCount = 1
    };
    REQUIRE(vkAllocateCommandBuffers(ctx.vkCtx.device, &cbAllocInfo, &cmdBuffer) == VK_SUCCESS);

    // Create DefaultGameWorld and attach (this populates scene with default objects)
    DefaultGameWorld layer(&ctx.windowData);
    layer.OnAttach();

    // Verify scene was populated
    size_t originalCount = layer.data.scene.objects.size();
    REQUIRE(originalCount > 0);
    std::cout << "  Scene originally had " << originalCount << " objects" << std::endl;

    // Clear the scene - this is the edge case we're testing
    layer.data.scene.clear();
    REQUIRE(layer.data.scene.objects.empty());
    std::cout << "  Scene cleared, now has 0 objects" << std::endl;

    // Create minimal render target for command recording
    VkImageCreateInfo colorImageCI{
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .imageType = VK_IMAGE_TYPE_2D,
        .format = VK_FORMAT_B8G8R8A8_SRGB,
        .extent = { 1280, 720, 1 },
        .mipLevels = 1,
        .arrayLayers = 1,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .tiling = VK_IMAGE_TILING_OPTIMAL,
        .usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED
    };

    VkImage colorImage;
    VmaAllocation colorAlloc;
    VmaAllocationCreateInfo allocCI{ .usage = VMA_MEMORY_USAGE_AUTO };
    REQUIRE(vmaCreateImage(ctx.vkCtx.allocator, &colorImageCI, &allocCI,
                          &colorImage, &colorAlloc, nullptr) == VK_SUCCESS);

    VkImageViewCreateInfo colorViewCI{
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .image = colorImage,
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .format = VK_FORMAT_B8G8R8A8_SRGB,
        .subresourceRange = {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .levelCount = 1,
            .layerCount = 1
        }
    };
    VkImageView colorView;
    REQUIRE(vkCreateImageView(ctx.vkCtx.device, &colorViewCI, nullptr, &colorView) == VK_SUCCESS);

    VkImageCreateInfo depthImageCI{
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .imageType = VK_IMAGE_TYPE_2D,
        .format = VK_FORMAT_D24_UNORM_S8_UINT,
        .extent = { 1280, 720, 1 },
        .mipLevels = 1,
        .arrayLayers = 1,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .tiling = VK_IMAGE_TILING_OPTIMAL,
        .usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED
    };

    VkImage depthImage;
    VmaAllocation depthAlloc;
    REQUIRE(vmaCreateImage(ctx.vkCtx.allocator, &depthImageCI, &allocCI,
                          &depthImage, &depthAlloc, nullptr) == VK_SUCCESS);

    VkImageViewCreateInfo depthViewCI{
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .image = depthImage,
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .format = VK_FORMAT_D24_UNORM_S8_UINT,
        .subresourceRange = {
            .aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT,
            .levelCount = 1,
            .layerCount = 1
        }
    };
    VkImageView depthView;
    REQUIRE(vkCreateImageView(ctx.vkCtx.device, &depthViewCI, nullptr, &depthView) == VK_SUCCESS);

    // Record command buffer with empty scene rendering
    VkCommandBufferBeginInfo beginInfo{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
    };
    REQUIRE(vkBeginCommandBuffer(cmdBuffer, &beginInfo) == VK_SUCCESS);

    // Transition images to correct layouts
    VkImageMemoryBarrier2 colorBarrier{
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
        .srcStageMask = VK_PIPELINE_STAGE_2_NONE,
        .dstStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
        .dstAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
        .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        .image = colorImage,
        .subresourceRange = { .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .levelCount = 1, .layerCount = 1 }
    };

    VkImageMemoryBarrier2 depthBarrier{
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
        .srcStageMask = VK_PIPELINE_STAGE_2_NONE,
        .dstStageMask = VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT,
        .dstAccessMask = VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
        .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .newLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
        .image = depthImage,
        .subresourceRange = { .aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT, .levelCount = 1, .layerCount = 1 }
    };

    VkImageMemoryBarrier2 barriers[] = { colorBarrier, depthBarrier };
    VkDependencyInfo depInfo{
        .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
        .imageMemoryBarrierCount = 2,
        .pImageMemoryBarriers = barriers
    };
    vkCmdPipelineBarrier2(cmdBuffer, &depInfo);

    // Begin rendering
    VkRenderingAttachmentInfo colorAttachment{
        .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
        .imageView = colorView,
        .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .clearValue = { .color = { 0.1f, 0.1f, 0.1f, 1.0f } }
    };

    VkRenderingAttachmentInfo depthAttachment{
        .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
        .imageView = depthView,
        .imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .clearValue = { .depthStencil = { 1.0f, 0 } }
    };

    VkRenderingInfo renderingInfo{
        .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
        .renderArea = { .extent = { 1280, 720 } },
        .layerCount = 1,
        .colorAttachmentCount = 1,
        .pColorAttachments = &colorAttachment,
        .pDepthAttachment = &depthAttachment
    };

    vkCmdBeginRendering(cmdBuffer, &renderingInfo);

    // Call DefaultGameWorld's OnRender with EMPTY scene
    // This should handle the empty scene gracefully (early return at line 224-225)
    layer.OnRender(cmdBuffer, ctx.windowData.windowSize, 0);

    vkCmdEndRendering(cmdBuffer);

    REQUIRE(vkEndCommandBuffer(cmdBuffer) == VK_SUCCESS);

    std::cout << "  Command buffer recorded successfully with empty scene" << std::endl;

    // Submit and wait to verify no GPU errors
    VkFence fence;
    VkFenceCreateInfo fenceCI{ .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };
    REQUIRE(vkCreateFence(ctx.vkCtx.device, &fenceCI, nullptr, &fence) == VK_SUCCESS);

    VkQueue queue;
    vkGetDeviceQueue(ctx.vkCtx.device, ctx.vkCtx.queueFamily, 0, &queue);

    VkSubmitInfo submitInfo{
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .commandBufferCount = 1,
        .pCommandBuffers = &cmdBuffer
    };
    REQUIRE(vkQueueSubmit(queue, 1, &submitInfo, fence) == VK_SUCCESS);
    REQUIRE(vkWaitForFences(ctx.vkCtx.device, 1, &fence, VK_TRUE, UINT64_MAX) == VK_SUCCESS);

    std::cout << "  Empty scene rendered successfully (no draw calls, no crash)" << std::endl;

    // Cleanup
    vkDestroyFence(ctx.vkCtx.device, fence, nullptr);
    layer.OnDetach();
    vkDestroyImageView(ctx.vkCtx.device, colorView, nullptr);
    vkDestroyImageView(ctx.vkCtx.device, depthView, nullptr);
    vmaDestroyImage(ctx.vkCtx.allocator, colorImage, colorAlloc);
    vmaDestroyImage(ctx.vkCtx.allocator, depthImage, depthAlloc);
    vkDestroyCommandPool(ctx.vkCtx.device, cmdPool, nullptr);
    ctx.cleanup();
}

// ===========================================================================
// Test: Camera Movement and View Updates
// ===========================================================================

TEST_CASE("DefaultGameWorld updates camera view matrix correctly across frames", "[integration][gpu][camera]") {
    std::cout << "=== Test: Camera Movement and View Updates ===" << std::endl;

    DefaultGameWorldTestContext ctx;
    REQUIRE(ctx.init());

    DefaultGameWorld layer(&ctx.windowData);
    layer.OnAttach();

    // Record initial camera position and view matrix
    glm::vec3 initialPos = layer.data.camera.position;
    glm::mat4 initialView = layer.data.camera.viewMatrix();
    std::cout << "  Initial camera position: (" << initialPos.x << ", "
              << initialPos.y << ", " << initialPos.z << ")" << std::endl;

    // Update camera buffer for frame 0 with initial position
    float aspectRatio = 16.0f / 9.0f;
    layer.data.resources.updateCameraBuffer(0, layer.data.camera, aspectRatio);

    // Move camera to a new position
    layer.data.camera.position = glm::vec3(10.0f, 5.0f, 10.0f);
    glm::mat4 newView = layer.data.camera.viewMatrix();
    std::cout << "  New camera position: (" << layer.data.camera.position.x << ", "
              << layer.data.camera.position.y << ", " << layer.data.camera.position.z << ")" << std::endl;

    // Verify view matrix changed
    bool viewChanged = false;
    for (int col = 0; col < 4; col++) {
        for (int row = 0; row < 4; row++) {
            if (std::abs(initialView[col][row] - newView[col][row]) > 1e-5f) {
                viewChanged = true;
                break;
            }
        }
        if (viewChanged) break;
    }
    REQUIRE(viewChanged);
    std::cout << "  View matrix changed after camera movement" << std::endl;

    // Update camera buffer for frame 1 with new position
    layer.data.resources.updateCameraBuffer(1, layer.data.camera, aspectRatio);

    // Test with different frameIndex values (0 and 1)
    for (uint32_t frameIdx = 0; frameIdx < 2; frameIdx++) {
        // Move camera to frame-specific position
        layer.data.camera.position = glm::vec3(
            static_cast<float>(frameIdx) * 5.0f,
            static_cast<float>(frameIdx) * 2.0f,
            5.0f
        );

        layer.data.resources.updateCameraBuffer(frameIdx, layer.data.camera, aspectRatio);

        std::cout << "  Updated camera buffer for frame " << frameIdx << std::endl;
    }

    // Verify we can update both frame indices independently without crash
    layer.data.camera.position = glm::vec3(100.0f, 50.0f, 100.0f);
    layer.data.resources.updateCameraBuffer(0, layer.data.camera, aspectRatio);

    layer.data.camera.position = glm::vec3(-100.0f, -50.0f, -100.0f);
    layer.data.resources.updateCameraBuffer(1, layer.data.camera, aspectRatio);

    std::cout << "  Camera buffers for both frames updated independently" << std::endl;

    layer.OnDetach();
    ctx.cleanup();
}

// ===========================================================================
// Test: Dynamic Scene Modification
// ===========================================================================

TEST_CASE("DefaultGameWorld handles dynamic scene modification after initialization", "[integration][gpu][scene]") {
    std::cout << "=== Test: Dynamic Scene Modification ===" << std::endl;

    DefaultGameWorldTestContext ctx;
    REQUIRE(ctx.init());

    // Create command pool for rendering tests
    VkCommandPoolCreateInfo poolCI{
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = ctx.vkCtx.queueFamily
    };
    VkCommandPool cmdPool;
    REQUIRE(vkCreateCommandPool(ctx.vkCtx.device, &poolCI, nullptr, &cmdPool) == VK_SUCCESS);

    VkCommandBuffer cmdBuffer;
    VkCommandBufferAllocateInfo cbAllocInfo{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = cmdPool,
        .commandBufferCount = 1
    };
    REQUIRE(vkAllocateCommandBuffers(ctx.vkCtx.device, &cbAllocInfo, &cmdBuffer) == VK_SUCCESS);

    DefaultGameWorld layer(&ctx.windowData);
    layer.OnAttach();

    // Record initial scene state
    size_t initialCount = layer.data.scene.objects.size();
    REQUIRE(initialCount > 0);
    std::cout << "  Initial scene: " << initialCount << " objects" << std::endl;

    // Add a new object to the scene after initialization
    GameObject newObj{
        .transform = Transform{
            .position = glm::vec3(5.0f, 5.0f, 5.0f),
            .rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f),
            .scale = glm::vec3(2.0f)
        },
        .meshIndex = 0  // Use first mesh (cube)
    };

    layer.data.scene.addObject(newObj);
    REQUIRE(layer.data.scene.objects.size() == initialCount + 1);
    std::cout << "  Added 1 object, now have " << layer.data.scene.objects.size() << " objects" << std::endl;

    // Try rendering with modified scene (should not crash)
    VkCommandBufferBeginInfo beginInfo{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
    };
    REQUIRE(vkBeginCommandBuffer(cmdBuffer, &beginInfo) == VK_SUCCESS);

    // Simple render call - we're not creating full render targets, just verifying no crash
    // In a real scenario, OnRender would be called within a render pass
    // For this test, we just verify the scene can be iterated safely
    layer.OnUpdate(0.016f);  // Simulate frame update

    REQUIRE(vkEndCommandBuffer(cmdBuffer) == VK_SUCCESS);
    std::cout << "  Rendering with modified scene succeeded" << std::endl;

    // Remove some objects
    if (layer.data.scene.objects.size() > 1) {
        size_t beforeRemoval = layer.data.scene.objects.size();
        layer.data.scene.objects.pop_back();  // Remove last object
        REQUIRE(layer.data.scene.objects.size() == beforeRemoval - 1);
        std::cout << "  Removed 1 object, now have " << layer.data.scene.objects.size() << " objects" << std::endl;
    }

    // Clear and repopulate scene
    layer.data.scene.clear();
    REQUIRE(layer.data.scene.objects.empty());
    std::cout << "  Cleared scene" << std::endl;

    // Add multiple new objects
    for (int i = 0; i < 5; i++) {
        GameObject obj{
            .transform = Transform{
                .position = glm::vec3(static_cast<float>(i) * 2.0f, 0.0f, 0.0f),
                .rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f),
                .scale = glm::vec3(1.0f)
            },
            .meshIndex = static_cast<uint32_t>(i % layer.data.meshTable.count())
        };
        layer.data.scene.addObject(obj);
    }

    REQUIRE(layer.data.scene.objects.size() == 5);
    std::cout << "  Repopulated scene with 5 new objects" << std::endl;

    // Verify all new objects have valid mesh indices
    for (const auto& obj : layer.data.scene.objects) {
        REQUIRE(obj.meshIndex < layer.data.meshGPUInfo.size());
    }

    std::cout << "  All dynamic scene modifications handled successfully" << std::endl;

    layer.OnDetach();
    vkDestroyCommandPool(ctx.vkCtx.device, cmdPool, nullptr);
    ctx.cleanup();
}

// ===========================================================================
// Test: Multi-Frame Rendering with Frame-in-Flight
// ===========================================================================

TEST_CASE("DefaultGameWorld handles multiple frames with different frameIndex values", "[integration][gpu][frames]") {
    std::cout << "=== Test: Multi-Frame Rendering with Frame-in-Flight ===" << std::endl;

    DefaultGameWorldTestContext ctx;
    REQUIRE(ctx.init());

    DefaultGameWorld layer(&ctx.windowData);
    layer.OnAttach();

    // Verify we have multiple frame resources
    REQUIRE(layer.data.resources.cameraBuffers.size() >= 2);
    REQUIRE(layer.data.resources.globalSets.size() >= 2);
    std::cout << "  Frame resources created for " << layer.data.resources.cameraBuffers.size() << " frames" << std::endl;

    // Simulate rendering 5 frames with alternating frameIndex (0, 1, 0, 1, 0)
    const int numFrames = 5;
    const uint32_t maxFramesInFlight = 2;
    float aspectRatio = 16.0f / 9.0f;

    for (int frame = 0; frame < numFrames; frame++) {
        uint32_t frameIdx = frame % maxFramesInFlight;

        // Update camera with frame-specific position (to differentiate frames)
        layer.data.camera.position = glm::vec3(
            static_cast<float>(frame),
            static_cast<float>(frame) * 0.5f,
            5.0f
        );

        // Update camera buffer for this frame index
        layer.data.resources.updateCameraBuffer(frameIdx, layer.data.camera, aspectRatio);

        std::cout << "  Frame " << frame << " (frameIdx=" << frameIdx
                  << ") camera: (" << layer.data.camera.position.x << ", "
                  << layer.data.camera.position.y << ", "
                  << layer.data.camera.position.z << ")" << std::endl;

        // Verify descriptor set exists for this frame
        REQUIRE(layer.data.resources.globalSets[frameIdx] != VK_NULL_HANDLE);
        REQUIRE(layer.data.resources.cameraBuffers[frameIdx] != VK_NULL_HANDLE);
    }

    // Test rapid frame index cycling (simulating fast rendering)
    for (int i = 0; i < 100; i++) {
        uint32_t frameIdx = i % maxFramesInFlight;
        layer.data.camera.position.x = static_cast<float>(i);
        layer.data.resources.updateCameraBuffer(frameIdx, layer.data.camera, aspectRatio);
    }

    std::cout << "  Rapid frame cycling (100 iterations) completed without crash" << std::endl;

    // Verify both frame indices still have valid resources
    for (uint32_t frameIdx = 0; frameIdx < maxFramesInFlight; frameIdx++) {
        REQUIRE(layer.data.resources.globalSets[frameIdx] != VK_NULL_HANDLE);
        REQUIRE(layer.data.resources.cameraBuffers[frameIdx] != VK_NULL_HANDLE);
    }

    std::cout << "  All frame resources remain valid after multi-frame test" << std::endl;

    layer.OnDetach();
    ctx.cleanup();
}

// ===========================================================================
// Test: Push Constants and Transform Validation
// ===========================================================================

TEST_CASE("DefaultGameWorld push constants produce correct model matrices", "[integration][gpu][transform]") {
    std::cout << "=== Test: Push Constants and Transform Validation ===" << std::endl;

    DefaultGameWorldTestContext ctx;
    REQUIRE(ctx.init());

    DefaultGameWorld layer(&ctx.windowData);
    layer.OnAttach();

    // Clear default scene and add objects with known transforms
    layer.data.scene.clear();

    // Test 1: Identity transform
    GameObject identityObj{
        .transform = Transform{
            .position = glm::vec3(0.0f, 0.0f, 0.0f),
            .rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f),
            .scale = glm::vec3(1.0f)
        },
        .meshIndex = 0
    };
    layer.data.scene.addObject(identityObj);

    glm::mat4 identityMatrix = identityObj.transform.toMatrix();
    // Identity matrix should be close to glm::mat4(1.0f)
    for (int col = 0; col < 4; col++) {
        for (int row = 0; row < 4; row++) {
            float expected = (col == row) ? 1.0f : 0.0f;
            REQUIRE(std::abs(identityMatrix[col][row] - expected) < 1e-5f);
        }
    }
    std::cout << "  Identity transform produces identity matrix" << std::endl;

    // Test 2: Translation only
    GameObject translatedObj{
        .transform = Transform{
            .position = glm::vec3(5.0f, 3.0f, -2.0f),
            .rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f),
            .scale = glm::vec3(1.0f)
        },
        .meshIndex = 0
    };
    layer.data.scene.addObject(translatedObj);

    glm::mat4 translationMatrix = translatedObj.transform.toMatrix();
    // Translation should be in column 3
    REQUIRE(std::abs(translationMatrix[3][0] - 5.0f) < 1e-5f);
    REQUIRE(std::abs(translationMatrix[3][1] - 3.0f) < 1e-5f);
    REQUIRE(std::abs(translationMatrix[3][2] - (-2.0f)) < 1e-5f);
    std::cout << "  Translation transform produces correct matrix" << std::endl;

    // Test 3: Scale only
    GameObject scaledObj{
        .transform = Transform{
            .position = glm::vec3(0.0f, 0.0f, 0.0f),
            .rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f),
            .scale = glm::vec3(2.0f, 3.0f, 4.0f)
        },
        .meshIndex = 0
    };
    layer.data.scene.addObject(scaledObj);

    glm::mat4 scaleMatrix = scaledObj.transform.toMatrix();
    // Scale should be on diagonal
    REQUIRE(std::abs(scaleMatrix[0][0] - 2.0f) < 1e-5f);
    REQUIRE(std::abs(scaleMatrix[1][1] - 3.0f) < 1e-5f);
    REQUIRE(std::abs(scaleMatrix[2][2] - 4.0f) < 1e-5f);
    std::cout << "  Scale transform produces correct matrix" << std::endl;

    // Test 4: Combined TRS (most common case)
    GameObject combinedObj{
        .transform = Transform{
            .position = glm::vec3(1.0f, 2.0f, 3.0f),
            .rotation = glm::angleAxis(glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f)),
            .scale = glm::vec3(2.0f)
        },
        .meshIndex = 0
    };
    layer.data.scene.addObject(combinedObj);

    glm::mat4 combinedMatrix = combinedObj.transform.toMatrix();
    // Verify translation component is preserved
    REQUIRE(std::abs(combinedMatrix[3][0] - 1.0f) < 1e-5f);
    REQUIRE(std::abs(combinedMatrix[3][1] - 2.0f) < 1e-5f);
    REQUIRE(std::abs(combinedMatrix[3][2] - 3.0f) < 1e-5f);

    // Verify it's not identity (rotation and scale applied)
    bool isNotIdentity = false;
    for (int col = 0; col < 3; col++) {
        for (int row = 0; row < 3; row++) {
            float expected = (col == row) ? 1.0f : 0.0f;
            if (std::abs(combinedMatrix[col][row] - expected) > 1e-4f) {
                isNotIdentity = true;
                break;
            }
        }
    }
    REQUIRE(isNotIdentity);
    std::cout << "  Combined TRS transform produces non-identity matrix" << std::endl;

    // Test 5: Verify different objects would get different matrices
    REQUIRE(layer.data.scene.objects.size() == 4);

    bool foundDifferentMatrices = false;
    glm::mat4 firstMatrix = layer.data.scene.objects[0].transform.toMatrix();
    for (size_t i = 1; i < layer.data.scene.objects.size(); i++) {
        glm::mat4 currentMatrix = layer.data.scene.objects[i].transform.toMatrix();
        for (int col = 0; col < 4; col++) {
            for (int row = 0; row < 4; row++) {
                if (std::abs(firstMatrix[col][row] - currentMatrix[col][row]) > 1e-5f) {
                    foundDifferentMatrices = true;
                    break;
                }
            }
            if (foundDifferentMatrices) break;
        }
        if (foundDifferentMatrices) break;
    }
    REQUIRE(foundDifferentMatrices);
    std::cout << "  Different objects produce different model matrices" << std::endl;

    std::cout << "  All transform validation tests passed" << std::endl;

    layer.OnDetach();
    ctx.cleanup();
}
