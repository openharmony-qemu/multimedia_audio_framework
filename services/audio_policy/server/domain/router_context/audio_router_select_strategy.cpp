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
#include "audio_router_infra.h"
#include "audio_device_manager.h"
#include "audio_policy_log.h"
#include <algorithm>

namespace OHOS {
namespace AudioStandard {

std::shared_ptr<AudioDeviceDescriptor> AudioRouterSelectStrategy::ConvertSimpleToDevice(
    const std::shared_ptr<AudioDeviceSimpleDescriptor>& simpleDesc)
{
    if (simpleDesc == nullptr) {
        return nullptr;
    }
    return simpleDesc->GetOnlineDeviceDescriptor();
}

std::shared_ptr<AudioDeviceSimpleDescriptor> AudioRouterSelectStrategy::ConvertDeviceToSimple(
    const std::shared_ptr<AudioDeviceDescriptor>& deviceDesc)
{
    if (deviceDesc == nullptr) {
        return nullptr;
    }
    return deviceDesc->CreateSimpleDescriptor();
}

std::shared_ptr<AudioDeviceDescriptor> AudioRouterSelectStrategy::JudgeFinalSelectDevice(
    const std::shared_ptr<AudioDeviceDescriptor>& desc, SourceType sourceType,
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
        [](const auto& desc1, const auto& desc2) {
            return desc1->connectTimeStamp_ < desc2->connectTimeStamp_;
        });
    
    return audioDeviceDescriptors.back();
}

std::shared_ptr<AudioDeviceDescriptor> AudioRouterSelectStrategy::GetFinalOutputDeviceForMedia(
    const std::shared_ptr<AudioDeviceDescriptor>& desc)
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

} 
} 
