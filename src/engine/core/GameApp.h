#pragma once
#include <memory>
struct SDL_Window;
struct SDL_Renderer;
namespace engine::core {
class Time;

/**
 * @brief 主游戏应用程序类，初始化SDL，管理游戏循环。
 */
class GameApp final {
private:
    SDL_Window* _m_window = nullptr;
    SDL_Renderer* _m_renderer = nullptr;
    bool isRunning = false;

    std::unique_ptr<engine::core::Time> _m_time;

public:
    GameApp();
    ~GameApp();

    /**
     * @brief 运行游戏应用程序，其中会调用init()，然后进入主循环，离开循环后自动调用close()。
     */
    void run();
    
    // 禁止拷贝和移动
    GameApp(const GameApp&) = delete;
    GameApp& operator=(const GameApp&) = delete;
    GameApp(GameApp&&) = delete;
    GameApp& operator=(GameApp&&) = delete;

private:
    [[nodiscard]] bool init();  // nodiscard 表示该函数返回值不应该被忽略

    /**
     * @brief 处理SDL事件
     */
    void handleEvenets();

    /**
     * @brief 根据delta_time更新游戏画面
     */
    void update(double delta_time);

    /**
     * @brief 渲染
     */
    void render();

    void close();
};
}