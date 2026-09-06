#include "GameManager.h"
#include "GameLoop.h"
#include <CoreMacro.h>
#include <memory>

GameManager::GameManager()
    : _gameLoop(std::make_unique<GameLoop>())
{
}

GameManager::~GameManager() {}

GameLoop &GameManager::GetGameLoop()
{
    ASSERT_CRASH(_gameLoop);
    return *_gameLoop;
}
