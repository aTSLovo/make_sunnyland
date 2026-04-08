#include "parallaxComponent.h"
#include "../object/gameObject.h"
#include "../component/transformComponent.h"
#include "../core/context.h"
#include "../render/renderer.h"
#include <spdlog/spdlog.h>
namespace engine::component {

ParallaxComponent::ParallaxComponent(const std::string& texture_id, const glm::vec2& scroll_factor, const glm::bvec2& repeat)
    : sprite_(texture_id),
      scroll_factor_(scroll_factor),
      repeat_(repeat)
{
    spdlog::trace("ParallaxComponent 初始化完成，纹理 ID: {}", texture_id);
}

void ParallaxComponent::init() {
    if(!owner_) {
        spdlog::error("ParallaxComponent 初始化时，GameObject 为空。");
        return;
    }
    transform_ = owner_->getComponent<engine::component::TransformComponent>();
    if (!transform_) {
        spdlog::error("ParallaxComponent 初始化时，GameObject 上没有找到 TransformComponent 组件。");
        return;
    }
}

void ParallaxComponent::render(engine::core::Context& context) {
    if (is_hidden_ || !transform_) {
        return;
    }
    context.getRenderer().drawParallax(context.getCamera(), sprite_, transform_->getPosition(), scroll_factor_, repeat_, transform_->getScale());
}

}