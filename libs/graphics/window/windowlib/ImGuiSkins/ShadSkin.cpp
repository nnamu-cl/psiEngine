#include "ShadSkin.h"
#include "imgui.h"
#include <iostream>

// Font headers
#include "../../assets/fonts/Geist-Regular.h"
#include "../../assets/fonts/Geist-Medium.h"
#include "../../assets/fonts/Geist-Bold.h"

namespace Application::Skins {
    // Initialize static font map
    std::unordered_map<std::string, ImFont*> ShadSkin::s_Fonts;
    void ShadSkin::ApplySkin() {
        ImGuiIO& io = ImGui::GetIO();
        auto &style = ImGui::GetStyle();
        auto &colors = ImGui::GetStyle().Colors;

        //Add some rounding
        style.TabRounding = 8.f;
        style.FrameRounding = 16.f;
        style.GrabRounding = 8.f;
        style.WindowRounding = 16.f;
        style.PopupRounding = 16.f;

        //No window borders
        style.WindowBorderSize = 0.0f;

        // DPI scale (defined here for use throughout the function)
        float x_scale = 1.0f;

        // Load custom fonts (only once)
        if (s_Fonts.empty()) {
            ImFontConfig fontConfig;
            fontConfig.FontDataOwnedByAtlas = false;

            float baseFontSize = 16.0f * x_scale;

            std::cout << "Base font size: " << baseFontSize << std::endl;

            ImFont* geistFont = io.Fonts->AddFontFromMemoryTTF((void*)geistMedium, geistMediumSize, baseFontSize, &fontConfig);
            s_Fonts["Default"] = geistFont;
            s_Fonts["Regular"] = io.Fonts->AddFontFromMemoryTTF((void*)geistRegular, geistRegularSize, baseFontSize, &fontConfig);
            s_Fonts["Bold"] = io.Fonts->AddFontFromMemoryTTF((void*)geistBold, geistBoldSize, baseFontSize, &fontConfig);
            io.FontDefault = geistFont;
        }

        //========================================================
        /// Colours

        // Headers
        colors[ImGuiCol_Header] = ImGui::ColorConvertU32ToFloat4(Colors::Theme::groupHeader);
        colors[ImGuiCol_HeaderHovered] = ImGui::ColorConvertU32ToFloat4(Colors::Theme::groupHeader);
        colors[ImGuiCol_HeaderActive] = ImGui::ColorConvertU32ToFloat4(Colors::Theme::groupHeader);

        // Buttons
        colors[ImGuiCol_Button] = ImColor(56, 56, 56, 200);
        colors[ImGuiCol_ButtonHovered] = ImColor(70, 70, 70, 255);
        colors[ImGuiCol_ButtonActive] = ImColor(56, 56, 56, 150);

        // Frame BG
        colors[ImGuiCol_FrameBg] = ImGui::ColorConvertU32ToFloat4(Colors::Theme::propertyField);
        colors[ImGuiCol_FrameBgHovered] = ImGui::ColorConvertU32ToFloat4(Colors::Theme::propertyField);
        colors[ImGuiCol_FrameBgActive] = ImGui::ColorConvertU32ToFloat4(Colors::Theme::propertyField);

        // Tabs
        colors[ImGuiCol_Tab] = ImGui::ColorConvertU32ToFloat4(Colors::Theme::titlebar);
        colors[ImGuiCol_TabHovered] = ImColor(255, 225, 135, 30);
        colors[ImGuiCol_TabActive] = ImColor(255, 225, 135, 60);
        colors[ImGuiCol_TabUnfocused] = ImGui::ColorConvertU32ToFloat4(Colors::Theme::titlebar);
        colors[ImGuiCol_TabUnfocusedActive] = colors[ImGuiCol_TabHovered];

        // Title
        colors[ImGuiCol_TitleBg] = ImGui::ColorConvertU32ToFloat4(Colors::Theme::titlebar);
        colors[ImGuiCol_TitleBgActive] = ImGui::ColorConvertU32ToFloat4(Colors::Theme::titlebar);
        colors[ImGuiCol_TitleBgCollapsed] = ImVec4{0.15f, 0.1505f, 0.151f, 1.0f};

        // Resize Grip
        colors[ImGuiCol_ResizeGrip] = ImVec4(0.91f, 0.91f, 0.91f, 0.25f);
        colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.81f, 0.81f, 0.81f, 0.67f);
        colors[ImGuiCol_ResizeGripActive] = ImVec4(0.46f, 0.46f, 0.46f, 0.95f);

