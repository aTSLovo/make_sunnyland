#include "Time.h"
#include <spdlog/spdlog.h>
#include <SDL3/SDL_timer.h>
namespace engine::core {

Time::Time() {
    last_time = SDL_GetTicksNS();
    frame_start_time = last_time;
    spdlog::trace("Time 初始化。Last time: {}", last_time);
}

void Time::update() {
    frame_start_time = SDL_GetTicksNS();
    auto current_delta_time = static_cast<double>(frame_start_time - last_time) / 1000000000.0;
    if(target_frame_time > 0.0) {
        limitFrameRate(current_delta_time);
    }
    else {
        delta_time = current_delta_time;
    }
    last_time = SDL_GetTicksNS();
}

void Time::limitFrameRate(double current_delta_time) {
    // 当前帧耗费时间小于目标等待时间，则等待剩余时间。最后delta_time记录当前帧与上一帧之间的时间间隔
    if(current_delta_time < target_frame_time) {
        double time_need_wait = target_frame_time - current_delta_time;
        Uint64 nstime_need_wait = static_cast<Uint64>(time_need_wait * 1000000000.0);
        SDL_DelayNS(nstime_need_wait);
        delta_time = static_cast<double>(SDL_GetTicksNS() - last_time) / 1000000000.0;
    }
}

double Time::getDeltaTime() const {
    return delta_time * time_scale;
}

double Time::getUnscaledDeltaTime() const {
    return delta_time;
}

void Time::setTimeScale(double scale) {
    if(scale < 0.0) {
        spdlog::warn("Time scale 不能为负。设置为 0");
        scale = 0.0;
    }
    time_scale = scale;
}

double Time::getTimeScale() const {
    return time_scale;
}

void Time::setTargetFps(int fps) {
    target_fps = fps;
}

int Time::getTargetFps() const {
    return target_fps;
}

}