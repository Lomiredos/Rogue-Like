
#include "FightScene.hpp"
#include "Components/Components.hpp"
#include "renderer/TextureManager.hpp"
#include "TileListLoader.hpp"

#include "input/InputManager.hpp"
#include "GameManager.hpp"
#include "Inventory.hpp"
#include <algorithm>



void FightScene::onEnter(ee::renderer::Renderer& _renderer)
{
	m_renderer = &_renderer;



	m_actionMap.bind(Action::downGradeInv, { { ee::input::Key::NumpadMinus, ee::input::TriggerState::Down} });
	m_actionMap.bind(Action::upGradeInv, { { ee::input::Key::NumpadPlus, ee::input::TriggerState::Down} });
	m_actionMap.bind(Action::reGenerate, { { ee::input::Key::G, ee::input::TriggerState::Down} });

	m_actionMap.bind(Action::swichDown, { { ee::input::GamepadButton::LeftShoulder, ee::input::TriggerState::Down} });
	m_actionMap.bind(Action::swichUp, { { ee::input::GamepadButton::RightShoulder, ee::input::TriggerState::Down} });




	_renderer.createTexture("Tileset", "assets/Tileset.bmp");
	DungeonGenerator gen;
	m_spawnPoint = gen.generate(m_mapDown, m_mapUp, getRng(), m_rooms);

	float cx = m_spawnPoint.x * 32.f - 450.f;
	float cy = m_spawnPoint.y * 32.f - 450.f;
	m_camera = ee::renderer::Camera{ cx, cy, 900, 900 };

	setUpSystem();

	m_tileList.load("assets/tile_list_v1.7");
	TileListLoader& tileList = m_tileList;

	AnimationComponent playerAnim;
	playerAnim.animationSet = tileList.loadAnimationSet("knight_m");
	playerAnim.currentAnimation = "idle";

	m_playerId = m_world.createEntity();
	m_world.addComponent(m_playerId, TransformComponent{ m_spawnPoint * 32.f });
	m_world.addComponent(m_playerId, HealthComponent{ 100, 100 });
	m_world.addComponent(m_playerId, SpriteComponent{ _renderer.getTexture("Tileset") });
	m_world.addComponent(m_playerId, playerAnim);
	m_world.addComponent(m_playerId, MotionComponent{ {0, 0}, 350 });
	m_world.addComponent(m_playerId, ColliderComponent{ {18, 24}, {11, 29} });

	auto& anim = m_world.getComponent<AnimationComponent>(m_playerId);
	anim.drawSize = { 36.f, 58.f };
	anim.hasDrawSize = true;
	m_world.addComponent(m_playerId, PlayerInfo{ 50 });

	m_itemPickupSystem->init(m_playerId, m_gameManager->getInventory());
	m_equippedItemSystem->init(m_playerId);
	m_meleeHitSystem->init(m_playerId);

	AnimationComponent demonAnim;
	demonAnim.animationSet = m_tileList.loadAnimationSet("big_demon");
	demonAnim.currentAnimation = "idle";
	demonAnim.drawSize = { 64.f, 72.f };
	demonAnim.hasDrawSize = true;

	m_enemyId = m_world.createEntity();
	m_world.addComponent(m_enemyId, TransformComponent{ m_spawnPoint * 32.f + ee::math::Vector2<float>{128.f, 0.f} });
	m_world.addComponent(m_enemyId, HealthComponent{ 100, 100 });
	m_world.addComponent(m_enemyId, SpriteComponent{ _renderer.getTexture("Tileset") });
	m_world.addComponent(m_enemyId, demonAnim);
	m_world.addComponent(m_enemyId, ColliderComponent{ {64, 72}, {0, 0} });
	m_world.addComponent(m_enemyId, EnemyTag{});

	m_meleeHitSystem->addEnemy(m_enemyId);

	WeaponFactory::spawnWeaponEntity(m_world, _renderer, WeaponFactory::makeEpee(25.f, 60.f), m_spawnPoint * 32.f + ee::math::Vector2<float>{64.f, 0.f});
}

