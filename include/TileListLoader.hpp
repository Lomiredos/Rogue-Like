#pragma once

#include "Components/Components.hpp"
#include "math/Rect.hpp"
#include <string>
#include <unordered_map>
#include <optional>

struct TileEntry {
    std::string name;
    int x, y, w, h;
};

class TileListLoader {
    std::unordered_map<std::string, TileEntry> m_tiles;

public:
    void load(const std::string& _path);

    //anime
    AnimationSet loadAnimationSet(const std::string& _entityName) const;

    //static
    std::optional<ee::math::Rect<float>> getSpriteRect(const std::string& _name) const;
};
