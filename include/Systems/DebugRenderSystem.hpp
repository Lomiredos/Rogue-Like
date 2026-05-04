#pragma once

#include "ecs/System.hpp"
#include "ecs/World.hpp"
#include "Components/Components.hpp"
#include "renderer/Renderer.hpp"
#include "renderer/Camera.hpp"

class DebugRenderSystem : public ee::ecs::System {


public:

	void render(ee::ecs::World& _world, ee::renderer::Renderer& _renderer, ee::renderer::Camera _cam) {

		for (auto& id : m_entities) {

			auto& transform = _world.getComponent<TransformComponent>(id);
			auto& col = _world.getComponent<ColliderComponent>(id);

			ee::math::Vector2<float> screenPos = {
			_cam.getScreenX(transform.position.x + col.offset.x),
			_cam.getScreenY(transform.position.y + col.offset.y)
			};
			_renderer.DrawRect({ screenPos, col.size }, { 255, 0, 0, 255 });
		}
	}
};