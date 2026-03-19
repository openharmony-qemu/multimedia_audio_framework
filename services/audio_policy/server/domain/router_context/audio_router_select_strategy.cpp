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
#define LOG_TAG "AudioRouterSelectStrategy"
#endif

#include "audio_router_select_strategy.h"
#include "audio_router_follow_strategy.h"
#include "audio_router_independent_strategy.h"
#include "audio_router_infra.h"
#include "audio_device_manager.h"
#include "audio_policy_log.h"
#include <algorithm>

namespace OHOS {
namespace AudioStandard {

AudioRouterSelectStrategy& AudioRouterSelectStrategy::GetInstance()
{
    static AudioRouterIndependentStrategy independent;
    static AudioRouterFollowStrategy follow;

    // 这儿要根据配置以及是否PC模式决定走独立还是跟随策略todo
    return follow;
}

std::shared_ptr<AudioDeviceDescriptor> AudioRouterSelectStrategy::ConvertSimpleToDevice(
    const std::shared_ptr<AudioDeviceSimpleDescriptor> &simpleDesc)
{
    if (simpleDesc == nullptr) {
        return nullptr;
    }
    return simpleDesc->GetOnlineDeviceDescriptor();
}

std::shared_ptr<AudioDeviceSimpleDescriptor> AudioRouterSelectStrategy::ConvertDeviceToSimple(
    const std::shared_ptr<AudioDeviceDescriptor> &deviceDesc)
{
    if (deviceDesc == nullptr) {
        return nullptr;
    }
    return std::make_shared<AudioDeviceSimpleDescriptor>(deviceDesc->deviceType_,
        deviceDesc->deviceRole_, deviceDesc->deviceId_, deviceDesc->networkId_);
}

std::vector<std::shared_ptr<AudioDeviceDescriptor>> AudioRouterSelectStrategy::ConvertSimpleToDevice(
    const std::vector<std::shared_ptr<AudioDeviceSimpleDescriptor>> &simpleDescs)
{
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> devices;
    for (const auto &simpleDesc : simpleDescs) {
        auto device = ConvertSimpleToDevice(simpleDesc);
        if (device == nullptr) {
            devices.clear();
            return devices;
        }
        devices.push_back(device);
    }
    return devices;
}

std::vector<std::shared_ptr<AudioDeviceSimpleDescriptor>> AudioRouterSelectStrategy::ConvertDeviceToSimple(
    const std::vector<std::shared_ptr<AudioDeviceDescriptor>> &deviceDescs)
{
    std::vector<std::shared_ptr<AudioDeviceSimpleDescriptor>> simpleDevices;
    for (const auto &device : deviceDescs) {
        auto simpleDesc = ConvertDeviceToSimple(device);
        if (simpleDesc == nullptr) {
            simpleDevices.clear();
            return simpleDevices;
        }
        simpleDevices.push_back(simpleDesc);
    }
    return simpleDevices;
}

std::shared_ptr<AudioDeviceDescriptor> AudioRouterSelectStrategy::JudgeFinalSelectDevice(
    const std::shared_ptr<AudioDeviceDescriptor> &desc, SourceType sourceType,
    BluetoothAndNearlinkPreferredRecordCategory category)
{
    if (desc == nullptr) {
        return nullptr;
    }

    bool isConnected = AudioDeviceManager::GetAudioDeviceManager().IsConnectedDevices(desc);

    if (desc->deviceType_ != DEVICE_TYPE_BLUETOOTH_SCO || category == PREFERRED_LOW_LATENCY) {
        return isConnected ? desc : std::make_shared<AudioDeviceDescriptor>();
    }

    bool isHighQuality = (sourceType == SOURCE_TYPE_MIC || sourceType == SOURCE_TYPE_CAMCORDER ||
                        sourceType == SOURCE_TYPE_LIVE || category == PREFERRED_HIGH_QUALITY);

    if (isHighQuality) {
        auto a2dpin = std::make_shared<AudioDeviceDescriptor>(desc);
        a2dpin->deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP_IN;
        bool isA2dpinConnected = AudioDeviceManager::GetAudioDeviceManager().IsConnectedDevices(a2dpin);
        if (isA2dpinConnected) {
            return AudioDeviceManager::GetAudioDeviceManager().GetExistedDevice(a2dpin);
        }
    }

    return isConnected ? desc : std::make_shared<AudioDeviceDescriptor>();
}

std::shared_ptr<AudioDeviceDescriptor> AudioRouterSelectStrategy::GetPreferDevice(
    BluetoothAndNearlinkPreferredRecordCategory category)
{
    if (category == BluetoothAndNearlinkPreferredRecordCategory::PREFERRED_NONE) {
        return nullptr;
    }

    std::vector<DeviceType> types = {
        DEVICE_TYPE_NEARLINK,
        DEVICE_TYPE_BLUETOOTH_SCO,
    };
    auto audioDeviceDescriptors =
        AudioDeviceManager::GetAudioDeviceManager().GetConnectedDevicesByTypesAndRole(types, INPUT_DEVICE);
    
    if (audioDeviceDescriptors.empty()) {
        AUDIO_INFO_LOG("No bluetooth or nearlink devices available");
        return nullptr;
    }

    std::sort(audioDeviceDescriptors.begin(), audioDeviceDescriptors.end(),
        [](const auto &desc1, const auto &desc2) {
            return desc1->connectTimeStamp_ < desc2->connectTimeStamp_;
        });

    return audioDeviceDescriptors.back();
}

std::shared_ptr<AudioDeviceDescriptor> AudioRouterSelectStrategy::GetFinalOutputDeviceForMedia(
    const std::shared_ptr<AudioDeviceDescriptor> &desc)
{
    if (desc == nullptr) {
        return nullptr;
    }
    bool isConnected = AudioDeviceManager::GetAudioDeviceManager().IsConnectedDevices(desc);
    if (desc->deviceType_ == DEVICE_TYPE_BLUETOOTH_SCO) {
        auto a2dp = std::make_shared<AudioDeviceDescriptor>(desc);
        a2dp->deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
        bool isA2dpConnected = AudioDeviceManager::GetAudioDeviceManager().IsConnectedDevices(a2dp);
        if (isA2dpConnected) {
            return AudioDeviceManager::GetAudioDeviceManager().GetExistedDevice(a2dp);
        }
    }

    return isConnected ? desc : nullptr;
}

std::shared_ptr<AudioDeviceDescriptor> AudioRouterSelectStrategy::GetFinalOutputDeviceForCall(
    const std::shared_ptr<AudioDeviceDescriptor>& desc)
{
    if (desc == nullptr) {
        return nullptr;
    }

    bool isConnected = AudioDeviceManager::GetAudioDeviceManager().IsConnectedDevices(desc);
    if (desc->deviceType_ == DEVICE_TYPE_BLUETOOTH_A2DP) {
        auto sco = std::make_shared<AudioDeviceDescriptor>(desc);
        sco->deviceType_ = DEVICE_TYPE_BLUETOOTH_SCO;
        bool isScoConnected = AudioDeviceManager::GetAudioDeviceManager().IsConnectedDevices(sco);
        if (isScoConnected) {
            return AudioDeviceManager::GetAudioDeviceManager().GetExistedDevice(sco);
        }
    }

    return isConnected ? desc : nullptr;
}

std::shared_ptr<AudioDeviceDescriptor> AudioRouterSelectStrategy::GetRecognitionInputDevice()
{
    AudioRouterInfra::GetInstance().Lock();
    
    auto simpleDesc = AudioRouterInfra::GetInstance().GetRecognitionCaptureDevice();
    auto result = ConvertSimpleToDevice(simpleDesc);
    
    AudioRouterInfra::GetInstance().Unlock();
    return result;
}

void AudioRouterSelectStrategy::SetRecognitionInputDevice(
    const std::shared_ptr<AudioDeviceDescriptor> &device)
{
    auto simpleDesc = ConvertDeviceToSimple(device);
    AudioRouterInfra::GetInstance().SetRecognitionCaptureDevice(simpleDesc);
}

void AudioRouterSelectStrategy::UpdateStreamDefaultDevice(int32_t uid, uint32_t streamId,
    const std::shared_ptr<AudioDeviceDescriptor> &device)
{
    auto simpleDesc = ConvertDeviceToSimple(device);
    AudioRouterInfra::GetInstance().UpdateStreamDefaultDevice(uid, streamId, simpleDesc);
}

std::shared_ptr<AudioDeviceDescriptor> AudioRouterSelectStrategy::GetStreamDefaultDevice(
    int32_t uid, uint32_t streamId)
{
    AudioRouterInfra::GetInstance().Lock();
    
    auto simpleDesc = AudioRouterInfra::GetInstance().GetStreamDefaultDevice(uid, streamId);
    auto result = ConvertSimpleToDevice(simpleDesc);
    
    AudioRouterInfra::GetInstance().Unlock();
    return result;
}

void AudioRouterSelectStrategy::UpdateCurrentOutputDevice(int32_t uid,
    const std::vector<std::shared_ptr<AudioDeviceDescriptor>>& devices)
{
    auto simpleDevices = ConvertDeviceToSimple(devices);
    AudioRouterInfra::GetInstance().UpdateCurrentOutputDevice(uid, simpleDevices);
}

std::vector<std::shared_ptr<AudioDeviceDescriptor>> AudioRouterSelectStrategy::GetCurrentOutputDevice(int32_t uid)
{
    AudioRouterInfra::GetInstance().Lock();
    
    auto simpleDescs = AudioRouterInfra::GetInstance().GetCurrentOutputDevice(uid);
    auto devices = ConvertSimpleToDevice(simpleDescs);

    AudioRouterInfra::GetInstance().Unlock();
    return devices;
}

void AudioRouterSelectStrategy::UpdateCurrentInputDevice(int32_t uid,
    const std::vector<std::shared_ptr<AudioDeviceDescriptor>>& devices)
{
    auto simpleDevices = ConvertDeviceToSimple(devices);
    AudioRouterInfra::GetInstance().UpdateCurrentInputDevice(uid, simpleDevices);
}

std::vector<std::shared_ptr<AudioDeviceDescriptor>> AudioRouterSelectStrategy::GetCurrentInputDevice(int32_t uid)
{
    AudioRouterInfra::GetInstance().Lock();
    
    auto simpleDescs = AudioRouterInfra::GetInstance().GetCurrentInputDevice(uid);
    auto devices = ConvertSimpleToDevice(simpleDescs);

    AudioRouterInfra::GetInstance().Unlock();
    return devices;
}

}
} 
