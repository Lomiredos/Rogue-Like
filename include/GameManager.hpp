
#include "engine/Engine.hpp"
#include "Inventory.hpp"



class GameManager {

	ee::Engine m_engine;
	Inventory m_inventory;

	ee::SceneId m_fightSceneId = 0;

public:

	GameManager(const char* _name, int _width, int _height, int _fpsTarget = 60) : m_engine(_name, _width, _height, _fpsTarget) {}

	void run();

	void switchToScene(ee::SceneId _id);
	void resetFightScene();

	Inventory& getInventory() { return m_inventory; }

};