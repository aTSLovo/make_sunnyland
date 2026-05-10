#include "physicsEngine.h"
#include "collision.h"
#include "../component/physicsComponent.h"
#include "../component/transformComponent.h"
#include "../component/colliderComponent.h"
#include "../component/tilelayerComponent.h"
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

void PhysicsEngine::registerCollisionLayer(engine::component::TileLayerComponent* layer) {
    collision_tilelayers_components_.push_back(layer);
    spdlog::trace("碰撞瓦片组件注册完成。");
}

void PhysicsEngine::unregisterCollisionLayer(engine::component::TileLayerComponent* layer) {
    auto it = std::remove(collision_tilelayers_components_.begin(), collision_tilelayers_components_.end(), layer);
    collision_tilelayers_components_.erase(it, collision_tilelayers_components_.end());
    spdlog::trace("碰撞瓦片组件注销完成。");
}

void PhysicsEngine::update(float delta_time) {
    // 每帧开始时先清空碰撞对列表
    collision_pairs_.clear();

    // 遍历所有注册的物理组件
    for(auto* pc : components_) {
        if (!pc || !pc->isEnabled()) { // 检查组件是否有效和启用
            continue;
        }

        pc->resetCollisionFlags();  // 重置碰撞标志

        // 应用重力 (如果组件受重力影响)：F = g * m
        if(pc->isUseGravity()) {
            pc->addForce(gravity_ * pc->getMass());
        }

        /* 还可以添加其它力影响，比如风力、摩擦力等，目前不考虑 */
        pc->velocity_ += (pc->getForce() / pc->getMass()) * delta_time;
        pc->clearForce(); // 清除当前帧的力

        // 移动到resolveTileCollisions中，进行轴分离碰撞解析算法
        // auto ts = pc->getTransform();
        // if(ts) {
        //     ts->translate(pc->velocity_ * delta_time);
        // }
        // // 限制最大速度：v = min(v, max_speed)
        // pc->velocity_ = glm::clamp(pc->velocity_, -max_speed_, max_speed_);
        
        // 处理瓦片层碰撞（速度和位置的更新移入此函数）
        resolveTileCollisions(pc, delta_time);

        // 应用世界边界
        applyWorldBounds(pc);
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
                // 如果是可移动物体与solid物体碰撞，则直接处理位置变化，不用记录碰撞对
                if(obj_a->getTag() == "solid" && obj_b->getTag() != "solid") {
                    resolveSolidObjectCollisions(obj_b, obj_a);
                }
                else if(obj_a->getTag() != "solid" && obj_b->getTag() == "solid") {
                    resolveSolidObjectCollisions(obj_a, obj_b);
                }
                else {
                    // 记录碰撞对
                    collision_pairs_.emplace_back(obj_a, obj_b);
                }
            }
        }
    }
}

