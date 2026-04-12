#include "physicsEngine.h"
#include "collision.h"
#include "../component/physicsComponent.h"
#include "../component/transformComponent.h"
#include "../component/colliderComponent.h"
#include "../object/gameObject.h"
#include <spdlog/spdlog.h>
#include <algorithm>
#include <glm/common.hpp>

namespace engine::physics {

void PhysicsEngine::registerComponent(engine::component::PhysicsComponent* component) {
    components_.push_back(component);
    spdlog::trace("物理组件注册完成。");
}

void PhysicsEngine::unregisterComponent(engine::component::PhysicsComponent* component) {
    // 使用 remove-erase 方法安全地移除指针
    auto it = std::remove(components_.begin(), components_.end(), component);
    components_.erase(it, components_.end());
    spdlog::trace("物理组件注销完成。");
}

void PhysicsEngine::update(float delta_time) {
    // 每帧开始时先清空碰撞对列表
    collision_pairs_.clear();

    // 遍历所有注册的物理组件
    for(auto component : components_) {

        if (!component || !component->isEnabled()) { // 检查组件是否有效和启用
            continue;
        }

        // 应用重力 (如果组件受重力影响)：F = g * m
        if(component->isUseGravity()) {
            component->addForce(gravity_ * component->getMass());
        }

        /* 还可以添加其它力影响，比如风力、摩擦力等，目前不考虑 */
        component->velocity_ += component->getForce() / component->getMass() * delta_time;
        component->clearForce(); // 清除当前帧的力

        auto ts = component->getTransform();
        if(ts) {
            ts->translate(component->velocity_ * delta_time);
        }

        // 限制最大速度：v = min(v, max_speed)
        component->velocity_ = glm::clamp(component->velocity_, -max_speed_, max_speed_);
    }
    
    // 处理对象间碰撞
    checkObjectCollisions();
}

void PhysicsEngine::checkObjectCollisions() {
    // 两层循环遍历所有包含物理组件的 GameObject
    for (size_t i = 0; i < components_.size(); ++i) {
        auto* pc_a = components_[i];
        if (!pc_a || !pc_a->isEnabled()) continue;
        auto* obj_a = pc_a->getOwner();
        if (!obj_a) continue;
        auto* cc_a = obj_a->getComponent<engine::component::ColliderComponent>();
        if (!cc_a || !cc_a->isActive()) continue;

        for (size_t j = i + 1; j < components_.size(); ++j) {
            auto* pc_b = components_[j];
            if (!pc_b || !pc_b->isEnabled()) continue;
            auto* obj_b = pc_b->getOwner();
            if (!obj_b) continue;
            auto* cc_b = obj_b->getComponent<engine::component::ColliderComponent>();
            if (!cc_b || !cc_b->isActive()) continue;
            /* --- 通过保护性测试后，正式执行逻辑 --- */

            if (collision::checkCollision(*cc_a, *cc_b)) {
                // TODO: 并不是所有碰撞都需要插入collision_pairs_，未来会添加过滤条件
                // 记录碰撞对
                collision_pairs_.emplace_back(obj_a, obj_b);
            }
        }
    }
}

}