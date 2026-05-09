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
#include "Systems/ItemPickupSystem.hpp"
#include "Systems/EquippedItemSystem.hpp"
#include "Systems/MeleeHitSystem.hpp"
#include "Systems/PathFindingSystem.hpp"
#include "Systems/HealthRenderSystem.hpp"
#include "TileListLoader.hpp"
#include "Items/Weapons/WeaponFactory.hpp"

class GameManager;

class FightScene : public ee::Scene {



    enum class Action {downGradeInv, upGradeInv, reGenerate, swichUp,swichDown};

    ee::input::ActionMap<Action> m_actionMap;

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
    std::optional<ee::ecs::EntityID> m_equippedEntityId;
    ee::renderer::Renderer* m_renderer = nullptr;

    std::shared_ptr<FlipSystem>           m_FlipSystem;
    std::shared_ptr<RenderSystem>         m_renderSystem;
    std::shared_ptr<MovementSystem>       m_movementSystem;
    std::shared_ptr<MeleeHitSystem>       m_meleeHitSystem;
    std::shared_ptr<AnimationSystem>      m_animationSystem;
    std::shared_ptr<CollisionSystem>      m_collisionSystem;
    std::shared_ptr<ItemPickUpSystem>     m_itemPickupSystem;
    std::shared_ptr<PathFindingSystem>    m_pathFindingSystem;
    std::shared_ptr<HealthRenderSystem>   m_healthRenderSystem;
    std::shared_ptr<DebugRenderSystem>    m_debugRenderSystem;
    std::shared_ptr<PlayerControlSystem>  m_playerControlSystem;
    std::shared_ptr<EquippedWeaponSystem> m_equippedItemSystem;

    ee::ecs::EntityID m_enemyId = 0;

public:
    FightScene(GameManager* _gm) : ee::Scene({ 0, 0, 880, 880 }), m_camera(450, 450, 880, 880), m_gameManager(_gm) {}

    void onEnter(ee::renderer::Renderer& _renderer) override;
    void onUpdate(float _dt) override;
    void onRender(ee::renderer::Renderer& _renderer) override;


private:
    void setUpSystem();
    void refreshEquippedEntity();
    void DrawUi(ee::renderer::Renderer& _renderer);
    ee::math::Rect<float> getRectFromType(Tile _tile) const;
    ee::math::Rect<float> getAnimatedRect(const std::string& _baseName, int _frameCount) const;
};