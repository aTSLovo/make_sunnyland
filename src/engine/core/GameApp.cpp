#include "GameApp.h"
#include "Time.h"
#include <SDL3/SDL.h>
#include <spdlog/spdlog.h>

namespace engine::core {

GameApp::GameApp() {
    _m_time = std::make_unique<Time>();
}

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

    isRunning = true;
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

}