#include "audio_mute_factor_manager.h"
#include "audio_utils.h"

namespace OHOS {
namespace AudioStandard {

AudioMuteFactorManager& AudioMuteFactorManager::GetInstance()
{
    static AudioMuteFactorManager instance_;
    return instance_;
}

AudioMuteFactorManager::AudioMuteFactorManager()
{
    AUDIO_INFO_LOG("AudioMuteFactorManager construct");
}

AudioMuteFactorManager::~AudioMuteFactorManager()
{
    AUDIO_INFO_LOG("AudioMuteFactorManager destruct");
}

bool AudioMuteFactorManager::GetMdmMuteStatus() const
{
    return isMdmMute_;
}

void AudioMuteFactorManager::SetMdmMuteStatus(bool mdmMute)
{
    isMdmMute_ = mdmMute;
}

} // namespace AudioStandard
} // namespace OHOS