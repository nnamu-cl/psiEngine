#define VOLK_IMPLEMENTATION
#define VMA_IMPLEMENTATION
#include "ApplicationWindow.h"

#include <vector>
#include <algorithm>
#include <volk/volk.h>
#include <SDL3/SDL_vulkan.h>

#include "Application.h"

#include "VulkanChecks.h"
#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_vulkan.h"
#include "IconsLucide.h"
#include "FontBinaries/lucide.h"

// Define the static members
ApplicationWindow* ApplicationWindow::instance = nullptr;
ImFont* ApplicationWindow::iconFont = nullptr;
float ApplicationWindow::iconFontSize = 0.0f;




ApplicationWindow::ApplicationWindow(ApplicationWindowSpecifications& spec)
{
    specification = &spec;
    instance = this;
}

bool ApplicationWindow::Init() {

    chk(SDL_Init(SDL_INIT_VIDEO));
    chk(SDL_Vulkan_LoadLibrary(NULL));
    volkInitialize();



    // Instance
    VkApplicationInfo appInfo{ .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO, .pApplicationName = specification->title, .apiVersion = VK_API_VERSION_1_3 };
    uint32_t instanceExtensionsCount{ 0 };
    char const* const* instanceExtensions{ SDL_Vulkan_GetInstanceExtensions(&instanceExtensionsCount) };
    VkInstanceCreateInfo instanceCI{
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pApplicationInfo = &appInfo,
        .enabledExtensionCount = instanceExtensionsCount,
        .ppEnabledExtensionNames = instanceExtensions,
    };
    chk(vkCreateInstance(&instanceCI, nullptr, &data.vkInstance));
    volkLoadInstance(data.vkInstance);

    // Physical Device
    uint32_t deviceCount{ 0 };
    chk(vkEnumeratePhysicalDevices(data.vkInstance, &deviceCount, nullptr));
    std::vector<VkPhysicalDevice> devices(deviceCount);
    chk(vkEnumeratePhysicalDevices(data.vkInstance, &deviceCount, devices.data()));
    uint32_t deviceIndex{ 0 };
    data.physicalDevice = devices[deviceIndex];

    vkGetPhysicalDeviceProperties2(data.physicalDevice, &data.physicalDeviceProperties);
    std::cout << "Selected device: " << data.physicalDeviceProperties.properties.deviceName << "\n";

    // Find a queue family for graphics
    uint32_t queueFamilyCount{ 0 };
    vkGetPhysicalDeviceQueueFamilyProperties(data.physicalDevice, &queueFamilyCount, nullptr);
    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(data.physicalDevice, &queueFamilyCount, queueFamilies.data());
    for (size_t i = 0; i < queueFamilies.size(); i++) {
        if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            data.queueFamily = i;
            break;
        }
    }

    // Logical device
    const float qfpriorities{ 1.0f };
    VkDeviceQueueCreateInfo queueCI{ .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO, .queueFamilyIndex = data.queueFamily, .queueCount = 1, .pQueuePriorities = &qfpriorities };
    VkPhysicalDeviceVulkan12Features enabledVk12Features{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES, .descriptorIndexing = true, .descriptorBindingVariableDescriptorCount = true, .runtimeDescriptorArray = true, .bufferDeviceAddress = true };
    VkPhysicalDeviceVulkan13Features enabledVk13Features{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES, .pNext = &enabledVk12Features, .synchronization2 = true, .dynamicRendering = true };
    const std::vector<const char*> deviceExtensions{ VK_KHR_SWAPCHAIN_EXTENSION_NAME };
    const VkPhysicalDeviceFeatures enabledVk10Features{ .samplerAnisotropy = VK_TRUE };
    VkDeviceCreateInfo deviceCI{
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .pNext = &enabledVk13Features,
        .queueCreateInfoCount = 1,
        .pQueueCreateInfos = &queueCI,
        .enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size()),
        .ppEnabledExtensionNames = deviceExtensions.data(),
        .pEnabledFeatures = &enabledVk10Features
    };
    chk(vkCreateDevice(data.physicalDevice, &deviceCI, nullptr, &data.device));
    vkGetDeviceQueue(data.device, data.queueFamily, 0, &data.queue);

    // VMA
    VmaVulkanFunctions vkFunctions{ .vkGetInstanceProcAddr = vkGetInstanceProcAddr, .vkGetDeviceProcAddr = vkGetDeviceProcAddr, .vkCreateImage = vkCreateImage };
    VmaAllocatorCreateInfo allocatorCI{ .flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT, .physicalDevice = data.physicalDevice, .device = data.device, .pVulkanFunctions = &vkFunctions, .instance = data.vkInstance };
    chk(vmaCreateAllocator(&allocatorCI, &data.allocator));

    // Window and surface
    data.sdlWindow = SDL_CreateWindow(specification->title, specification->w, specification->h, specification->flags);
    chk(SDL_Vulkan_CreateSurface(data.sdlWindow, data.vkInstance, nullptr, &data.surface));
    chk(SDL_GetWindowSize(data.sdlWindow, &data.windowSize.x, &data.windowSize.y));
    VkSurfaceCapabilitiesKHR surfaceCaps{};
    chk(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(data.physicalDevice, data.surface, &surfaceCaps));

    // Swapchain
    data.swapchainImageFormat = VK_FORMAT_B8G8R8A8_UNORM;
    VkSwapchainCreateInfoKHR swapchainCI{
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .surface = data.surface,
        .minImageCount = surfaceCaps.minImageCount,
        .imageFormat = data.swapchainImageFormat,
        .imageColorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR,
        .imageExtent{.width = surfaceCaps.currentExtent.width, .height = surfaceCaps.currentExtent.height},
        .imageArrayLayers = 1,
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        .preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,
        .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .presentMode = VK_PRESENT_MODE_FIFO_KHR
    };
    chk(vkCreateSwapchainKHR(data.device, &swapchainCI, nullptr, &data.swapchain));
    uint32_t imageCount{ 0 };
    chk(vkGetSwapchainImagesKHR(data.device, data.swapchain, &imageCount, nullptr));
    data.swapchainImages.resize(imageCount);
    chk(vkGetSwapchainImagesKHR(data.device, data.swapchain, &imageCount, data.swapchainImages.data()));
    data.swapchainImageViews.resize(imageCount);
    for (uint32_t i = 0; i < imageCount; i++) {
        VkImageViewCreateInfo viewCI{ .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO, .image = data.swapchainImages[i], .viewType = VK_IMAGE_VIEW_TYPE_2D, .format = data.swapchainImageFormat, .subresourceRange{.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .levelCount = 1, .layerCount = 1} };
        chk(vkCreateImageView(data.device, &viewCI, nullptr, &data.swapchainImageViews[i]));
    }

    // Depth attachment
    std::vector<VkFormat> depthFormatList{ VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT };
    for (VkFormat format : depthFormatList) {
        VkFormatProperties2 formatProperties{ .sType = VK_STRUCTURE_TYPE_FORMAT_PROPERTIES_2 };
        vkGetPhysicalDeviceFormatProperties2(data.physicalDevice, format, &formatProperties);
        if (formatProperties.formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) {
            data.depthFormat = format;
            break;
        }
    }
    VkImageCreateInfo depthImageCI{
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .imageType = VK_IMAGE_TYPE_2D,
        .format = data.depthFormat,
        .extent{.width = static_cast<uint32_t>(data.windowSize.x), .height = static_cast<uint32_t>(data.windowSize.y), .depth = 1},
        .mipLevels = 1,
        .arrayLayers = 1,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .tiling = VK_IMAGE_TILING_OPTIMAL,
        .usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
    };
    VmaAllocationCreateInfo depthAllocCI{ .flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT, .usage = VMA_MEMORY_USAGE_AUTO };
    chk(vmaCreateImage(data.allocator, &depthImageCI, &depthAllocCI, &data.depthImage, &data.depthImageAllocation, nullptr));
    VkImageViewCreateInfo depthViewCI{ .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO, .image = data.depthImage, .viewType = VK_IMAGE_VIEW_TYPE_2D, .format = data.depthFormat, .subresourceRange{.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT, .levelCount = 1, .layerCount = 1} };
    chk(vkCreateImageView(data.device, &depthViewCI, nullptr, &data.depthImageView));

    // Sync objects
    VkSemaphoreCreateInfo semaphoreCI{ .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
    VkFenceCreateInfo fenceCI{ .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO, .flags = VK_FENCE_CREATE_SIGNALED_BIT };
    for (uint32_t i = 0; i < maxFramesInFlight; i++) {
        chk(vkCreateFence(data.device, &fenceCI, nullptr, &data.fences[i]));
        chk(vkCreateSemaphore(data.device, &semaphoreCI, nullptr, &data.presentSemaphores[i]));
    }
    data.renderSemaphores.resize(data.swapchainImages.size());
    for (auto& semaphore : data.renderSemaphores) {
        chk(vkCreateSemaphore(data.device, &semaphoreCI, nullptr, &semaphore));
    }

    // Command pool and command buffers
    VkCommandPoolCreateInfo commandPoolCI{ .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO, .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT, .queueFamilyIndex = data.queueFamily };
    chk(vkCreateCommandPool(data.device, &commandPoolCI, nullptr, &data.commandPool));
    VkCommandBufferAllocateInfo cbAllocCI{ .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO, .commandPool = data.commandPool, .commandBufferCount = maxFramesInFlight };
    chk(vkAllocateCommandBuffers(data.device, &cbAllocCI, data.commandBuffers.data()));

    // ImGui
    init_imgui();

    return true;
}

