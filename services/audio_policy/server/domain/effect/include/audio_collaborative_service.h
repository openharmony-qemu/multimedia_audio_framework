#ifndef ST_AUDIO_COLLABORATIVE_SERVICE_H
#define ST_AUDIO_COLLABORATIVE_SERVICE_H
#include <mutex>
#include <map>
#include "audio_device_descriptor.h"
#include "audio_effect.h"
#include "audio_policy_manager_factory.h"
#include "iaudio_policy_interface.h"
namespace OHOS {
namespace AudioStandard {
class AudioCollaborativeService {
public:
    static AudioCollaborativeService& GetAudioCollaborativeService()
    {
        static AudioCollaborativeService audioCollaborativeService;
        return audioCollaborativeService;
    }
    void Init(const std::vector<EffectChain> &effectChains);
    bool IsCollaborativePlaybackSupported();
    bool IsCollaborativePlaybackEnabledForDevice(const std::shared_ptr<AudioDeviceDescriptor> &selectedAudioDevice);
    // only function to change map state
    int32_t SetCollaborativePlaybackEnabledForDevice(const std::shared_ptr<AudioDeviceDescriptor> &selectedAudioDevice, bool enabled);
    // current device change, map state unchanged
    void UpdateCurrentDevice(const AudioDeviceDescriptor &selectedAudioDevice);
private:
    AudioCollaborativeService()
        :audioPolicyManager_(AudioPolicyManagerFactory::GetAudioPolicyManager())
    {}
    ~AudioCollaborativeService();
    // outputDeviceChange differentiate if updation is caused by output device change
    int32_t UpdateCollaborativeStateReal();
    bool isCollaborativePlaybackSupported_ = false;
    // same with current device in map
    bool isCollaborativeStateEnabled_ = false;
    std::string curDeviceAddress_;
    std::mutex collaborativeServiceMutex_;
    std::map<std::string, bool> addressToCollaborativeEnabledMap_;
    std::map<std::string, bool> addressToCollaborativeMemoryMap_;
    IAudioPolicyInterface& audioPolicyManager_;
};
} // OHOS
} // AudioStandard
#endif