#pragma once

#include "NodeEditorPinTypes.h"
#include "imgui.h"

// Forward declarations
struct InputSocket;
struct OutputSocket;

/**
 * Gets the icon color for a specific socket type
 * @param type The socket type
 * @return The color associated with the socket type
 */
ImColor GetIconColor(SocketType type);

/**
 * Draws a pin icon with appropriate styling
 * @param pin The pin to draw
 * @param connected Whether the pin is currently connected
 * @param alpha Alpha value (0-255)
 */
void DrawPinIcon(const Pin& pin, bool connected, int alpha);

/**
 * Draws a pin icon for an input socket
 * @param inputSocket The input socket to draw
 * @param pinId The pin ID for the editor
 * @param alpha Alpha value (0-255)
 */
void DrawInputSocketPin(const InputSocket* inputSocket, uint64_t pinId, int alpha = 255);

/**
 * Draws a pin icon for an output socket
 * @param outputSocket The output socket to draw
 * @param pinId The pin ID for the editor
 * @param alpha Alpha value (0-255)
 */
void DrawOutputSocketPin(const OutputSocket* outputSocket, uint64_t pinId, int alpha = 255);