bool ApplicationWindow::init_imgui() {
    // Tear down any existing ImGui state -- context is global, so a previous
    // ApplicationWindow (or a prior Init() call) may have left it live.
    if (ImGui::GetCurrentContext()) {
        ImGuiIO& prevIo = ImGui::GetIO();
        if (prevIo.BackendRendererUserData) ImGui_ImplVulkan_Shutdown();
        if (prevIo.BackendPlatformUserData) ImGui_ImplSDL3_Shutdown();
        ImGui::DestroyContext();
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();

    // Quickly add fonts for icons

    io.Fonts->AddFontDefault();
    float baseFontSize = 16.0f;
    float iconFontSize = baseFontSize * 1.1f;

    // merge in icons from Font Awesome
    static const ImWchar icons_ranges[] = { ICON_MIN_LC, ICON_MAX_16_LC, 0 };
    ImFontConfig icons_config;
    icons_config.MergeMode = true;
    icons_config.PixelSnapH = true;
    icons_config.FontDataOwnedByAtlas = false;
    icons_config.GlyphMinAdvanceX = iconFontSize;
    iconFont = io.Fonts->AddFontFromMemoryTTF(lucidIcons, lucidFontSize, iconFontSize, &icons_config);
    ApplicationWindow::iconFontSize = iconFontSize;



    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    ImGui_ImplSDL3_InitForVulkan(data.sdlWindow);

    ImGui_ImplVulkan_InitInfo init_info{};
    init_info.ApiVersion = VK_API_VERSION_1_3;
    init_info.Instance = data.vkInstance;
    init_info.PhysicalDevice = data.physicalDevice;
    init_info.Device = data.device;
    init_info.QueueFamily = data.queueFamily;
    init_info.Queue = data.queue;
    init_info.DescriptorPoolSize = IMGUI_IMPL_VULKAN_MINIMUM_IMAGE_SAMPLER_POOL_SIZE;
    init_info.MinImageCount = 2;
    init_info.ImageCount = static_cast<uint32_t>(data.swapchainImages.size());
    init_info.UseDynamicRendering = true;
    init_info.PipelineInfoMain.PipelineRenderingCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR,
        .colorAttachmentCount = 1,
        .pColorAttachmentFormats = &data.swapchainImageFormat,
    };
    init_info.PipelineInfoMain.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
    init_info.CheckVkResultFn = [](VkResult err) {
        if (err != VK_SUCCESS) {
            std::cerr << "ImGui Vulkan call failed: " << err << "\n";
            exit(err);
        }
    };
    ImGui_ImplVulkan_Init(&init_info);

    return true;
}

