#include "DungeonGenerator.hpp"
#include <algorithm>
#include <unordered_set>

static constexpr int MAP_SIZE = 110;
static constexpr int MAP_MARGIN = 11;

ee::math::Vector2<float> DungeonGenerator::generate(
	std::vector<std::vector<Cell>>& mapDown,
	std::vector<std::vector<Cell>>& mapUp,
	std::mt19937& rng,
	std::vector<RoomInfo>& outRooms)
{
	m_mapDown = &mapDown;
	m_mapUp = &mapUp;
	m_rng = &rng;

	ee::math::Vector2<float> spawnPos;

	do {
		m_rooms.clear();
		// Init maps
		mapDown.assign(MAP_SIZE, std::vector<Cell>(MAP_SIZE, { Tile::EMPTY }));
		mapUp.assign(MAP_SIZE, std::vector<Cell>(MAP_SIZE, { Tile::EMPTY }));

		std::uniform_int_distribution<int> dist_pos(15, 45);
		spawnPos = { (float)dist_pos(rng), (float)dist_pos(rng) };

		createMaze((int)spawnPos.x, (int)spawnPos.y);

		// Ouvrir les murs isol�s
		for (int x = 1; x < MAP_SIZE - 2; x++) {
			for (int y = 1; y < MAP_SIZE - 2; y++) {
				if (isWalkable(mapDown[x][y].type)) continue;

				if ((isWalkable(mapDown[x - 1][y].type) && isWalkable(mapDown[x + 1][y].type))
					|| (isWalkable(mapDown[x][y - 1].type) && isWalkable(mapDown[x][y + 1].type))) {

					std::uniform_int_distribution<int> dist_isBreak(1, 4);
					std::uniform_int_distribution<int> dist_floor(1, 8);
					mapDown[x][y] = { (Tile)(dist_isBreak(rng) == 1 ? dist_floor(rng) : 1) };
				}
			}
		}

		// Supprimer les ilots trop petits
		for (int x = 2; x < MAP_SIZE - 2; x++) {
			for (int y = 2; y < MAP_SIZE - 2; y++) {
				if (!isWall(mapDown[x][y].type)) continue;

				std::vector<std::pair<int, int>> group;
				countAdj(x, y, group);

				// Calculer la bounding box du groupe
				int minX = group[0].first, maxX = group[0].first;
				int minY = group[0].second, maxY = group[0].second;
				for (auto& p : group) {
					minX = std::min(minX, p.first);  maxX = std::max(maxX, p.first);
					minY = std::min(minY, p.second); maxY = std::max(maxY, p.second);
				}

				// Un ilot doit faire minimum 2x3 (ou 3x2) pour bien tiler
				int dimX = maxX - minX + 1;
				int dimY = maxY - minY + 1;
				if (std::min(dimX, dimY) < 2 || std::max(dimX, dimY) < 3) {
					for (auto& p : group)
						mapDown[p.first][p.second].type = Tile::EMPTY;
				}
			}
		}

	} while (countWalkableSize() < 1000);


	// Passe de d�coration des murs
	for (int x = MAP_SIZE - 2; x > 1; x--) {
		for (int y = MAP_SIZE - 2; y > 1; y--) {

			if (isWalkable(mapDown[x][y].type)) continue;

			bool up = isWalkable(mapDown[x][y + 1].type);
			bool down = isWalkable(mapDown[x][y - 1].type);
			bool left = isWalkable(mapDown[x - 1][y].type);
			bool right = isWalkable(mapDown[x + 1][y].type);

			if (up || down) {
				if (up) {
					if (!isWalkable(mapUp[x + 1][y].type) && !isWalkable(mapDown[x + 1][y].type) && mapUp[x + 1][y - 1].type == Tile::EMPTY)
						mapUp[x + 1][y - 1].type = Tile::WALL_OUTER_TOP_RIGHT;
					if (!isWalkable(mapUp[x - 1][y].type) && !isWalkable(mapDown[x - 1][y].type) && mapUp[x - 1][y - 1].type == Tile::EMPTY)
						mapUp[x - 1][y - 1].type = Tile::WALL_OUTER_TOP_LEFT;

					if (isWall(mapDown[x - 1][y].type) && isWall(mapDown[x + 1][y].type)) {
						mapDown[x][y].type = Tile::WALL_MID;
						mapUp[x][y - 1].type = Tile::WALL_TOP_MID;
					}
					else if (isWall(mapDown[x - 1][y].type)) {
						mapDown[x][y].type = Tile::WALL_MID_LEFT;
						mapUp[x][y - 1].type = isWall(mapDown[x][y - 1].type)
							? Tile::WALL_EDGE_BOTTOM_RIGHT
							: Tile::WALL_TOP_LEFT;
					}
					else {
						mapDown[x][y].type = Tile::WALL_MID_RIGHT;
						if (isWall(mapDown[x][y - 1].type)) {
							mapUp[x][y - 1].type = Tile::WALL_EDGE_BOTTOM_LEFT;
						}
						else {
							mapUp[x][y - 1].type = Tile::WALL_TOP_RIGHT;
							if (mapUp[x + 1][y - 1].type == Tile::EMPTY && isWall(mapDown[x + 1][y].type))
								mapUp[x + 1][y - 1].type = Tile::WALL_OUTER_TOP_RIGHT;
						}
					}
				}
				else if (down) {
					mapDown[x][y].type = Tile::EMPTY;

					if (!isWalkable(mapDown[x - 1][y].type) && !isWalkable(mapDown[x + 1][y].type)) {
						mapUp[x][y].type = Tile::WALL_MID;
						mapUp[x][y - 1].type = Tile::WALL_TOP_MID;
					}
					else if (!isWalkable(mapDown[x - 1][y].type)) {
						mapUp[x][y].type = Tile::WALL_MID;
						if (!isWalkable(mapDown[x][y + 1].type)) {
							mapUp[x][y].type = Tile::WALL_EDGE_MID_RIGHT;
							if (mapUp[x][y + 1].type == Tile::EMPTY && !isWall(mapDown[x][y + 1].type))
								mapUp[x][y + 1].type = Tile::WALL_OUTER_FRONT_LEFT;
						}
						mapUp[x][y - 1].type = Tile::WALL_TOP_LEFT;
					}
					else {
						mapUp[x][y].type = Tile::WALL_MID;
						if (!isWalkable(mapDown[x][y + 1].type)) {
							mapUp[x][y].type = Tile::WALL_EDGE_MID_LEFT;
							if (mapUp[x][y + 1].type == Tile::EMPTY && !isWall(mapDown[x][y + 1].type))
								mapUp[x][y + 1].type = Tile::WALL_OUTER_FRONT_RIGHT;
						}
						mapUp[x][y - 1].type = Tile::WALL_TOP_RIGHT;
					}
				}
			}
			else if (left || right) {
				if (left) {
					mapDown[x][y].type = Tile::EMPTY;
					if (mapUp[x][y].type == Tile::EMPTY) mapUp[x][y].type = Tile::WALL_EDGE_MIDDOWN_LEFT;
					if (mapUp[x][y - 1].type == Tile::EMPTY) mapUp[x][y - 1].type = Tile::WALL_EDGE_MIDDOWN_LEFT;
					if (mapUp[x][y + 1].type == Tile::EMPTY && !isWalkable(mapDown[x][y].type) && !isWall(mapDown[x][y + 1].type))
						mapUp[x][y + 1].type = Tile::WALL_OUTER_FRONT_RIGHT;
				}
				if (right) {
					mapDown[x][y].type = Tile::EMPTY;
					if (mapUp[x][y].type == Tile::EMPTY) mapUp[x][y].type = Tile::WALL_EDGE_MIDDOWN_RIGHT;
					if (mapUp[x][y - 1].type == Tile::EMPTY) mapUp[x][y - 1].type = Tile::WALL_EDGE_MIDDOWN_RIGHT;
					if (mapUp[x][y + 1].type == Tile::EMPTY && !isWalkable(mapDown[x][y].type) && !isWall(mapDown[x][y + 1].type))
						mapUp[x][y + 1].type = Tile::WALL_OUTER_FRONT_LEFT;
				}
			}
			else {
				mapDown[x][y].type = Tile::EMPTY;
			}

		}
	}



	//##TODO fix
	/*
	for (int x = 2; x < MAP_SIZE - 2; x++) {
		for (int y = 2; y < MAP_SIZE - 2; y++) {
			if (mapUp[x][y].type == Tile::WALL_MID && mapDown[x][y + 1].type == Tile::WALL_MID) {

				if (isWall(mapUp[x - 1][y].type) && isWall(mapUp[x + 1][y].type))
					mapUp[x][y].type = Tile::WALL_WITH_BOTTOM_MID;
				else if (isWall(mapUp[x - 1][y].type))
					mapUp[x][y].type = Tile::WALL_WITH_BOTTOM_RIGHT;
				else
					mapUp[x][y].type = Tile::WALL_WITH_BOTTOM_LEFT;

			}

			if (mapUp[x][y].type == Tile::WALL_EDGE_MID_RIGHT && isWall(mapDown[x][y + 1].type) && isWall(mapDown[x+1][y+1].type)) {
				mapUp[x][y].type == Tile::WALL_WITH_BOTTOM_RIGHT;
			}
			if (mapUp[x][y].type == Tile::WALL_EDGE_MID_LEFT && isWall(mapDown[x][y + 1].type) && isWall(mapDown[x - 1][y + 1].type)) {
				mapUp[x][y].type == Tile::WALL_WITH_BOTTOM_RIGHT;
			}
		}
	}
	*/

	placeDeco();

	outRooms = m_rooms;
	return spawnPos;
}

