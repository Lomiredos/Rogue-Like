#pragma once
#include "math/Vector2.hpp"
#include "Items/Weapons/WeaponData.hpp"
#include "Items/Passive/PassiveData.hpp"
#include "Items/Consomable/ConsomableData.hpp"
#include "ecs/EntityManager.hpp"
#include <variant>

struct ItemPickUpComponent {
	std::variant<WeaponData, ConsomableData, PassiveData> data;
};

struct ItemEquippedComponent {
	ee::ecs::EntityID ownerID;
	float rotation = 0.f;
	float baseRotation = 0.f;
	ee::math::Vector2<float> drawOffset = { 0, 0 };
	ee::math::Vector2<float> drawOffsetFlipped = { 0, 0 };
	float breathAmplitude = 0.f;
	float breathSpeed = 1.f;
	float breathTimer = 0.f;
	float hitCooldown = 0.5f;
	float coneSize = 36.f;
};

struct MeleeComponent {
	float damage;
	float range;
};

struct ProjectileComponent {
	float damage;
	float speed;
};

struct ChargeComponent {
	float maxChargeTime;
	float currentTime;
};

struct ManaComponent {
	int manaCost;
};

struct HealComponent {
	int healAmount;
};