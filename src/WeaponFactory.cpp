#include "Items/Weapons/WeaponFactory.hpp"
#include "Components/Components.hpp"
#include "Components/WeaponsComponents.hpp"

WeaponData WeaponFactory::makeEpee(float _damage, float _range)
{
	WeaponData data;
	data.name = "Epee";
	data.spriteName = "Tileset";
	data.srcRect = { 322, 65, 12, 30 };
	data.equippedDrawSize = { data.srcRect.getSize().x * 2.f, data.srcRect.getSize().y * 2.f };
	data.rotationCenter = { data.equippedDrawSize.x / 2.f, data.equippedDrawSize.y };
	data.baseRotation = 90.f;
	data.breathAmplitude = 3.f;
	data.breathSpeed = 2.f;
	data.equippedOffset = { 22, -7 };
	data.equippedOffsetFlipped = { -3, -7 };
	data.tags = { WeaponData::Tag::Melee };
	data.damage = _damage;
	data.range = _range;
	return data;
}

WeaponData WeaponFactory::makeArc(float _damage, float _speed, float _maxChargeTime)
{
	WeaponData data;
	data.name = "Arc";
	data.spriteName = "Tileset";
	data.tags = { WeaponData::Tag::Projectile, WeaponData::Tag::Charge };
	data.damage = _damage;
	data.projectileSpeed = _speed;
	data.maxChargeTime = _maxChargeTime;
	return data;
}

WeaponData WeaponFactory::makeStaffOffensif(float _damage, float _speed, int _manaCost)
{
	WeaponData data;
	data.name = "Staff Offensif";
	data.spriteName = "Tileset";
	data.tags = { WeaponData::Tag::Projectile, WeaponData::Tag::Mana };
	data.damage = _damage;
	data.projectileSpeed = _speed;
	data.manaCost = _manaCost;
	return data;
}

WeaponData WeaponFactory::makeStaffSoin(int _healAmount, int _manaCost)
{
	WeaponData data;
	data.name = "Staff de Soin";
	data.spriteName = "Tileset";
	data.tags = { WeaponData::Tag::Mana, WeaponData::Tag::Heal };
	data.healAmount = _healAmount;
	data.manaCost = _manaCost;
	return data;
}

ee::ecs::EntityID WeaponFactory::spawnWeaponEntity(ee::ecs::World& _world, ee::renderer::Renderer& _renderer, const WeaponData& _data, ee::math::Vector2<float> _position)
{
	ee::ecs::EntityID id = _world.createEntity();
	_world.addComponent(id, TransformComponent{ _position });

	SpriteComponent sprite{ _renderer.getTexture(_data.spriteName), _data.srcRect };
	sprite.offset = { -_data.srcRect.getSize().x / 2.f, -_data.srcRect.getSize().y / 2.f }; // centré sur la position au sol
	_world.addComponent(id, sprite);

	_world.addComponent(id, ItemPickUpComponent{ _data });
	return id;
}

ee::ecs::EntityID WeaponFactory::equipWeapon(ee::ecs::World& _world, ee::renderer::Renderer& _renderer, const WeaponData& _data, ee::ecs::EntityID _ownerID, ee::math::Vector2<float> _playerPos)
{
	ee::ecs::EntityID id = _world.createEntity();
	_world.addComponent(id, TransformComponent{ _playerPos });

	SpriteComponent sprite{ _renderer.getTexture(_data.spriteName), _data.srcRect };
	sprite.center = _data.rotationCenter; // point de pivot pour la rotation (ex: poignée de l'épée)
	if (_data.equippedDrawSize.x > 0.f) {
		sprite.hasDrawSize = true;
		sprite.drawSize = _data.equippedDrawSize;
	}
	_world.addComponent(id, sprite);

	ItemEquippedComponent equipped;
	equipped.ownerID = _ownerID;
	equipped.baseRotation = _data.baseRotation;
	equipped.drawOffset = _data.equippedOffset;
	equipped.drawOffsetFlipped = _data.equippedOffsetFlipped;
	equipped.breathAmplitude = _data.breathAmplitude;
	equipped.breathSpeed = _data.breathSpeed;
	equipped.hitCooldown = _data.hitCooldown;
	equipped.coneSize = _data.coneSize;
	_world.addComponent(id, equipped);

	for (auto tag : _data.tags)
	{
		switch (tag)
		{
		case WeaponData::Tag::Melee:
			_world.addComponent(id, MeleeComponent{ _data.damage, _data.range });
			break;
		case WeaponData::Tag::Projectile:
			_world.addComponent(id, ProjectileComponent{ _data.damage, _data.projectileSpeed });
			break;
		case WeaponData::Tag::Charge:
			_world.addComponent(id, ChargeComponent{ _data.maxChargeTime, 0.f });
			break;
		case WeaponData::Tag::Mana:
			_world.addComponent(id, ManaComponent{ _data.manaCost });
			break;
		case WeaponData::Tag::Heal:
			_world.addComponent(id, HealComponent{ _data.healAmount });
			break;
		}
	}
	return id;
}