void FightScene::onUpdate(float _dt)
{
	m_tileAnimTimer = fmod(m_tileAnimTimer + _dt, 0.12f * 3);
	m_FlipSystem->update(m_world, _dt);
	m_animationSystem->update(m_world, _dt);
	m_playerControlSystem->update(m_world, _dt);
	m_collisionSystem->update(m_world, _dt);
	m_movementSystem->update(m_world, _dt);
	int prevItemCount = (int)m_gameManager->getInventory().getItems().size();
	m_itemPickupSystem->update(m_world, _dt);
	int newItemCount = (int)m_gameManager->getInventory().getItems().size();

	if (newItemCount > prevItemCount && prevItemCount == m_gameManager->getInventory().getCurrentIndex())
		refreshEquippedEntity();

	m_equippedItemSystem->update(m_world, _dt);
	m_meleeHitSystem->update(m_world, _dt);

	auto& trans = m_world.getComponent<TransformComponent>(m_playerId);
	m_camera = ee::renderer::Camera{
		trans.position.x - 450.f,
		trans.position.y - 450.f,
		900, 900
	};


	if (m_actionMap.isActive(Action::reGenerate) || (ee::input::InputManager::getInstance().getAxisForce(ee::input::GamepadAxis::LeftTrigger) > 0.9f && ee::input::InputManager::getInstance().getAxisForce(ee::input::GamepadAxis::RightTrigger) > 0.9f))
		m_gameManager->resetFightScene();


	if (m_actionMap.isActive(Action::upGradeInv))
		m_gameManager->getInventory().upgradeInventory();


	if (m_actionMap.isActive(Action::downGradeInv))
		m_gameManager->getInventory().downgradeInventory();

	if (int mouseDelta = ee::input::InputManager::getInstance().getMouseWhellDeltaY(); mouseDelta != 0)
	{
		m_gameManager->getInventory().changeCurrentIndex(-mouseDelta);
		refreshEquippedEntity();
	}

	if (m_actionMap.isActive(Action::swichUp))
	{
		m_gameManager->getInventory().changeCurrentIndex(1);
		refreshEquippedEntity();
	}

	if (m_actionMap.isActive(Action::swichDown))
	{
		m_gameManager->getInventory().changeCurrentIndex(-1);
		refreshEquippedEntity();
	}



}

void FightScene::onRender(ee::renderer::Renderer& _renderer)
{
	m_spriteEntry.clear();


	constexpr float tileSize = 32.f;
	constexpr int screenW = 900;
	constexpr int screenH = 900;

	int startX = std::max(0, (int)(m_camera.getX() / tileSize) - 1);
	int startY = std::max(0, (int)(m_camera.getY() / tileSize) - 1);
	int endX = std::min((int)m_mapDown.size(), startX + (int)(screenW / tileSize) + 2);
	int endY = std::min((int)m_mapDown[0].size(), startY + (int)(screenH / tileSize) + 2);

	for (int x = startX; x < endX; x++) {
		for (int y = startY; y < endY; y++) {
			if (m_mapDown[x][y].type == Tile::EMPTY) continue;
			ee::math::Rect<float> src = getRectFromType(m_mapDown[x][y].type);
			ee::math::Rect<float> dst = { (float)(x * 32), (float)(y * 32), tileSize, tileSize };
			m_spriteEntry.push_back({ _renderer.getTexture("Tileset").get(), dst, src });
		}
	}

	m_renderSystem->render(m_world, _renderer);
	for (const ee::renderer::SpriteEntry& entry : m_renderSystem->getEntries())
		m_spriteEntry.push_back(entry);

	for (int x = startX; x < endX; x++) {
		for (int y = startY; y < endY; y++) {
			if (m_mapUp[x][y].type == Tile::EMPTY) continue;
			ee::math::Rect<float> src = getRectFromType(m_mapUp[x][y].type);
			ee::math::Rect<float> dst = { (float)(x * 32), (float)(y * 32), tileSize, tileSize };
			m_spriteEntry.push_back({ _renderer.getTexture("Tileset").get(), dst, src });
		}
	}


	ee::renderer::SpriteBatch sb;
	sb.DrawAll(_renderer, m_camera, m_spriteEntry);


	DrawUi(_renderer);

	m_debugRenderSystem->render(m_world, _renderer, m_camera);
	m_meleeHitSystem->debugDraw(m_world, _renderer, m_camera);

}

