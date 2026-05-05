#include "GameManager.hpp"
#include "FightScene.hpp"

void GameManager::run()
{
	m_engine.addScene(std::make_unique<FightScene>(this));
	m_engine.run();
}