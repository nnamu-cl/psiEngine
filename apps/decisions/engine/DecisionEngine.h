#pragma once
#include <vector>

#include "Game.h"


class DecisionEngine
{
public:
    DecisionEngine();
    std::vector<Game*> games;
    bool AddGame(Game *game);



};
