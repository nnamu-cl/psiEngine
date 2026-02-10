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
#include "nodes/MathNodes.h"
#include "drawers/NodeSystemDrawer.h"
#include "nodes/ValueNodes.h"

namespace ed = ax::NodeEditor;


class DemoNodeGraph : public Application::Layer
{



public:
    NodeGraph nodeGraph;
    std::unique_ptr<NodeSystemDrawer> nodeSystemDrawer;


    void OnAttach() override {
        // Create node system drawer
        nodeSystemDrawer = std::make_unique<NodeSystemDrawer>(&nodeGraph);

        // Add a demo node to the node system
        TimeNode *timeNode = nodeGraph.createNode<TimeNode>();

        SinNode *sineNode = nodeGraph.createNode<SinNode>();

    }

    void OnUIRender() override
    {
        //Tell the node system drawer to draw all the nodes
        nodeSystemDrawer->DrawNodeGraph();
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
