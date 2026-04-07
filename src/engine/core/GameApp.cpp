#include "gameApp.h"
#include "time.h"
#include "config.h"
#include "context.h"
#include "../input/inputManager.h"
#include "../resource/resourceManager.h"
#include "../render/camera.h"
#include "../render/renderer.h"
#include "../scene/sceneManager.h"
#include "../../game/scene/gameScene.h"
#include <SDL3/SDL.h>
#include <spdlog/spdlog.h>

namespace engine::core {



GameApp::GameApp() = default;

GameApp::~GameApp() {
    if(isRunning) {
        spdlog::warn("GameApp被销毁时没有显式关闭, 现在关闭...");
        close();
    }
}

void GameApp::run() {
    if(!init()) {
        spdlog::error("初始化失败，无法运行游戏");
        return ;
    }
    
    while(isRunning) {
        _m_time->update();
        double delta_time = _m_time->getDeltaTime();
        _m_input_manager->update();

        handleEvenets();
        update(delta_time);
        render();

        // spdlog::info("delta_time: {}", delta_time);
    }

    close();
}

bool GameApp::init() {
    spdlog::trace("初始化GameApp...");

    if(!initConfig()) return false;
    if(!initSDL()) return false;
    if(!initTime()) return false;
    if(!initResourceManager()) return false;
    if(!initCamera()) return false;
    if(!initRenderer()) return false;
    if(!initInputManager()) return false;

    if(!initContext()) return false;
    if(!initSceneManager()) return false;

    // 创建第一个场景并压入栈
    auto first_scene = std::make_unique<game::scene::GameScene>("GameApp", *_m_context, *_m_scene_manager);
    _m_scene_manager->requestPushScene(std::move(first_scene));

    isRunning = true;
    spdlog::trace("初始化GameAPP完成!");
    
    return true;
}

void GameApp::handleEvenets() {
    if (_m_input_manager->shouldQuit()) {
        spdlog::trace("GameApp 收到来自 InputManager 的退出请求。");
        isRunning = false;
        return;
    }

    _m_scene_manager->handleInput();
}

void GameApp::update(double delta_time) {
    // 游戏逻辑更新
    _m_scene_manager->update(delta_time);
}

void GameApp::render() {
    // 1. 清除屏幕
    _m_renderer->clearScreen();

    // 2. 具体渲染代码
    _m_scene_manager->render();

    // 3. 更新屏幕显示
    _m_renderer->present();
}

void GameApp::close() {
    spdlog::trace("关闭GameApp...");

    // 为了确保正确的销毁顺序，有些智能指针对象也需要手动管理
    _m_resource_manager.reset();

    if(_m_sdl_renderer != nullptr) {
        SDL_DestroyRenderer(_m_sdl_renderer);
        _m_sdl_renderer = nullptr;
    }
    if(_m_window != nullptr) {
        SDL_DestroyWindow(_m_window);
        _m_window = nullptr;
    }
    SDL_Quit();
    isRunning = false;
}

bool GameApp::initConfig() {
    try {
        _m_config = std::make_unique<engine::core::Config>("assets/config.json");
    } catch (const std::exception& e) {
        spdlog::error("初始化配置失败: {}", e.what());
        return false;
    }
    spdlog::trace("配置初始化成功。");
    return true;
}

bool GameApp::initSDL() {
    if(!SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO)) {
        spdlog::error("无法初始化SDL，错误：{}", SDL_GetError());
        return false;
    }

    _m_window = SDL_CreateWindow(_m_config->window_title_.c_str(), _m_config->window_width_, _m_config->window_height_, SDL_WINDOW_RESIZABLE);
    if(_m_window == nullptr) {
        spdlog::error("无法创建窗口，错误：{}", SDL_GetError());
        return false;
    }

    _m_sdl_renderer = SDL_CreateRenderer(_m_window, NULL);
    if(_m_sdl_renderer == nullptr) {
        spdlog::error("无法创建渲染器，错误：{}", SDL_GetError());
        return false;
    }

    // 设置 VSync (注意: VSync 开启时，驱动程序会尝试将帧率限制到显示器刷新率，有可能会覆盖我们手动设置的 target_fps)
    int vsync_mode = _m_config->vsync_enabled_ ? SDL_RENDERER_VSYNC_ADAPTIVE : SDL_RENDERER_VSYNC_DISABLED;
    SDL_SetRenderVSync(_m_sdl_renderer, vsync_mode);
    spdlog::trace("VSync 设置为: {}", _m_config->vsync_enabled_ ? "Enabled" : "Disabled");

    // 设置逻辑分辨率
    SDL_SetRenderLogicalPresentation(_m_sdl_renderer,  _m_config->window_width_ / 2, _m_config->window_height_ / 2, SDL_LOGICAL_PRESENTATION_LETTERBOX);
    spdlog::trace("SDL 初始化成功。");
    return true;
}

bool GameApp::initTime() {
    try {
        _m_time = std::make_unique<Time>();
    } catch (const std::exception& e) {
        spdlog::error("初始化时间管理失败: {}", e.what());
        return false;
    }
    _m_time->setTargetFps(_m_config->target_fps_);
    spdlog::trace("时间管理初始化成功。");
    return true;
}

bool GameApp::initResourceManager() {
    try {
        _m_resource_manager = std::make_unique<engine::resource::ResourceManager>(_m_sdl_renderer);
    } catch (const std::exception& e) {
        spdlog::error("初始化资源管理器失败: {}", e.what());
        return false;
    }
    spdlog::trace("资源管理器初始化成功。");
    return true;
}

bool GameApp::initCamera() {
    try {
        _m_camera = std::make_unique<engine::render::Camera>(glm::vec2(_m_config->window_width_ / 2, _m_config->window_height_ / 2));
    } catch (const std::exception& e) {
        spdlog::error("初始化相机失败: {}", e.what());
        return false;
    }
    spdlog::trace("相机初始化成功。");
    return true;
}

bool GameApp::initRenderer() {
    try {
        _m_renderer = std::make_unique<engine::render::Renderer>(_m_sdl_renderer, _m_resource_manager.get());
    } catch (const std::exception& e) {
        spdlog::error("初始化渲染器失败: {}", e.what());
        return false;
    }
    spdlog::trace("渲染器初始化成功。");
    return true;
}

bool GameApp::initInputManager() {
    try {
        _m_input_manager = std::make_unique<engine::input::InputManager>(_m_sdl_renderer, _m_config.get());
    } catch (const std::exception& e) {
        spdlog::error("初始化输入管理器失败: {}", e.what());
        return false;
    }
    spdlog::trace("初始化输入管理器成功。");
    return true;
}

bool GameApp::initContext() {
    try {
        _m_context = std::make_unique<engine::core::Context>(
            *_m_input_manager,
            *_m_camera,
            *_m_renderer,
            *_m_resource_manager
        );
    } catch (const std::exception& e) {
        spdlog::error("初始化上下文失败: {}", e.what());
        return false;
    }
    return true;
}

bool GameApp::initSceneManager() {
    try {
        _m_scene_manager = std::make_unique<engine::scene::SceneManager>(*_m_context);
    } catch (const std::exception& e) {
        spdlog::error("初始化场景管理器失败: {}", e.what());
        return false;
    }
    return true;
}

}