#ifndef LOG_TAG
#define LOG_TAG "AudioCollaborativeManager"
#endif

#include "audio_collaborative_manager.h"

#include "iservice_registry.h"
#include "system_ability_definition.h"
#include "bundle_mgr_interface.h"

#include "audio_service_log.h"
#include "audio_errors.h"
#include "audio_manager_base.h"
#include "audio_manager_proxy.h"
#include "audio_server_death_recipient.h"
#include "audio_policy_manager.h"

#include "audio_collaborative_manager.h"

namespace OHOS {
namespace AudioStandard {

AudioCollaborativeManager::AudioCollaborativeManager()
{
    AUDIO_DEBUG_LOG("AudioCollaborativeManager start");
}

AudioCollaborativeManager::~AudioCollaborativeManager()
{
    AUDIO_DEBUG_LOG("AudioCollaborativeManager::~AudioCollaborativeManager");
}

AudioCollaborativeManager *AudioCollaborativeManager::GetInstance()
{
    static AudioCollaborativeManager audioCollaborativeManager;
    return &audioCollaborativeManager;
}

bool AudioCollaborativeManager::IsCollaborativePlaybackSupported()
{
    return AudioPolicyManager::GetInstance().IsCollaborativePlaybackSupported();
}

bool AudioCollaborativeManager::IsCollaborativePlaybackEnabledForDevice(
    const std::shared_ptr<AudioDeviceDescriptor> &selectedAudioDevice)
{
    return AudioPolicyManager::GetInstance().IsCollaborativePlaybackEnabledForDevice(selectedAudioDevice);
}

int32_t AudioCollaborativeManager::SetCollaborativePlaybackEnabledForDevice(
    const std::shared_ptr<AudioDeviceDescriptor> &selectedAudioDevice, bool enabled)
{
    return AudioPolicyManager::GetInstance().SetCollaborativePlaybackEnabledForDevice(
        selectedAudioDevice, enabled);
}


} // AudioStandard
} // OHOS