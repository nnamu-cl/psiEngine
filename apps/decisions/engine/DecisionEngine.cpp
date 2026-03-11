#include "DecisionEngine.h"

bool DecisionEngine::AddGame(Game* game)
{
    games.push_back(game);
    return true;
}
