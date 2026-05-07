#pragma once
#include <vector>
#include <string>
#include "math/Rect.hpp"

struct WeaponData {
	enum class Tag { Melee, Projectile, Charge, Mana, Heal };

	std::string name;
	std::string spriteName;
	ee::math::Rect<float> srcRect = { 0, 0, 16, 16 };
	ee::math::Vector2<float> rotationCenter = { 0, 0 };
	ee::math::Vector2<float> equippedOffset = { 0, 0 };
	ee::math::Vector2<float> equippedOffsetFlipped = { 0, 0 };
	ee::math::Vector2<float> equippedDrawSize = { 0, 0 };
	float baseRotation = 0.f;
	float breathAmplitude = 0.f;
	float breathSpeed = 1.f;
	std::vector<Tag> tags;

	float damage = 0.f;
	float range = 0.f;
	float hitCooldown = 0.5f;
	float coneSize = 36.f;
	float projectileSpeed = 0.f;
	float maxChargeTime = 0.f;
	int manaCost = 0;
	int healAmount = 0;
};
