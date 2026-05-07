#include "GameManager.hpp"
#include "FightScene.hpp"

void GameManager::run()
{
	m_fightSceneId = m_engine.addScene(std::make_unique<FightScene>(this));
	m_engine.run();
}

void GameManager::switchToScene(ee::SceneId _id)
{
	m_engine.switchScene(_id);
}

void GameManager::resetFightScene()
{
	m_engine.replaceScene(m_fightSceneId, std::make_unique<FightScene>(this));
}