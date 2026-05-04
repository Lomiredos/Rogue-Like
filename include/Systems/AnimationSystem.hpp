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
                int frame = clip.frames[anim.currentFrame];
                int cols  = (int)(_world.getComponent<SpriteComponent>(id).texture->getWidth() / anim.animationSet.frameWidth);
                anim.srcRect = {
                    (float)(frame % cols * anim.animationSet.frameWidth),
                    (float)(frame / cols * anim.animationSet.frameHeight),
                    (float)anim.animationSet.frameWidth,
                    (float)anim.animationSet.frameHeight
                };
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
