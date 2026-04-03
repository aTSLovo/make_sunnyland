#include "time.h"
#include <spdlog/spdlog.h>
#include <SDL3/SDL_timer.h>
namespace engine::core {

Time::Time() {
    last_time_ = SDL_GetTicksNS();
    frame_start_time_ = last_time_;
    spdlog::trace("Time 初始化。Last time: {}", last_time_);
}

void Time::update() {
    frame_start_time_ = SDL_GetTicksNS();
    auto current_delta_time = static_cast<double>(frame_start_time_ - last_time_) / 1000000000.0;
    if(target_frame_time_ > 0.0) {
        limitFrameRate(current_delta_time);
    }
    else {
        delta_time_ = current_delta_time;
    }
    last_time_ = SDL_GetTicksNS();
}

void Time::limitFrameRate(double current_delta_time) {
    // 当前帧耗费时间小于目标等待时间，则等待剩余时间。最后delta_time记录当前帧与上一帧之间的时间间隔
    if(current_delta_time < target_frame_time_) {
        double time_need_wait = target_frame_time_ - current_delta_time;
        Uint64 nstime_need_wait = static_cast<Uint64>(time_need_wait * 1000000000.0);
        SDL_DelayNS(nstime_need_wait);
        delta_time_ = static_cast<double>(SDL_GetTicksNS() - last_time_) / 1000000000.0;
    }
}

double Time::getDeltaTime() const {
    return delta_time_ * time_scale_;
}

double Time::getUnscaledDeltaTime() const {
    return delta_time_;
}

void Time::setTimeScale(double scale) {
    if(scale < 0.0) {
        spdlog::warn("Time scale 不能为负。设置为 0");
        scale = 0.0;
    }
    time_scale_ = scale;
}

double Time::getTimeScale() const {
    return time_scale_;
}

void Time::setTargetFps(int fps) {
    if (fps < 0) {
        spdlog::warn("Target FPS 不能为负。Setting to 0 (unlimited).");
        target_fps_ = 0;
    } else {
        target_fps_ = fps;
    }
    target_fps_ = fps;

    if (target_fps_ > 0) {
        target_frame_time_ = 1.0 / static_cast<double>(target_fps_);
        spdlog::info("Target FPS 设置为: {} (Frame time: {:.6f}s)", target_fps_, target_frame_time_);
    } else {
        target_frame_time_ = 0.0;
        spdlog::info("Target FPS 设置为: Unlimited");
    }
}

int Time::getTargetFps() const {
    return target_fps_;
}

}