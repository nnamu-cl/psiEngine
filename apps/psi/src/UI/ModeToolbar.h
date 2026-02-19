#pragma once

#include "PsiMode.h"

class PsiWorldLayer;
class PsiNodeEditorLayer;

/**
 * ModeToolbar - Bottom-center toolbar for switching between World Viewport and Graph Editor.
 *
 * Owns the current mode state. Tab key toggles between modes.
 */
class ModeToolbar
{
public:
    ModeToolbar(PsiWorldLayer* worldLayer, PsiNodeEditorLayer* nodeEditorLayer);

    void Render();

    PsiMode GetMode() const { return m_CurrentMode; }

private:
    PsiWorldLayer*       m_WorldLayer;
    PsiNodeEditorLayer*  m_NodeEditorLayer;
    PsiMode              m_CurrentMode = PsiMode::WorldViewport;

    void applyMode(PsiMode mode);
};
