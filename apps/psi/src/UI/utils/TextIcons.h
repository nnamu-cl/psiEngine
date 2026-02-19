#pragma once
#include "imgui.h"
#include "ApplicationWindow.h"
#include "../UISettings.h"
#include <cstdio>

#include "imgui_internal.h"

enum class IconPosition { Left, Right };

namespace UIUtils {

    // Renders an icon with optional text, both vertically centered.
    // Uses DrawList so the two fonts never fight over cursor position.
    inline void IconText(const char* icon, IconPosition pos = IconPosition::Left, const char* text = nullptr) {
        const bool  hasText = text && text[0] != '\0';
        const float iconSz  = ImGui::GetFontSize();
        const float textSz  = ImGui::GetFontSize();
        const float lineH   = ImMax(iconSz, textSz);
        const float spacing = hasText ? ImGui::GetStyle().ItemSpacing.x : 0.0f;
        const float textW   = hasText ? ImGui::CalcTextSize(text).x : 0.0f;
        const float totalW  = iconSz + (hasText ? spacing + textW : 0.0f);

        ImGui::Dummy(ImVec2(totalW, lineH));
        ImVec2      origin = ImGui::GetItemRectMin();
        ImDrawList* dl     = ImGui::GetWindowDrawList();
        ImU32       col    = ImGui::GetColorU32(ImGuiCol_Text);

        const float iconX = (pos == IconPosition::Left) ? origin.x : origin.x + textW + spacing;
        const float textX = (pos == IconPosition::Left) ? origin.x + iconSz + spacing : origin.x;
        const float iconY = origin.y + (lineH - iconSz) * 0.5f;
        const float textY = origin.y + (lineH - textSz) * 0.5f;

        ApplicationWindow::PushIconFont();
        dl->AddText(ApplicationWindow::iconFont, iconSz, ImVec2(iconX, iconY), col, icon);
        ApplicationWindow::PopIconFont();

        if (hasText)
            dl->AddText(ImGui::GetFont(), textSz, ImVec2(textX, textY), col, text);
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

    // Renders a clickable button with an icon prefix and text label.
    // The icon is drawn via DrawList so the two fonts don't fight over the cursor.
    // Returns true when the button is clicked.
    inline bool IconTextButton(const char* icon, const char* label, ImVec2 size = ImVec2(0, 0)) {
        char buf[256];
        snprintf(buf, sizeof(buf), "      %s", label);
        bool clicked = ImGui::Button(buf, size);

        ImVec2      pos   = ImGui::GetItemRectMin();
        ImVec2      itemSize = ImGui::GetItemRectSize();
        const float iSz   = ImGui::GetFontSize() * (2.0f / 3.0f);
        const float iconY = pos.y + (itemSize.y - iSz) * 0.5f;

        ApplicationWindow::PushIconFont();
        ImGui::GetWindowDrawList()->AddText(
            ApplicationWindow::iconFont, iSz,
            ImVec2(pos.x + ImGui::GetStyle().FramePadding.x, iconY),
            ImGui::GetColorU32(ImGuiCol_Text), icon);
        ApplicationWindow::PopIconFont();

        return clicked;
    }

    // Renders a BeginMenu with an icon prefix. No merged font required —
    // leading spaces reserve horizontal room and the icon is drawn via DrawList.
    // Returns true when the menu is open (call ImGui::EndMenu() if true).
    inline bool IconBeginMenu(const char* icon, const char* label, bool enabled = true) {
        char buf[256];
        snprintf(buf, sizeof(buf), "      %s", label);
        bool open = ImGui::BeginMenu(buf, enabled);

        ImVec2      pos   = ImGui::GetItemRectMin();
        ImVec2      size  = ImGui::GetItemRectSize();
        const float iSz   = ImGui::GetFontSize();
        float       iconY = pos.y + (size.y - iSz) * 0.5f;

        ApplicationWindow::PushIconFont();
        ImGui::GetForegroundDrawList()->AddText(
            ApplicationWindow::iconFont, iSz,
            ImVec2(pos.x + 4.0f, iconY),
            ImGui::GetColorU32(ImGuiCol_Text), icon);
        ApplicationWindow::PopIconFont();

        return open;
    }

    // Renders a MenuItem with an icon prefix. No merged font required —
    // leading spaces reserve horizontal room and the icon is drawn via DrawList.
    // Returns true when the item is clicked.
    inline bool IconMenuItem(const char* icon, const char* label, bool selected = false, bool enabled = true) {
        char buf[256];
        snprintf(buf, sizeof(buf), "      %s", label);
        bool clicked = ImGui::MenuItem(buf, nullptr, selected, enabled);

        ImVec2      pos   = ImGui::GetItemRectMin();
        ImVec2      size  = ImGui::GetItemRectSize();
        const float iSz   = ImGui::GetFontSize();
        float       iconY = pos.y + (size.y - iSz) * 0.5f;

        ApplicationWindow::PushIconFont();
        ImGui::GetForegroundDrawList()->AddText(
            ApplicationWindow::iconFont, iSz,
            ImVec2(pos.x + 4.0f, iconY),
            ImGui::GetColorU32(ImGuiCol_Text), icon);
        ApplicationWindow::PopIconFont();

        return clicked;
    }

} // namespace UIUtils