void DungeonGenerator::createMaze(int posX, int posY)
{
	std::uniform_int_distribution<int> dist_size(15, 22);
	createRoom({ (float)posX - 5, (float)posY - 5, (float)dist_size(*m_rng), (float)dist_size(*m_rng) }, true);
}

void DungeonGenerator::createRoom(ee::math::Rect<float> rect, bool first)
{
	std::uniform_int_distribution<int> dist_family(0, 5);
	int family = first ? -1 : dist_family(*m_rng);
	m_rooms.push_back({ rect, family });

	createRect(rect);

	std::uniform_int_distribution<int> dist_size(10, std::max(10, rect.w() == rect.h() ? (int)rect.w() : (int)std::max(rect.w(), rect.h())));
	std::uniform_int_distribution<int> dist_large(4, 8);
	std::uniform_int_distribution<int> dist_plaX(1, (int)rect.w());
	std::uniform_int_distribution<int> dist_plaY(1, (int)rect.h());

	bool exist[4];
	std::vector<int> index = { 2, 0, 3, 1 };
	int cree = 1;
	for (int i = 0; i < 4; i++) {
		std::uniform_int_distribution<int> dist_exist(0, cree * (first ? 2 : 4));
		std::uniform_int_distribution<int> dist_which(0, 3 - i);
		bool succes = dist_exist(*m_rng) == 1;
		int j = dist_which(*m_rng);
		exist[index[j]] = succes;
		if (succes) cree++;
		index[j] = index[3 - i];
	}

	float size = 0, large = 0, pla = 0;

	if (exist[0]) {
		size = dist_size(*m_rng); large = dist_large(*m_rng);
		pla = std::max(large, (float)(dist_plaX(*m_rng) - large));
		createTunnel({ rect.x() + pla - large / 2, rect.y() - size + 1, large, size }, 0, 0);
	}
	if (exist[1]) {
		size = dist_size(*m_rng); large = dist_large(*m_rng);
		pla = std::max(large, (float)(dist_plaX(*m_rng) - large));
		createTunnel({ rect.x() + pla - large / 2, rect.y() + rect.h() - 1, large, size }, 0, 1);
	}
	if (exist[2]) {
		size = dist_size(*m_rng); large = dist_large(*m_rng);
		pla = std::max(large, (float)(dist_plaY(*m_rng) - large));
		createTunnel({ rect.x() - size + 1, rect.y() + pla - large / 2, size, large }, 0, 0);
	}
	if (exist[3]) {
		size = dist_size(*m_rng); large = dist_large(*m_rng);
		pla = std::max(large, (float)(dist_plaY(*m_rng) - large));
		createTunnel({ rect.x() + rect.w() - 1, rect.y() + pla - large / 2, size, large }, 1, 0);
	}
}

