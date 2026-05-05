#pragma once

#include "engine/Scene.hpp"

#include "DungeonGenerator.hpp"


#include "renderer/Camera.hpp"
#include "renderer/SpriteBatch.hpp"

#include "input/ActionMap.hpp"

#include "Systems/AnimationSystem.hpp"
#include "Systems/RenderSystem.hpp"
#include "Systems/MovementSystem.hpp"
#include "Systems/PlayerControlSystem.hpp"
#include "Systems/CollisionSystem.hpp"
#include "Systems/DebugRenderSystem.hpp"
#include "Systems/FlipSystem.hpp"
#include "TileListLoader.hpp"

class GameManager;

class FightScene : public ee::Scene {

    GameManager* m_gameManager;

    ee::renderer::Camera m_camera;

    std::vector<std::vector<Cell>> m_mapDown;
    std::vector<std::vector<Cell>> m_mapUp;
    std::vector<RoomInfo> m_rooms;
    ee::math::Vector2<float> m_spawnPoint;

    std::vector<ee::renderer::SpriteEntry> m_spriteEntry;
    TileListLoader m_tileList;
    float m_tileAnimTimer = 0.f;


    ee::ecs::EntityID m_playerId = 0;

    std::shared_ptr<FlipSystem>          m_FlipSystem;
    std::shared_ptr<RenderSystem>        m_renderSystem;
    std::shared_ptr<MovementSystem>      m_movementSystem;
    std::shared_ptr<AnimationSystem>     m_animationSystem;
    std::shared_ptr<CollisionSystem>     m_collisionSystem;
    std::shared_ptr<DebugRenderSystem>   m_debugRenderSystem;
    std::shared_ptr<PlayerControlSystem> m_playerControlSystem;

public:
    FightScene(GameManager* _gm) : ee::Scene({ 0, 0, 880, 880 }), m_camera(450, 450, 880, 880), m_gameManager(_gm) {}

    void onEnter(ee::renderer::Renderer& _renderer) override;
    void onUpdate(float _dt) override;
    void onRender(ee::renderer::Renderer& _renderer) override;


private:
    void setUpSystem();
    ee::math::Rect<float> getRectFromType(Tile _tile) const;
    ee::math::Rect<float> getAnimatedRect(const std::string& _baseName, int _frameCount) const;
};