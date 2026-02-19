#pragma once

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

/**
 * psi::Colors - Color palette for PSI application
 *
 * Defines the color scheme for dark mode UI and rendering.
 * Colors are stored as normalized float values (0.0 - 1.0) for use with graphics APIs.
 */
namespace psi::Colors
{
    // Helper function to convert hex color to normalized RGB
    constexpr glm::vec3 HexToRGB(uint32_t hex)
    {
        return glm::vec3(
            ((hex >> 16) & 0xFF) / 255.0f,  // Red
            ((hex >> 8) & 0xFF) / 255.0f,   // Green
            (hex & 0xFF) / 255.0f           // Blue
        );
    }

    // Helper function to convert hex color to normalized RGBA
    constexpr glm::vec4 HexToRGBA(uint32_t hex, float alpha = 1.0f)
    {
        return glm::vec4(
            ((hex >> 16) & 0xFF) / 255.0f,  // Red
            ((hex >> 8) & 0xFF) / 255.0f,   // Green
            (hex & 0xFF) / 255.0f,          // Blue
            alpha                            // Alpha
        );
    }

    // ========================================================================
    // Background Colors (Dark Mode)
    // ========================================================================

    // Primary Background: The main/closest background layer (e.g., panels, windows)
    constexpr glm::vec3 PrimaryBackground = HexToRGB(0x1E2228);

    // Secondary Background: Further back than primary - used for the world/scene background
    // This is the deepest layer, providing depth and separation from UI elements
    constexpr glm::vec3 SecondaryBackground = HexToRGB(0x000000);

    // 3D Background: Background color for the 3D viewport/scene area
    constexpr glm::vec3 Background3D = HexToRGB(0x080808);

    // Tertiary Background: Slightly lighter than primary, for nested panels or hover states
    constexpr glm::vec3 TertiaryBackground = HexToRGB(0x2A2F38);

    // ========================================================================
    // Accent Colors
    // ========================================================================

    constexpr glm::vec3 AccentBlue = HexToRGB(0x31BCFD);
    constexpr glm::vec3 AccentGreen = HexToRGB(0x12FEAA);
    constexpr glm::vec3 AccentRed = HexToRGB(0xFE1155);
    constexpr glm::vec3 AccentYellow = HexToRGB(0xF59E0B);

}
