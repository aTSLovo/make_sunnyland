#include "gameScene.h"
#include "../../engine/core/context.h"
#include "../../engine/object/gameObject.h"
#include "../../engine/component/spriteComponent.h"
#include "../../engine/component/transformComponent.h"
#include "../../engine/component/physicsComponent.h"
#include "../../engine/component/colliderComponent.h"
#include "../../engine/scene/levelLoader.h"
#include "../../engine/input/inputManager.h"
#include "../../engine/render/camera.h"
#include "../../engine/physics/physicsEngine.h"

#include <spdlog/spdlog.h>
#include <SDL3/SDL_rect.h>
namespace game::scene {

GameScene::GameScene(std::string name, engine::core::Context& context, engine::scene::SceneManager& scene_manager)
                    : Scene(name, context, scene_manager)
{
    spdlog::trace("GameScene 构造完成。");
}

// 覆盖场景基类的核心方法
void GameScene::init() {
    spdlog::trace("GameScene 正在初始化");
    // 加载关卡（level_loader通常加载完成后即可销毁，因此不存为成员变量）
    engine::scene::LevelLoader level_loader;
    level_loader.loadLevel("assets/maps/level1.tmj", *this);

    // 创建 test_object
    createTestObject();

    Scene::init();
    spdlog::trace("GameScene 初始化完成。");
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
    testObject();
}

void GameScene::clean() {
    Scene::clean();
}

// --- 测试方法 ---

void GameScene::createTestObject() {
    spdlog::trace("在 GameScene 中创建 test_object...");
    auto test_object = std::make_unique<engine::object::GameObject>("test_object");
    test_object_ = test_object.get();

    // 添加组件
    test_object->addComponent<engine::component::TransformComponent>(glm::vec2(100.0f, 100.0f)); 
    test_object->addComponent<engine::component::SpriteComponent>("assets/textures/Props/big-crate.png", context_.getResourceManager());
    test_object->addComponent<engine::component::PhysicsComponent>(&context_.getPhysicsEngine());
    test_object->addComponent<engine::component::ColliderComponent>(
        std::make_unique<engine::physics::AABBCollider>(glm::vec2(32.0f, 32.0f))
    );
    // 将创建好的 GameObject 添加到场景中 （一定要用std::move，否则传递的是左值）
    addGameObject(std::move(test_object)); 

    // 添加第二个游戏对象（不受重力影响），用于测试碰撞
    auto test_object2 = std::make_unique<engine::object::GameObject>("test_object2");
    test_object2->addComponent<engine::component::TransformComponent>(glm::vec2(50.0f, 50.0f));
    test_object2->addComponent<engine::component::SpriteComponent>("assets/textures/Props/big-crate.png", context_.getResourceManager());
    test_object2->addComponent<engine::component::PhysicsComponent>(&context_.getPhysicsEngine(), false);
    test_object2->addComponent<engine::component::ColliderComponent>(
        std::make_unique<engine::physics::CircleCollider>(16.0f)
    );
    addGameObject(std::move(test_object2));

    spdlog::trace("test_object 创建并添加到 GameScene 中。");
}

void GameScene::testCamera() {
    auto& camera = context_.getCamera();
    auto& input_manager = context_.getInputManager();
    if (input_manager.isActionDown("move_up")) camera.move(glm::vec2(0, -1));   
    if (input_manager.isActionDown("move_down")) camera.move(glm::vec2(0, 1));
    if (input_manager.isActionDown("move_left")) camera.move(glm::vec2(-1, 0));
    if (input_manager.isActionDown("move_right")) camera.move(glm::vec2(1, 0));
    // if (input_manager.isActionDown("jump")) { spdlog::debug("testCamera跳跃键触发");     }
    
}

void GameScene::testObject()
{
    if (!test_object_) return;
    auto& input_manager = context_.getInputManager();
    
    if (input_manager.isActionDown("move_left")) {
        test_object_->getComponent<engine::component::TransformComponent>()->translate(glm::vec2(-1, 0));
    }
    if (input_manager.isActionDown("move_right")) {
        test_object_->getComponent<engine::component::TransformComponent>()->translate(glm::vec2(1, 0));
    }
    if (input_manager.isActionPressed("jump")) {
        test_object_->getComponent<engine::component::PhysicsComponent>()->setVelocity(glm::vec2(0, -400));
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