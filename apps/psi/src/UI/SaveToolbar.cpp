#include "SaveToolbar.h"
#include "imgui.h"
#include "IconsLucide.h"
#include "UISettings.h"
#include "utils/TextIcons.h"
#include "project/PsiProjectManager.h"
#include "layers/PsiWorldLayer.h"

#include <cstdio>

SaveToolbar::SaveToolbar(PsiWorldLayer* worldLayer)
    : m_WorldLayer(worldLayer)
{
}

void SaveToolbar::Render()
{
    const UISettings& s = g_UISettings;
    const ImGuiViewport* vp = ImGui::GetMainViewport();

    const bool hasProject = PsiProjectManager::GetCurrentProject() != nullptr;

    // --- Auto-save countdown ---
    if (hasProject)
    {
        float dt = ImGui::GetIO().DeltaTime;
        m_AutoSaveTimer -= dt;
        if (m_AutoSaveTimer <= 0.0f)
        {
            PsiProjectManager::SaveProject();
            m_AutoSaveTimer = m_AutoSaveInterval;
        }
    }
    else
    {
        m_AutoSaveTimer = m_AutoSaveInterval;
    }

    // --- Camera control hints (always visible, above the save toolbar) ---
    CameraController::Mode camMode = CameraController::Mode::None;
    if (m_WorldLayer)
        camMode = m_WorldLayer->data.cameraController.activeMode();

    {
        ImGui::SetNextWindowPos(
            ImVec2(vp->WorkPos.x + s.gizmoLeftMargin, vp->WorkPos.y + vp->WorkSize.y - 60.0f),
            ImGuiCond_Always, ImVec2(0.0f, 1.0f));
        ImGui::SetNextWindowSize(ImVec2(0.0f, 0.0f), ImGuiCond_Always);
        ImGui::SetNextWindowBgAlpha(0.75f);

        constexpr ImGuiWindowFlags hintFlags =
            ImGuiWindowFlags_NoSavedSettings       |
            ImGuiWindowFlags_NoBringToFrontOnFocus |
            ImGuiWindowFlags_NoFocusOnAppearing;

        ImGui::Begin("Camera Controls", nullptr, hintFlags);

        // Highlight the currently active mode with text color + subtle border
        auto hintRow = [&](const char* icon, const char* label, CameraController::Mode mode)
        {
            bool active = (camMode == mode);
            if (active)
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
            else
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.6f, 0.6f, 0.6f, 0.8f));

            UIUtils::IconText(icon, IconPosition::Left, label);

            if (active)
            {
                ImVec2 min = ImGui::GetItemRectMin();
                ImVec2 max = ImGui::GetItemRectMax();
                ImGui::GetWindowDrawList()->AddRect(
                    ImVec2(min.x - 2.0f, min.y - 1.0f),
                    ImVec2(max.x + 2.0f, max.y + 1.0f),
                    ImGui::GetColorU32(ImVec4(1.0f, 1.0f, 1.0f, 0.15f)),
                    2.0f);
            }

            ImGui::PopStyleColor();
        };

        hintRow(ICON_LC_NAVIGATION, "Fly       RMB + WASD  |  Scroll = speed",   CameraController::Mode::Fly);
        hintRow(ICON_LC_ORBIT,      "Orbit     Alt + LMB   |  Scroll = dolly",    CameraController::Mode::Orbit);
        hintRow(ICON_LC_HAND,       "Pan       Shift + RMB",                       CameraController::Mode::Pan);
        hintRow(ICON_LC_ZOOM_IN,    "Zoom      Scroll wheel",                     CameraController::Mode::None);

        ImGui::End();
    }

    // --- Save button + auto-save indicator ---
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

    if (!hasProject)
        ImGui::BeginDisabled();

    // if (UIUtils::IconTextButton(ICON_LC_SAVE, "Save"))
    // {
    //     PsiProjectManager::SaveProject();
    //     m_AutoSaveTimer = m_AutoSaveInterval;
    // }

    // Auto-save countdown text
    ImGui::SameLine();
    int countdown = static_cast<int>(m_AutoSaveTimer) + 1;
    char buf[64];
    snprintf(buf, sizeof(buf), " Auto-saving in %ds", countdown);
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.6f, 0.6f, 0.6f, 1.0f));
    ImGui::TextUnformatted(buf);
    ImGui::PopStyleColor();

    if (!hasProject)
        ImGui::EndDisabled();

    ImGui::End();

    ImGui::PopStyleColor(4);
    ImGui::PopStyleVar(1);
}
