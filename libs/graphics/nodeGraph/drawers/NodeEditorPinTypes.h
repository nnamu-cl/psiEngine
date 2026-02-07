#pragma once

#include "imgui_node_editor.h"
#include <string>

// Forward declaration
class Node;

namespace ed = ax::NodeEditor;

enum class PinKind
{
    Output,
    Input
};

enum class PinType
{
    Flow,
    Bool,
    Int,
    Float,
    String,
    Object,
    Function,
    Delegate,
};

struct Pin
{
    ed::PinId   ID;
    ::Node*     Node;
    std::string Name;
    PinType     Type;
    PinKind     Kind;

    Pin(int id, const char* name, PinType type):
        ID(id), Node(nullptr), Name(name), Type(type), Kind(PinKind::Input)
    {
    }
};
