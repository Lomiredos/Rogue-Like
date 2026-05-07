#pragma once

#include "ecs/System.hpp"
#include "ecs/World.hpp"
#include "Components/Components.hpp"
#include "Components/WeaponsComponents.hpp"
#include "Inventory.hpp"


class ItemPickUpSystem : public ee::ecs::System {
private:
	ee::ecs::EntityID m_playerID;
	Inventory* m_inventory;

public:

	void init(ee::ecs::EntityID _playerID, Inventory& _inv) { m_playerID = _playerID; m_inventory = &_inv; }

	void update(ee::ecs::World& _world, float _dt) {


		auto& playerTransform = _world.getComponent<TransformComponent>(m_playerID);
		auto& playerInfos = _world.getComponent<PlayerInfo>(m_playerID);

		std::vector<ee::ecs::EntityID> entityToDestroy;

		for (auto& id : m_entities) {

			auto& itemTransfrom = _world.getComponent<TransformComponent>(id);

			// position brute, ok car les items sont centrés sur leur position (pas de collider)
			float dist = playerTransform.position.Distance(itemTransfrom.position);

			if (dist > playerInfos.pickupRange) continue;


			m_inventory->addItem(_world.getComponent<ItemPickUpComponent>(id).data);
			entityToDestroy.push_back(id);
		}

		for (auto& id : entityToDestroy)
			_world.destroyEntity(id);
	}


};