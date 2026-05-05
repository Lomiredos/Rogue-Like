#pragma once
#include "ecs/System.hpp"
#include "ecs/World.hpp"
#include "renderer/SpriteBatch.hpp"
#include "renderer/Camera.hpp"
#include "Components/Components.hpp"

class RenderSystem : public ee::ecs::System {
private:
    std::vector<ee::renderer::SpriteEntry> m_spriteEntries;

public:
    void render(ee::ecs::World& _world, ee::renderer::Renderer& _renderer)
    {
        m_spriteEntries.clear();

        for (auto& id : m_entities)
        {
            auto& trans  = _world.getComponent<TransformComponent>(id);
            auto& sprite = _world.getComponent<SpriteComponent>(id);

            ee::math::Rect<float> srcRect = sprite.srcRect;
            float drawW = srcRect.getSize().x;
            float drawH = srcRect.getSize().y;

            if (_world.hasComponent<AnimationComponent>(id))
            {
                auto& anim = _world.getComponent<AnimationComponent>(id);
                srcRect = anim.srcRect;
                drawW = anim.hasDrawSize ? anim.drawSize.x : (float)anim.animationSet.frameWidth;
                drawH = anim.hasDrawSize ? anim.drawSize.y : (float)anim.animationSet.frameHeight;
            }

            ee::math::Rect<float> dst = {
                trans.position.x,
                trans.position.y,
                drawW,
                drawH
            };

            m_spriteEntries.push_back({ sprite.texture.get(), dst, srcRect, sprite.angle, sprite.center, sprite.flip });
        }

    }

    const std::vector<ee::renderer::SpriteEntry>& getEntries() const {
        return m_spriteEntries;
    }
};
