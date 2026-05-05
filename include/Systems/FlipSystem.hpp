#pragma once

#include "ecs/System.hpp"
#include "ecs/World.hpp"
#include "Components/Components.hpp"

class FlipSystem : public ee::ecs::System {
public:

	void update(ee::ecs::World& _world, float _dt) {
			
		for (auto& id : m_entities) {
			auto& sprite = _world.getComponent<SpriteComponent>(id);
			auto& motion = _world.getComponent<MotionComponent>(id);

			if (motion.velocity.x < -0.1f) sprite.flip = ee::renderer::FlipMode((int)(sprite.flip) | 1);
			if (motion.velocity.x > 0.1f) sprite.flip = ee::renderer::FlipMode((int)(sprite.flip) & ~1);
		}
	}
};