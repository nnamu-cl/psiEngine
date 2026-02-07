#pragma once

#include "NodeEditorPinTypes.h"
#include "imgui.h"

/**
 * Gets the icon color for a specific pin type
 * @param type The pin type
 * @return The color associated with the pin type
 */
ImColor GetIconColor(PinType type);

/**
 * Draws a pin icon with appropriate styling
 * @param pin The pin to draw
 * @param connected Whether the pin is currently connected
 * @param alpha Alpha value (0-255)
 */
void DrawPinIcon(const Pin& pin, bool connected, int alpha);
