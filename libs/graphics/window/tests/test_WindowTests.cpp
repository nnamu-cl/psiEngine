//
// Created by namus on 2/5/2026.
//

#include <catch2/catch_test_macros.hpp>

#include "ApplicationWindow.h"

TEST_CASE("RUNNING A RANDOM BASIC TEST") {

    //Create the window specifications
    ApplicationWindowSpecifications specs {1920,1080, "Main Window", SDL_WINDOW_VULKAN| SDL_WINDOW_RESIZABLE};
    // Create the window
    ApplicationWindow window  (specs);

    // The window returns true if it's able to initialize
    SECTION("Application window can initialize successfully") {
        REQUIRE(window.Init() == true);

        // Initialization should involve the creation of am sdl window
        REQUIRE( window.data.sdlWindow != nullptr );

        // Initialization should create and return a vulkan instance
        REQUIRE (window.data.vkInstance != nullptr );
        REQUIRE (window.data.device != VK_NULL_HANDLE );
        REQUIRE (window.data.queue != VK_NULL_HANDLE );
        REQUIRE (window.data.surface != VK_NULL_HANDLE );

    }

}
