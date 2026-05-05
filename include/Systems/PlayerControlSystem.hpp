#pragma once

#include "ecs/System.hpp"
#include "ecs/World.hpp"
#include "Components/Components.hpp"
#include "input/ActionMap.hpp"
#include "input/InputManager.hpp"

class PlayerControlSystem : public ee::ecs::System {
private:
	enum class PlayerAction { MoveUp, MoveDown, MoveLeft, MoveRight };
	ee::input::ActionMap<PlayerAction> m_actionMap;

public:


	void init() {

		m_actionMap.bind(PlayerAction::MoveUp, { {ee::input::Key::W, ee::input::TriggerState::Held} });
		m_actionMap.bind(PlayerAction::MoveDown, { {ee::input::Key::S, ee::input::TriggerState::Held} });
		m_actionMap.bind(PlayerAction::MoveLeft, { {ee::input::Key::A, ee::input::TriggerState::Held} });
		m_actionMap.bind(PlayerAction::MoveRight, { {ee::input::Key::D, ee::input::TriggerState::Held} });

		m_actionMap.bind(PlayerAction::MoveUp, { {ee::input::Key::Up, ee::input::TriggerState::Held} });
		m_actionMap.bind(PlayerAction::MoveDown, { {ee::input::Key::Down, ee::input::TriggerState::Held} });
		m_actionMap.bind(PlayerAction::MoveLeft, { {ee::input::Key::Left, ee::input::TriggerState::Held} });
		m_actionMap.bind(PlayerAction::MoveRight, { {ee::input::Key::Right, ee::input::TriggerState::Held} });

	}
	void update(ee::ecs::World& _world, float _dt) {
		for (auto& id : m_entities) {

			auto& motion = _world.getComponent<MotionComponent>(id);

			motion.velocity = {};

			if (m_actionMap.isActive(PlayerAction::MoveUp)) motion.velocity.y = -1;
			if (m_actionMap.isActive(PlayerAction::MoveDown)) motion.velocity.y = 1;
			if (m_actionMap.isActive(PlayerAction::MoveRight)) motion.velocity.x = 1;
			if (m_actionMap.isActive(PlayerAction::MoveLeft)) motion.velocity.x = -1;

			if (ee::input::InputManager::getInstance().getAxisForce(ee::input::GamepadAxis::LeftX) > 0.1f) motion.velocity.x = ee::input::InputManager::getInstance().getAxisForce(ee::input::GamepadAxis::LeftX);
			if (ee::input::InputManager::getInstance().getAxisForce(ee::input::GamepadAxis::LeftX) < -0.1f) motion.velocity.x = ee::input::InputManager::getInstance().getAxisForce(ee::input::GamepadAxis::LeftX);
			if (ee::input::InputManager::getInstance().getAxisForce(ee::input::GamepadAxis::LeftY) > 0.1f) motion.velocity.y = ee::input::InputManager::getInstance().getAxisForce(ee::input::GamepadAxis::LeftY);
			if (ee::input::InputManager::getInstance().getAxisForce(ee::input::GamepadAxis::LeftY) < -0.1f) motion.velocity.y = ee::input::InputManager::getInstance().getAxisForce(ee::input::GamepadAxis::LeftY);

			motion.velocity = motion.velocity.Normalize();

			if (_world.hasComponent<AnimationComponent>(id))
			{
				auto& anim = _world.getComponent<AnimationComponent>(id);
				if (motion.velocity.x != 0 || motion.velocity.y != 0)
					anim.play("run");
				else
					anim.play("idle");
			}
		}
	}
};