void DungeonGenerator::createTunnel(ee::math::Rect<float> rect, float anchorX, float anchorY)
{

	std::uniform_int_distribution<int> dist_type(0, 1);
	int type = dist_type(*m_rng);
	m_tunnels.push_back({ rect, type });


	createRect(rect);

	std::uniform_int_distribution<int> dist_what(1, 4);
	std::uniform_int_distribution<int> dist_size(5, 12);
	float side = std::min(rect.w(), rect.h());

	if (dist_what(*m_rng) == 1)
		createRoom({ rect.x() + rect.w() * anchorX - 1, rect.y() + rect.h() * anchorY - 1, side + dist_size(*m_rng), side + dist_size(*m_rng) }, false);
	else
		createIntersection({ rect.x() + rect.w() * anchorX - 1, rect.y() + rect.h() * anchorY - 1, side + 2, side + 2 });
}

void DungeonGenerator::createIntersection(ee::math::Rect<float> rect)
{
	createRect(rect);

	std::uniform_int_distribution<int> dist_size(10, 19);
	std::uniform_int_distribution<int> dist_large(4, 8);
	std::uniform_int_distribution<int> dist_plaX(1, (int)rect.w());
	std::uniform_int_distribution<int> dist_plaY(1, (int)rect.h());

	bool exist[4];
	std::vector<int> index = { 2, 0, 3, 1 };
	int cree = 1;
	for (int i = 0; i < 4; i++) {
		std::uniform_int_distribution<int> dist_exist(0, cree * 3);
		std::uniform_int_distribution<int> dist_which(0, 3 - i);
		bool succes = dist_exist(*m_rng) == 1;
		int j = dist_which(*m_rng);
		exist[index[j]] = succes;
		if (succes) cree++;
		index[j] = index[3 - i];
	}

	float size = 0, large = 0, pla = 0;

	if (exist[0]) {
		size = dist_size(*m_rng); large = dist_large(*m_rng);
		pla = std::max(large, (float)(dist_plaX(*m_rng) - large));
		createTunnel({ rect.x() + pla - large / 2, rect.y() - size + 1, large, size }, 0, 0);
	}
	if (exist[1]) {
		size = dist_size(*m_rng); large = dist_large(*m_rng);
		pla = std::max(large, (float)(dist_plaX(*m_rng) - large));
		createTunnel({ rect.x() + pla - large / 2, rect.y() + rect.h() - 1, large, size }, 0, 1);
	}
	if (exist[2]) {
		size = dist_size(*m_rng); large = dist_large(*m_rng);
		pla = std::max(large, (float)(dist_plaY(*m_rng) - large));
		createTunnel({ rect.x() - size + 1, rect.y() + pla - large / 2, size, large }, 0, 0);
	}
	if (exist[3]) {
		size = dist_size(*m_rng); large = dist_large(*m_rng);
		pla = std::max(large, (float)(dist_plaY(*m_rng) - large));
		createTunnel({ rect.x() + rect.w() - 1, rect.y() + pla - large / 2, size, large }, 1, 0);
	}
}

