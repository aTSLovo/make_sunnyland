#include "textureManager.h"
#include <spdlog/spdlog.h>
#include <SDL3_image/SDL_image.h>   // 用于 IMG_LoadTexture, IMG_Init, IMG_Quit
#include <stdexcept>

namespace engine::resource {

TextureManager::TextureManager(SDL_Renderer* renderer) : _m_renderer(renderer) {
    if(!_m_renderer) {
        // 关键错误，无法继续，抛出异常 （它将由catch语句捕获（位于GameApp），并进行处理）
        throw std::runtime_error("TextureManager 构造失败: 渲染器指针为空。");
    }
    // SDL3中不再需要手动调用IMG_Init/IMG_Quit
    spdlog::trace("TextureManager 构造成功。");
}

SDL_Texture* TextureManager::loadTexture(const std::string& file_path) {
    // 检查是否已加载
    auto it = _m_textures.find(file_path);
    if(it != _m_textures.end()) {
        return it->second.get();
    }

    // 如果没加载则尝试加载纹理
    SDL_Texture* raw_texture = IMG_LoadTexture(_m_renderer, file_path.c_str());
    
    // 载入纹理时，设置纹理缩放模式为最邻近插值a
    if(!SDL_SetTextureScaleMode(raw_texture, SDL_SCALEMODE_NEAREST)) {
        spdlog::warn("无法设置纹理缩放模式为最邻近插值");
    }
    
    if(!raw_texture) {
        spdlog::error("加载纹理失败: '{}': {}", file_path, SDL_GetError());
        return nullptr;
    }

    // 使用带有自定义删除器的 unique_ptr 存储加载的纹理
    _m_textures.emplace(file_path, std::unique_ptr<SDL_Texture, SDLTextureDeleter>(raw_texture));
    spdlog::debug("成功加载并缓存纹理: {}", file_path);

    return raw_texture;
}

SDL_Texture* TextureManager::getTexture(const std::string& file_path) {
    auto it = _m_textures.find(file_path);
    if(it != _m_textures.end()) {
        return it->second.get();
    }

    // 如果未找到，尝试加载它
    spdlog::warn("纹理 '{}' 未找到缓存，尝试加载。", file_path);
    return loadTexture(file_path);
}

glm::vec2 TextureManager::getTextureSize(const std::string& file_path) {
    // 获取纹理
    SDL_Texture* texture = getTexture(file_path);
    if(!texture) {
        spdlog::error("无法获取纹理: {}", file_path);
        return glm::vec2(0);
    }

    // 获取纹理尺寸
    glm::vec2 size;
    if(!SDL_GetTextureSize(texture, &size.x, &size.y)) {
        spdlog::error("无法查询纹理尺寸: {}", file_path);
        return glm::vec2(0);
    }

    return size;
}

void TextureManager::unloadTexture(const std::string& file_path) {
    auto it = _m_textures.find(file_path);
    if(it != _m_textures.end()) {
        spdlog::debug("卸载纹理: {}", file_path);
        _m_textures.erase(it);
    }
    else {
        spdlog::warn("尝试卸载不存在的纹理: {}", file_path);
    }
}

void TextureManager::clearTextures() {
    if(!_m_textures.empty()) {
        spdlog::debug("正在清除所有 {} 个缓存的纹理。", _m_textures.size());
        _m_textures.clear();
    }
}

}