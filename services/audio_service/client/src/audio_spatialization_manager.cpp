/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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
#define LOG_TAG "AudioSpatializationManager"
#endif

#include "audio_spatialization_manager.h"

#include "iservice_registry.h"
#include "system_ability_definition.h"
#include "bundle_mgr_interface.h"

#include "audio_service_log.h"
#include "audio_errors.h"
#include "audio_manager_base.h"
#include "audio_server_death_recipient.h"
#include "audio_policy_manager.h"

namespace OHOS {
namespace AudioStandard {
using namespace std;

AudioSpatializationManager::AudioSpatializationManager()
{
    AUDIO_DEBUG_LOG("AudioSpatializationManager start");
}

AudioSpatializationManager::~AudioSpatializationManager()
{
    AUDIO_DEBUG_LOG("AudioSpatializationManager::~AudioSpatializationManager");
}

AudioSpatializationManager *AudioSpatializationManager::GetInstance()
{
    static AudioSpatializationManager audioSpatializationManager;
    return &audioSpatializationManager;
}

bool AudioSpatializationManager::IsSpatializationEnabled()
{
    return AudioPolicyManager::GetInstance().IsSpatializationEnabled();
}

bool AudioSpatializationManager::IsSpatializationEnabled(
    const std::shared_ptr<AudioDeviceDescriptor> &selectedAudioDevice)
{
    CHECK_AND_RETURN_RET_LOG(selectedAudioDevice != nullptr, false, "selectedAudioDevice is nullptr");
    return AudioPolicyManager::GetInstance().IsSpatializationEnabled(selectedAudioDevice->macAddress_);
}

bool AudioSpatializationManager::IsSpatializationEnabledForCurrentDevice()
{
    return AudioPolicyManager::GetInstance().IsSpatializationEnabledForCurrentDevice();
}

int32_t AudioSpatializationManager::SetSpatializationEnabled(const bool enable)
{
    return AudioPolicyManager::GetInstance().SetSpatializationEnabled(enable);
}

int32_t AudioSpatializationManager::SetSpatializationEnabled(
    const std::shared_ptr<AudioDeviceDescriptor> &selectedAudioDevice, const bool enable)
{
    return AudioPolicyManager::GetInstance().SetSpatializationEnabled(selectedAudioDevice, enable);
}

bool AudioSpatializationManager::IsHeadTrackingEnabled()
{
    return AudioPolicyManager::GetInstance().IsHeadTrackingEnabled();
}

bool AudioSpatializationManager::IsHeadTrackingEnabled(
    const std::shared_ptr<AudioDeviceDescriptor> &selectedAudioDevice)
{
    CHECK_AND_RETURN_RET_LOG(selectedAudioDevice != nullptr, false, "selectedAudioDevice is nullptr");
    return AudioPolicyManager::GetInstance().IsHeadTrackingEnabled(selectedAudioDevice->macAddress_);
}

int32_t AudioSpatializationManager::SetHeadTrackingEnabled(const bool enable)
{
    return AudioPolicyManager::GetInstance().SetHeadTrackingEnabled(enable);
}

int32_t AudioSpatializationManager::SetHeadTrackingEnabled(
    const std::shared_ptr<AudioDeviceDescriptor> &selectedAudioDevice, const bool enable)
{
    return AudioPolicyManager::GetInstance().SetHeadTrackingEnabled(selectedAudioDevice, enable);
}

int32_t AudioSpatializationManager::RegisterSpatializationEnabledEventListener(
    const std::shared_ptr<AudioSpatializationEnabledChangeCallback> &callback)
{
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, ERR_INVALID_PARAM, "callback is null");
    return AudioPolicyManager::GetInstance().RegisterSpatializationEnabledEventListener(callback);
}

int32_t AudioSpatializationManager::RegisterSpatializationEnabledForCurrentDeviceEventListener(
    const std::shared_ptr<AudioSpatializationEnabledChangeForCurrentDeviceCallback> &callback)
{
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, ERR_INVALID_PARAM, "callback is null");
    return AudioPolicyManager::GetInstance().RegisterSpatializationEnabledForCurrentDeviceEventListener(callback);
}

