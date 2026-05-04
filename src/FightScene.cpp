
#include "FightScene.hpp"
#include "Components/Components.hpp"
#include "renderer/TextureManager.hpp"
#include "AnimationLoader.hpp"

#include <algorithm>

void FightScene::onEnter(ee::renderer::Renderer& _renderer)
{
	_renderer.createTexture("Tileset", "assets/Tileset.bmp");
	DungeonGenerator gen;
	m_spawnPoint = gen.generate(m_mapDown, m_mapUp, getRng(), m_rooms);

	float cx = m_spawnPoint.x * 32.f - 450.f;
	float cy = m_spawnPoint.y * 32.f - 450.f;
	m_camera = ee::renderer::Camera{ cx, cy, 900, 900 };

	setUpSystem();

	m_playerId = m_world.createEntity();
	m_world.addComponent(m_playerId, TransformComponent{ m_spawnPoint * 32.f });
	m_world.addComponent(m_playerId, HealthComponent{ 100, 100 });
	m_world.addComponent(m_playerId, SpriteComponent{ _renderer.getTexture("Tileset")});
	m_world.addComponent(m_playerId, AnimationLoader::loadFromJson("assets/JsonAnimation/player.json"));
	m_world.addComponent(m_playerId, MotionComponent{{0, 0}, 200});
	
	auto& anim = m_world.getComponent<AnimationComponent>(m_playerId);
	anim.drawSize = { 36.f, 59.f };
	anim.hasDrawSize = true;
	m_world.addComponent(m_playerId, PlayerTag{});

}

void FightScene::onUpdate(float _dt)
{
	m_animationSystem->update(m_world, _dt);
	m_movementSystem->update(m_world, _dt);
	m_playerControlSystem->update(m_world, _dt);

	auto& trans = m_world.getComponent<TransformComponent>(m_playerId);
	m_camera = ee::renderer::Camera{
		trans.position.x - 450.f,
		trans.position.y - 450.f,
		900, 900
	};

}

void FightScene::onRender(ee::renderer::Renderer& _renderer)
{

	m_spriteEntry.clear();

	constexpr float tileSize = 32.f;
	constexpr int screenW = 900;
	constexpr int screenH = 900;

	int startX = std::max(0, (int)(m_camera.getX() / tileSize))-1;
	int startY = std::max(0, (int)(m_camera.getY() / tileSize))-1;
	int endX   = std::min((int)m_mapDown.size(),    startX + (int)(screenW / tileSize) + 2);
	int endY   = std::min((int)m_mapDown[0].size(), startY + (int)(screenH / tileSize) + 2);

	for (int x = startX; x < endX; x++) {
		for (int y = startY; y < endY; y++) {
			if (m_mapDown[x][y].type == Tile::EMPTY) continue;
			ee::math::Rect<float> src = getRectFromType(m_mapDown[x][y].type);
			ee::math::Rect<float> dst = { x * tileSize, y * tileSize, tileSize, tileSize };
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
			ee::math::Rect<float> dst = { x * tileSize, y * tileSize, tileSize, tileSize };
			m_spriteEntry.push_back({ _renderer.getTexture("Tileset").get(), dst, src });
		}
	}


	ee::renderer::SpriteBatch sb;
	sb.DrawAll(_renderer, m_camera, m_spriteEntry);
	 
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
	sig.set(ee::ecs::getComponentID<PlayerTag>());
	m_world.setSystemSignature<PlayerControlSystem>(sig);
	m_playerControlSystem->init();

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


	case Tile::WALL_MID_FOUNTAIN_W:    return { 64, 16, 16, 16 };
	case Tile::WALL_MID_FOUNTAIN_L:    return { 64, 48, 16, 16 };
	case Tile::WALL_BOTTOM_FOUNTAIN_W: return { 64, 32, 16, 16 };
	case Tile::WALL_BOTTOM_FOUNTAIN_L: return { 64, 64, 16, 16 };
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

	default: return { 0, 0, 0, 0 };
	}


}