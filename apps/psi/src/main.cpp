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
#include "layers/PsiNodeEditorLayer.h"
#include "PsiColors.h"
#include <iostream>

#include "nodes/ValueNodes.h"
#include "project/PsiProjectManager.h"
#include <nfd.h>

int main(int argc, char* argv[])
{
    // Window specifications for PSI application
    ApplicationWindowSpecifications windowSpecs{
        1920,
        1080,
        "psiQuantum",
        SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_MAXIMIZED | SDL_WINDOW_FULLSCREEN
    };

    // Initialize the application window
    ApplicationWindow window(windowSpecs);
    if (!window.Init())
    {
        std::cerr << "Failed to initialize application window!" << std::endl;
        return -1;
    }

    // Initialise NFD (native file dialog) for the lifetime of the application
    NFD_Init();

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

    // Load the saved project registry from ~/Documents/psiEngine/psi_projects.json
    PsiProjectManager::Load();

    // Create layers
    PsiWorldLayer worldLayer(&window.data);                  // PSI 3D world rendering layer
    PsiNodeEditorLayer nodeEditorLayer(&worldLayer);         // Node editor layer (below UI)
    PsiUILayer uiLayer(&worldLayer, &nodeEditorLayer, &app); // PSI-specific UI layer (with app reference for settings)

    // Give the project manager a reference to the active node graph so that
    // SaveProject() / LoadProject() know which graph to operate on.
    PsiProjectManager::SetNodeGraph(&nodeEditorLayer.getNodeGraph());

    // Attach layers to the application
    // Order matters: worldLayer renders first, then node editor, then UI overlays on top
    app.PushLayer(&worldLayer);
    app.PushLayer(&nodeEditorLayer);

    // Give the project manager a pointer to the drawer (OnAttach creates it during PushLayer).
    PsiProjectManager::SetNodeSystemDrawer(nodeEditorLayer.getNodeSystemDrawer());

    //Add demo time node
    nodeEditorLayer.getNodeGraph().createNode<TimeNode>();

    app.PushLayer(&uiLayer);

    // Start the main application loop
    std::cout << "Starting PSI Application..." << std::endl;
    window.Start(app);

    NFD_Quit();
    return 0;
}
