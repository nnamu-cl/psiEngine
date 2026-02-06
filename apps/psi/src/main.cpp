//
// PSI Application - Physics Simulation and Evaluation
// Main entry point
//

#include "main.h"
#include "ApplicationWindow.h"
#include "Application.h"
#include "Layers/Layer.h"
#include "ImGuiSkins/ShadSkin.h"
#include "layers/PsiUILayer.h"
#include "layers/PsiWorldLayer.h"
#include "PsiColors.h"
#include <iostream>

int main(int argc, char* argv[])
{
    // Window specifications for PSI application
    ApplicationWindowSpecifications windowSpecs{
        1920,
        1080,
        "psiQuantum",
        SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE
    };

    // Initialize the application window
    ApplicationWindow window(windowSpecs);
    if (!window.Init())
    {
        std::cerr << "Failed to initialize application window!" << std::endl;
        return -1;
    }

    // Create the application instance
    Application::Application app;

    // Set the background clear color to PSI secondary background
    app.clearColorValue.float32[0] = psi::Colors::SecondaryBackground.r;
    app.clearColorValue.float32[1] = psi::Colors::SecondaryBackground.g;
    app.clearColorValue.float32[2] = psi::Colors::SecondaryBackground.b;
    app.clearColorValue.float32[3] = 1.0f;

    // Apply UI theme (ShadSkin for consistent look)
    Application::Skins::ShadSkin shadSkin;
    shadSkin.ApplySkin();

    // Create layers
    PsiWorldLayer worldLayer(&window.data);  // PSI 3D world rendering layer
    PsiUILayer uiLayer(&worldLayer, &app);   // PSI-specific UI layer (with app reference for settings)

    // Attach layers to the application
    // Order matters: worldLayer renders first, then UI overlays
    app.PushLayer(&worldLayer);
    app.PushLayer(&uiLayer);

    // Start the main application loop
    std::cout << "Starting PSI Application..." << std::endl;
    window.Start(app);

    return 0;
}
