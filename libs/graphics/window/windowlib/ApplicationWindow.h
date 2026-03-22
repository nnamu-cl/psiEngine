#pragma once

#include <array>
#include <string>
#include <vector>
#include <glm/vec2.hpp>
#include <volk/volk.h>
#include <vma/vk_mem_alloc.h>
#include <SDL3/SDL.h>

#include "imgui.h"

constexpr uint32_t maxFramesInFlight{2};

struct ApplicationWindowSpecifications {
    int w;
    int h;
    const char *title;
    SDL_WindowFlags flags = SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE;
    std::string stateFilePath;  // If non-empty, window state is saved/loaded from this path
};

struct ApplicationWindowData {
    // SDL
    SDL_Window *sdlWindow{nullptr};
    glm::ivec2 windowSize{};

    // Vulkan core
    VkInstance vkInstance{VK_NULL_HANDLE};
    VkPhysicalDevice physicalDevice{VK_NULL_HANDLE};
    VkPhysicalDeviceProperties2 physicalDeviceProperties{.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2};
    VkDevice device{VK_NULL_HANDLE};
    VkQueue queue{VK_NULL_HANDLE};
    uint32_t queueFamily{NULL};
    VkSurfaceKHR surface{VK_NULL_HANDLE};

    // VMA
    VmaAllocator allocator{VK_NULL_HANDLE};

    // Swapchain
    VkSwapchainKHR swapchain{VK_NULL_HANDLE};
    std::vector<VkImage> swapchainImages;
    std::vector<VkImageView> swapchainImageViews;
    VkFormat swapchainImageFormat{VK_FORMAT_UNDEFINED};

    // Depth attachment
    VkImage depthImage{VK_NULL_HANDLE};
    VkImageView depthImageView{VK_NULL_HANDLE};
    VmaAllocation depthImageAllocation{VK_NULL_HANDLE};
    VkFormat depthFormat{VK_FORMAT_UNDEFINED};

    // Sync objects
    std::array<VkFence, maxFramesInFlight> fences{};
    std::array<VkSemaphore, maxFramesInFlight> presentSemaphores{};
    std::vector<VkSemaphore> renderSemaphores;

    // Command pool
    VkCommandPool commandPool{VK_NULL_HANDLE};
    std::array<VkCommandBuffer, maxFramesInFlight> commandBuffers{};
};

namespace Application { class Application; }

class ApplicationWindow {
    ApplicationWindowSpecifications *specification;
    bool m_Running{false};

public:
    ApplicationWindowData data;

    ApplicationWindow(ApplicationWindowSpecifications &spec);


    //Time
    Uint64 now;
    float timestep;
    float currentTime = 0;

    static ApplicationWindow *instance;
    static ImFont* iconFont;
    static float iconFontSize;
    static ImFont* boldFont;

    static void PushIconFont() { if (iconFont) ImGui::PushFont(iconFont); }
    static void PopIconFont()  { if (iconFont) ImGui::PopFont(); }
    static void PushBoldFont() { if (boldFont) ImGui::PushFont(boldFont); }
    static void PopBoldFont()  { if (boldFont) ImGui::PopFont(); }

    static ImTextureID brandIconTexture; // 0 = not loaded
    static ImVec2 brandIconNativeSize;

    bool Init();
    void Start(Application::Application& app);
    void Close();

private:
    bool init_imgui();
    void rebuildSwapchain();
    void loadWindowState();
    void saveWindowState();
};
