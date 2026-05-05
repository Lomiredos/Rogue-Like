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

    // Pour les entités animées : regroupe les frames par clip et construit l'AnimationSet
    // Convention : {entity}_{clip}_anim_f{N} ou {entity}_{clip}_f{N}
    AnimationSet loadAnimationSet(const std::string& _entityName) const;

    // Pour les sprites statiques (items, weapons...)
    std::optional<ee::math::Rect<float>> getSpriteRect(const std::string& _name) const;
};
