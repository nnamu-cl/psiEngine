#pragma once

/**
 * PsiMode - Application-wide editing mode for the PSI application.
 *
 * WorldViewport: The user interacts with the 3D scene — placing, selecting,
 *                and manipulating objects. The node editor is hidden.
 *
 * GraphEditor:   The user edits the node graph. The node editor canvas is
 *                shown; the 3D viewport takes a secondary role.
 */
enum class PsiMode
{
    WorldViewport,
    GraphEditor,
};
