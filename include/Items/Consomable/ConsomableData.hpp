#pragma once
#include <vector>
#include <string>
#include "math/Rect.hpp"

struct ConsomableData {
	enum class Tag {};

	std::string name;
	std::string spriteName;
	ee::math::Rect<float> srcRect = { 0, 0, 16, 16 };

	std::vector<Tag> tags;
};