void DungeonGenerator::createRect(ee::math::Rect<float> rect)
{
	auto& map = *m_mapDown;
	const int limit = MAP_SIZE - 1 - MAP_MARGIN;
	const int origin = 2 + MAP_MARGIN;

	for (int x = 0; x < (int)rect.w(); x++) {
		for (int y = 0; y < (int)rect.h(); y++) {
			int ix = std::min(limit, std::max(origin, (int)(rect.x() + x)));
			int iy = std::min(limit, std::max(origin, (int)(rect.y() + y)));

			if (isWalkable(map[ix][iy].type)) continue;

			if (x == 0 || x == (int)rect.w() - 1 || y == 0 || y == (int)rect.h() - 1) {
				map[ix][iy] = { Tile::WALL_MID };
			}
			else {
				std::uniform_int_distribution<int> dist_isBreak(1, 4);
				std::uniform_int_distribution<int> dist_floor(1, 8);
				map[ix][iy] = { (Tile)(dist_isBreak(*m_rng) == 1 ? dist_floor(*m_rng) : 1) };
			}
		}
	}
}

int DungeonGenerator::countWalkableSize()
{
	int count = 0;
	for (auto& col : *m_mapDown)
		for (auto& cell : col)
			if (isWalkable(cell.type)) count++;
	return count;
}

void DungeonGenerator::countAdj(int x, int y, std::vector<std::pair<int, int>>& visited)
{
	const int mapW = MAP_SIZE;

	std::unordered_set<int> seen;
	std::vector<std::pair<int, int>> stack = { { x, y } };

	while (!stack.empty()) {
		auto pair = stack.back();
		int cx = pair.first, cy = pair.second;
		stack.pop_back();

		if (cx <= 1 || cx >= mapW - 2) continue;
		if (cy <= 1 || cy >= (int)(*m_mapDown)[cx].size() - 2) continue;
		if (!isWall((*m_mapDown)[cx][cy].type)) continue;

		if (!seen.insert(cx + cy * mapW).second) continue;

		visited.push_back({ cx, cy });
		stack.push_back({ cx - 1, cy });
		stack.push_back({ cx + 1, cy });
		stack.push_back({ cx, cy - 1 });
		stack.push_back({ cx, cy + 1 });
	}
}