int32_t AudioSpatializationManager::RegisterHeadTrackingEnabledEventListener(
    const std::shared_ptr<AudioHeadTrackingEnabledChangeCallback> &callback)
{
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, ERR_INVALID_PARAM, "callback is null");
    return AudioPolicyManager::GetInstance().RegisterHeadTrackingEnabledEventListener(callback);
}

int32_t AudioSpatializationManager::UnregisterSpatializationEnabledEventListener()
{
    return AudioPolicyManager::GetInstance().UnregisterSpatializationEnabledEventListener();
}

int32_t AudioSpatializationManager::UnregisterSpatializationEnabledForCurrentDeviceEventListener()
{
    return AudioPolicyManager::GetInstance().UnregisterSpatializationEnabledForCurrentDeviceEventListener();
}

int32_t AudioSpatializationManager::UnregisterHeadTrackingEnabledEventListener()
{
    return AudioPolicyManager::GetInstance().UnregisterHeadTrackingEnabledEventListener();
}

bool AudioSpatializationManager::IsSpatializationSupported()
{
    return AudioPolicyManager::GetInstance().IsSpatializationSupported();
}

bool AudioSpatializationManager::IsSpatializationSupportedForDevice(
    const std::shared_ptr<AudioDeviceDescriptor> &selectedAudioDevice)
{
    CHECK_AND_RETURN_RET_LOG(selectedAudioDevice != nullptr, false, "selectedAudioDevice is nullptr");
    return AudioPolicyManager::GetInstance().IsSpatializationSupportedForDevice(selectedAudioDevice->macAddress_);
}

bool AudioSpatializationManager::IsHeadTrackingSupported()
{
    return AudioPolicyManager::GetInstance().IsHeadTrackingSupported();
}

bool AudioSpatializationManager::IsHeadTrackingSupportedForDevice(
    const std::shared_ptr<AudioDeviceDescriptor> &selectedAudioDevice)
{
    CHECK_AND_RETURN_RET_LOG(selectedAudioDevice != nullptr, false, "selectedAudioDevice is nullptr");
    return AudioPolicyManager::GetInstance().IsHeadTrackingSupportedForDevice(selectedAudioDevice->macAddress_);
}

int32_t AudioSpatializationManager::UpdateSpatialDeviceState(const AudioSpatialDeviceState audioSpatialDeviceState)
{
    return AudioPolicyManager::GetInstance().UpdateSpatialDeviceState(audioSpatialDeviceState);
}

AudioSpatializationSceneType AudioSpatializationManager::GetSpatializationSceneType()
{
    return AudioPolicyManager::GetInstance().GetSpatializationSceneType();
}

int32_t AudioSpatializationManager::SetSpatializationSceneType(
    const AudioSpatializationSceneType spatializationSceneType)
{
    return AudioPolicyManager::GetInstance().SetSpatializationSceneType(spatializationSceneType);
}

bool AudioSpatializationManager::IsHeadTrackingDataRequested(const std::string &macAddress)
{
    return AudioPolicyManager::GetInstance().IsHeadTrackingDataRequested(macAddress);
}

int32_t AudioSpatializationManager::RegisterHeadTrackingDataRequestedEventListener(const std::string &macAddress,
    const std::shared_ptr<HeadTrackingDataRequestedChangeCallback> &callback)
{
    return AudioPolicyManager::GetInstance().RegisterHeadTrackingDataRequestedEventListener(macAddress, callback);
}

int32_t AudioSpatializationManager::UnregisterHeadTrackingDataRequestedEventListener(const std::string &macAddress)
{
    return AudioPolicyManager::GetInstance().UnregisterHeadTrackingDataRequestedEventListener(macAddress);
}
} // namespace AudioStandard
} // namespace OHOS
