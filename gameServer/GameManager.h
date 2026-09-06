#pragma once

#include "GameTypes.h"

#define GGameLoop GameManager::Instance().GetGameLoop()

class GameManager
{
    public:
        static GameManager &Instance()
        {
            static GameManager instance;
            return instance;
        }

        GameLoop &GetGameLoop();

    private:
        GameManager();
        ~GameManager();
        GameManager(const GameManager &) = delete;
        GameManager &operator=(const GameManager &) = delete;

        GameLoopUniqueRef _gameLoop;
};
