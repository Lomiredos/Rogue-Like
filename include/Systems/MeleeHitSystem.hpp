#pragma once

#include "ecs/System.hpp"
#include "ecs/World.hpp"
#include "Components/Components.hpp"
#include "Components/WeaponsComponents.hpp"
#include "renderer/Renderer.hpp"
#include "renderer/Camera.hpp"
#include "Items/Weapons/WeaponFactory.hpp"
#include <vector>
#include <cmath>
#include <algorithm>
#include <cstdlib>

class MeleeHitSystem : public ee::ecs::System {

private:
	std::vector<ee::ecs::EntityID> m_enemies;
	std::unordered_map<ee::ecs::EntityID, std::pair<ee::ecs::EntityID, WeaponData>> m_enemyWeapons;
	ee::ecs::EntityID m_playerID = 0;
	float m_playerHitTimer = 0.f;
	ee::renderer::Renderer* m_renderer = nullptr;

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

	// 8 points
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
	void init(ee::ecs::EntityID _playerID, ee::renderer::Renderer& _renderer) {
		m_playerID = _playerID;
		m_renderer = &_renderer;
	}

	void addEnemy(ee::ecs::EntityID _id) { m_enemies.push_back(_id); }

	void addEnemyWeapon(ee::ecs::EntityID _enemyId, ee::ecs::EntityID _weaponId, const WeaponData& _data) {
		m_enemyWeapons[_enemyId] = { _weaponId, _data };
	}


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

			
			ee::math::Vector2<float> pivotWorld = {
				weaponT.position.x + weaponS.center.x,
				weaponT.position.y + weaponS.center.y
			};

			
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
		std::vector<ee::ecs::EntityID> toDestroy;
		auto& playerInfo = _world.getComponent<PlayerInfo>(m_playerID);

		// expire invincibilité par ennemi (quand le joueur les touche)
		for (auto it = playerInfo.hitList.begin(); it != playerInfo.hitList.end();)
		{
			it->second -= _dt;
			if (it->second <= 0.f)
				it = playerInfo.hitList.erase(it);
			else
				++it;
		}

		// expire invincibilité universelle du joueur (quand un mob le touche)
		if (m_playerHitTimer > 0.f) m_playerHitTimer -= _dt;

		for (auto& weaponId : m_entities)
		{
			auto& melee    = _world.getComponent<MeleeComponent>(weaponId);
			auto& equipped = _world.getComponent<ItemEquippedComponent>(weaponId);
			auto& weaponT  = _world.getComponent<TransformComponent>(weaponId);
			auto& weaponS  = _world.getComponent<SpriteComponent>(weaponId);

			float weaponAngle = equipped.rotation;
			float coneHalf    = equipped.coneSize;

			ee::math::Vector2<float> pivot = {
				weaponT.position.x + weaponS.center.x,
				weaponT.position.y + weaponS.center.y
			};

			if (equipped.ownerID == m_playerID)
			{
				// arme du joueur → touche les ennemis
				for (auto& enemyId : m_enemies)
				{
					if (!_world.hasComponent<TransformComponent>(enemyId)) continue;
					if (!_world.hasComponent<HealthComponent>(enemyId))    continue;
					if (playerInfo.hitList.count(enemyId))                 continue;

					auto points = getTestPoints(_world, enemyId);
					bool hit = false;
					for (auto& pt : points)
					{
						ee::math::Vector2<float> dir = pt - pivot;
						if (dir.Magnetude() > melee.range) continue;
						if (angleDiff(weaponAngle, dir.Angle()) <= coneHalf) { hit = true; break; }
					}
					if (!hit) continue;

					playerInfo.hitList[enemyId] = equipped.hitCooldown;
					auto& hp = _world.getComponent<HealthComponent>(enemyId);
					hp.current = std::max(0, hp.current - (int)melee.damage);
					if (hp.current <= 0) toDestroy.push_back(enemyId);
				}
			}
			else
			{
				// arme ennemie → touche le joueur uniquement (jamais son propre owner)
				if (m_playerHitTimer > 0.f) continue;
				if (!_world.hasComponent<HealthComponent>(m_playerID)) continue;

				auto points = getTestPoints(_world, m_playerID);
				bool hit = false;
				for (auto& pt : points)
				{
					ee::math::Vector2<float> dir = pt - pivot;
					if (dir.Magnetude() > melee.range) continue;
					if (angleDiff(weaponAngle, dir.Angle()) <= coneHalf) { hit = true; break; }
				}
				if (!hit) continue;

				m_playerHitTimer = equipped.hitCooldown;
				auto& hp = _world.getComponent<HealthComponent>(m_playerID);
				hp.current = std::max(0, hp.current - (int)melee.damage);
			}
		}

		// traitement des morts après la boucle pour ne pas invalider les itérateurs
		for (auto& deadId : toDestroy)
		{
			if (!_world.hasComponent<TransformComponent>(deadId)) continue;
			ee::math::Vector2<float> pos = _world.getComponent<TransformComponent>(deadId).position;

			if (m_enemyWeapons.count(deadId))
			{
				auto& [weaponId, weaponData] = m_enemyWeapons[deadId];
				_world.destroyEntity(weaponId);

				if (rand() % 2 == 0 && m_renderer)
					WeaponFactory::spawnWeaponEntity(_world, *m_renderer, weaponData, pos);

				m_enemyWeapons.erase(deadId);
			}

			_world.destroyEntity(deadId);
			m_enemies.erase(std::remove(m_enemies.begin(), m_enemies.end(), deadId), m_enemies.end());
		}
	}
};
