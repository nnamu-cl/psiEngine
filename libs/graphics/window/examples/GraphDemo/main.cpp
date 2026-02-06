//
// Created by namus on 2/4/2026.
//

#include "ApplicationWindow.h"
#include "Application.h"
#include "../../windowlib/Layers/Layer.h"
#include "imgui.h"
#include "ImGuiSkins/ShadSkin.h"
#include "ImGuiSkins/SkinsBase.h"
#include "imgui_node_editor.h"
#include "NodeSystemDrawer.h"

namespace ed = ax::NodeEditor;


class DemoNodeGraph : public Application::Layer
{



public:
    ax::NodeEditor::EditorContext * m_NodeEditorContext;
    NodeGraph m_NodeGraph;

    std::unique_ptr<NodeSystemDrawer> m_NodeSystemDrawer;


    void OnAttach() override {
        // Create node editor context
        ed::Config config;
        m_NodeEditorContext = ed::CreateEditor(&config);


        // Create node system drawer
        m_NodeSystemDrawer = std::make_unique<NodeSystemDrawer>(&m_NodeGraph);


    }

    void OnUIRender() override
    {
        ImGui::ShowDemoWindow();
    }
};

int main()
{
    ApplicationWindowSpecifications windowSpecs{1920, 1080, "Main Demo Window",
                                                SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_MAXIMIZED};

    ApplicationWindow window(windowSpecs);
    window.Init();

    Application::Application app;


    //Apply out theme
    Application::Skins::ShadSkin shadSkin;
    shadSkin.ApplySkin();

    // Create layers
    DemoNodeGraph guiLayer; // GUI layer



    // Attach layers
    app.PushLayer(&guiLayer);


    window.Start(app);
    return 0;
}
