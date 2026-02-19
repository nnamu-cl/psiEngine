#pragma once

/**
 * UISettings - Global runtime-tunable UI layout values.
 *
 * Modify at runtime via the Control Panel "UI Layout" tab.
 * g_UISettings is an inline global (C++20) — safe to include in
 * multiple translation units without ODR violations.
 */
struct UISettings
{
    // --- GizmoToolbar ---
    float gizmoLeftMargin   = 8.0f;    // window X offset from screen left
    float gizmoTopMargin    = 148.0f;  // window Y offset from screen top
    float gizmoFramePadX    = 10.0f;   // ImGuiStyleVar_FramePadding X (button internal pad)
    float gizmoFramePadY    = 12.0f;   // ImGuiStyleVar_FramePadding Y
    float gizmoItemSpacingX = 4.0f;    // ImGuiStyleVar_ItemSpacing X (gap between buttons)
    float gizmoItemSpacingY = 8.0f;    // ImGuiStyleVar_ItemSpacing Y
    float gizmoFontScale    = 2.0f;    // SetWindowFontScale multiplier for icon glyphs
    float gizmoHoverAlpha   = 0.12f;   // ButtonHovered white overlay alpha
    float gizmoActiveAlpha  = 0.22f;   // ButtonActive  white overlay alpha

    // --- TextIcons (IconText vertical centering) ---
    float iconTextHeight    = 30.0f;   // Assumed icon glyph height used to center icon+text pairs
};

inline UISettings g_UISettings;