void DungeonGenerator::placeDeco()
{
	auto& mapDown = *m_mapDown;
	auto& mapUp = *m_mapUp;
	int mapW = (int)mapDown.size();
	int mapH = (int)mapDown[0].size();

	Tile mixedFlags[] = {
		Tile::REDFLAG, Tile::BLUEFLAG, Tile::GREENFLAG, Tile::YELLOWFLAG
	};

	Tile TopFontain[] = {
		Tile::WALL_TOP_FOUNTAIN_1,Tile::WALL_TOP_FOUNTAIN_2,
		Tile::WALL_TOP_FOUNTAIN_3
	};

	Tile mixedFloorBone[] = {
		Tile::FLOOR_1B, Tile::FLOOR_2B, Tile::FLOOR_3B,
		Tile::FLOOR_4B, Tile::FLOOR_5B, Tile::FLOOR_6B,
		Tile::FLOOR_7B, Tile::FLOOR_8B
	};

	//fonction lambda pour pas rajouter une decla
	auto familyToFlag = [&](int family) -> Tile {
		switch (family) {
		case 0: return Tile::BLUEFLAG;    // Slimes
		case 2: return Tile::GREENFLAG;   // Orcs
		case 3: return Tile::REDFLAG;     // Imps
		case 5: return Tile::YELLOWFLAG;  // DarkGuy
		default: {
			std::uniform_int_distribution<int> d(0, 3);
			return mixedFlags[d(*m_rng)];  // mixte
		}
		}
		};

	std::uniform_int_distribution<int> flagChance(0, 7);
	std::uniform_int_distribution<int> fountainChance(0, 7);
	std::uniform_int_distribution<int> TopfountainChance(0, 2);
	std::uniform_int_distribution<int> boneChance(0, 40);

	for (const RoomInfo& room : m_rooms)
	{
		if (room.family < 0) continue;//spawn

		Tile flagTile = familyToFlag(room.family);

		int x0 = std::min(mapW - 1, std::max(0, (int)room.rect.x()));
		int x1 = std::min(mapW - 1, (int)(room.rect.x() + room.rect.w()));
		int y0 = std::min(mapW - 1, std::max(0, (int)room.rect.y()));

		for (int x = x0; x <= x1; x++) {

			if (isWalkable(mapDown[x][y0].type)) {
				if (boneChance(*m_rng) == 0) {
					mapDown[x][y0].type = mixedFloorBone[flagChance(*m_rng)]; // flag chance c'es pile entre 0 et 7 et y'a 7 index donc on reutilise
					continue;
				}
			}
			if (!isWall(mapDown[x][y0].type)) continue;
			if (flagChance(*m_rng) != 0)      continue;
			mapDown[x][y0].type = flagTile;


		}
	}

	for (const TunnelInfo& tunnel : m_tunnels) {

		int x0 = std::min(mapW - 1, std::max(0, (int)tunnel.rect.x()));
		int x1 = std::min(mapW - 1, (int)(tunnel.rect.x() + tunnel.rect.w()));
		int y0 = std::min(mapW - 1, std::max(1, (int)tunnel.rect.y()));

		for (int x = x0; x <= x1; x++) {

			if (!isWall(mapDown[x][y0].type)) continue;
			if (mapDown[x - 1][y0].type != Tile::WALL_MID || mapDown[x + 1][y0].type != Tile::WALL_MID) continue;
			if (fountainChance(*m_rng) != 0)  continue;

			mapDown[x][y0].type = tunnel.type == 0 ? Tile::WALL_MID_FOUNTAIN_L : Tile::WALL_MID_FOUNTAIN_W;
			mapDown[x][y0 + 1].type = tunnel.type == 0 ? Tile::WALL_BOTTOM_FOUNTAIN_L : Tile::WALL_BOTTOM_FOUNTAIN_W;
			mapDown[x][y0 - 1].type = TopFontain[TopfountainChance(*m_rng)];


		}
	}
}
