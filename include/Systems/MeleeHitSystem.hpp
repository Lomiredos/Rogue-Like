#pragma once

#include "ecs/System.hpp"
#include "ecs/World.hpp"
#include "Components/Components.hpp"
#include "Components/WeaponsComponents.hpp"
#include "renderer/Renderer.hpp"
#include "renderer/Camera.hpp"
#include <vector>
#include <cmath>
#include <iostream>

class MeleeHitSystem : public ee::ecs::System {

private:
	std::vector<ee::ecs::EntityID> m_enemies;
	ee::ecs::EntityID m_playerID = 0;

	// retour sur [-180, 180] pour que la comparaison soit symétrique dans les deux sens
	float angleDiff(float _a, float _b)
	{
		float diff = std::fmod(_a - _b + 540.f, 360.f) - 180.f;
		return std::abs(diff);
	}

	// centre du collider si dispo, sinon position brute
	ee::math::Vector2<float> getCenter(ee::ecs::World& _world, ee::ecs::EntityID _id)
	{
		auto& t = _world.getComponent<TransformComponent>(_id);
		if (_world.hasComponent<ColliderComponent>(_id))
		{
			auto& col = _world.getComponent<ColliderComponent>(_id);
			return {
				t.position.x + col.offset.x + col.size.x / 2.f,
				t.position.y + col.offset.y + col.size.y / 2.f
			};
		}
		return t.position;
	}

	// 8 points (coins + milieux) plutôt que le centre seul → évite les faux-négatifs sur les grands ennemis
	std::vector<ee::math::Vector2<float>> getTestPoints(ee::ecs::World& _world, ee::ecs::EntityID _id)
	{
		auto& t = _world.getComponent<TransformComponent>(_id);
		if (!_world.hasComponent<ColliderComponent>(_id))
			return { t.position };

		auto& col = _world.getComponent<ColliderComponent>(_id);
		float x = t.position.x + col.offset.x;
		float y = t.position.y + col.offset.y;
		float w = col.size.x;
		float h = col.size.y;
		float hw = w / 2.f;
		float hh = h / 2.f;

		return {
			{ x,      y      },
			{ x + hw, y      },
			{ x + w,  y      },
			{ x + w,  y + hh },
			{ x + w,  y + h  },
			{ x + hw, y + h  },
			{ x,      y + h  },
			{ x,      y + hh },
		};
	}

public:
	void init(ee::ecs::EntityID _playerID) { m_playerID = _playerID; }

	void addEnemy(ee::ecs::EntityID _id) { m_enemies.push_back(_id); }

	// visualise range (cercle) et cone (deux lignes) pour calibrer les armes
	void debugDraw(ee::ecs::World& _world, ee::renderer::Renderer& _renderer, const ee::renderer::Camera& _camera)
	{
		constexpr float DEG2RAD = 3.141592f / 180.f;

		for (auto& weaponId : m_entities)
		{
			auto& melee    = _world.getComponent<MeleeComponent>(weaponId);
			auto& equipped = _world.getComponent<ItemEquippedComponent>(weaponId);
			auto& weaponT  = _world.getComponent<TransformComponent>(weaponId);
			auto& weaponS  = _world.getComponent<SpriteComponent>(weaponId);

			float weaponAngle = equipped.rotation;
			float coneHalf = equipped.coneSize;

			// pivot en world space (sprite.center = point de rotation défini dans WeaponFactory)
			ee::math::Vector2<float> pivotWorld = {
				weaponT.position.x + weaponS.center.x,
				weaponT.position.y + weaponS.center.y
			};

			// converti en screen space
			ee::math::Vector2<float> origin = {
				pivotWorld.x - _camera.getX(),
				pivotWorld.y - _camera.getY()
			};

			_renderer.DrawCircle(origin, melee.range, { 255, 80, 80, 180 });

			auto lineEnd = [&](float _angleDeg) {
				return ee::math::Vector2<float>{
					origin.x + std::cos(_angleDeg * DEG2RAD) * melee.range,
					origin.y + std::sin(_angleDeg * DEG2RAD) * melee.range
				};
			};

			_renderer.DrawLine(origin, lineEnd(weaponAngle - coneHalf), { 255, 200, 0, 200 });
			_renderer.DrawLine(origin, lineEnd(weaponAngle + coneHalf), { 255, 200, 0, 200 });
		}
	}


	void update(ee::ecs::World& _world, float _dt)
	{
		auto& playerInfo = _world.getComponent<PlayerInfo>(m_playerID);

		// expire les cooldowns d'invincibilité par ennemi (évite le damage spam)
		for (auto it = playerInfo.hitList.begin(); it != playerInfo.hitList.end();)
		{
			it->second -= _dt;
			if (it->second <= 0.f)
				it = playerInfo.hitList.erase(it);
			else
				++it;
		}

		for (auto& weaponId : m_entities)
		{
			auto& melee    = _world.getComponent<MeleeComponent>(weaponId);
			auto& equipped = _world.getComponent<ItemEquippedComponent>(weaponId);
			auto& weaponT  = _world.getComponent<TransformComponent>(weaponId);
			auto& weaponS  = _world.getComponent<SpriteComponent>(weaponId);

			float weaponAngle = equipped.rotation;
			float coneHalf = equipped.coneSize;

			// pivot = point de rotation de l'arme (sprite.center, défini dans WeaponFactory)
			ee::math::Vector2<float> pivot = {
				weaponT.position.x + weaponS.center.x,
				weaponT.position.y + weaponS.center.y
			};

			for (auto& enemyId : m_enemies)
			{
				if (!_world.hasComponent<TransformComponent>(enemyId)) continue;
				if (!_world.hasComponent<HealthComponent>(enemyId))    continue;
				if (playerInfo.hitList.count(enemyId))                 continue; // encore en invincibilité

				// on teste les 8 points du collider pour pas rater un grand ennemi
				auto points = getTestPoints(_world, enemyId);
				bool hit = false;
				for (auto& pt : points)
				{
					ee::math::Vector2<float> dir = pt - pivot;
					if (dir.Magnetude() > melee.range) continue;
					if (angleDiff(weaponAngle, dir.Angle()) <= coneHalf) { hit = true; break; }
				}
				if (!hit) continue;

				// démarre le cooldown d'invincibilité pour cet ennemi
				playerInfo.hitList[enemyId] = equipped.hitCooldown;
				std::cout << "damaged" << std::endl;
			}
		}
	}
};