void FightScene::setUpSystem()
{
	m_animationSystem = m_world.registerSystem<AnimationSystem>();
	ee::ecs::Signature sig;
	sig.set(ee::ecs::getComponentID<AnimationComponent>());
	sig.set(ee::ecs::getComponentID<SpriteComponent>());
	m_world.setSystemSignature<AnimationSystem>(sig);


	m_renderSystem = m_world.registerSystem<RenderSystem>();
	sig.reset();
	sig.set(ee::ecs::getComponentID<SpriteComponent>());
	sig.set(ee::ecs::getComponentID<TransformComponent>());
	m_world.setSystemSignature<RenderSystem>(sig);

	m_movementSystem = m_world.registerSystem<MovementSystem>();
	sig.reset();
	sig.set(ee::ecs::getComponentID<TransformComponent>());
	sig.set(ee::ecs::getComponentID<MotionComponent>());
	m_world.setSystemSignature<MovementSystem>(sig);

	m_playerControlSystem = m_world.registerSystem<PlayerControlSystem>();
	sig.reset();
	sig.set(ee::ecs::getComponentID<MotionComponent>());
	sig.set(ee::ecs::getComponentID<PlayerInfo>());
	m_world.setSystemSignature<PlayerControlSystem>(sig);
	m_playerControlSystem->init();

	m_collisionSystem = m_world.registerSystem<CollisionSystem>();
	sig.reset();
	sig.set(ee::ecs::getComponentID<TransformComponent>());
	sig.set(ee::ecs::getComponentID<MotionComponent>());
	sig.set(ee::ecs::getComponentID<ColliderComponent>());
	m_world.setSystemSignature<CollisionSystem>(sig);
	m_collisionSystem->init(m_mapDown);

	m_debugRenderSystem = m_world.registerSystem<DebugRenderSystem>();
	sig.reset();
	sig.set(ee::ecs::getComponentID<TransformComponent>());
	sig.set(ee::ecs::getComponentID<ColliderComponent>());
	m_world.setSystemSignature<DebugRenderSystem>(sig);


	m_FlipSystem = m_world.registerSystem<FlipSystem>();
	sig.reset();
	sig.set(ee::ecs::getComponentID<SpriteComponent>());
	sig.set(ee::ecs::getComponentID<MotionComponent>());
	m_world.setSystemSignature<FlipSystem>(sig);

	m_itemPickupSystem = m_world.registerSystem<ItemPickUpSystem>();
	sig.reset();
	sig.set(ee::ecs::getComponentID<TransformComponent>());
	sig.set(ee::ecs::getComponentID<ItemPickUpComponent>());
	m_world.setSystemSignature<ItemPickUpSystem>(sig);

	m_equippedItemSystem = m_world.registerSystem<EquippedWeaponSystem>();
	sig.reset();
	sig.set(ee::ecs::getComponentID<TransformComponent>());
	sig.set(ee::ecs::getComponentID<ItemEquippedComponent>());
	m_world.setSystemSignature<EquippedWeaponSystem>(sig);

	m_meleeHitSystem = m_world.registerSystem<MeleeHitSystem>();
	sig.reset();
	sig.set(ee::ecs::getComponentID<MeleeComponent>());
	sig.set(ee::ecs::getComponentID<ItemEquippedComponent>());
	m_world.setSystemSignature<MeleeHitSystem>(sig);

}

