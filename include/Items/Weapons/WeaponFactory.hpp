#pragma once
#include "Items/Weapons/WeaponData.hpp"
#include "ecs/World.hpp"
#include "renderer/Renderer.hpp"
#include "math/Vector2.hpp"

namespace WeaponFactory
{
	WeaponData makeEpee(float _damage, float _range);
	WeaponData makeArc(float _damage, float _speed, float _maxChargeTime);
	WeaponData makeStaffOffensif(float _damage, float _speed, int _manaCost);
	WeaponData makeStaffSoin(int _healAmount, int _manaCost);

	ee::ecs::EntityID spawnWeaponEntity(ee::ecs::World& _world, ee::renderer::Renderer& _renderer, const WeaponData& _data, ee::math::Vector2<float> _position);
	ee::ecs::EntityID equipWeapon(ee::ecs::World& _world, ee::renderer::Renderer& _renderer, const WeaponData& _data, ee::ecs::EntityID _ownerID, ee::math::Vector2<float> _playerPos);
}
