


#include <iostream>

#include "../../../../project_management/include/Project.h"
#include "nodes/NodeSystem.h"
#include "nodes/ValueNodes.h"

int main() {

    // //Create a new NodeGraph
    // NodeGraph graph;
    //
    //
    // //Add some demo nodes
    // graph.createNode<FloatConstantNode>(10.0f);
    //
    //
    //
    //
    // //Allow the node graph to load its nodes
    // graph.Save()
    //
    //


    fs::path homePath = Project::GetUserHome();

    fs::path savePath = homePath / "Documents" / "TestSavePath";


    fs::create_directories(savePath);

    std::cout << savePath << std::endl;



    return 0;
}
