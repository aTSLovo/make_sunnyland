#include "transformComponent.h"
#include "../object/gameObject.h"
#include "spriteComponent.h"
#include "colliderComponent.h"
namespace engine::component {

void TransformComponent::setScale(const glm::vec2& scale) {
    scale_ = scale;
    if(owner_) {
        auto sprite_component = owner_->getComponent<SpriteComponent>();
        if(sprite_component) {
            sprite_component->updateOffset();
        }

        auto collider_component = owner_->getComponent<ColliderComponent>();
        if (collider_component) {
            collider_component->updateOffset();
        }
    }
}

}