        // Scrollbar
        colors[ImGuiCol_ScrollbarBg] = ImVec4(0.02f, 0.02f, 0.02f, 0.53f);
        colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.31f, 0.31f, 0.31f, 1.0f);
        colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.41f, 0.41f, 0.41f, 1.0f);
        colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.51f, 0.51f, 0.51f, 1.0f);

        // Check Mark
        colors[ImGuiCol_CheckMark] = ImColor(200, 200, 200, 255);

        // Slider
        colors[ImGuiCol_SliderGrab] = ImVec4(0.51f, 0.51f, 0.51f, 0.7f);
        colors[ImGuiCol_SliderGrabActive] = ImVec4(0.66f, 0.66f, 0.66f, 1.0f);

        // Text
        colors[ImGuiCol_Text] = ImGui::ColorConvertU32ToFloat4(Colors::Theme::text);

        // Checkbox
        colors[ImGuiCol_CheckMark] = ImGui::ColorConvertU32ToFloat4(Colors::Theme::text);

        // Separator
        colors[ImGuiCol_Separator] = ImGui::ColorConvertU32ToFloat4(Colors::Theme::backgroundDark);
        colors[ImGuiCol_SeparatorActive] = ImGui::ColorConvertU32ToFloat4(Colors::Theme::highlight);
        colors[ImGuiCol_SeparatorHovered] = ImColor(39, 185, 242, 150);

        // Window Background
        colors[ImGuiCol_WindowBg] = ImColor(20, 24, 27, 255);  // #14181b
        colors[ImGuiCol_ChildBg] = ImColor(20, 24, 27, 255);   // #14181b
        colors[ImGuiCol_PopupBg] = ImColor(20, 24, 27, 255);   // #14181b
        colors[ImGuiCol_Border] = ImGui::ColorConvertU32ToFloat4(Colors::Theme::backgroundDark);

        // Tables
        colors[ImGuiCol_TableHeaderBg] = ImGui::ColorConvertU32ToFloat4(Colors::Theme::groupHeader);
        colors[ImGuiCol_TableBorderLight] = ImGui::ColorConvertU32ToFloat4(Colors::Theme::backgroundDark);

        // Menubar
        colors[ImGuiCol_MenuBarBg] = ImVec4{0.0f, 0.0f, 0.0f, 0.0f};

        //========================================================
        /// Style
        style.WindowPadding = ImVec2(10.0f, 10.0f);
        style.FramePadding = ImVec2(8.0f, 6.0f);
        style.ItemSpacing = ImVec2(6.0f, 6.0f);
        style.ChildRounding = 6.0f;
        style.PopupRounding = 6.0f;
        style.FrameRounding = 6.0f;
        style.FrameBorderSize = 1.0f;
        style.IndentSpacing = 11.0f;
        style.WindowTitleAlign = ImVec2(0.5f, 0.5f);

        // Prepare to handle proper DPI
        style.ScaleAllSizes(x_scale);

        // When viewports are enabled, tweak WindowRounding/WindowBg so platform windows can look identical to regular ones
#ifdef ImGuiConfigFlags_ViewportsEnable
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            style.WindowRounding = 0.0f;
            colors[ImGuiCol_WindowBg].w = 1.0f;
        }
#endif
    }

    ImFont* ShadSkin::GetFont(const std::string& name) {
        auto it = s_Fonts.find(name);
        if (it != s_Fonts.end()) {
            return it->second;
        }
        return nullptr;
    }
}
