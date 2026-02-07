#pragma once

#include "imgui_node_editor.h"
#include "../nodes/NodeSystem.h"
#include <string>

// Forward declaration
class Node;

namespace ed = ax::NodeEditor;

enum class PinKind
{
    Output,
    Input
};

struct Pin
{
    ed::PinId   ID;
    ::Node*     Node;
    std::string Name;
    SocketType  Type;
    PinKind     Kind;

    Pin(int id, const char* name, SocketType type):
        ID(id), Node(nullptr), Name(name), Type(type), Kind(PinKind::Input)
    {
    }
};
