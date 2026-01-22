/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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
#define LOG_TAG "AudioRoutingClientManager"
#endif

#include "audio_routing_client_manager.h"

#include "audio_common_log.h"
#include "audio_errors.h"
#include "audio_policy_manager.h"
#include "audio_type_convert.h"
#include "audio_volume_client_manager.h"

namespace OHOS {
namespace AudioStandard {
AudioRoutingClientManager &AudioRoutingClientManager::GetInstance()
{
    static AudioRoutingClientManager instance;
    return instance;
}

int32_t AudioRoutingClientManager::SetMicStateChangeCallback(
    const std::shared_ptr<AudioManagerMicStateChangeCallback> &callback)
{
    auto groupManager = AudioVolumeClientManager::GetInstance().GetGroupManager(DEFAULT_VOLUME_GROUP_ID);
    CHECK_AND_RETURN_RET_LOG(groupManager != nullptr, ERR_INVALID_PARAM,
        "setMicrophoneMuteCallback falied, groupManager is null");
    return groupManager->SetMicStateChangeCallback(callback);
}

int32_t AudioRoutingClientManager::GetPreferredOutputDeviceForRendererInfo(AudioRendererInfo rendererInfo,
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> &desc)
{
    desc = AudioPolicyManager::GetInstance().GetPreferredOutputDeviceDescriptors(rendererInfo);

    return SUCCESS;
}

int32_t AudioRoutingClientManager::GetPreferredInputDeviceForCapturerInfo(AudioCapturerInfo captureInfo,
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> &desc)
{
    desc = AudioPolicyManager::GetInstance().GetPreferredInputDeviceDescriptors(captureInfo);

    return SUCCESS;
}

int32_t AudioRoutingClientManager::SetPreferredOutputDeviceChangeCallback(AudioRendererInfo rendererInfo,
    const std::shared_ptr<AudioPreferredOutputDeviceChangeCallback>& callback, const int32_t uid)
{
    AUDIO_INFO_LOG("Entered %{public}s", __func__);
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, ERR_INVALID_PARAM, "callback is nullptr");

    return AudioPolicyManager::GetInstance().SetPreferredOutputDeviceChangeCallback(rendererInfo, callback, uid);
}

int32_t AudioRoutingClientManager::SetPreferredInputDeviceChangeCallback(AudioCapturerInfo capturerInfo,
    const std::shared_ptr<AudioPreferredInputDeviceChangeCallback> &callback)
{
    AUDIO_INFO_LOG("Entered %{public}s", __func__);
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, ERR_INVALID_PARAM, "callback is nullptr");

    return AudioPolicyManager::GetInstance().SetPreferredInputDeviceChangeCallback(capturerInfo, callback);
}

int32_t AudioRoutingClientManager::UnsetPreferredOutputDeviceChangeCallback(
    const std::shared_ptr<AudioPreferredOutputDeviceChangeCallback> &callback)
{
    AUDIO_INFO_LOG("Entered %{public}s", __func__);
    return AudioPolicyManager::GetInstance().UnsetPreferredOutputDeviceChangeCallback(callback);
}

int32_t AudioRoutingClientManager::UnsetPreferredInputDeviceChangeCallback(
    const std::shared_ptr<AudioPreferredInputDeviceChangeCallback> &callback)
{
    AUDIO_INFO_LOG("Entered %{public}s", __func__);
    return AudioPolicyManager::GetInstance().UnsetPreferredInputDeviceChangeCallback(callback);
}

std::vector<sptr<MicrophoneDescriptor>> AudioRoutingClientManager::GetAvailableMicrophones()
{
    return AudioPolicyManager::GetInstance().GetAvailableMicrophones();
}

std::vector<std::shared_ptr<AudioDeviceDescriptor>> AudioRoutingClientManager::GetAvailableDevices(
    AudioDeviceUsage usage)
{
    return AudioPolicyManager::GetInstance().GetAvailableDevices(usage);
}

std::shared_ptr<AudioDeviceDescriptor> AudioRoutingClientManager::GetActiveBluetoothDevice()
{
    return AudioPolicyManager::GetInstance().GetActiveBluetoothDevice();
}

int32_t AudioRoutingClientManager::SetAudioDeviceRefinerCallback(const std::shared_ptr<AudioDeviceRefiner> &callback)
{
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, ERR_INVALID_PARAM, "callback is nullptr");

    return AudioPolicyManager::GetInstance().SetAudioDeviceRefinerCallback(callback);
}

int32_t AudioRoutingClientManager::UnsetAudioDeviceRefinerCallback()
{
    return AudioPolicyManager::GetInstance().UnsetAudioDeviceRefinerCallback();
}

int32_t AudioRoutingClientManager::SetPreferredDevice(const PreferredType preferredType,
    const std::shared_ptr<AudioDeviceDescriptor> &desc, const int32_t uid)
{
    return AudioPolicyManager::GetInstance().SetPreferredDevice(preferredType, desc, uid);
}

int32_t AudioRoutingClientManager::RestoreOutputDevice(sptr<AudioRendererFilter> audioRendererFilter)
{
    CHECK_AND_RETURN_RET_LOG(audioRendererFilter != nullptr, ERR_INVALID_PARAM, "invalid parameter");

    audioRendererFilter->streamType = AudioTypeConvert::GetStreamType(
        audioRendererFilter->rendererInfo.contentType, audioRendererFilter->rendererInfo.streamUsage);

    CHECK_AND_RETURN_RET_LOG(audioRendererFilter->uid >= -1, ERR_INVALID_PARAM, "invalid uid.");

    AUDIO_DEBUG_LOG("[%{public}d] RestoreOutputDevice: uid<%{public}d> streamType<%{public}d>",
        getpid(), audioRendererFilter->uid, static_cast<int32_t>(audioRendererFilter->streamType));

    return AudioPolicyManager::GetInstance().RestoreOutputDevice(audioRendererFilter);
}

int32_t AudioRoutingClientManager::SetDeviceVolumeBehavior(const std::string &networkId,
    DeviceType deviceType, VolumeBehavior volumeBehavior)
{
    return AudioPolicyManager::GetInstance().SetDeviceVolumeBehavior(networkId, deviceType, volumeBehavior);
}

int32_t AudioRoutingClientManager::SetDeviceConnectionStatus(const std::shared_ptr<AudioDeviceDescriptor> &desc,
    const bool isConnected)
{
    CHECK_AND_RETURN_RET_LOG(desc != nullptr, ERR_INVALID_PARAM, "desc is nullptr");
    return AudioPolicyManager::GetInstance().SetDeviceConnectionStatus(desc, isConnected);
}

int32_t AudioRoutingClientManager::SetCustomAudioMix(const std::string &zoneName, const std::vector<AudioZoneMix> &audioMixes)
{
    CHECK_AND_RETURN_RET_LOG(zoneName != "" && audioMixes.size() > 0, ERR_INVALID_PARAM,
                             "zoneName is empty or audioMix is empty.");
    return AudioPolicyManager::GetInstance().SetCustomAudioMix(zoneName, audioMixes);
}
} // namespace AudioStandard
} // namespace OHOS
