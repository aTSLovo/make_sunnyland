#include "musicManager.h"
#include <spdlog/spdlog.h>
namespace engine::resource {
MusicManager::MusicManager() {
    // 初始化SDL_mixer，MIX_Init内部是引用计数，几次MIX_Init就需要几次MIX_Quit
    if(!MIX_Init()) {
        throw std::runtime_error("MusicManager 错误: MIX_Init 失败: " + std::string(SDL_GetError()));
    }

    // 在默认设备上创建mixer
    _m_mixer = MIX_CreateMixerDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, NULL);
    if(!_m_mixer) {
        MIX_Quit(); // 如果MIX_CreateMixerDevice失败，先清理Mix_Init，再抛出异常
        throw std::runtime_error("MusicManager 错误: MIX_CreateMixerDevice 失败: " + std::string(SDL_GetError()));
    }

    // 在mixer上有track才能播放音频，每个轨道都有相应的音频内容，并且所有正在播放的轨道会被混合在一起以生成最终的输出效果。
    _m_track = MIX_CreateTrack(_m_mixer);
    if(!_m_track) {
        MIX_Quit(); // 如果MIX_CreateTrack失败，先清理Mix_Init，再抛出异常
        throw std::runtime_error("MusicManager 错误: MIX_CreateTrack 失败: " + std::string(SDL_GetError()));
    }

    spdlog::trace("MusicManager 构造成功。");
}

MusicManager::~MusicManager() {
    // 立即停止所有音频播放
    MIX_StopAllTracks(_m_mixer, 0);   // 停止_m_mixer上所有音效


    // 清理资源映射（unique_ptrs会调用删除器）
    clearAudio();

    // 关闭音频设备
    MIX_DestroyMixer(_m_mixer);

    // 退出SDL_mixer子系统
    MIX_Quit();
    spdlog::trace("MusicManager 析构成功。");
}

// --- 音频管理 ---
MIX_Audio* MusicManager::loadAudio(const std::string& file_path) {
    // 首先检查缓存
    auto it = _m_audio.find(file_path);
    if (it != _m_audio.end()) {
        return it->second.get();
    }

    // 加载音频
    spdlog::debug("加载音频: {}", file_path);
    MIX_Audio* raw_audio = MIX_LoadAudio(_m_mixer, file_path.c_str(), false);
    if(!raw_audio) {
        spdlog::error("加载音频失败: '{}': {}", file_path, SDL_GetError());
        return nullptr;
    }

    // 使用unique_ptr存储在缓存中
    _m_audio.emplace(file_path, std::unique_ptr<MIX_Audio, SDLMIXAudioDeleter>(raw_audio));
    spdlog::debug("成功加载并缓存音频: {}", file_path);
    return raw_audio;
}

MIX_Audio* MusicManager::getAudio(const std::string& file_path) {
    auto it = _m_audio.find(file_path);
    if (it != _m_audio.end()) {
        return it->second.get();
    }
    spdlog::warn("音频 '{}' 未找到缓存，尝试加载。", file_path);
    return loadAudio(file_path);
}

void MusicManager::unloadAudio(const std::string& file_path) {
    auto it = _m_audio.find(file_path);
    if (it != _m_audio.end()) {
        spdlog::debug("卸载音频: {}", file_path);
        _m_audio.erase(it); // unique_ptr处理MIX_DestroyAudio
    }
    else {
        spdlog::warn("尝试卸载不存在的音频: {}", file_path);
    }
}

void MusicManager::clearAudio() {
    if(!_m_audio.empty()) {
        spdlog::info("正在清理所有 {} 个缓存的音频", _m_audio.size());
        _m_audio.clear();
    }
}

}