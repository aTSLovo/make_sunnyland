#include "transformComponent.h"
#include "../object/gameObject.h"
#include "spriteComponent.h"
namespace engine::component {

void TransformComponent::setScale(const glm::vec2& scale) {
    scale_ = scale;
    if(owner_) {
        auto sprite_comp = owner_->getComponent<SpriteComponent>();
        if(sprite_comp) {
            sprite_comp->updateOffset();
        }
    }
}

}