#pragma once
#include "ecs/System.hpp"
#include "ecs/World.hpp"
#include "Components/Components.hpp"

class AnimationSystem : public ee::ecs::System {
public:
    void update(ee::ecs::World& _world, float _dt) override
    {
        for (auto& id : m_entities)
        {
            auto& anim = _world.getComponent<AnimationComponent>(id);

            auto it = anim.animationSet.clips.find(anim.currentAnimation);
            if (it == anim.animationSet.clips.end()) continue;

            const AnimationClip& clip = it->second;

            auto updateSrcRect = [&]() {
                anim.srcRect = clip.frames[anim.currentFrame];
            };

            if (anim.srcRect.getSize().x == 0)
                updateSrcRect();

            anim.timer += _dt;
            if (anim.timer < clip.frameTime) continue;
            anim.timer = 0.f;
            anim.currentFrame++;

            if (anim.currentFrame >= (int)clip.frames.size())
            {
                if (clip.loop) anim.currentFrame = 0;
                else anim.currentFrame = (int)clip.frames.size() - 1;
            }

            updateSrcRect();
        }
    }
};
