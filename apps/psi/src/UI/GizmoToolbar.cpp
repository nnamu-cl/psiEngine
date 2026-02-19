#include "GizmoToolbar.h"
#include "imgui.h"
#include "IconsLucide.h"
#include "utils/TextIcons.h"

void GizmoToolbar::Render()
{
    constexpr int opTranslate = 7;   // ImGuizmo::TRANSLATE
    constexpr int opRotate    = 120; // ImGuizmo::ROTATE
    constexpr int opScale     = 896; // ImGuizmo::SCALE

    // ViewManipulatorPanel occupies (0,0)→(128,128); start below it with a gap.
    const float leftMargin = 8.0f;
    const float topMargin  = 148.0f;

    ImGui::SetNextWindowPos(ImVec2(leftMargin, topMargin), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(0.0f, 0.0f), ImGuiCond_Always);
    ImGui::SetNextWindowBgAlpha(0.0f);

    constexpr ImGuiWindowFlags flags =
        ImGuiWindowFlags_NoDecoration        |
        ImGuiWindowFlags_NoMove              |
        ImGuiWindowFlags_NoSavedSettings     |
        ImGuiWindowFlags_NoBringToFrontOnFocus;

    // Larger buttons and more breathing room between them
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10.0f, 10.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing,  ImVec2(4.0f, 8.0f));

    // Transparent button backgrounds; hover/active provide the only visual feedback
    ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered,  ImVec4(1.0f, 1.0f, 1.0f, 0.12f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive,   ImVec4(1.0f, 1.0f, 1.0f, 0.22f));

    ImGui::Begin("##gizmo_toolbar", nullptr, flags);

    // Scale up the icon glyphs inside this window
    ImGui::SetWindowFontScale(1.6f);

    auto toolButton = [&](const char* icon, int op)
    {
        if (UIUtils::IconButton(icon, m_Operation == op))
            m_Operation = op;
    };

    toolButton(ICON_LC_MOVE,       opTranslate);
    toolButton(ICON_LC_ROTATE_CCW, opRotate);
    toolButton(ICON_LC_SCALE_3D,   opScale);

    ImGui::End();

    ImGui::PopStyleColor(3);
    ImGui::PopStyleVar(2);
}
