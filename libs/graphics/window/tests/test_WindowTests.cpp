//
// Created by namus on 2/5/2026.
//

#include <catch2/catch_test_macros.hpp>

#include "ApplicationWindow.h"
#include "Application.h"
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

// ---------------------------------------------------------------------------
// Application layer-stack management  (no GPU / no loop required)
// ---------------------------------------------------------------------------

TEST_CASE("Application starts with an empty layer stack") {
    Application::Application app;
    REQUIRE(app.GetLayerStack().empty());
}

TEST_CASE("Application PushLayer and PopLayer manage the stack") {
    struct TrackingLayer : Application::Layer {
        bool attached = false;
        bool detached = false;
        void OnAttach() override { attached = true; }
        void OnDetach() override { detached = true; }
    };

    Application::Application app;
    TrackingLayer layerA, layerB;

    SECTION("PushLayer adds to stack and calls OnAttach") {
        app.PushLayer(&layerA);
        REQUIRE(app.GetLayerStack().size() == 1);
        REQUIRE(app.GetLayerStack()[0] == &layerA);
        REQUIRE(layerA.attached);
    }

    SECTION("PopLayer removes from stack and calls OnDetach") {
        app.PushLayer(&layerA);
        app.PopLayer(&layerA);
        REQUIRE(app.GetLayerStack().empty());
        REQUIRE(layerA.detached);
    }

    SECTION("multiple layers maintain insertion order") {
        app.PushLayer(&layerA);
        app.PushLayer(&layerB);
        REQUIRE(app.GetLayerStack().size() == 2);
        REQUIRE(app.GetLayerStack()[0] == &layerA);
        REQUIRE(app.GetLayerStack()[1] == &layerB);
    }

    SECTION("PopLayer on a layer not in the stack is a no-op") {
        app.PushLayer(&layerA);
        app.PopLayer(&layerB);
        REQUIRE(app.GetLayerStack().size() == 1);
        REQUIRE(!layerB.detached);
    }
}

// ---------------------------------------------------------------------------
// ApplicationWindow post-Init invariants  (GPU required, no loop entered)
// ---------------------------------------------------------------------------

TEST_CASE("ApplicationWindow fences are initially signaled") {
    ApplicationWindowSpecifications specs{800, 600, "Fence Test", SDL_WINDOW_VULKAN};
    ApplicationWindow window(specs);
    REQUIRE(window.Init());

    // Fences were created with VK_FENCE_CREATE_SIGNALED_BIT; the render loop
    // relies on this being true before the first frame.
    for (uint32_t i = 0; i < maxFramesInFlight; i++) {
        VkResult status = vkGetFenceStatus(window.data.device, window.data.fences[i]);
        REQUIRE(status == VK_SUCCESS);
    }
}

TEST_CASE("ApplicationWindow physical device properties are populated") {
    ApplicationWindowSpecifications specs{800, 600, "DevProps Test", SDL_WINDOW_VULKAN};
    ApplicationWindow window(specs);
    REQUIRE(window.Init());

    REQUIRE(window.data.physicalDeviceProperties.properties.deviceName[0] != '\0');
    REQUIRE(window.data.physicalDeviceProperties.properties.apiVersion >= VK_API_VERSION_1_3);
}

TEST_CASE("ApplicationWindow swapchain image count meets surface minimum") {
    ApplicationWindowSpecifications specs{800, 600, "SwapMin Test", SDL_WINDOW_VULKAN};
    ApplicationWindow window(specs);
    REQUIRE(window.Init());

    VkSurfaceCapabilitiesKHR caps{};
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
        window.data.physicalDevice, window.data.surface, &caps);

    REQUIRE(window.data.swapchainImages.size() >= caps.minImageCount);
}

TEST_CASE("ApplicationWindow Close does not crash and is idempotent") {
    ApplicationWindowSpecifications specs{800, 600, "Close Test", SDL_WINDOW_VULKAN};
    ApplicationWindow window(specs);
    REQUIRE(window.Init());

    // Close just flips m_Running; calling it multiple times must be safe.
    window.Close();
    window.Close();
}
