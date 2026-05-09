#pragma once

#include "ecs/System.hpp"
#include "ecs/World.hpp"
#include "Components/Components.hpp"
#include "TileListLoader.hpp"
#include "renderer/Renderer.hpp"
#include "renderer/Camera.hpp"
#include <cmath>

class HealthRenderSystem : public ee::ecs::System {

private:
    ee::math::Rect<float> m_srcFull;
    ee::math::Rect<float> m_srcHalf;
    ee::math::Rect<float> m_srcEmpty;
    std::shared_ptr<ee::renderer::Texture> m_texture;

    float m_breathAmplitude = 3.f;
    float m_breathSpeed     = 2.f;
    float m_breathTimer     = 0.f;

public:
    void init(const TileListLoader& _tileList, ee::renderer::Renderer& _renderer) {
        m_srcFull  = _tileList.getSpriteRect("ui_heart_full") .value_or(ee::math::Rect<float>{});
        m_srcHalf  = _tileList.getSpriteRect("ui_heart_half") .value_or(ee::math::Rect<float>{});
        m_srcEmpty = _tileList.getSpriteRect("ui_heart_empty").value_or(ee::math::Rect<float>{});
        m_texture  = _renderer.getTexture("Tileset");
    }

    void update(float _dt) {
        m_breathTimer += _dt * m_breathSpeed;
    }

    void render(ee::ecs::World& _world, ee::renderer::Renderer& _renderer, const ee::renderer::Camera& _cam) {
        if (!m_texture) return;

        for (auto& id : m_entities) {
            auto& hp = _world.getComponent<HealthComponent>(id);
            auto& t  = _world.getComponent<TransformComponent>(id);

            int slots = (hp.max + 1) / 2;

            // centre et haut du sprite rendu
            float cx = t.position.x;
            float cy = t.position.y;
            float spriteOffX = 0.f, spriteOffY = 0.f;
            float spriteW = 0.f;

            if (_world.hasComponent<SpriteComponent>(id)) {
                auto& s = _world.getComponent<SpriteComponent>(id);
                spriteOffX = s.offset.x;
                spriteOffY = s.offset.y;
                spriteW = s.hasDrawSize ? s.drawSize.x : s.srcRect.getSize().x;
            }
            if (_world.hasComponent<AnimationComponent>(id)) {
                auto& a = _world.getComponent<AnimationComponent>(id);
                if (a.hasDrawSize) spriteW = a.drawSize.x;
            }

            cx = t.position.x + spriteOffX + spriteW / 2.f;
            cy = t.position.y + spriteOffY;

            float breathOffset = std::sin(m_breathTimer) * m_breathAmplitude;

            float totalW = slots * (26 + 3) - 3;
            float startX = _cam.getScreenX(cx) - totalW *0.75f;
            float startY = _cam.getScreenY(cy)  - 24 - 20 + breathOffset;

            for (int i = 0; i < slots; i++) {
                int hpInSlot = hp.current - i * 2;
                ee::math::Rect<float> src;
                if      (hpInSlot >= 2) src = m_srcFull;
                else if (hpInSlot == 1) src = m_srcHalf;
                else                    src = m_srcEmpty;

                ee::math::Rect<float> dst = { startX + i * (26 + 3), startY, 26, 24 };
                _renderer.Draw(*m_texture, dst, src, 0.f, { 0.f, 0.f }, ee::renderer::FlipMode::None);
            }
        }
    }
};
