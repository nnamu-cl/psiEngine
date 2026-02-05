#define VOLK_IMPLEMENTATION
#include "ApplicationWindow.h"

#include <vector>
#include <volk/volk.h>
#include <SDL3/SDL_vulkan.h>

#include "VulkanChecks.h"


ApplicationWindow::ApplicationWindow(ApplicationWindowSpecifications& spec)
{
    specification = &spec;
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

    //Physical Device
    uint32_t deviceCount{ 0 };
    chk(vkEnumeratePhysicalDevices(data.vkInstance, &deviceCount, nullptr));
    std::vector<VkPhysicalDevice> devices(deviceCount);
    chk(vkEnumeratePhysicalDevices(data.vkInstance, &deviceCount, devices.data()));
    uint32_t deviceIndex{ 0 };

    vkGetPhysicalDeviceProperties2(devices[deviceIndex], &data.physicalDeviceProperties);
    std::cout << "Selected device: " << &data.physicalDeviceProperties.properties.deviceName << "\n";


    // Find a queue family for graphics
    uint32_t queueFamilyCount{ 0 };
    vkGetPhysicalDeviceQueueFamilyProperties(devices[deviceIndex], &queueFamilyCount, nullptr);
    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(devices[deviceIndex], &queueFamilyCount, queueFamilies.data());
    uint32_t queueFamily{ 0 };
    for (size_t i = 0; i < queueFamilies.size(); i++) {
        if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            queueFamily = i;
            break;
        }
    }

    // Logical device
    const float qfpriorities{ 1.0f };
    VkDeviceQueueCreateInfo queueCI{ .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO, .queueFamilyIndex = queueFamily, .queueCount = 1, .pQueuePriorities = &qfpriorities };
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
    chk(vkCreateDevice(devices[deviceIndex], &deviceCI, nullptr, &data.device));
    vkGetDeviceQueue(data.device, queueFamily, 0, &data.queue);

    //Window and surface
    data.sdlWindow = SDL_CreateWindow( specification->title, specification->w, specification->h, specification->flags);
    chk(SDL_Vulkan_CreateSurface(data.sdlWindow, data.vkInstance, nullptr, &data.surface));
    chk(SDL_GetWindowSize(data.sdlWindow, &data.windowSize.x, &data.windowSize.y));
    VkSurfaceCapabilitiesKHR surfaceCaps{};
    chk(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(devices[deviceIndex], data.surface, &surfaceCaps));


    return true;

}

