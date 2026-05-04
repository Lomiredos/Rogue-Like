
#include "engine/Engine.hpp"



class GameManager {


	ee::Engine m_engine;


public:

	GameManager(const char* _name, int _width, int _height, int _fpsTarget = 60) : m_engine(_name, _width, _height, _fpsTarget) {}

	void run();

};