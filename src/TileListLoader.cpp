#include "TileListLoader.hpp"
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <algorithm>

void TileListLoader::load(const std::string& _path)
{
    std::ifstream file(_path);
    if (!file.is_open())
        throw std::runtime_error("TileListLoader: cannot open " + _path);

    std::string line;
    while (std::getline(file, line))
    {
        if (line.empty()) continue;

        std::istringstream ss(line);
        TileEntry entry;
        ss >> entry.name >> entry.x >> entry.y >> entry.w >> entry.h;
        if (entry.name.empty()) continue;

        m_tiles[entry.name] = entry;
    }
}


static bool parseFrameName(const std::string& _name, std::string& _entity, std::string& _clip, int& _frameIndex)
{
    // cherche "_f{N}" a la fin
    size_t fPos = _name.rfind("_f");
    if (fPos == std::string::npos) return false;

    std::string frameStr = _name.substr(fPos + 2);
    for (char c : frameStr)
        if (!std::isdigit(c)) return false;

    _frameIndex = std::stoi(frameStr);
    std::string prefix = _name.substr(0, fPos); // "knight_m_idle_anim" ou "goblin_run_anim"

    // retire "_anim" si present
    const std::string animSuffix = "_anim";
    if (prefix.size() > animSuffix.size() &&
        prefix.substr(prefix.size() - animSuffix.size()) == animSuffix)
        prefix = prefix.substr(0, prefix.size() - animSuffix.size());

    // le clip est le dernier segment, le reste est l'entity
    size_t lastUnderscore = prefix.rfind('_');
    if (lastUnderscore == std::string::npos) return false;

    _entity = prefix.substr(0, lastUnderscore);
    _clip   = prefix.substr(lastUnderscore + 1);
    return true;
}

AnimationSet TileListLoader::loadAnimationSet(const std::string& _entityName) const
{
    // collecte les frames groupées par clip : clip -> {frameIndex, TileEntry}
    std::unordered_map<std::string, std::vector<std::pair<int, const TileEntry*>>> clips;

    for (auto& [name, entry] : m_tiles)
    {
        std::string entity, clip;
        int frameIndex;
        if (!parseFrameName(name, entity, clip, frameIndex)) continue;
        if (entity != _entityName) continue;

        clips[clip].push_back({ frameIndex, &entry });
    }

    AnimationSet animSet;

    for (auto& [clipName, frames] : clips)
    {
        std::sort(frames.begin(), frames.end(),
            [](const auto& a, const auto& b) { return a.first < b.first; });

        AnimationClip ac;
        ac.frameTime = 0.12f;
        ac.loop = true;

        for (auto& [idx, entry] : frames)
            ac.frames.push_back({ (float)entry->x, (float)entry->y, (float)entry->w, (float)entry->h });

        animSet.clips[clipName] = ac;
    }

    return animSet;
}

std::optional<ee::math::Rect<float>> TileListLoader::getSpriteRect(const std::string& _name) const
{
    auto it = m_tiles.find(_name);
    if (it == m_tiles.end()) return std::nullopt;

    const TileEntry& e = it->second;
    return ee::math::Rect<float>{ (float)e.x, (float)e.y, (float)e.w, (float)e.h };
}