void FightScene::refreshEquippedEntity()
{
	if (m_equippedEntityId)
	{
		m_world.destroyEntity(*m_equippedEntityId);
		m_equippedEntityId.reset();
	}

	Inventory& inv = m_gameManager->getInventory();
	const auto& items = inv.getItems();
	int index = inv.getCurrentIndex();

	if (index >= (int)items.size()) return;

	std::visit([&](const auto& item) {
		if constexpr (std::is_same_v<std::decay_t<decltype(item)>, WeaponData>)
		{
			auto& playerTrans = m_world.getComponent<TransformComponent>(m_playerId);
			m_equippedEntityId = WeaponFactory::equipWeapon(m_world, *m_renderer, item, m_playerId, playerTrans.position);
		}
	}, items[index]);
}

void FightScene::DrawUi(ee::renderer::Renderer& _renderer)
{


	Inventory& inv = m_gameManager->getInventory();
	int currentIndex = inv.getCurrentIndex();


	//inv
	{
		int slotSize = 64;
		int slotInter = 16;
		int startY = 772;
		int maxSize = inv.getInventorySize();

		int screenW = 880, screenH = 880;


		int totalWidth = maxSize * slotSize + (maxSize - 1) * slotInter;
		int startX = (screenW - totalWidth) / 2;

		const auto& items = inv.getItems();

		for (int i = 0; i < maxSize; i++)
		{
			ee::math::Rect<int> slot = { startX + i * (slotSize + slotInter), startY, slotSize, slotSize };
			_renderer.DrawRect(ee::math::Rect<float>(slot), { 30, 30, 30, 128 }, true);

			if (i < (int)items.size())
			{
				ee::math::Rect<int> dst = { slot.x() - slotSize / 2, slot.y() - slotSize / 2, slotSize, slotSize };

				std::visit([&](const auto& item) {
					auto tex = _renderer.getTexture(item.spriteName);
					if (tex)
						_renderer.Draw(*tex, ee::math::Rect<float>(dst), item.srcRect, 0.f, { 0.f, 0.f }, ee::renderer::FlipMode::None);
					}, items[i]);
			}

			if (i == currentIndex)
			{
				for (int t = 0; t < 3; t++)
				{
					ee::math::Rect<int> border = { slot.x() - t, slot.y() - t, slot.w() + t * 2, slot.h() + t * 2 };
					_renderer.DrawRect(ee::math::Rect<float>(border), { 255, 255, 255, 255 });
				}
			}
			else
				_renderer.DrawRect(ee::math::Rect<float>(slot), { 120, 120, 120, 220 });
		}
	}
}



ee::math::Rect<float> FightScene::getAnimatedRect(const std::string& _baseName, int _frameCount) const
{
	int frame = (int)(m_tileAnimTimer / 0.12f) % _frameCount;
	std::string frameName = _baseName + "_anim_f" + std::to_string(frame);
	auto rect = m_tileList.getSpriteRect(frameName);
	if (rect) return *rect;
	return m_tileList.getSpriteRect(_baseName + "_anim_f0").value_or(ee::math::Rect<float>{0, 0, 16, 16});
}

