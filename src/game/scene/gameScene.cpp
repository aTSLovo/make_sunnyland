#include "gameScene.h"
#include "../../engine/core/context.h"
#include "../../engine/object/gameObject.h"
#include "../../engine/component/spriteComponent.h"
#include "../../engine/component/transformComponent.h"
#include "../../engine/component/physicsComponent.h"
#include "../../engine/component/colliderComponent.h"
#include "../../engine/component/tilelayerComponent.h"
#include "../../engine/scene/levelLoader.h"
#include "../../engine/input/inputManager.h"
#include "../../engine/render/camera.h"
#include "../../engine/physics/physicsEngine.h"

#include <spdlog/spdlog.h>
#include <SDL3/SDL_rect.h>

namespace game::scene {

GameScene::GameScene(std::string name, engine::core::Context& context, engine::scene::SceneManager& scene_manager)
                    : Scene(name, context, scene_manager) {
    spdlog::trace("GameScene 构造完成。");
}

// 覆盖场景基类的核心方法
void GameScene::init() {
    spdlog::info("GameScene 正在初始化");
    // 加载关卡（level_loader通常加载完成后即可销毁，因此不存为成员变量）
    engine::scene::LevelLoader level_loader;
    level_loader.loadLevel("assets/maps/level1.tmj", *this);

    // 注册"main"层到物理引擎
    auto* main_layer = findGameObjectByName("main");
    if (main_layer) {
        auto* tile_layer = main_layer->getComponent<engine::component::TileLayerComponent>();
        if (tile_layer) {
            context_.getPhysicsEngine().registerCollisionLayer(tile_layer);
            spdlog::info("注册\"main\"层到物理引擎");
        }
    }

    player_ = findGameObjectByName("player");
    if(!player_) {
        spdlog::error("未找到玩家对象");
        return;
    }
    // 相机跟随玩家
    auto* player_transform = player_->getComponent<engine::component::TransformComponent>();
    if(player_transform) {
        context_.getCamera().setTarget(player_transform);
    }

    // 设置相机边界
    auto world_size = main_layer->getComponent<engine::component::TileLayerComponent>()->getWorldSize();
    context_.getCamera().setLimitBounds(engine::utils::Rect(glm::vec2(0.0f), world_size));
    
    // 设置世界边界
    context_.getPhysicsEngine().setWorldBound(engine::utils::Rect(glm::vec2(0.0f), world_size));

    Scene::init();
    spdlog::info("GameScene 初始化完成。");
}

void GameScene::update(float delta_time) {
    Scene::update(delta_time);
    TestCollisionPairs();
}

void GameScene::render() {
    Scene::render();

}

void GameScene::handleInput() {
    Scene::handleInput();
    // testCamera();
    testPlayer();
}

void GameScene::clean() {
    Scene::clean();
}

// --- 测试方法 ---

void GameScene::testCamera() {
    auto& camera = context_.getCamera();
    auto& input_manager = context_.getInputManager();
    if (input_manager.isActionDown("move_up")) camera.move(glm::vec2(0, -1));   
    if (input_manager.isActionDown("move_down")) camera.move(glm::vec2(0, 1));
    if (input_manager.isActionDown("move_left")) camera.move(glm::vec2(-1, 0));
    if (input_manager.isActionDown("move_right")) camera.move(glm::vec2(1, 0));
    // if (input_manager.isActionDown("jump")) { spdlog::debug("testCamera跳跃键触发");     }
    
}

void GameScene::testPlayer()
{
    if (!player_) return;
    auto& input_manager = context_.getInputManager();
    auto* pc = player_->getComponent<engine::component::PhysicsComponent>();
    
    if (input_manager.isActionDown("move_left")) {
        pc->velocity_.x = -100.0f;
    } else {
        pc->velocity_.x *= 0.9f;
    }
    if (input_manager.isActionDown("move_right")) {
        pc->velocity_.x = 100.0f;
    } else {
        pc->velocity_.x *= 0.9f;
    }
    if (input_manager.isActionPressed("jump")) {
        pc->setVelocity(glm::vec2(0, -400));
    }
}

void GameScene::TestCollisionPairs()
{
    auto collision_pairs = context_.getPhysicsEngine().getCollisionPairs();
    for (auto& pair : collision_pairs) {
        spdlog::info("碰撞对: {} 和 {}", pair.first->getName(), pair.second->getName());
    }
}

}