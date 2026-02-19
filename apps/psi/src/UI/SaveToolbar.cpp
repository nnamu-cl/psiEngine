#include "SaveToolbar.h"
#include "imgui.h"
#include "IconsLucide.h"
#include "UISettings.h"
#include "utils/TextIcons.h"
#include "project/PsiProjectManager.h"

void SaveToolbar::Render()
{
    const UISettings& s = g_UISettings;
    const ImGuiViewport* vp = ImGui::GetMainViewport();

    // Anchor to the bottom-left of the viewport
    ImGui::SetNextWindowPos(
        ImVec2(vp->WorkPos.x + s.gizmoLeftMargin, vp->WorkPos.y + vp->WorkSize.y - 8.0f),
        ImGuiCond_Always, ImVec2(0.0f, 1.0f));
    ImGui::SetNextWindowSize(ImVec2(0.0f, 0.0f), ImGuiCond_Always);
    ImGui::SetNextWindowBgAlpha(0.0f);

    constexpr ImGuiWindowFlags flags =
        ImGuiWindowFlags_NoDecoration          |
        ImGuiWindowFlags_NoMove                |
        ImGuiWindowFlags_NoSavedSettings       |
        ImGuiWindowFlags_NoBringToFrontOnFocus |
        ImGuiWindowFlags_NoBackground;

    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(s.gizmoFramePadX, s.gizmoFramePadY));
    ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 1.0f, 1.0f, s.gizmoHoverAlpha));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImVec4(1.0f, 1.0f, 1.0f, s.gizmoActiveAlpha));
    ImGui::PushStyleColor(ImGuiCol_NavHighlight,  ImVec4(0.0f, 0.0f, 0.0f, 0.0f));

    ImGui::Begin("##save_toolbar", nullptr, flags);
    ImGui::SetWindowFontScale(s.gizmoFontScale);

    const bool hasProject = PsiProjectManager::GetCurrentProject() != nullptr;
    if (!hasProject)
        ImGui::BeginDisabled();

    if (UIUtils::IconTextButton(ICON_LC_SAVE, "Save"))
        PsiProjectManager::SaveProject();

    if (!hasProject)
        ImGui::EndDisabled();

    ImGui::End();

    ImGui::PopStyleColor(4);
    ImGui::PopStyleVar(1);
}
