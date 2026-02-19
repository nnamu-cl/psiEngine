#include "GizmoToolbar.h"
#include "imgui.h"
#include "IconsLucide.h"
#include "utils/TextIcons.h"
#include "UISettings.h"

void GizmoToolbar::Render()
{
    constexpr int opTranslate = 7;   // ImGuizmo::TRANSLATE
    constexpr int opRotate    = 120; // ImGuizmo::ROTATE
    constexpr int opScale     = 896; // ImGuizmo::SCALE

    const UISettings& s = g_UISettings;

    ImGui::SetNextWindowPos(ImVec2(s.gizmoLeftMargin, s.gizmoTopMargin), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(0.0f, 0.0f), ImGuiCond_Always);
    ImGui::SetNextWindowBgAlpha(0.0f);

    constexpr ImGuiWindowFlags flags =
        ImGuiWindowFlags_NoDecoration        |
        ImGuiWindowFlags_NoMove              |
        ImGuiWindowFlags_NoSavedSettings     |
        ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoBackground;

    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(s.gizmoFramePadX, s.gizmoFramePadY));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing,  ImVec2(s.gizmoItemSpacingX, s.gizmoItemSpacingY));

    ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered,  ImVec4(1.0f, 1.0f, 1.0f, s.gizmoHoverAlpha));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive,   ImVec4(1.0f, 1.0f, 1.0f, s.gizmoActiveAlpha));
    ImGui::PushStyleColor(ImGuiCol_NavHighlight,   ImVec4(0.0f, 0.0f, 0.0f, 0.0f));

    ImGui::Begin("##gizmo_toolbar", nullptr, flags);

    ImGui::SetWindowFontScale(s.gizmoFontScale);

    auto toolButton = [&](const char* icon, int op)
    {
        if (UIUtils::IconButton(icon, m_Operation == op))
            m_Operation = op;
    };

    toolButton(ICON_LC_MOVE_3D,       opTranslate);
    toolButton(ICON_LC_ROTATE_3D, opRotate);
    toolButton(ICON_LC_SCALE_3D,   opScale);

    ImGui::End();

    ImGui::PopStyleColor(4);
    ImGui::PopStyleVar(2);
}
