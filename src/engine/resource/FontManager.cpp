#include "fontManager.h"
#include <spdlog/spdlog.h>
namespace engine::resource {

FontManager::FontManager() {
    if(!TTF_WasInit() && ! TTF_Init()) {
        throw std::runtime_error("FontManager 错误: TTF_Init 失败：" + std::string(SDL_GetError()));
    }
    spdlog::trace("FontManager 构造成功。");
}

FontManager::~FontManager() {
    if (!_m_font.empty()) {
        spdlog::debug("FontManager 不为空，调用 clearFonts 处理清理逻辑。");
        clearFonts();       // 调用 clearFonts 处理清理逻辑
    }
    TTF_Quit();
    spdlog::trace("FontManager 析构成功。");
}

TTF_Font* FontManager::loadFont(const std::string& file_path, int point_size) {
    if(point_size <= 0) {
        spdlog::error("无法加载字体 '{}'：无效的点大小 {}。", file_path, point_size);
        return nullptr;
    }

    // 创建映射表的键
    FontKey key = {file_path, point_size};

    // 首先检查缓存
    auto it = _m_font.find(key);
    if(it != _m_font.end()) {
        return it->second.get();
    }

    // 缓存中不存在，则加载字体
    spdlog::debug("正在加载字体：{} ({}pt)", file_path, point_size);
    TTF_Font* raw_font = TTF_OpenFont(file_path.c_str(), point_size);
    if (!raw_font) {
        spdlog::error("加载字体 '{}' ({}pt) 失败：{}", file_path, point_size, SDL_GetError());
        return nullptr;
    }
    _m_font.emplace(key, std::unique_ptr<TTF_Font, SDLFontDeleter>(raw_font));
    spdlog::debug("成功加载并缓存字体：{} ({}pt)", file_path, point_size);
    return raw_font;
}

TTF_Font* FontManager::getFont(const std::string& file_path, int point_size) {
    if(point_size <= 0) {
        spdlog::error("无法加载字体 '{}'：无效的点大小 {}。", file_path, point_size);
        return nullptr;
    }

    FontKey key = {file_path, point_size};

    auto it = _m_font.find(key);
    if(it != _m_font.end()) {
        return it->second.get();
    }

    spdlog::warn("字体 '{}' ({}pt) 不在缓存中，尝试加载。", file_path, point_size);
    return loadFont(file_path, point_size);
}

void FontManager::unloadFont(const std::string& file_path, int point_size) {
    if(point_size <= 0) {
        spdlog::error("无法卸载字体 '{}'：无效的点大小 {}。", file_path, point_size);
        return ;
    }

    // 创建映射表的键
    FontKey key = {file_path, point_size};

    // 首先检查缓存
    auto it = _m_font.find(key);
    if(it != _m_font.end()) {
        spdlog::debug("正在卸载字体 '{}' ({}pt)", file_path, point_size);
        _m_font.erase(it);
    }
    else {
        spdlog::warn("尝试卸载不存在的字体：{} ({}pt)", file_path, point_size);
    }
}

void FontManager::clearFonts() {
    if(!_m_font.empty()) {
        spdlog::debug("正在清理所有 {} 个缓存的字体。", _m_font.size());
        _m_font.clear();
    }
}

}