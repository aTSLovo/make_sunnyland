#include "ResourceManager.h"
#include "FontManager.h"
#include "MusicManager.h"
#include "TextureManager.h"

#include <SDL3_mixer/SDL_mixer.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <glm/glm.hpp>
#include <spdlog/spdlog.h>

namespace engine::resource {

ResourceManager::~ResourceManager() = default;

ResourceManager::ResourceManager(SDL_Renderer* renderer) {
    // --- 初始化各个子系统 --- (如果出现错误会抛出异常，由上层捕获)
    _m_font_manager = std::make_unique<FontManager>();
    _m_music_manager = std::make_unique<MusicManager>();
    _m_texture_manager = std::make_unique<TextureManager>(renderer);

    spdlog::trace("ResourceManager 构造成功。");
    // RAII: 构造成功即代表资源管理器可以正常工作，无需再初始化，无需检查指针是否为空
}

void ResourceManager::clear() {
    _m_font_manager->clearFonts();
    _m_music_manager->clearAudio();
    _m_texture_manager->clearTextures();
    spdlog::trace("ResourceManager 中的资源通过 clear() 清空。");
}

// --- 字体接口实现 ---
TTF_Font* ResourceManager::loadFont(const std::string& file_path, int point_size) {
    return _m_font_manager->loadFont(file_path, point_size);
}
TTF_Font* ResourceManager::getFont(const std::string& file_path, int point_size) {
    return _m_font_manager->getFont(file_path, point_size);
}
void ResourceManager::unloadFont(const std::string& file_path, int point_size) {
    _m_font_manager->unloadFont(file_path, point_size);
}
void ResourceManager::clearFonts() {
    _m_font_manager->clearFonts();
}

// --- 音频接口实现 ---
MIX_Audio* ResourceManager::loadAudio(const std::string& file_path) {
    return _m_music_manager->loadAudio(file_path);
}
MIX_Audio* ResourceManager::getAudio(const std::string& file_path) {
    return _m_music_manager->getAudio(file_path);
}
void ResourceManager::unloadAudio(const std::string& file_path) {
    _m_music_manager->unloadAudio(file_path);
}
void ResourceManager::clearAudio() {
    _m_music_manager->clearAudio();
}                   

// --- 纹理接口实现 ---
SDL_Texture* ResourceManager::loadTexture(const std::string& file_path) {
    return _m_texture_manager->loadTexture(file_path);
}
SDL_Texture* ResourceManager::getTexture(const std::string& file_path) {
    return _m_texture_manager->getTexture(file_path);
}
glm::vec2 ResourceManager::getTextureSize(const std::string& file_path) {
    return _m_texture_manager->getTextureSize(file_path);
}
void ResourceManager::unloadTexture(const std::string& file_path){
    _m_texture_manager->unloadTexture(file_path);
}
void ResourceManager::clearTextures() {
    _m_texture_manager->clearTextures();
}

}