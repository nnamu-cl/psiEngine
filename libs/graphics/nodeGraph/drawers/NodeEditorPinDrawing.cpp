#include "NodeEditorPinDrawing.h"
#include "NodeEditorIcons.h"
#include "../nodes/NodeSystem.h"

ImColor GetIconColor(SocketType type)
{
    switch (type)
    {
        default:
        case SocketType::Any:   return ImColor( 51, 150, 215);
        case SocketType::Int:   return ImColor( 68, 201, 156);
        case SocketType::Float: return ImColor(147, 226,  74);
        case SocketType::Vec2:  return ImColor(159, 214, 102);
        case SocketType::Vec3:  return ImColor(124, 252, 0);
        case SocketType::Vec4:  return ImColor( 85, 255,  85);
        case SocketType::Mat4:  return ImColor(180, 130, 255);
    }
}

void DrawPinIcon(const Pin& pin, bool connected, int alpha)
{
    IconType iconType;
    ImColor  color = GetIconColor(pin.Type);
    color.Value.w = alpha / 255.0f;
    switch (pin.Type)
    {
        case SocketType::Int:   iconType = IconType::Circle; break;
        case SocketType::Float: iconType = IconType::Circle; break;
        case SocketType::Vec2:  iconType = IconType::Circle; break;
        case SocketType::Vec3:  iconType = IconType::Circle; break;
        case SocketType::Vec4:  iconType = IconType::Circle; break;
        case SocketType::Mat4:  iconType = IconType::Square; break;
        case SocketType::Any:   iconType = IconType::Circle; break;
        default:
            return;
    }

    Icon(ImVec2(static_cast<float>(20), static_cast<float>(20)), iconType, connected, color, ImColor(32, 32, 32, alpha));
}

void DrawInputSocketPin(const InputSocket* inputSocket, uint64_t pinId, int alpha)
{
    if (!inputSocket) return;

    bool connected = inputSocket->isConnected();

    Pin pin{static_cast<int>(pinId), inputSocket->name.c_str(), inputSocket->type};

    // Draw icon first, then name on the same line
    DrawPinIcon(pin, connected, alpha);
    ImGui::SameLine();
    ImGui::Text("%s", inputSocket->name.c_str());
}

void DrawOutputSocketPin(const OutputSocket* outputSocket, uint64_t pinId, int alpha)
{
    if (!outputSocket) return;

    bool connected = !outputSocket->connections.empty();

    Pin pin{static_cast<int>(pinId), outputSocket->name.c_str(), outputSocket->type};

    // Draw name first, then icon on the same line for outputs
    ImGui::Text("%s", outputSocket->name.c_str());
    ImGui::SameLine();
    DrawPinIcon(pin, connected, alpha);
}
