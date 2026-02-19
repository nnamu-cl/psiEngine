#include "ModeToolbar.h"
#include "layers/PsiWorldLayer.h"
#include "layers/PsiNodeEditorLayer.h"
#include "imgui.h"
#include "IconsLucide.h"
#include "utils/TextIcons.h"
#include "PsiColors.h"
#include <glm/vec3.hpp>

ModeToolbar::ModeToolbar(PsiWorldLayer* worldLayer, PsiNodeEditorLayer* nodeEditorLayer)
    : m_WorldLayer(worldLayer)
    , m_NodeEditorLayer(nodeEditorLayer)
{
}

void ModeToolbar::applyMode(PsiMode mode)
{
    m_CurrentMode = mode;
    if (m_NodeEditorLayer)
        m_NodeEditorLayer->SetShowNodeEditor(mode == PsiMode::GraphEditor);
    if (m_WorldLayer)
        m_WorldLayer->data.cameraController.dynamicMainCamera = (mode == PsiMode::WorldViewport);
}

void ModeToolbar::Render()
{
    ImGuiIO& io = ImGui::GetIO();

    // Tab key toggles mode (skip when typing in an input field)
    if (!io.WantTextInput && ImGui::IsKeyPressed(ImGuiKey_Tab))
    {
        applyMode(m_CurrentMode == PsiMode::WorldViewport
            ? PsiMode::GraphEditor
            : PsiMode::WorldViewport);
    }

    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImVec2 pos = ImVec2(
        viewport->WorkPos.x + viewport->WorkSize.x * 0.5f,
        viewport->WorkPos.y + viewport->WorkSize.y - 20.0f);
    ImGui::SetNextWindowPos(pos, ImGuiCond_Always, ImVec2(0.5f, 1.0f));
    ImGui::SetNextWindowSize(ImVec2(0, 0), ImGuiCond_Always);
    ImGui::SetNextWindowBgAlpha(0.0f);

    constexpr ImGuiWindowFlags flags =
        ImGuiWindowFlags_NoDecoration        |
        ImGuiWindowFlags_NoMove              |
        ImGuiWindowFlags_NoSavedSettings     |
        ImGuiWindowFlags_NoBringToFrontOnFocus |
        ImGuiWindowFlags_NoBackground        |
        ImGuiWindowFlags_NoNav;

    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding,  ImVec2(20.0f, 10.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 6.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);

    ImGui::Begin("##mode_toolbar", nullptr, flags);

    ImGui::SetWindowFontScale(1.4f);

    auto modeButton = [&](const char* icon, PsiMode mode, glm::vec3 accentColor, bool darkText = false)
    {
        const bool active = (m_CurrentMode == mode);
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(accentColor.r, accentColor.g, accentColor.b, 1.0f));
        if (active && darkText)
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 0.0f, 0.0f, 1.0f));
        if (UIUtils::IconButton(icon, active))
            applyMode(mode);
        if (active && darkText)
            ImGui::PopStyleColor();
        ImGui::PopStyleColor();
    };

    modeButton(ICON_LC_BOX,      PsiMode::WorldViewport, psi::Colors::AccentGreen, true);
    ImGui::SameLine();
    modeButton(ICON_LC_WORKFLOW, PsiMode::GraphEditor,   psi::Colors::AccentRed);

    ImGui::End();

    ImGui::PopStyleVar(3);
}
