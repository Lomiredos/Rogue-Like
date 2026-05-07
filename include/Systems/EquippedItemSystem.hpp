#pragma once

#include "ecs/System.hpp"
#include "ecs/World.hpp"
#include "Components/Components.hpp"
#include "Components/WeaponsComponents.hpp"
#include "input/InputManager.hpp"
#include "math/Vector2.hpp"

#include <cmath>

class EquippedWeaponSystem : public ee::ecs::System {

private:
	ee::ecs::EntityID m_playerID;

public:
	void init(ee::ecs::EntityID _playerID) { m_playerID = _playerID; }

	void update(ee::ecs::World& _world, float _dt) {

		auto& playerTransform = _world.getComponent<TransformComponent>(m_playerID);

		for (auto& id : m_entities) {
			auto& itemEquip = _world.getComponent<ItemEquippedComponent>(id);
			auto& itemTransform = _world.getComponent<TransformComponent>(id);
			auto& sprite = _world.getComponent<SpriteComponent>(id);

			if (itemEquip.ownerID == m_playerID) {
				// stick droit → direction de l'arme
				ee::math::Vector2<float> joy;
				joy.x = ee::input::InputManager::getInstance().getAxisForce(ee::input::GamepadAxis::RightX);
				joy.y = ee::input::InputManager::getInstance().getAxisForce(ee::input::GamepadAxis::RightY);
				if (joy.Magnetude() > 0.2f)  // deadzone pour éviter le drift
					itemEquip.rotation = joy.Angle();
				else
					itemEquip.rotation = 0 - itemEquip.baseRotation; // retour au repos

				// flip le joueur selon la direction de l'arme
				// overFlip = joystick actif → on met à jour, sinon on garde la dernière direction
				bool overFlip = itemEquip.rotation != -itemEquip.baseRotation;
				bool isFlipped = itemEquip.rotation < -itemEquip.baseRotation || itemEquip.rotation > itemEquip.baseRotation;
				auto& ownerSprite = _world.getComponent<SpriteComponent>(itemEquip.ownerID);
				if (overFlip) {
					if (isFlipped)
						ownerSprite.flip = ee::renderer::FlipMode((int)(ownerSprite.flip) | 1);
					else
						ownerSprite.flip = ee::renderer::FlipMode((int)(ownerSprite.flip) & ~1);
				}

				// si le stick est relâché, on garde le flip précédent du owner
				bool finalFlip = isFlipped || ownerSprite.flip == ee::renderer::FlipMode::Horizontale;

				// oscillation verticale de l'arme ("respiration")
				itemEquip.breathTimer += _dt * itemEquip.breathSpeed;
				float breathOffset = std::sin(itemEquip.breathTimer) * itemEquip.breathAmplitude;

				ee::math::Vector2<float> offset = finalFlip ? itemEquip.drawOffsetFlipped : itemEquip.drawOffset;
				offset.y += breathOffset;
				itemTransform.position = playerTransform.position + offset;
				sprite.flip = finalFlip ? ee::renderer::FlipMode::Horizontale : ee::renderer::FlipMode::None;

				if (_world.hasComponent<MotionComponent>(itemEquip.ownerID)) {
					auto& motion = _world.getComponent<MotionComponent>(itemEquip.ownerID);
					// marche arrière = se déplacer dans le sens opposé au flip
					bool backpedaling = (finalFlip && motion.velocity.x > 0.1f) || (!finalFlip && motion.velocity.x < -0.1f);
					if (backpedaling) {
						float sum = std::abs(motion.velocity.x) + std::abs(motion.velocity.y);
						// plus on marche en crabe, plus la pénalité est forte (max -50% si full horizontal)
						float xFactor = (sum > 0.01f) ? std::abs(motion.velocity.x) / sum : 0.f;
						motion.speedMultiplier = 1.f - xFactor * 0.5f;
					}
					else
						motion.speedMultiplier = 1.f;
				}

			}
			else {
				// ennemi : l'arme pointe toujours vers le joueur
				ee::math::Vector2<float> dir = playerTransform.position - itemTransform.position;
				itemEquip.rotation = dir.Angle();

				// copie le flip du owner pour que l'offset soit cohérent
				bool isFlipped = _world.hasComponent<SpriteComponent>(itemEquip.ownerID) &&
					_world.getComponent<SpriteComponent>(itemEquip.ownerID).flip == ee::renderer::FlipMode::Horizontale;
				itemTransform.position = _world.getComponent<TransformComponent>(itemEquip.ownerID).position +
					(isFlipped ? itemEquip.drawOffsetFlipped : itemEquip.drawOffset);
				sprite.flip = isFlipped ? ee::renderer::FlipMode::Horizontale : ee::renderer::FlipMode::None;
			}

			// baseRotation = rotation au repos de l'arme (ex: 90° pour que l'épée pointe vers la droite)
			sprite.angle = itemEquip.rotation + itemEquip.baseRotation;
		}
	}
};