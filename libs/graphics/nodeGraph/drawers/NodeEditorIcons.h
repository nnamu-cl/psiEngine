#pragma once

#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui.h"

enum class IconType: ImU32 { Flow, Circle, Square, Grid, RoundSquare, Diamond };

/**
 * Draws an icon shape using ImGui draw list
 * @param drawList The ImGui draw list to draw on
 * @param a Top-left corner position
 * @param b Bottom-right corner position
 * @param type The type of icon to draw
 * @param filled Whether the icon should be filled or outlined
 * @param color The icon color
 * @param innerColor The inner fill color (for outlined icons)
 */
void DrawIcon(ImDrawList* drawList, const ImVec2& a, const ImVec2& b, IconType type, bool filled, ImU32 color, ImU32 innerColor);

/**
 * Draws an icon at the current ImGui cursor position
 * @param size Size of the icon
 * @param type The type of icon to draw
 * @param filled Whether the icon should be filled or outlined
 * @param color The icon color
 * @param innerColor The inner fill color (for outlined icons)
 */
void Icon(const ImVec2& size, IconType type, bool filled, const ImVec4& color = ImVec4(1, 1, 1, 1), const ImVec4& innerColor = ImVec4(0, 0, 0, 0));