void ApplicationWindow::rebuildSwapchain()
{
    chk(vkDeviceWaitIdle(data.device));

    chk(SDL_GetWindowSize(data.sdlWindow, &data.windowSize.x, &data.windowSize.y));
    if (data.windowSize.x == 0 || data.windowSize.y == 0)
        return;

    // Destroy old image views
    for (auto view : data.swapchainImageViews)
        vkDestroyImageView(data.device, view, nullptr);
    data.swapchainImageViews.clear();

    // Surface capabilities for new extent
    VkSurfaceCapabilitiesKHR surfaceCaps{};
    chk(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(data.physicalDevice, data.surface, &surfaceCaps));

    // New swapchain, reusing old for efficient transition
    VkSwapchainKHR oldSwapchain = data.swapchain;
    VkSwapchainCreateInfoKHR swapchainCI{
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .surface = data.surface,
        .minImageCount = surfaceCaps.minImageCount,
        .imageFormat = data.swapchainImageFormat,
        .imageColorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR,
        .imageExtent{.width = surfaceCaps.currentExtent.width, .height = surfaceCaps.currentExtent.height},
        .imageArrayLayers = 1,
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        .preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,
        .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .presentMode = VK_PRESENT_MODE_FIFO_KHR,
        .oldSwapchain = oldSwapchain
    };
    chk(vkCreateSwapchainKHR(data.device, &swapchainCI, nullptr, &data.swapchain));
    vkDestroySwapchainKHR(data.device, oldSwapchain, nullptr);

    // New swapchain images and views
    uint32_t imageCount{0};
    chk(vkGetSwapchainImagesKHR(data.device, data.swapchain, &imageCount, nullptr));
    data.swapchainImages.resize(imageCount);
    chk(vkGetSwapchainImagesKHR(data.device, data.swapchain, &imageCount, data.swapchainImages.data()));
    data.swapchainImageViews.resize(imageCount);
    for (uint32_t i = 0; i < imageCount; i++) {
        VkImageViewCreateInfo viewCI{ .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO, .image = data.swapchainImages[i], .viewType = VK_IMAGE_VIEW_TYPE_2D, .format = data.swapchainImageFormat, .subresourceRange{.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .levelCount = 1, .layerCount = 1} };
        chk(vkCreateImageView(data.device, &viewCI, nullptr, &data.swapchainImageViews[i]));
    }

    // Rebuild render semaphores if image count changed
    if (data.renderSemaphores.size() != imageCount) {
        for (auto sem : data.renderSemaphores)
            vkDestroySemaphore(data.device, sem, nullptr);
        data.renderSemaphores.resize(imageCount);
        VkSemaphoreCreateInfo semaphoreCI{ .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
        for (auto& semaphore : data.renderSemaphores)
            chk(vkCreateSemaphore(data.device, &semaphoreCI, nullptr, &semaphore));
    }

    // Rebuild depth image at new dimensions
    vkDestroyImageView(data.device, data.depthImageView, nullptr);
    vmaDestroyImage(data.allocator, data.depthImage, data.depthImageAllocation);

    VkImageCreateInfo depthImageCI{
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .imageType = VK_IMAGE_TYPE_2D,
        .format = data.depthFormat,
        .extent{.width = surfaceCaps.currentExtent.width, .height = surfaceCaps.currentExtent.height, .depth = 1},
        .mipLevels = 1,
        .arrayLayers = 1,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .tiling = VK_IMAGE_TILING_OPTIMAL,
        .usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
    };
    VmaAllocationCreateInfo depthAllocCI{ .flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT, .usage = VMA_MEMORY_USAGE_AUTO };
    chk(vmaCreateImage(data.allocator, &depthImageCI, &depthAllocCI, &data.depthImage, &data.depthImageAllocation, nullptr));

    VkImageViewCreateInfo depthViewCI{ .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO, .image = data.depthImage, .viewType = VK_IMAGE_VIEW_TYPE_2D, .format = data.depthFormat, .subresourceRange{.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT, .levelCount = 1, .layerCount = 1} };
    chk(vkCreateImageView(data.device, &depthViewCI, nullptr, &data.depthImageView));
}

void ApplicationWindow::Close()
{
    m_Running = false;
}

void ApplicationWindow::Start(Application::Application& app)
{
    m_Running = true;
    uint32_t currentFrame = 0;
    uint64_t lastTime = SDL_GetTicks();
    bool needsRebuild = false;

    while (m_Running)
    {
        // --- Events ---
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            ImGui_ImplSDL3_ProcessEvent(&event);
            for (auto* layer : app.GetLayerStack())
                layer->OnEvent(event);
            switch (event.type)
            {
                case SDL_EVENT_QUIT:
                case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
                    m_Running = false;
                    break;
                case SDL_EVENT_WINDOW_RESIZED:
                    needsRebuild = true;
                    break;
            }
        }

        if (needsRebuild)
        {
            rebuildSwapchain();
            needsRebuild = false;
        }

        // --- Timestep (capped at ~30 ms to avoid physics jumps on alt-tab) ---
        now = SDL_GetTicks();
        timestep = std::min(static_cast<float>(now - lastTime) / 1000.0f, 1.0f / 30.0f);
        lastTime = now;
        currentTime += timestep;

        // --- Layer updates ---
        for (auto* layer : app.GetLayerStack())
            layer->OnUpdate(timestep);

        // --- ImGui frame ---
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();

        for (auto* layer : app.GetLayerStack())
            layer->OnUIRender();

        ImGui::Render();
        ImDrawData* drawData = ImGui::GetDrawData();

        // Skip GPU work entirely while minimised
        if (drawData->DisplaySize.x <= 0.0f || drawData->DisplaySize.y <= 0.0f)
        {
            SDL_Delay(5);
            continue;
        }

        // --- Acquire swapchain image ---
        uint32_t imageIndex;
        VkResult result = vkAcquireNextImageKHR(data.device, data.swapchain, UINT64_MAX,
                                                 data.presentSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);
        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
        {
            rebuildSwapchain();
            continue;
        }
        chk(result);

        // --- Fence for this frame slot ---
        chk(vkWaitForFences(data.device, 1, &data.fences[currentFrame], VK_TRUE, UINT64_MAX));
        chk(vkResetFences(data.device, 1, &data.fences[currentFrame]));

        // --- Record command buffer ---
        VkCommandBuffer cb = data.commandBuffers[currentFrame];
        VkCommandBufferBeginInfo beginInfo{
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
        };
        chk(vkBeginCommandBuffer(cb, &beginInfo));

        // Transition swapchain image -> COLOR_ATTACHMENT, depth -> DEPTH_STENCIL_ATTACHMENT
        VkImageMemoryBarrier2 colorBarrier{
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
            .srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
            .dstStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
            .dstAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
            .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .image = data.swapchainImages[imageIndex],
            .subresourceRange{.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .levelCount = 1, .layerCount = 1}
        };
        VkImageMemoryBarrier2 depthBarrier{
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
            .srcStageMask = VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT,
            .dstStageMask = VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT,
            .dstAccessMask = VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
            .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .newLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .image = data.depthImage,
            .subresourceRange{.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT, .levelCount = 1, .layerCount = 1}
        };
        VkImageMemoryBarrier2 preBarriers[] = { colorBarrier, depthBarrier };
        VkDependencyInfo preDepInfo{
            .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
            .imageMemoryBarrierCount = 2,
            .pImageMemoryBarriers = preBarriers
        };
        vkCmdPipelineBarrier2(cb, &preDepInfo);

        // --- Begin dynamic rendering ---
        VkRenderingAttachmentInfo colorAttach{
            .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
            .imageView = data.swapchainImageViews[imageIndex],
            .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
            .clearValue{.color{app.clearColorValue}}
        };
        VkRenderingAttachmentInfo depthAttach{
            .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
            .imageView = data.depthImageView,
            .imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
            .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .clearValue{.depthStencil{.depth = 1.0f}}
        };
        VkRenderingInfo renderInfo{
            .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
            .renderArea{.extent{.width = static_cast<uint32_t>(data.windowSize.x), .height = static_cast<uint32_t>(data.windowSize.y)}},
            .layerCount = 1,
            .colorAttachmentCount = 1,
            .pColorAttachments = &colorAttach,
            .pDepthAttachment = &depthAttach,
        };
        vkCmdBeginRendering(cb, &renderInfo);

        // --- Layer rendering + ImGui draw calls ---
        for (auto* layer : app.GetLayerStack())
            layer->OnRender(cb, data.windowSize, currentFrame);

        ImGui_ImplVulkan_RenderDrawData(drawData, cb);

        vkCmdEndRendering(cb);

        // Transition swapchain image -> PRESENT
        VkImageMemoryBarrier2 presentBarrier{
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
            .srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
            .srcAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
            .oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            .newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .image = data.swapchainImages[imageIndex],
            .subresourceRange{.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .levelCount = 1, .layerCount = 1}
        };
        VkDependencyInfo postDepInfo{
            .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
            .imageMemoryBarrierCount = 1,
            .pImageMemoryBarriers = &presentBarrier
        };
        vkCmdPipelineBarrier2(cb, &postDepInfo);

        chk(vkEndCommandBuffer(cb));

        // --- Submit ---
        VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        VkSubmitInfo submitInfo{
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = &data.presentSemaphores[currentFrame],
            .pWaitDstStageMask = &waitStage,
            .commandBufferCount = 1,
            .pCommandBuffers = &cb,
            .signalSemaphoreCount = 1,
            .pSignalSemaphores = &data.renderSemaphores[imageIndex]
        };
        chk(vkQueueSubmit(data.queue, 1, &submitInfo, data.fences[currentFrame]));

        // --- Present ---
        VkPresentInfoKHR presentInfo{
            .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = &data.renderSemaphores[imageIndex],
            .swapchainCount = 1,
            .pSwapchains = &data.swapchain,
            .pImageIndices = &imageIndex
        };
        result = vkQueuePresentKHR(data.queue, &presentInfo);
        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
            needsRebuild = true;

        currentFrame = (currentFrame + 1) % maxFramesInFlight;
    }

    chk(vkDeviceWaitIdle(data.device));
}

