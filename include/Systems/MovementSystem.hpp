#pragma once

#include "ecs/System.hpp"
#include "ecs/World.hpp"
#include "Components/Components.hpp"
#include "Utils.hpp"

class MovementSystem : public ee::ecs::System {

public:

	void update(ee::ecs::World& _world, float _dt) {

		for (auto& id : m_entities) {

			auto& transform = _world.getComponent<TransformComponent>(id);
			auto& motion = _world.getComponent<MotionComponent>(id);

			ee::math::Vector2<float> translation = motion.velocity * (motion.speed * motion.speedMultiplier) * _dt;

			transform.position += translation;

		}
	}
};