void PhysicsEngine::resolveTileCollisions(engine::component::PhysicsComponent* pc, float delta_time) {
    // 检查组件是否有效
    auto* obj = pc->getOwner();
    if(!obj) return;
    auto* tc = obj->getComponent<engine::component::TransformComponent>();  // TransformComponent
    auto* cc = obj->getComponent<engine::component::ColliderComponent>();   // ColliderComponent
    if (!tc || !cc || !cc->isActive() || cc->isTrigger()) return;
    auto world_aabb = cc->getWorldAABB();   // 使用最小包围盒进行碰撞检测（简化）
    auto obj_pos = world_aabb.position;     // 当前物体obj的世界坐标
    auto obj_size = world_aabb.size;        // 当前物体obj的大小
    if (world_aabb.size.x <= 0.0f || world_aabb.size.y <= 0.0f) return;
    // -- 检查结束, 正式开始处理 --
    
    // 物体底部y坐标32(贴在边缘了)，瓦片高度16，瓦片坐标y是2了，物体是在坐标y=2的地面上左右滑动，那这样就不让物体滑动了，所以需要减1像素
    auto tolerance = 1.0f;                  // 检查右边缘和下边缘时，需要减1像素，否则会检查到下一行/列的瓦片
    auto ds = pc->velocity_ * delta_time;   // 计算物体在delta_time内的位移
    auto new_obj_pos = obj_pos + ds;        // 计算物体在delta_time后的新位置

    // 遍历所有注册的碰撞瓦片层
    for(auto* layer : collision_tilelayers_components_) {
        if(!layer) continue;
        auto tile_size = layer->getTileSize();  // 当前检查瓦片的大小
        // 轴分离碰撞检测：先检查X方向是否有碰撞 (y方向使用初始值obj_pos.y)
        if (ds.x > 0.0f) {
            // 检查右侧碰撞，需要分别测试右上和右下角
            auto right_top_x = new_obj_pos.x + obj_size.x;                      // 物体右侧新位置
            auto tile_x = static_cast<int>(floor(right_top_x / tile_size.x));   // 获取新位置右侧瓦片坐标
            // y方向坐标有两个，右上和右下
            auto tile_y_top = static_cast<int>(floor(obj_pos.y / tile_size.y));     // 获取新位置顶部瓦片坐标
            auto tile_type_top = layer->getTileTypeAt({tile_x, tile_y_top});        // 右上角瓦片类型
            auto tile_y_bottom = static_cast<int>(floor((obj_pos.y + obj_size.y - tolerance) / tile_size.y));   // 获取新位置底部瓦片坐标
            auto tile_type_bottom = layer->getTileTypeAt({tile_x, tile_y_bottom});     // 右下角瓦片类型

            if (tile_type_top == engine::component::TileType::SOLID || tile_type_bottom == engine::component::TileType::SOLID) {
                // 撞墙了！速度归零，x方向移动到贴着墙的位置
                new_obj_pos.x = tile_x * layer->getTileSize().x - obj_size.x;   // 注意这里是减去一个物体x方向大小
                pc->velocity_.x = 0.0f;
                pc->setCollidedRight(true);
                spdlog::trace("对象和瓦片碰撞: 右侧发生碰撞, 速度x设置为0");
            }
            else {
                // 检测右下角斜坡瓦片
                auto width_right = new_obj_pos.x + obj_size.x - tile_x * tile_size.x;
                auto height_right = getTileHeightAtWidth(width_right, tile_type_bottom, tile_size);

                if(height_right > 0.0f) {
                    if(new_obj_pos.y > (tile_y_bottom + 1) * layer->getTileSize().y - obj_size.y - height_right) {
                        new_obj_pos.y = (tile_y_bottom + 1) * layer->getTileSize().y - obj_size.y - height_right;
                        pc->setCollidedBelow(true);
                        spdlog::trace("对象和瓦片碰撞: 右下角斜坡发生碰撞, 速度y设置为0");
                    }
                }
            }
        }
        else if (ds.x < 0.0f) {
            // 检查左侧碰撞，需要分别测试左上和左下角
            auto left_top_x = new_obj_pos.x;
            auto tile_x = static_cast<int>(floor(left_top_x / tile_size.x));    // 获取x方向瓦片坐标
            // y方向坐标有两个，左上和左下
            auto tile_y_top = static_cast<int>(floor(obj_pos.y / tile_size.y));
            auto tile_type_top = layer->getTileTypeAt({tile_x, tile_y_top});        // 左上角瓦片类型
            auto tile_y_bottom = static_cast<int>(floor((obj_pos.y + obj_size.y - tolerance) / tile_size.y));
            auto tile_type_bottom = layer->getTileTypeAt({tile_x, tile_y_bottom});     // 左下角瓦片类型

            if (tile_type_top == engine::component::TileType::SOLID || tile_type_bottom == engine::component::TileType::SOLID) {
                // 撞墙了！速度归零，x方向移动到贴着墙的位置
                new_obj_pos.x = (tile_x + 1) * layer->getTileSize().x;
                pc->velocity_.x = 0.0f;
                pc->setCollidedLeft(true);
                spdlog::trace("对象和瓦片碰撞: 左侧发生碰撞, 速度x设置为0");
            }
            else {
                auto width_left = new_obj_pos.x - tile_x * tile_size.x;
                auto height_left = getTileHeightAtWidth(width_left, tile_type_bottom, tile_size);
                if(height_left > 0.0f) {
                    if(new_obj_pos.y > (tile_y_bottom + 1) * layer->getTileSize().y - obj_size.y - height_left) {
                        new_obj_pos.y = (tile_y_bottom + 1) * layer->getTileSize().y - obj_size.y - height_left;
                        pc->setCollidedBelow(true);
                        spdlog::trace("对象和瓦片碰撞: 左下角斜坡发生碰撞, 速度y设置为0");
                    }
                }
            }
        }
        // 轴分离碰撞检测：再检查Y方向是否有碰撞 (x方向使用初始值obj_pos.x)
        if (ds.y > 0.0f) {
            // 下落，检查底部碰撞，需要分别测试左下和右下角
            auto bottom_left_y = new_obj_pos.y + obj_size.y;
            auto tile_y = static_cast<int>(floor(bottom_left_y / tile_size.y));

            auto tile_x_left = static_cast<int>(floor(obj_pos.x / tile_size.x));
            auto tile_type_left = layer->getTileTypeAt({tile_x_left, tile_y});           // 左下角瓦片类型   
            auto tile_x_right = static_cast<int>(floor((obj_pos.x + obj_size.x - tolerance) / tile_size.x));
            auto tile_type_right = layer->getTileTypeAt({tile_x_right, tile_y});     // 右下角瓦片类型

            if (tile_type_left == engine::component::TileType::SOLID || tile_type_right == engine::component::TileType::SOLID ||
                tile_type_left == engine::component::TileType::UNISOLID || tile_type_right == engine::component::TileType::UNISOLID) {
                // 到达地面！速度归零，y方向移动到贴着地面的位置
                new_obj_pos.y = tile_y * layer->getTileSize().y - obj_size.y;
                pc->velocity_.y = 0.0f;
                pc->setCollidedBelow(true);
            }
            else {
                auto width_left = obj_pos.x - tile_x_left * tile_size.x;
                auto width_right = obj_pos.x + obj_size.x - tile_x_right * tile_size.x; // 需要用右侧的瓦片宽度，否则在上坡的时候会卡斜坡
                auto height_left = getTileHeightAtWidth(width_left, tile_type_left, tile_size);
                auto height_right = getTileHeightAtWidth(width_right, tile_type_right, tile_size);
                auto height = glm::max(height_left, height_right);  // 找到两个角点的最高点进行检测
                if (height > 0.0f) {    // 说明至少有一个角点处于斜坡瓦片
                    if (new_obj_pos.y > (tile_y + 1) * layer->getTileSize().y - obj_size.y - height) {
                        new_obj_pos.y = (tile_y + 1) * layer->getTileSize().y - obj_size.y - height;
                        pc->velocity_.y = 0.0f;     // 只有向下运动时才需要让 y 速度归零
                        pc->setCollidedBelow(true);
                        spdlog::trace("对象和瓦片碰撞: 发生斜坡碰撞, 速度y设置为0");
                    }
                }
            }
        }
        else if (ds.y < 0.0f) {
            // 上升，检查顶部碰撞，需要分别测试左上和右上角
            auto top_left_y = new_obj_pos.y;
            auto tile_y = static_cast<int>(floor(top_left_y / tile_size.y));

            auto tile_x_left = static_cast<int>(floor(obj_pos.x / tile_size.x));
            auto tile_type_left = layer->getTileTypeAt({tile_x_left, tile_y});        // 左上角瓦片类型
            auto tile_x_right = static_cast<int>(floor((obj_pos.x + obj_size.x - tolerance) / tile_size.x));
            auto tile_type_right = layer->getTileTypeAt({tile_x_right, tile_y});     // 右上角瓦片类型

            if (tile_type_left == engine::component::TileType::SOLID || tile_type_right == engine::component::TileType::SOLID) {
                // 撞到天花板！速度归零，y方向移动到贴着天花板的位置
                new_obj_pos.y = (tile_y + 1) * layer->getTileSize().y;
                pc->velocity_.y = 0.0f;
                pc->setCollidedAbove(true);
            }
        }
    }
    // 更新物体位置，并限制最大速度
    tc->translate(new_obj_pos - obj_pos);   // 避免直接设置位置，碰撞盒可能有偏移
    pc->velocity_ = glm::clamp(pc->velocity_, -max_speed_, max_speed_);
}

