#pragma once
#include <vector>
#include <random>
#include <utility>


#include "math/Rect.hpp"
#include "math/Vector2.hpp"
#include "Utils.hpp"


struct RoomInfo
{
	ee::math::Rect<float> rect;
	int family;
};

struct TunnelInfo {
	ee::math::Rect<float> rect;
	int type;
};

class DungeonGenerator
{
public:

	ee::math::Vector2<float> generate(
		std::vector<std::vector<Cell>>& mapDown,
		std::vector<std::vector<Cell>>& mapUp,
		std::mt19937& rng,
		std::vector<RoomInfo>& outRooms
	);

private:
	std::vector<std::vector<Cell>>* m_mapDown = nullptr;
	std::vector<std::vector<Cell>>* m_mapUp = nullptr;
	std::mt19937* m_rng = nullptr;

	std::vector<RoomInfo> m_rooms;
	std::vector<TunnelInfo> m_tunnels;

	void createMaze(int posX, int posY);
	void createRoom(ee::math::Rect<float> rect, bool first);
	void createTunnel(ee::math::Rect<float> rect, float anchorX, float anchorY);
	void createIntersection(ee::math::Rect<float> rect);
	void createRect(ee::math::Rect<float> rect);

	int  countWalkableSize();
	void countAdj(int x, int y, std::vector<std::pair<int, int>>& visited);
	void placeDeco();

};
