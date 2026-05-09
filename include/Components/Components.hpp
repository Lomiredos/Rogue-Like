#pragma once
#include "math/Vector2.hpp"
#include "math/Rect.hpp"
#include "renderer/Texture.hpp"
#include "renderer/Renderer.hpp"
#include "ecs/EntityManager.hpp"
#include <unordered_map>
#include <string>
#include <vector>
#include <memory>
#include <queue>

struct AnimationClip {
    std::vector<ee::math::Rect<float>> frames;
    float frameTime = 4.f;
    bool loop = true;
};

struct AnimationSet {
    std::unordered_map<std::string, AnimationClip> clips;
};
struct ColliderComponent{
    ee::math::Vector2<float> size;
    ee::math::Vector2<float> offset;
};

struct TransformComponent {
    ee::math::Vector2<float> position;
};

struct MotionComponent {
    ee::math::Vector2<float> velocity;
    float speed;
    float speedMultiplier = 1.f;
};

struct SpriteComponent {
    std::shared_ptr<ee::renderer::Texture> texture;
    ee::math::Rect<float> srcRect;
    float angle = 0;
    ee::math::Vector2<float> center = { 0, 0 };
    ee::math::Vector2<float> offset = { 0, 0 };
    bool hasDrawSize = false;
    ee::math::Vector2<float> drawSize = { 0, 0 };
    ee::renderer::FlipMode flip = ee::renderer::FlipMode::None;
};

struct AnimationComponent {
    AnimationSet animationSet;
    std::string currentAnimation;
    int currentFrame = 0;
    float timer = 0.f;
    ee::math::Rect<float> srcRect;
    bool hasDrawSize = false;
    ee::math::Vector2<float> drawSize = {};
    ee::math::Vector2<float> offset = {};

    void play(const std::string& _name)
    {
        if (animationSet.clips.count(_name) == 0) return;
        if (currentAnimation == _name) return;
        currentAnimation = _name;
        currentFrame = 0;
        timer = 0.f;
    }

    bool isFinished() const
    {
        auto it = animationSet.clips.find(currentAnimation);
        if (it == animationSet.clips.end()) return true;
        if (it->second.loop) return false;
        return currentFrame >= (int)it->second.frames.size() - 1;
    }
};

struct HealthComponent {
    int max;
    int current;

    bool isDead() const { return current <= 0; }
    float ratio() const { return (float)current / max; }
};

struct PathFindingComponent {
    enum class Type {};
    float range;
    Type type;
    float timer;
    std::queue<ee::math::Vector2<int>> path;
};

struct PlayerInfo
{
    float pickupRange;
    std::unordered_map<ee::ecs::EntityID, float> hitList;
};

struct EnemyTag {};