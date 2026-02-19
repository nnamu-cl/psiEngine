#include "BrandLabel.h"
#include "imgui.h"
#include "ApplicationWindow.h"

void BrandLabel::Render()
{
    const ImGuiViewport* vp = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(
        ImVec2(vp->WorkPos.x + vp->WorkSize.x * 0.5f, vp->WorkPos.y + 7.0f),
        ImGuiCond_Always, ImVec2(0.5f, 0.0f));
    ImGui::SetNextWindowSize(ImVec2(0, 0), ImGuiCond_Always);
    ImGui::SetNextWindowBgAlpha(0.0f);

    constexpr ImGuiWindowFlags flags =
        ImGuiWindowFlags_NoDecoration          |
        ImGuiWindowFlags_NoMove                |
        ImGuiWindowFlags_NoSavedSettings       |
        ImGuiWindowFlags_NoBringToFrontOnFocus |
        ImGuiWindowFlags_NoBackground          |
        ImGuiWindowFlags_NoNav                 |
        ImGuiWindowFlags_NoMouseInputs;

    ImGui::Begin("##brand_label", nullptr, flags);

    const float iconH = 28.0f * 1.3f;
    const ImTextureID tex       = ApplicationWindow::brandIconTexture;
    const ImVec2      nativeSz  = ApplicationWindow::brandIconNativeSize;

    if (tex != 0 && nativeSz.x > 0.0f)
    {
        const float iconW = iconH * (nativeSz.x / nativeSz.y);
        ImGui::Image(tex, ImVec2(iconW, iconH));
        ImGui::SameLine(0.0f, 8.0f);
    }

    // Bottom-align the text with the icon
    ApplicationWindow::PushBoldFont();
    ImGui::SetWindowFontScale(2.0f);
    const float textH = ImGui::GetFontSize(); // already reflects the 2x scale
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + (iconH - textH));
    ImGui::Text("Quantum");
    ApplicationWindow::PopBoldFont();

    ImGui::End();
}
