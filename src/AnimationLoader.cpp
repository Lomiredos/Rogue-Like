#include "AnimationLoader.hpp"
#include <nlohmann/json.hpp>
#include <fstream>

using json = nlohmann::json;

AnimationComponent AnimationLoader::loadFromJson(const std::string& _path)
{
    std::ifstream file(_path);
    if (!file.is_open())
        throw std::runtime_error("AnimationLoader: cannot open " + _path);

    json j = json::parse(file);

    AnimationComponent comp;
    comp.animationSet.frameWidth  = j["frame_size"]["width"];
    comp.animationSet.frameHeight = j["frame_size"]["height"];

    for (auto& [name, anim] : j["animations"].items())
    {
        AnimationClip clip;
        clip.loop      = anim["loop"].get<bool>();
        clip.frameTime = anim["frame_time"].get<float>();
        for (auto& frame : anim["frames"])
            clip.frames.push_back(frame.get<int>());
        comp.animationSet.clips[name] = clip;

        if (comp.currentAnimation.empty())
            comp.currentAnimation = name;
    }

    return comp;
}
