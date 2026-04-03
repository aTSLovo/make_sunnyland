#pragma once
#include <memory>   // 用于 std::unique_ptr
#include <string>   // 用于 std::string
#include <glm/glm.hpp>

// 前向声明 SDL 类型
struct SDL_Renderer;
struct SDL_Texture;
struct TTF_Font;
struct MIX_Audio;
struct MIX_Track;

namespace engine::resource {

// 前向声明内部管理器
class FontManager;
class MusicManager;
class TextureManager;

/**
 * @brief 作为访问各种资源管理器的中央控制点（外观模式 Facade）。
 * 在构造时初始化其管理的子系统。构造失败会抛出异常。
 */
class ResourceManager final {
private:
    // 使用 unique_ptr 确保所有权和自动清理
    std::unique_ptr<FontManager> _m_font_manager;
    std::unique_ptr<MusicManager> _m_music_manager;
    std::unique_ptr<TextureManager> _m_texture_manager;

public:
    /**
     * @brief 构造函数，执行初始化。
     * @param renderer SDL_Renderer 的指针，传递给需要它的子管理器。不能为空。
     */
    explicit ResourceManager(SDL_Renderer* renderer);       // explicit 关键字用于防止隐式转换, 对于单一参数的构造函数，通常考虑添加

    ~ResourceManager();  // 显式声明析构函数，这是为了能让智能指针正确管理仅有前向声明的类

    void clear();        ///< @brief 清空所有资源

    // 当前设计中，我们只需要一个ResourceManager，所有权不变，所以不需要拷贝、移动相关构造及赋值运算符
    ResourceManager(const ResourceManager&) = delete;
    ResourceManager& operator=(const ResourceManager&) = delete;
    ResourceManager(ResourceManager&&) = delete;
    ResourceManager& operator=(ResourceManager&&) = delete;

    // --- 统一资源访问接口 ---
    // -- Font --
    TTF_Font* loadFont(const std::string& file_path, int point_size);   ///< @brief 从文件路径加载指定点大小的字体
    TTF_Font* getFont(const std::string& file_path, int point_size);    ///< @brief 尝试获取已加载字体的指针，如果未加载则尝试加载
    void unloadFont(const std::string& file_path, int point_size);      ///< @brief 卸载特定字体（通过路径和大小标识）
    void clearFonts();                                                  ///< @brief 清空所有缓存的字体

    // -- Music --
    MIX_Audio* loadAudio(const std::string& file_path);     ///< @brief 从文件路径加载音频
    MIX_Audio* getAudio(const std::string& file_path);      ///< @brief 尝试获取已加载音频的指针，如果未加载则尝试加载
    void unloadAudio(const std::string& file_path);         ///< @brief 卸载指定的音频资源
    void clearAudio();                                      ///< @brief 清空所有音频资源

    // -- Texture --
    SDL_Texture* loadTexture(const std::string& file_path);      ///< @brief 从文件路径加载纹理
    SDL_Texture* getTexture(const std::string& file_path);       ///< @brief 尝试获取已加载纹理的指针，如果未加载则尝试加载
    glm::vec2 getTextureSize(const std::string& file_path);      ///< @brief 获取指定纹理的尺寸
    void unloadTexture(const std::string& file_path);            ///< @brief 卸载指定的纹理资源
    void clearTextures();                                        ///< @brief 清空所有纹理资源

};

}