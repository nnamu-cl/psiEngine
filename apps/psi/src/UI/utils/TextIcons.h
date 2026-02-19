#pragma once
#include "imgui.h"
#include "ApplicationWindow.h"
#include "../UISettings.h"

enum class IconPosition { Left, Right };

namespace UIUtils {

    // Renders an icon with optional text, vertically centered on the same line.
    // pos controls whether the icon sits to the Left or Right of the text.
    // text is optional — pass nullptr or omit it to render only the icon.
    inline void IconText(const char* icon, IconPosition pos = IconPosition::Left, const char* text = nullptr) {

        const float textH  = ImGui::GetFontSize();
        const float iconH  = g_UISettings.iconTextHeight;
        const float offset = (iconH - textH) * -0.5f;
        const float lineY  = ImGui::GetCursorPosY();

        const bool hasText = text && text[0] != '\0';

        if (pos == IconPosition::Left) {
            ApplicationWindow::PushIconFont();
            ImGui::Text("%s", icon);
            ApplicationWindow::PopIconFont();

            if (hasText) {
                ImGui::SameLine();
                //ImGui::SetCursorPosY(lineY + offset);
                ImGui::Text("%s", text);
            }
        } else {
            if (hasText) {
                //ImGui::SetCursorPosY(lineY + offset);
                ImGui::Text("%s", text);
                ImGui::SameLine();
                //ImGui::SetCursorPosY(lineY);
            }

            ApplicationWindow::PushIconFont();
            ImGui::Text("%s", icon);
            ApplicationWindow::PopIconFont();
        }
    }

    // Renders a clickable icon-only button using the icon font.
    // active: when true the button is tinted with ImGuiCol_ButtonActive to show selection.
    // Returns true if the button was clicked.
    inline bool IconButton(const char* icon, bool active = false) {
        if (active)
            ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive));

        ApplicationWindow::PushIconFont();
        bool clicked = ImGui::Button(icon);
        ApplicationWindow::PopIconFont();

        if (active)
            ImGui::PopStyleColor();

        return clicked;
    }

} // namespace UIUtils
