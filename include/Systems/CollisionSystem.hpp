#pragma once

#include "ecs/System.hpp"
#include "ecs/World.hpp"
#include "Components/Components.hpp"

class CollisionSystem : public ee::ecs::System {

	std::vector<std::vector<bool>> m_walkable;

	bool isWalkableAt(float _worldX, float _worldY) const
	{
		int tx = (int)(_worldX / 32.f);
		int ty = (int)(_worldY / 32.f);
		if (tx < 0 || ty < 0) return false;
		if (tx >= (int)m_walkable.size()) return false;
		if (ty >= (int)m_walkable[tx].size()) return false;
		return m_walkable[tx][ty];
	}

public:

	void init(const std::vector<std::vector<Cell>>& _mapDown)
	{
		m_walkable.assign(_mapDown.size(), std::vector<bool>(_mapDown[0].size(), false));
		for (int i = 0; i < (int)_mapDown.size(); i++)
			for (int j = 0; j < (int)_mapDown[i].size(); j++)
				m_walkable[i][j] = isWalkable(_mapDown[i][j].type);
	}

	void update(ee::ecs::World& _world, float _dt) override
	{
		for (auto& id : m_entities)
		{
			auto& transform = _world.getComponent<TransformComponent>(id);
			auto& motion    = _world.getComponent<MotionComponent>(id);
			auto& col       = _world.getComponent<ColliderComponent>(id);

			ee::math::Vector2<float> translation = motion.velocity * motion.speed * _dt;

			{
				ee::math::Vector2<float> nextPos = transform.position + ee::math::Vector2<float>{translation.x, 0} + col.offset;
				float left   = nextPos.x;
				float right  = nextPos.x + col.size.x;
				float top    = nextPos.y;
				float bottom = nextPos.y + col.size.y;

				if (motion.velocity.x < 0 && (!isWalkableAt(left, top) || !isWalkableAt(left, bottom)))
					motion.velocity.x = 0;
				if (motion.velocity.x > 0 && (!isWalkableAt(right, top) || !isWalkableAt(right, bottom)))
					motion.velocity.x = 0;
			}

			{
				ee::math::Vector2<float> nextPos = transform.position + ee::math::Vector2<float>{0, translation.y} + col.offset;
				float left   = nextPos.x;
				float right  = nextPos.x + col.size.x;
				float top    = nextPos.y;
				float bottom = nextPos.y + col.size.y;

				if (motion.velocity.y < 0 && (!isWalkableAt(left, top) || !isWalkableAt(right, top)))
					motion.velocity.y = 0;
				if (motion.velocity.y > 0 && (!isWalkableAt(left, bottom) || !isWalkableAt(right, bottom)))
					motion.velocity.y = 0;
			}
		}
	}
};
