#pragma once
#include <memory>
#include <string>
#include <unordered_map>
#include <SDL3_mixer/SDL_mixer.h>

namespace engine::resource {

/**
 * @brief 管理 SDL_mixer 音效 (Mix_Chunk) 和音乐 (Mix_Music)。
 *
 * 提供音频资源的加载和缓存功能。构造失败时会抛出异常。
 * 仅供 ResourceManager 内部使用。
 */
class MusicManager final {
    friend class ResourceManager;
private:
    // MIX_Track的自定义删除器
    struct SDLMIXTrackDeleter {
        void operator()(MIX_Track* track) const {
            MIX_DestroyTrack(track);
        }
    };
    // MIX_Audio的自定义删除器
    struct SDLMIXAudioDeleter {
        void operator()(MIX_Audio* audio) const {
            MIX_DestroyAudio(audio);
        }
    };

    // mixer
    MIX_Mixer* _m_mixer;

    // track轨道，暂时只有一个轨道
    MIX_Track* _m_track;

    // 音效存储 (文件路径 -> MIX_Track)
    // std::unordered_map<std::string, std::unique_ptr<MIX_Track, SDLMIXTrackDeleter>> _m_track;
    // 音乐存储 (文件路径 -> MIX_Audio)
    std::unordered_map<std::string, std::unique_ptr<MIX_Audio, SDLMIXAudioDeleter>> _m_audio;

public:
    /**
     * @brief 构造函数。初始化 MIX_Mixer 并打开音频设备。
     * @throws std::runtime_error 如果 MIX_Mixer 初始化或打开音频设备失败。
     */
    MusicManager();

    ~MusicManager();        ///< @brief 需要手动添加析构函数，清理资源并关闭 SDL_mixer。

private:  // 仅供 ResourceManager 访问的方法
    MIX_Audio* loadAudio(const std::string& file_path);     ///< @brief 从文件路径加载音频
    MIX_Audio* getAudio(const std::string& file_path);      ///< @brief 尝试获取已加载音频的指针，如果未加载则尝试加载
    void unloadAudio(const std::string& file_path);         ///< @brief 卸载指定的音频资源
    void clearAudio();                                      ///< @brief 清空所有音频资源

};

}