#pragma once

// Forward declarations
class PsiWorldLayer;

/**
 * StatsPanel - Performance and simulation statistics overlay
 *
 * Positioned at top right of the screen as a semi-transparent overlay.
 * Displays FPS, frame time, and object count.
 */
class StatsPanel
{
public:
    explicit StatsPanel(PsiWorldLayer* worldLayer);

    void Render();

    bool IsVisible() const { return m_Visible; }
    void SetVisible(bool visible) { m_Visible = visible; }

private:
    PsiWorldLayer* m_WorldLayer;
    bool m_Visible = true;
};
