#include "PsiWorldLayer.h"

PsiWorldLayer::PsiWorldLayer(ApplicationWindowData* windowData)
    : DefaultGameWorld(windowData)
    , backgroundColor(psi::Colors::SecondaryBackground)
{
    // Constructor - initialize PSI-specific world state
    // backgroundColor initialized to SecondaryBackground (deepest layer)
}
