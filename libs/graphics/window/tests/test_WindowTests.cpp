//
// Created by namus on 2/5/2026.
//

#include <catch2/catch_test_macros.hpp>

#include "ApplicationWindow.h"
#include "imgui.h"

TEST_CASE("ApplicationWindow initializes all Vulkan resources") {

    ApplicationWindowSpecifications specs{1920, 1080, "Main Window", SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE};
    ApplicationWindow window(specs);
    REQUIRE(window.Init() == true);

    SECTION("SDL window and surface are created") {
        REQUIRE(window.data.sdlWindow != nullptr);
        REQUIRE(window.data.surface != VK_NULL_HANDLE);
        REQUIRE(window.data.windowSize.x > 0);
        REQUIRE(window.data.windowSize.y > 0);
    }

    SECTION("Vulkan instance, physical device, and logical device are created") {
        REQUIRE(window.data.vkInstance != VK_NULL_HANDLE);
        REQUIRE(window.data.physicalDevice != VK_NULL_HANDLE);
        REQUIRE(window.data.device != VK_NULL_HANDLE);
        REQUIRE(window.data.queue != VK_NULL_HANDLE);
    }

    SECTION("VMA allocator is created") {
        REQUIRE(window.data.allocator != VK_NULL_HANDLE);
    }

    SECTION("Swapchain is created with valid image views") {
        REQUIRE(window.data.swapchain != VK_NULL_HANDLE);
        REQUIRE(window.data.swapchainImageFormat != VK_FORMAT_UNDEFINED);
        REQUIRE(!window.data.swapchainImages.empty());
        REQUIRE(window.data.swapchainImageViews.size() == window.data.swapchainImages.size());
        for (auto view : window.data.swapchainImageViews) {
            REQUIRE(view != VK_NULL_HANDLE);
        }
    }

    SECTION("Depth attachment is created") {
        REQUIRE(window.data.depthFormat != VK_FORMAT_UNDEFINED);
        REQUIRE(window.data.depthImage != VK_NULL_HANDLE);
        REQUIRE(window.data.depthImageView != VK_NULL_HANDLE);
    }

    SECTION("Sync objects are created") {
        for (uint32_t i = 0; i < maxFramesInFlight; i++) {
            REQUIRE(window.data.fences[i] != VK_NULL_HANDLE);
            REQUIRE(window.data.presentSemaphores[i] != VK_NULL_HANDLE);
        }
        REQUIRE(!window.data.renderSemaphores.empty());
        REQUIRE(window.data.renderSemaphores.size() == window.data.swapchainImages.size());
        for (auto sem : window.data.renderSemaphores) {
            REQUIRE(sem != VK_NULL_HANDLE);
        }
    }

    SECTION("Command pool and command buffers are created") {
        REQUIRE(window.data.commandPool != VK_NULL_HANDLE);
        for (uint32_t i = 0; i < maxFramesInFlight; i++) {
            REQUIRE(window.data.commandBuffers[i] != VK_NULL_HANDLE);
        }
    }

    SECTION("ImGui context and backends are initialized") {
        REQUIRE(ImGui::GetCurrentContext() != nullptr);

        ImGuiIO& io = ImGui::GetIO();
        REQUIRE((io.ConfigFlags & ImGuiConfigFlags_NavEnableKeyboard) != 0);
        REQUIRE((io.ConfigFlags & ImGuiConfigFlags_NavEnableGamepad) != 0);

        // SDL3 platform backend sets this on successful init
        REQUIRE(io.BackendPlatformName != nullptr);
        // Vulkan renderer backend sets this on successful init
        REQUIRE(io.BackendRendererName != nullptr);
    }
}
