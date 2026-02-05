#pragma once

#include <glm/vec2.hpp>
#include <volk/volk.h>
#include <SDL3/SDL.h>


struct ApplicationWindowSpecifications {
    int w;
    int h;
    const char *title;
    SDL_WindowFlags flags;
};

struct ApplicationWindowData {
    SDL_Window *sdlWindow{nullptr};
    VkInstance vkInstance{VK_NULL_HANDLE};
    VkPhysicalDeviceProperties2 physicalDeviceProperties{.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2};
    VkDevice device{VK_NULL_HANDLE};
    VkQueue queue{VK_NULL_HANDLE};
    VkSurfaceKHR surface{VK_NULL_HANDLE};
    glm::ivec2 windowSize{};
};

class ApplicationWindow {
    ApplicationWindowSpecifications *specification;

public:
    ApplicationWindowData data;

    ApplicationWindow(ApplicationWindowSpecifications &spec);

    bool Init();
};