void PhysicsEngine::resolveSolidObjectCollisions(engine::object::GameObject* move_obj, engine::object::GameObject* solid_obj) {
    // 进入此函数前，已经检查了各个组件的有效性，因此直接进行计算
    auto* move_tc = move_obj->getComponent<engine::component::TransformComponent>();
    auto* move_pc = move_obj->getComponent<engine::component::PhysicsComponent>();
    auto* move_cc = move_obj->getComponent<engine::component::ColliderComponent>();
    auto* solid_cc = solid_obj->getComponent<engine::component::ColliderComponent>();

    // 这里只能获取期望位置，无法获取当前帧初始位置，因此无法进行轴分离碰撞检测
    /* 未来可以进行重构，让这里可以获取初始位置。但是我们展示另外一种处理方法 */
    auto move_aabb = move_cc->getWorldAABB();
    auto solid_aabb = solid_cc->getWorldAABB();

    // --- 使用最小平移向量解决碰撞问题 ---
    auto move_center = move_aabb.position + move_aabb.size / 2.0f;
    auto solid_center = solid_aabb.position + solid_aabb.size / 2.0f;

    // 计算两个包围盒的重叠部分
    auto overlap = glm::vec2(move_aabb.size / 2.0f + solid_aabb.size / 2.0f) - glm::abs(move_center - solid_center);
    if (overlap.x < 0.1f && overlap.y < 0.1f) return;  // 如果重叠部分太小，则认为没有碰撞

    if(overlap.x < overlap.y) {  // 如果重叠部分在x方向上更小，则认为碰撞发生在x方向上（推出x方向平移向量最小）
        if(move_center.x < solid_center.x) {    // 移动物体在左边，让它贴着右边SOLID物体（相当于向左移出重叠部分），y方向正常移动
            move_tc->translate(glm::vec2(-overlap.x, 0.0f));
            // 如果速度为正(向右移动)，则归零 （if判断不可少，否则可能出现错误吸附）
            if(move_pc->velocity_.x > 0.0f) {
                move_pc->velocity_.x = 0.0f;
                move_pc->setCollidedRight(true);
            }
        }
        else {  // 移动物体在右边，让它贴着左边SOLID物体（相当于向右移出重叠部分），y方向正常移动
            move_tc->translate(glm::vec2(overlap.x, 0.0f));
            // 如果速度为负(向左移动)，则归零 （if判断不可少，否则可能出现错误吸附）
            if(move_pc->velocity_.x < 0.0f) {
                move_pc->velocity_.x = 0.0f;
                move_pc->setCollidedLeft(true);
            }
        }
    }
    else {  // 重叠部分在y方向上更小，则认为碰撞发生在y方向上（推出y方向平移向量最小）
        if(move_center.y < solid_center.y) {    // 移动物体在上面，让它贴着下面SOLID物体（相当于向上移出重叠部分），x方向正常移动
            move_tc->translate(glm::vec2(0.0f, -overlap.y));
            if(move_pc->velocity_.y > 0.0f) {
                move_pc->velocity_.y = 0.0f;
                move_pc->setCollidedBelow(true);
            }
        }
        else {  // 移动物体在下面，让它贴着上面SOLID物体（相当于向下移出重叠部分），x方向正常移动
            move_tc->translate(glm::vec2(0.0f, overlap.y));
            if(move_pc->velocity_.y < 0.0f) {
                move_pc->velocity_.y = 0.0f;
                move_pc->setCollidedAbove(true);
            }
        }
    }
}

