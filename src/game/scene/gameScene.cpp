#include "gameScene.h"
#include "../../engine/core/context.h"
#include "../../engine/object/gameObject.h"
#include "../../engine/component/spriteComponent.h"
#include "../../engine/component/transformComponent.h"
#include <spdlog/spdlog.h>
namespace game::scene {

GameScene::GameScene(std::string name, engine::core::Context& context, engine::scene::SceneManager& scene_manager)
                    : Scene(name, context, scene_manager)
{
    spdlog::trace("GameScene 构造完成。");
}

// 覆盖场景基类的核心方法
void GameScene::init() {
    createTestObject();

    Scene::init();
    spdlog::trace("GameScene 初始化完成。");
}

void GameScene::update(float delta_time) {
    Scene::update(delta_time);
}

void GameScene::render() {
    Scene::render();
}

void GameScene::handleInput() {
    Scene::handleInput();
}

void GameScene::clean() {
    Scene::clean();
}

void GameScene::createTestObject() {
    spdlog::trace("在 GameScene 中创建 test_object...");
    auto test_object = std::make_unique<engine::object::GameObject>("test_object");

    // 添加组件
    test_object->addComponent<engine::component::TransformComponent>(glm::vec2(100.0f, 100.0f)); 
    test_object->addComponent<engine::component::SpriteComponent>("assets/textures/Props/big-crate.png", context_.getResourceManager());

    // 将创建好的 GameObject 添加到场景中 （一定要用std::move，否则传递的是左值）
    addGameObject(std::move(test_object)); 
    spdlog::trace("test_object 创建并添加到 GameScene 中。");
}

}