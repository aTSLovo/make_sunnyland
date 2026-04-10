#include "physicsComponent.h"
#include "transformComponent.h"
#include "../object/gameObject.h"
#include "../physics/physicsEngine.h"
#include <spdlog/spdlog.h>

namespace engine::component {

PhysicsComponent::PhysicsComponent(engine::physics::PhysicsEngine* physics_engine, bool use_gravity, float mass)
    : physics_engine_(physics_engine),
      use_gravity_(use_gravity),
      mass_(mass)
{
    if (!physics_engine_) {
        spdlog::error("PhysicsComponent构造函数中，PhysicsEngine指针不能为nullptr！");
    }
    spdlog::trace("物理组件创建完成，质量: {}, 使用重力: {}", mass_, use_gravity_);
}

void PhysicsComponent::init() {
    if(!owner_) {
        spdlog::error("物理组件初始化时，容器持有GameObject对象不存在，错误");
        return ;
    }
    if(!physics_engine_) {
        spdlog::error("物理组件初始化时，PhysicsEngine未正确初始化。");
        return;
    }
    transform_ = owner_->getComponent<engine::component::TransformComponent>();
    if (!transform_) {
        spdlog::warn("物理组件初始化时，同一GameObject上没有找到TransformComponent组件。");
    }
    // 注册到PhysicsEngine
    physics_engine_->registerComponent(this);
    spdlog::trace("物理组件初始化完成。");
}

void PhysicsComponent::clean() {
    physics_engine_->unregisterComponent(this);
    spdlog::trace("物理组件清理完成。");
}

}