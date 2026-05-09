#pragma once

#include "ecs/System.hpp"
#include "ecs/World.hpp"
#include "Components/Components.hpp"
#include "Utils.hpp"
#include "math/Vector2.hpp"
#include "renderer/Renderer.hpp"
#include <queue>
#include <cstdlib>


struct Node {
	ee::math::Vector2<int> pos;
	int g;
	int h;
	ee::math::Vector2<int> parent;
};


class PathFindingSystem : public ee::ecs::System {

private:
	ee::ecs::EntityID m_playerID;
	std::vector<std::vector<bool>> m_map;

public:

	void init(const std::vector<std::vector<Cell>>& _map, ee::ecs::EntityID _playerID)
	{
		m_playerID = _playerID;

		m_map.assign(_map.size(), std::vector<bool>(_map.size(), { true }));

		for (int x = 1; x < _map.size()-1; x++) {
			for (int y = 1; y < _map[x].size()-1; y++) {
				for (int k = -1; k < 2; k++) {
					for (int l = -1; l < 2; l++) {
					m_map[x][y] = m_map[x][y] && isWalkable(_map[x + k][y + l].type);
					}
				}
			}
		}
	}

	void drawDebug(ee::ecs::World& _world, ee::renderer::Renderer& _renderer, const ee::renderer::Camera& _cam) {

		for (auto& id : m_entities) {

			auto& monsterPF = _world.getComponent<PathFindingComponent>(id);

			if (monsterPF.path.empty()) continue;

			std::queue<ee::math::Vector2<int>> copy = monsterPF.path;


			while (copy.empty() == false) {
				ee::math::Vector2<int> c = copy.front();
				copy.pop();
				c.x = c.x * 32 + 8;
				c.y = c.y * 32 + 8;
				ee::math::Rect<float> rect(_cam.getScreenX(c.x), _cam.getScreenY(c.y), 16, 16);
				_renderer.DrawRect(rect, { 0, 255, 0, 255 });
			}

		}
	}

	void update(ee::ecs::World& _world, float _dt) {

		auto& playerT = _world.getComponent<TransformComponent>(m_playerID);
		auto& playerC = _world.getComponent<ColliderComponent>(m_playerID);

		for (auto& id : m_entities) {
			auto& monsterT = _world.getComponent<TransformComponent>(id);
			auto& monsterC = _world.getComponent<ColliderComponent>(id);
			auto& monsterPF = _world.getComponent<PathFindingComponent>(id);
			auto& monsterM = _world.getComponent<MotionComponent>(id);

			monsterPF.timer -= _dt;

			if (monsterPF.path.empty() == false) {
				if (ee::math::Vector2<int>((monsterT.position + monsterC.size / 2 + monsterC.offset) / 32.f).Distance(monsterPF.path.front()) < 0.2) { //##TODO changeValue for somthing better
					monsterPF.path.pop();
					if (monsterPF.path.empty()) continue;
					ee::math::Vector2<float> obj = ee::math::Vector2<float>(monsterPF.path.front());
					obj = obj * 32.f; // on passe en position world
					obj += {16.f, 16.f}; // mettre au centre de la case
					ee::math::Vector2<float> position = (monsterT.position + monsterC.size / 2 + monsterC.offset);

					ee::math::Vector2<float> dir = obj - position;
					monsterM.velocity = dir.Normalize();
				}
			}
			else {
				monsterM.velocity = { 0, 0 };
			}
			if (playerT.position.Distance(monsterT.position) > monsterPF.range) continue;
			if (monsterPF.timer > 0) continue;

			//pathFinding
			ee::math::Vector2<int> playerIndex = ee::math::Vector2<int>((playerT.position + playerC.size / 2 + playerC.offset) / 32.f);
			ee::math::Vector2<int> monsterIndex = ee::math::Vector2<int>((monsterT.position + monsterC.size / 2 + monsterC.offset) / 32.f);

			std::queue<ee::math::Vector2<int>> path = getPath(playerIndex, monsterIndex);
			monsterPF.path = path;
			monsterPF.timer = .0f;


		}

	}

private:

	std::queue<ee::math::Vector2<int>> getPath(ee::math::Vector2<int> _player, ee::math::Vector2<int> _monster) {

		auto cmp = [](const Node& a, const Node& b) { return (a.g + a.h) > (b.g + b.h); };
		std::priority_queue<Node, std::vector<Node>, decltype(cmp)> openList(cmp);
		std::vector<Node> closedList;

		auto inClosed = [&](ee::math::Vector2<int> _pos) {
			for (auto& n : closedList)
				if (n.pos == _pos) return true;
			return false;
		};

		auto heuristic = [&](ee::math::Vector2<int> _pos) {
			return std::max(0, std::abs(_pos.x - _player.x) - 1)
				 + std::max(0, std::abs(_pos.y - _player.y) - 1);
		};

		auto isGoal = [&](ee::math::Vector2<int> _pos) {
			return std::abs(_pos.x - _player.x) <= 1 && std::abs(_pos.y - _player.y) <= 1;
		};

		openList.push({ _monster, 0, heuristic(_monster), _monster });

		while (!openList.empty()) {

			Node current = openList.top();
			openList.pop();

			if (inClosed(current.pos)) continue;
			closedList.push_back(current);

			if (isGoal(current.pos)) {
				// reconstruction du chemin en remontant les parents
				std::vector<ee::math::Vector2<int>> pathVec;
				ee::math::Vector2<int> pos = current.pos;

				while (pos != _monster) {
					pathVec.push_back(pos);
					for (auto& n : closedList) {
						if (n.pos == pos) { pos = n.parent; break; }
					}
				}
				pathVec.push_back(_monster);

				std::queue<ee::math::Vector2<int>> result;
				for (int i = (int)pathVec.size() - 1; i >= 0; i--)
					result.push(pathVec[i]);
				return result;
			}

			const ee::math::Vector2<int> dirs[4] = { {0,-1}, {0,1}, {-1,0}, {1,0} };
			for (auto& d : dirs) {
				ee::math::Vector2<int> next = { current.pos.x + d.x, current.pos.y + d.y };
				if (!isInBoundsAndWalkable(next) || inClosed(next)) continue;

				int g_new = current.g + 1;
				int h_new = heuristic(next);
				openList.push({ next, g_new, h_new, current.pos });
			}
		}

		return std::queue<ee::math::Vector2<int>>();
	}

	bool isInBoundsAndWalkable(ee::math::Vector2<int> _node) {
		return _node.x >= 0 && _node.x < (int)m_map.size()
			&& _node.y >= 0 && _node.y < (int)m_map[_node.x].size()
			&& m_map[_node.x][_node.y];
	}
};