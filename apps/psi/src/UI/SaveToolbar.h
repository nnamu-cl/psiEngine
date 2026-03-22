#pragma once

#include "Layers/DefaultGameWorld/CameraController.h"

class PsiWorldLayer;

/**
 * SaveToolbar - Floating save button with auto-save countdown and camera hints.
 *
 * Positioned at the bottom-left of the viewport.
 * Auto-saves every 30 seconds when a project is loaded.
 * Shows a hint above the toolbar when camera controls are active.
 */
class SaveToolbar
{
public:
    explicit SaveToolbar(PsiWorldLayer* worldLayer);
    void Render();

private:
    PsiWorldLayer* m_WorldLayer = nullptr;

    // Auto-save state
    float m_AutoSaveInterval = 30.0f;   // seconds between auto-saves
    float m_AutoSaveTimer    = 30.0f;   // counts down to 0
};
