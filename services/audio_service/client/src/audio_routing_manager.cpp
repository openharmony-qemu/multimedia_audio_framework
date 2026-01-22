/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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
#define LOG_TAG "AudioRoutingManager"
#endif

#include "audio_routing_manager.h"
#include "audio_routing_client_manager.h"

namespace OHOS {
namespace AudioStandard {
using namespace std;
AudioRoutingManager *AudioRoutingManager::GetInstance()
{
    static AudioRoutingManager audioRoutingManager;
    return &audioRoutingManager;
}

int32_t AudioRoutingManager::SetMicStateChangeCallback(
    const std::shared_ptr<AudioManagerMicStateChangeCallback> &callback)
{
    return AudioRoutingClientManager::GetInstance().SetMicStateChangeCallback(callback);
}

int32_t AudioRoutingManager::GetPreferredOutputDeviceForRendererInfo(AudioRendererInfo rendererInfo,
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> &desc)
{
    return AudioRoutingClientManager::GetInstance().GetPreferredOutputDeviceForRendererInfo(rendererInfo, desc);
}

int32_t AudioRoutingManager::GetPreferredInputDeviceForCapturerInfo(AudioCapturerInfo captureInfo,
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> &desc)
{
    return AudioRoutingClientManager::GetInstance().GetPreferredInputDeviceForCapturerInfo(captureInfo, desc);
}

RecommendInputDevices AudioRoutingManager::GetRecommendInputDevices(
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> &descs)
{
    AudioCapturerInfo captureInfo;
    captureInfo.sourceType = SOURCE_TYPE_CAMCORDER;
    GetPreferredInputDeviceForCapturerInfo(captureInfo, descs);

    return ConvertRecommendInputDevices(descs);
}

RecommendInputDevices AudioRoutingManager::ConvertRecommendInputDevices(
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> &descs)
{
    if (descs.size() == 0) {
        return RecommendInputDevices::NO_UNAVAILABLE_DEVICE;
    }

    auto it = std::find_if(descs.begin(), descs.end(), [](const auto &desc) {
        return desc && desc->deviceType_ == DEVICE_TYPE_MIC && desc->networkId_ == LOCAL_NETWORK_ID;
    });
    if (it != descs.end()) {
        auto desc = *it;
        descs.clear();
        descs.push_back(desc);
        return RecommendInputDevices::RECOMMEND_BUILT_IN_MIC;
    } else {
        return RecommendInputDevices::RECOMMEND_EXTERNAL_MIC;
    }
}

int32_t AudioRoutingManager::SetPreferredOutputDeviceChangeCallback(AudioRendererInfo rendererInfo,
    const std::shared_ptr<AudioPreferredOutputDeviceChangeCallback>& callback, const int32_t uid)
{
    return AudioRoutingClientManager::GetInstance().SetPreferredOutputDeviceChangeCallback(rendererInfo, callback, uid);
}

int32_t AudioRoutingManager::SetPreferredInputDeviceChangeCallback(AudioCapturerInfo capturerInfo,
    const std::shared_ptr<AudioPreferredInputDeviceChangeCallback> &callback)
{
    return AudioRoutingClientManager::GetInstance().SetPreferredInputDeviceChangeCallback(capturerInfo, callback);
}

int32_t AudioRoutingManager::UnsetPreferredOutputDeviceChangeCallback(
    const std::shared_ptr<AudioPreferredOutputDeviceChangeCallback> &callback)
{
    return AudioRoutingClientManager::GetInstance().UnsetPreferredOutputDeviceChangeCallback(callback);
}

int32_t AudioRoutingManager::UnsetPreferredInputDeviceChangeCallback(
    const std::shared_ptr<AudioPreferredInputDeviceChangeCallback> &callback)
{
    return AudioRoutingClientManager::GetInstance().UnsetPreferredInputDeviceChangeCallback(callback);
}

vector<sptr<MicrophoneDescriptor>> AudioRoutingManager::GetAvailableMicrophones()
{
    return AudioRoutingClientManager::GetInstance().GetAvailableMicrophones();
}

std::vector<std::shared_ptr<AudioDeviceDescriptor>> AudioRoutingManager::GetAvailableDevices(AudioDeviceUsage usage)
{
    return AudioRoutingClientManager::GetInstance().GetAvailableDevices(usage);
}

std::shared_ptr<AudioDeviceDescriptor> AudioRoutingManager::GetActiveBluetoothDevice()
{
    return AudioRoutingClientManager::GetInstance().GetActiveBluetoothDevice();
}

int32_t AudioRoutingManager::SetAudioDeviceRefinerCallback(const std::shared_ptr<AudioDeviceRefiner> &callback)
{
    return AudioRoutingClientManager::GetInstance().SetAudioDeviceRefinerCallback(callback);
}

int32_t AudioRoutingManager::UnsetAudioDeviceRefinerCallback()
{
    return AudioRoutingClientManager::GetInstance().UnsetAudioDeviceRefinerCallback();
}

int32_t AudioRoutingManager::SetPreferredDevice(const PreferredType preferredType,
    const std::shared_ptr<AudioDeviceDescriptor> &desc, const int32_t uid)
{
    return AudioRoutingClientManager::GetInstance().SetPreferredDevice(preferredType, desc, uid);
}

int32_t AudioRoutingManager::RestoreOutputDevice(sptr<AudioRendererFilter> audioRendererFilter)
{
    return AudioRoutingClientManager::GetInstance().RestoreOutputDevice(audioRendererFilter);
}

int32_t AudioRoutingManager::SetDeviceVolumeBehavior(const std::string &networkId,
    DeviceType deviceType, VolumeBehavior volumeBehavior)
{
    return AudioRoutingClientManager::GetInstance().SetDeviceVolumeBehavior(networkId, deviceType, volumeBehavior);
}

int32_t AudioRoutingManager::SetDeviceConnectionStatus(const std::shared_ptr<AudioDeviceDescriptor> &desc,
    const bool isConnected)
{
    return AudioRoutingClientManager::GetInstance().SetDeviceConnectionStatus(desc, isConnected);
}
} // namespace AudioStandard
} // namespace OHOS
