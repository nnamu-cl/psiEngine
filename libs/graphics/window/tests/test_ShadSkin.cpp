//
// Test for ShadSkin ImGui theme
//

#include <catch2/catch_test_macros.hpp>
#include "imgui.h"
#include "ImGuiSkins/ShadSkin.h"
#include "ImGuiSkins/SkinsBase.h"

TEST_CASE("ShadSkin can be instantiated and applied") {
    // Create a temporary ImGui context for testing
    ImGui::CreateContext();
    ImGui::GetIO().Fonts->Build(); // Build font atlas

    SECTION("ShadSkin object can be created") {
        Application::Skins::ShadSkin skin;
        REQUIRE(true); // Object created successfully
    }

    SECTION("ApplySkin can be called without crashing") {
        Application::Skins::ShadSkin skin;
        REQUIRE_NOTHROW(skin.ApplySkin());
    }

    SECTION("ApplySkin modifies ImGui style properties") {
        Application::Skins::ShadSkin skin;

        // Apply the skin
        skin.ApplySkin();

        auto& style = ImGui::GetStyle();

        // Verify style properties were changed
        REQUIRE(style.FrameRounding == 6.0f);
        REQUIRE(style.FrameBorderSize == 1.0f);
        REQUIRE(style.IndentSpacing == 11.0f);
        REQUIRE(style.WindowPadding.x == 10.0f);
        REQUIRE(style.WindowPadding.y == 10.0f);
        REQUIRE(style.FramePadding.x == 8.0f);
        REQUIRE(style.FramePadding.y == 6.0f);
        REQUIRE(style.ItemSpacing.x == 6.0f);
        REQUIRE(style.ItemSpacing.y == 6.0f);
        REQUIRE(style.ChildRounding == 6.0f);
        REQUIRE(style.PopupRounding == 6.0f);
        REQUIRE(style.WindowTitleAlign.x == 0.5f);
        REQUIRE(style.WindowTitleAlign.y == 0.5f);
    }

    SECTION("ApplySkin sets expected color values") {
        Application::Skins::ShadSkin skin;
        skin.ApplySkin();

        auto& colors = ImGui::GetStyle().Colors;

        // Verify some key colors are set correctly
        // Button color should be (56, 56, 56, 200)
        ImVec4 expectedButton = ImColor(56, 56, 56, 200);
        REQUIRE(colors[ImGuiCol_Button].x == expectedButton.x);
        REQUIRE(colors[ImGuiCol_Button].y == expectedButton.y);
        REQUIRE(colors[ImGuiCol_Button].z == expectedButton.z);
        REQUIRE(colors[ImGuiCol_Button].w == expectedButton.w);

        // Text color should match Theme::text
        ImVec4 expectedText = ImGui::ColorConvertU32ToFloat4(Application::Skins::Colors::Theme::text);
        REQUIRE(colors[ImGuiCol_Text].x == expectedText.x);
        REQUIRE(colors[ImGuiCol_Text].y == expectedText.y);
        REQUIRE(colors[ImGuiCol_Text].z == expectedText.z);
        REQUIRE(colors[ImGuiCol_Text].w == expectedText.w);

        // Header color should match Theme::groupHeader
        ImVec4 expectedHeader = ImGui::ColorConvertU32ToFloat4(Application::Skins::Colors::Theme::groupHeader);
        REQUIRE(colors[ImGuiCol_Header].x == expectedHeader.x);
        REQUIRE(colors[ImGuiCol_Header].y == expectedHeader.y);
        REQUIRE(colors[ImGuiCol_Header].z == expectedHeader.z);
        REQUIRE(colors[ImGuiCol_Header].w == expectedHeader.w);
    }

    SECTION("ApplySkin can be called multiple times") {
        Application::Skins::ShadSkin skin;

        // Apply multiple times - should not crash
        REQUIRE_NOTHROW(skin.ApplySkin());
        REQUIRE_NOTHROW(skin.ApplySkin());
        REQUIRE_NOTHROW(skin.ApplySkin());

        // Style should still be correct after multiple applications
        auto& style = ImGui::GetStyle();
        REQUIRE(style.FrameRounding == 6.0f);
        REQUIRE(style.FrameBorderSize == 1.0f);
    }

    SECTION("ApplySkin loads custom fonts") {
        Application::Skins::ShadSkin skin;
        skin.ApplySkin();

        // Verify fonts are loaded and accessible
        ImFont* defaultFont = Application::Skins::ShadSkin::GetFont("Default");
        ImFont* regularFont = Application::Skins::ShadSkin::GetFont("Regular");
        ImFont* boldFont = Application::Skins::ShadSkin::GetFont("Bold");

        REQUIRE(defaultFont != nullptr);
        REQUIRE(regularFont != nullptr);
        REQUIRE(boldFont != nullptr);
    }

    SECTION("Default font is set correctly") {
        Application::Skins::ShadSkin skin;
        skin.ApplySkin();

        ImGuiIO& io = ImGui::GetIO();
        ImFont* defaultFont = Application::Skins::ShadSkin::GetFont("Default");

        REQUIRE(io.FontDefault != nullptr);
        REQUIRE(io.FontDefault == defaultFont);
    }

    SECTION("GetFont returns nullptr for non-existent font") {
        Application::Skins::ShadSkin skin;
        skin.ApplySkin();

        ImFont* nonExistentFont = Application::Skins::ShadSkin::GetFont("NonExistent");
        REQUIRE(nonExistentFont == nullptr);
    }

    SECTION("Fonts are only loaded once on multiple ApplySkin calls") {
        Application::Skins::ShadSkin skin;

        skin.ApplySkin();
        ImFont* firstDefault = Application::Skins::ShadSkin::GetFont("Default");

        skin.ApplySkin();
        ImFont* secondDefault = Application::Skins::ShadSkin::GetFont("Default");

        // Should be the same font pointer (not reloaded)
        REQUIRE(firstDefault == secondDefault);
    }

    // Clean up ImGui context
    ImGui::DestroyContext();
}

TEST_CASE("Multiple ShadSkin instances work correctly") {
    ImGui::CreateContext();

    Application::Skins::ShadSkin skin1;
    Application::Skins::ShadSkin skin2;

    SECTION("Multiple instances can be created") {
        REQUIRE(true); // Both created successfully
    }

    SECTION("Different instances produce same result") {
        skin1.ApplySkin();
        auto& colors1 = ImGui::GetStyle().Colors;
        ImVec4 button1 = colors1[ImGuiCol_Button];

        skin2.ApplySkin();
        auto& colors2 = ImGui::GetStyle().Colors;
        ImVec4 button2 = colors2[ImGuiCol_Button];

        // Both should set the same button color
        REQUIRE(button1.x == button2.x);
        REQUIRE(button1.y == button2.y);
        REQUIRE(button1.z == button2.z);
        REQUIRE(button1.w == button2.w);
    }

    ImGui::DestroyContext();
}
