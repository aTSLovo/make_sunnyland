#include "scene.h"
#include "../object/gameObject.h"
#include "../core/context.h"
#include "../physics/physicsEngine.h"
#include "sceneManager.h"
#include <algorithm> // for std::remove_if
#include <spdlog/spdlog.h>

namespace engine::scene {

Scene::Scene(std::string name, engine::core::Context& context, engine::scene::SceneManager& scene_manager)
    : scene_name_(name),
      context_(context),
      scene_manager_(scene_manager),
      is_initialized_(false) {
    spdlog::trace("场景 {} 构造完成。", scene_name_);
}

Scene::~Scene() = default;

void Scene::init() {
    is_initialized_ = true;     // 子类应该最后调用父类的 init 方法
    spdlog::trace("场景 {} 初始化完成。", scene_name_);
}

void Scene::update(float delta_time) {
    if(!is_initialized_) return ;

    // 先更新物理引擎
    context_.getPhysicsEngine().update(delta_time);

    // 更新所有游戏对象，并删除需要移除的对象
    for(auto it = game_objects_.begin(); it != game_objects_.end();) {
        if(*it && !(*it)->isNeedRemove()) { // 正常更新游戏对象
            (*it)->update(delta_time, context_);
            ++it;
        }
        else {
            if(*it) {   // 安全删除需要移除的对象
                (*it)->clean();
            }
            it = game_objects_.erase(it);   // 删除需要移除的对象，智能指针自动管理内存
        }
    }
    
    // 处理待添加（延时添加）的游戏对象
    processPendingAdditions();
}

void Scene::render() {
    if(!is_initialized_) return;
    // 渲染所有游戏对象
    for(const auto& obj : game_objects_) {
        if(obj) obj->render(context_);
    }
}

void Scene::handleInput() {
     if (!is_initialized_) return;

    // 遍历所有游戏对象，并删除需要移除的对象
    for (auto it = game_objects_.begin(); it != game_objects_.end();) {
        if (*it && !(*it)->isNeedRemove()) {
            (*it)->handleInput(context_);
            ++it;
        } else {
            // 安全删除需要移除的对象
            if (*it) (*it)->clean();
            it = game_objects_.erase(it);
        }
    }
}

void Scene::clean() {
    if (!is_initialized_) return ;
    
    for (const auto& obj : game_objects_) {
        if (obj) obj->clean();
    }
    game_objects_.clear();

    is_initialized_ = false;        // 清理完成后，设置场景为未初始化
    spdlog::trace("场景 {} 清理完成。", scene_name_);
}

void Scene::addGameObject(std::unique_ptr<engine::object::GameObject>&& game_object) {
    if(!game_object) {
        spdlog::warn("尝试直接向场景: {} 添加一个空游戏对象。", scene_name_);
        return ;
    }
    spdlog::debug("正在向场景: {} 添加游戏对象: {}", scene_name_, game_object->getName());
    game_objects_.push_back(std::move(game_object));
}

void Scene::safeAddGameObject(std::unique_ptr<engine::object::GameObject>&& game_object) {
    if(!game_object) {
        spdlog::warn("尝试直接向场景: {} 安全地添加一个空游戏对象。", scene_name_);
        return ;
    }
    spdlog::debug("正在向场景: {} 安全地添加游戏对象: {}", scene_name_, game_object->getName());
    pending_additions_.push_back(std::move(game_object));
}

void Scene::removeGameObject(engine::object::GameObject* game_object_ptr) {
    if(!game_object_ptr) {
        spdlog::warn("尝试从场景 {} 中移除一个空的游戏对象指针。", scene_name_);
        return;
    }

    // erase-remove 移除法不可用，因为智能指针与裸指针无法比较
    // 需要使用 std::remove_if 和 lambda 表达式自定义比较的方式
    auto it = std::remove_if(game_objects_.begin(), game_objects_.end(),
                    [game_object_ptr](const std::unique_ptr<engine::object::GameObject>& p) {
                        return p.get() == game_object_ptr;  // 比较裸指针是否相等（自定义比较方式）
                    });

    if(it != game_objects_.end()) {
        (*it)->clean();
        game_objects_.erase(it, game_objects_.end());
        spdlog::trace("从场景 {} 中移除游戏对象。", scene_name_);
    }
    else {
        spdlog::warn("游戏对象指针未在场景 {} 中找到。", scene_name_);
    }
}

void Scene::safeRemoveGameObject(engine::object::GameObject* game_object_ptr) {
    game_object_ptr->setNeedRemove(true);
}

engine::object::GameObject* Scene::findGameObjectByName(const std::string& name) const {
    // 找到第一个符合条件的游戏对象就返回
    for(const auto& obj : game_objects_) {
        if (obj && obj->getName() == name) {
            return obj.get();
        }
    }
    return nullptr;
}

void Scene::processPendingAdditions() {
    // 处理待添加的游戏对象
    for(auto& game_object : pending_additions_) {
        addGameObject(std::move(game_object));
    }
    pending_additions_.clear();
}

}