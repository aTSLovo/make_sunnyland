#include "physicsEngine.h"
#include "../component/physicsComponent.h"
#include "../component/transformComponent.h"
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
}

}