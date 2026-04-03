#include "gameApp.h"
#include "time.h"
#include "../resource/resourceManager.h"
#include "../render/camera.h"
#include "../render/renderer.h"
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
    _m_time->setTargetFps(144);
    while(isRunning) {
        _m_time->update();
        double delta_time = _m_time->getDeltaTime();

        handleEvenets();
        update(delta_time);
        render();

        // spdlog::info("delta_time: {}", delta_time);
    }

    close();
}

bool GameApp::init() {
    spdlog::trace("初始化GameApp...");

    if(!initSDL()) return false;
    if(!initTime()) return false;
    if(!initResourceManager()) return false;
    if(!initCamera()) return false;
    if(!initRenderer()) return false;

    // 测试资源管理器
    testResourceManager();

    isRunning = true;
    spdlog::trace("初始化GameAPP完成!");
    return true;
}

void GameApp::handleEvenets() {
    SDL_Event event;
    while(SDL_PollEvent(&event)) {
        if(event.type == SDL_EVENT_QUIT) {
            isRunning = false;
        }
    }
}

void GameApp::update(double delta_time) {
    // 游戏逻辑更新
    testCamera();
}

void GameApp::render() {
    // 1. 清除屏幕
    _m_renderer->clearScreen();

    // 2. 具体渲染代码
    testRenderer();

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

bool GameApp::initSDL() {
    if(!SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO)) {
        spdlog::error("无法初始化SDL，错误：{}", SDL_GetError());
        return false;
    }

    _m_window = SDL_CreateWindow("SunnyLand", 1280, 720, SDL_WINDOW_RESIZABLE);
    if(_m_window == nullptr) {
        spdlog::error("无法创建窗口，错误：{}", SDL_GetError());
        return false;
    }

    _m_sdl_renderer = SDL_CreateRenderer(_m_window, NULL);
    if(_m_sdl_renderer == nullptr) {
        spdlog::error("无法创建渲染器，错误：{}", SDL_GetError());
        return false;
    }

    // 设置逻辑分辨率
    SDL_SetRenderLogicalPresentation(_m_sdl_renderer, 640, 360, SDL_LOGICAL_PRESENTATION_LETTERBOX);
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
        _m_camera = std::make_unique<engine::render::Camera>(glm::vec2(640, 360));
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

void GameApp::testResourceManager() {
    _m_resource_manager->getFont("assets/fonts/VonwaonBitmap-16px.ttf", 16);
    _m_resource_manager->getAudio("assets/audio/button_click.wav");
    _m_resource_manager->getTexture("assets/textures/Actors/eagle-attack.png");

    _m_resource_manager->unloadFont("assets/fonts/VonwaonBitmap-16px.ttf", 16);
    _m_resource_manager->unloadAudio("assets/audio/button_click.wav");
    _m_resource_manager->unloadTexture("assets/textures/Actors/eagle-attack.png");
}

void GameApp::testCamera() {
    auto key_state = SDL_GetKeyboardState(nullptr);
    if (key_state[SDL_SCANCODE_UP]) _m_camera->move(glm::vec2(0, -1));   
    if (key_state[SDL_SCANCODE_DOWN]) _m_camera->move(glm::vec2(0, 1));
    if (key_state[SDL_SCANCODE_LEFT]) _m_camera->move(glm::vec2(-1, 0));
    if (key_state[SDL_SCANCODE_RIGHT]) _m_camera->move(glm::vec2(1, 0));
}

void GameApp::testRenderer()
{
    engine::render::Sprite sprite_world("assets/textures/Actors/frog.png");
    engine::render::Sprite sprite_ui("assets/textures/UI/buttons/Start1.png");
    engine::render::Sprite sprite_parallax("assets/textures/Layers/back.png");

    static float rotation = 0.0f;
    rotation += 0.1f;

    // 注意渲染顺序
    _m_renderer->drawParallax(*_m_camera, sprite_parallax, glm::vec2(100, 100), glm::vec2(0.5f, 0.5f), glm::bvec2(true, false));
    _m_renderer->drawSprite(*_m_camera, sprite_world, glm::vec2(200, 200), glm::vec2(1.0f, 1.0f), rotation);
    _m_renderer->drawUISprite(sprite_ui, glm::vec2(100, 100));

}

}