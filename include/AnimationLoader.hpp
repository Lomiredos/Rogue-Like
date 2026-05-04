#pragma once
#include "Components/Components.hpp"
#include <string>

class AnimationLoader {
public:
    static AnimationComponent loadFromJson(const std::string& _path);
};
