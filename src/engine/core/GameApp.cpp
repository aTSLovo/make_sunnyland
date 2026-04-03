#include "GameApp.h"
#include "Time.h"
#include "../resource/ResourceManager.h"
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
    // 更新，暂时为空
}

void GameApp::render() {
    // 渲染，暂时为空
}

void GameApp::close() {
    spdlog::trace("关闭GameApp...");
    if(_m_renderer != nullptr) {
        SDL_DestroyRenderer(_m_renderer);
        _m_renderer = nullptr;
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

    _m_renderer = SDL_CreateRenderer(_m_window, NULL);
    if(_m_renderer == nullptr) {
        spdlog::error("无法创建渲染器，错误：{}", SDL_GetError());
        return false;
    }
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
        _m_resource_manager = std::make_unique<engine::resource::ResourceManager>(_m_renderer);
    } catch (const std::exception& e) {
        spdlog::error("初始化资源管理器失败: {}", e.what());
        return false;
    }
    spdlog::trace("资源管理器初始化成功。");
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

}