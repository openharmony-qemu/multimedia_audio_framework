/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
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

int32_t AudioCollaborativeManager::RegisterCollaborationEnabledForCurrentDeviceEventListener(
    const std::shared_ptr<AudioCollaborationEnabledChangeForCurrentDeviceCallback> &callback)
{
    return AudioPolicyManager::GetInstance().RegisterCollaborationEnabledForCurrentDeviceEventListener(callback);
}

int32_t AudioCollaborativeManager::UnregisterCollaborationEnabledForCurrentDeviceEventListener()
{
    return AudioPolicyManager::GetInstance().UnregisterCollaborationEnabledForCurrentDeviceEventListener();
}
} // AudioStandard
} // OHOS