ee::math::Rect<float> FightScene::getRectFromType(Tile _tile) const
{
	switch (_tile) {
	case Tile::FLOOR_1: return { 16, 64, 16, 16 };
	case Tile::FLOOR_2: return { 32, 64, 16, 16 };
	case Tile::FLOOR_3: return { 48, 64, 16, 16 };
	case Tile::FLOOR_4: return { 16, 80, 16, 16 };
	case Tile::FLOOR_5: return { 32, 80, 16, 16 };
	case Tile::FLOOR_6: return { 48, 80, 16, 16 };
	case Tile::FLOOR_7: return { 16, 96, 16, 16 };
	case Tile::FLOOR_8: return { 32, 96, 16, 16 };

	case Tile::REDFLAG:    return { 16, 32, 16, 16 };
	case Tile::BLUEFLAG:   return { 32, 32, 16, 16 };
	case Tile::GREENFLAG:  return { 16, 48, 16, 16 };
	case Tile::YELLOWFLAG: return { 32, 48, 16, 16 };


	case Tile::FLOOR_1B: return { 16, 288, 16, 16 };
	case Tile::FLOOR_2B: return { 32, 288, 16, 16 };
	case Tile::FLOOR_3B: return { 48, 288, 16, 16 };
	case Tile::FLOOR_4B: return { 16, 304, 16, 16 };
	case Tile::FLOOR_5B: return { 32, 304, 16, 16 };
	case Tile::FLOOR_6B: return { 48, 304, 16, 16 };
	case Tile::FLOOR_7B: return { 16, 320, 16, 16 };
	case Tile::FLOOR_8B: return { 32, 320, 16, 16 };


	case Tile::WALL_MID_FOUNTAIN_W:    return getAnimatedRect("wall_fountain_mid_red", 3);
	case Tile::WALL_MID_FOUNTAIN_L:    return getAnimatedRect("wall_fountain_mid_blue", 3);
	case Tile::WALL_BOTTOM_FOUNTAIN_W: return getAnimatedRect("wall_fountain_basin_red", 3);
	case Tile::WALL_BOTTOM_FOUNTAIN_L: return getAnimatedRect("wall_fountain_basin_blue", 3);
	case Tile::WALL_TOP_FOUNTAIN_1:    return { 64, 0 , 16, 16 };
	case Tile::WALL_TOP_FOUNTAIN_2:    return { 80, 0 , 16, 16 };
	case Tile::WALL_TOP_FOUNTAIN_3:    return { 96, 0 , 16, 16 };


	case Tile::WALL_TOP_LEFT:  return { 16,  0, 16, 16 };
	case Tile::WALL_TOP_MID:   return { 32,  0, 16, 16 };
	case Tile::WALL_TOP_RIGHT: return { 48,  0, 16, 16 };

	case Tile::WALL_MID_LEFT:  return { 16, 16, 16, 16 };
	case Tile::WALL_MID:       return { 32, 16, 16, 16 };
	case Tile::WALL_MID_RIGHT: return { 48, 16, 16, 16 };

	case Tile::WALL_OUTER_TOP_LEFT:    return { 0, 136, 16, 16 };
	case Tile::WALL_OUTER_TOP_RIGHT:   return { 16, 136, 16, 16 };
	case Tile::WALL_OUTER_MID_LEFT:    return { 0, 152, 16, 16 };
	case Tile::WALL_OUTER_MID_RIGHT:   return { 16, 152, 16, 16 };
	case Tile::WALL_OUTER_FRONT_LEFT:  return { 0, 168, 16, 16 };
	case Tile::WALL_OUTER_FRONT_RIGHT: return { 16, 168, 16, 16 };

	case Tile::WALL_EDGE_TOP_LEFT:      return { 32, 120, 16, 16 };
	case Tile::WALL_EDGE_TOP_RIGHT:     return { 48, 120, 16, 16 };
	case Tile::WALL_EDGE_MID_LEFT:      return { 32, 136, 16, 16 };
	case Tile::WALL_EDGE_MID_RIGHT:     return { 48, 136, 16, 16 };
	case Tile::WALL_EDGE_MIDDOWN_LEFT:  return { 32, 152, 16, 16 };
	case Tile::WALL_EDGE_MIDDOWN_RIGHT: return { 48, 152, 16, 16 };
	case Tile::WALL_EDGE_BOTTOM_LEFT:   return { 32, 168, 16, 16 };
	case Tile::WALL_EDGE_BOTTOM_RIGHT:  return { 48, 168, 16, 16 };

	case Tile::WALL_TSHAPE_BOTTOM_RIGHT: return { 64, 152, 16, 16 };
	case Tile::WALL_TSHAPE_BOTTOM_LEFT:  return { 80, 152, 16, 16 };
	case Tile::WALL_TSHAPE_RIGHT:        return { 64, 168, 16, 16 };
	case Tile::WALL_TSHAPE_LEFT:         return { 80, 168, 16, 16 };
	case Tile::WALL_WITH_BOTTOM_MID:     return { 16, 112, 16, 16 };
	case Tile::WALL_WITH_BOTTOM_LEFT:     return { 32, 112, 16, 16 };
	case Tile::WALL_WITH_BOTTOM_RIGHT:     return { 48, 112, 16, 16 };

	default: return { 0, 0, 0, 0 };
	}


}