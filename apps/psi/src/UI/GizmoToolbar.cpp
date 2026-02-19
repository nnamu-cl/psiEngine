#include "GizmoToolbar.h"
#include "imgui.h"

void GizmoToolbar::Render()
{
    constexpr int opTranslate = 7;   // ImGuizmo::TRANSLATE
    constexpr int opRotate    = 120; // ImGuizmo::ROTATE
    constexpr int opScale     = 896; // ImGuizmo::SCALE

    ImGuiIO& io = ImGui::GetIO();

    const float toolbarWidth = 260.0f;
    const float topMargin    = 8.0f;
    ImGui::SetNextWindowPos(ImVec2((io.DisplaySize.x - toolbarWidth) * 0.5f, topMargin),
                            ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(toolbarWidth, 0.0f), ImGuiCond_Always);
    ImGui::SetNextWindowBgAlpha(0.75f);

    constexpr ImGuiWindowFlags flags =
        ImGuiWindowFlags_NoDecoration        |
        ImGuiWindowFlags_NoMove              |
        ImGuiWindowFlags_NoSavedSettings     |
        ImGuiWindowFlags_NoBringToFrontOnFocus;

    ImGui::Begin("##gizmo_toolbar", nullptr, flags);

    auto toolButton = [&](const char* label, int op)
    {
        const bool active = (m_Operation == op);
        if (active)
            ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive));

        if (ImGui::Button(label))
            m_Operation = op;

        if (active)
            ImGui::PopStyleColor();
    };

    toolButton("Translate", opTranslate);
    ImGui::SameLine();
    toolButton("Rotate", opRotate);
    ImGui::SameLine();
    toolButton("Scale", opScale);

    ImGui::End();
}