float PhysicsEngine:: getTileHeightAtWidth(float width, engine::component::TileType type, glm::vec2 tile_size) {
    auto rel_x = glm::clamp(width / tile_size.x, 0.0f, 1.0f);
    switch(type) {
        case engine::component::TileType::SLOPE_0_1:    // 左0  右1
            return rel_x * tile_size.y;
        case engine::component::TileType::SLOPE_1_0:    // 左1  右0
            return (1.0f - rel_x) * tile_size.y;
        case engine::component::TileType::SLOPE_0_2:    // 左0  右1/2
            return rel_x * tile_size.y * 0.5f;
        case engine::component::TileType::SLOPE_2_1:    // 左1/2  右1
            return rel_x * tile_size.y * 0.5f + tile_size.y * 0.5f;
        case engine::component::TileType::SLOPE_1_2:    // 左1  右1/2
            return (1.0f - rel_x) * tile_size.y * 0.5f + tile_size.y * 0.5f;
        case engine::component::TileType::SLOPE_2_0:    // 左1/2  右0
            return (1.0f - rel_x) * tile_size.y * 0.5f;
        default:
            return 0.0f;    // 默认返回0，表示没有斜坡
    }
}

void PhysicsEngine::applyWorldBounds(engine::component::PhysicsComponent* pc) {
    if (!pc || !world_bound_) return;

    // 只限定左、上、右边界，不限定下边界，以碰撞盒作为判断依据
    auto* obj = pc->getOwner();
    auto* cc = obj->getComponent<engine::component::ColliderComponent>();
    auto* tc = obj->getComponent<engine::component::TransformComponent>();
    auto world_aabb = cc->getWorldAABB();
    auto obj_pos = world_aabb.position;
    auto obj_size = world_aabb.size;

    // 检查左边界
    if (obj_pos.x < world_bound_->position.x) {
        pc->velocity_.x = 0.0f;
        obj_pos.x = world_bound_->position.x;
        pc->setCollidedLeft(true);
    }
    // 检查上边界
    if (obj_pos.y < world_bound_->position.y) {
        pc->velocity_.y = 0.0f;
        obj_pos.y = world_bound_->position.y;
        pc->setCollidedAbove(true);
    }
    // 检查右边界
    if (obj_pos.x + obj_size.x > world_bound_->position.x + world_bound_->size.x) {
        pc->velocity_.x = 0.0f;
        obj_pos.x = world_bound_->position.x + world_bound_->size.x - obj_size.x;
        pc->setCollidedRight(true);
    }
    // 更新物体位置(使用translate方法，新位置 - 旧位置)
    tc->translate(obj_pos - world_aabb.position);
}

}