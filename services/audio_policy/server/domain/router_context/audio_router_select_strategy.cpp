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
#define LOG_TAG "AudioRouterSelectStrategy"
#endif

#include "audio_router_select_strategy.h"
#include <algorithm>
#include "audio_router_follow_strategy.h"
#include "audio_router_independent_strategy.h"
#include "audio_router_infra.h"
#include "audio_device_manager.h"
#include "audio_policy_log.h"
#include "audio_core_config_manager.h"
#include "parameters.h"

namespace OHOS {
namespace AudioStandard {
constexpr const char *MULTI_STREAM_DEVICE_SUPPORT = "const.multimedia.audio.sys_multidevice_capability.enable";
const bool ENHANCE_ROUTING_SUPPORTED = OHOS::system::GetBoolParameter(MULTI_STREAM_DEVICE_SUPPORT, false);
AudioRouterSelectStrategy& AudioRouterSelectStrategy::GetInstance()
{
    if (ENHANCE_ROUTING_SUPPORTED) {
        static AudioRouterIndependentStrategy independent;
        return independent;
    }

    static AudioRouterFollowStrategy follow;
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
        deviceDesc->deviceRole_, deviceDesc->deviceId_, deviceDesc->networkId_, deviceDesc->macAddress_);
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
            AUDIO_INFO_LOG("prefer high quality input for source %{public}d with category %{public}d",
                sourceType, category);
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
            AUDIO_INFO_LOG("use a2dp device for media output");
            return AudioDeviceManager::GetAudioDeviceManager().GetExistedDevice(a2dp);
        }
    }

    return isConnected ? desc : std::make_shared<AudioDeviceDescriptor>();
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
            AUDIO_INFO_LOG("use sco device for call output");
            return AudioDeviceManager::GetAudioDeviceManager().GetExistedDevice(sco);
        }
    }

    return isConnected ? desc : make_shared<AudioDeviceDescriptor>();
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

bool AudioRouterSelectStrategy::GetEnhancedRoutingSupported() const
{
    return ENHANCE_ROUTING_SUPPORTED;
}

int32_t AudioRouterSelectStrategy::UpdateDefaultOutputDevice(DeviceType deviceType, int32_t uid,
    uint32_t streamId, StreamUsage streamUsage)
{
    AUDIO_INFO_LOG("stream %{public}u with usage %{public}d selects output device %{public}d",
        streamId, streamUsage, deviceType);

    auto setDev = AudioDeviceManager::GetAudioDeviceManager().GetBuiltinOutputDevice(deviceType);
    if (streamUsage == STREAM_USAGE_VOICE_MESSAGE) {
        auto before = GetMediaDefaultOutputDevice(uid, streamId);
        UpdateMediaDefaultOutputDevice(uid, streamId, setDev);
        auto after = GetMediaDefaultOutputDevice(uid, streamId);
        if (!IsSameDeviceDeviceDescriptor(before, after)) {
            AUDIO_WARNING_LOG("media default output device change");
            return NEED_TO_FETCH;
        }
    } else if (streamUsage == STREAM_USAGE_VOICE_COMMUNICATION ||
        streamUsage == STREAM_USAGE_VIDEO_COMMUNICATION ||
        streamUsage == STREAM_USAGE_VOICE_MODEM_COMMUNICATION ||
        streamUsage == STREAM_USAGE_INTERPHONE) {
        auto before = GetCallDefaultOutputDevice(uid, streamId);
        UpdateCallDefaultOutputDevice(uid, streamId, setDev);
        auto after = GetCallDefaultOutputDevice(uid, streamId);
        if (!IsSameDeviceDeviceDescriptor(before, after)) {
            AUDIO_WARNING_LOG("call default output device change");
            return NEED_TO_FETCH;
        }
    } else {
        AUDIO_ERR_LOG("Invalid stream usage %{public}d", streamUsage);
        return ERROR;
    }
    return SUCCESS;
}

bool AudioRouterSelectStrategy::IsStreamSetDefaultOutputDevice(int32_t uid, uint32_t streamId)
{
    AudioRouterInfra::GetInstance().Lock();
    auto mediaOutputDevice = AudioRouterInfra::GetInstance().GetStreamDefaultDevice(uid, streamId,
        SelectDeviceType::MEDIA_OUTPUT);
    auto callOutputDevice = AudioRouterInfra::GetInstance().GetStreamDefaultDevice(uid, streamId,
        SelectDeviceType::CALL_OUTPUT);

    bool isSetDefault = (mediaOutputDevice != nullptr) || (callOutputDevice != nullptr);
    AudioRouterInfra::GetInstance().Unlock();
    return isSetDefault;
}

void AudioRouterSelectStrategy::UpdateMediaDefaultOutputDevice(int32_t uid, uint32_t streamId,
    const std::shared_ptr<AudioDeviceDescriptor> &device)
{
    auto simpleDesc = ConvertDeviceToSimple(device);
    AudioRouterInfra::GetInstance().UpdateStreamDefaultDevice(uid, streamId,
        SelectDeviceType::MEDIA_OUTPUT, simpleDesc);
}

std::shared_ptr<AudioDeviceDescriptor> AudioRouterSelectStrategy::GetMediaDefaultOutputDevice(
    int32_t uid, uint32_t streamId)
{
    AudioRouterInfra::GetInstance().Lock();

    auto simpleDesc = AudioRouterInfra::GetInstance().GetStreamDefaultDevice(uid, streamId,
        SelectDeviceType::MEDIA_OUTPUT);
    auto result = ConvertSimpleToDevice(simpleDesc);

    AudioRouterInfra::GetInstance().Unlock();
    return result;
}

void AudioRouterSelectStrategy::UpdateCallDefaultOutputDevice(int32_t uid, uint32_t streamId,
    const std::shared_ptr<AudioDeviceDescriptor> &device)
{
    auto simpleDesc = ConvertDeviceToSimple(device);
    AudioRouterInfra::GetInstance().UpdateStreamDefaultDevice(uid, streamId,
        SelectDeviceType::CALL_OUTPUT, simpleDesc);
}

std::shared_ptr<AudioDeviceDescriptor> AudioRouterSelectStrategy::GetCallDefaultOutputDevice(
    int32_t uid, uint32_t streamId)
{
    AudioRouterInfra::GetInstance().Lock();

    auto simpleDesc = AudioRouterInfra::GetInstance().GetStreamDefaultDevice(uid, streamId,
        SelectDeviceType::CALL_OUTPUT);
    auto result = ConvertSimpleToDevice(simpleDesc);

    AudioRouterInfra::GetInstance().Unlock();
    return result;
}

void AudioRouterSelectStrategy::ExcludeDevices(const std::vector<std::shared_ptr<AudioDeviceDescriptor>> &devices,
    AudioDeviceUsage usage)
{
    auto &routerInfra = AudioRouterInfra::GetInstance();

    routerInfra.Lock();
    for (const auto &device : devices) {
        if (device == nullptr) {
            continue;
        }
        AUDIO_INFO_LOG("exclude device %{public}d with role %{public}d for usage %{public}d",
            device->deviceType_, device->deviceRole_, usage);
        ExcludePairDevice(device, usage);
        auto simpleDesc = ConvertDeviceToSimple(device);
        routerInfra.ExcludeDevice(simpleDesc, usage);

        std::vector<std::pair<AudioDeviceUsage, SelectDeviceType>> types = {
            { MEDIA_OUTPUT_DEVICES, SelectDeviceType::MEDIA_OUTPUT },
            { MEDIA_INPUT_DEVICES, SelectDeviceType::MEDIA_INPUT },
            { CALL_OUTPUT_DEVICES, SelectDeviceType::CALL_OUTPUT },
            { CALL_INPUT_DEVICES, SelectDeviceType::CALL_INPUT },
        };
        for (const auto &type : types) {
            if ((usage & type.first) != 0) {
                routerInfra.ClearAllSelectSelectDevice(type.second, simpleDesc);
            }
        }
    }
    routerInfra.Unlock();
}

void AudioRouterSelectStrategy::ExcludePairDevice(const std::shared_ptr<AudioDeviceDescriptor> &device,
    AudioDeviceUsage usage)
{
    auto &routerInfra = AudioRouterInfra::GetInstance();
    auto &deviceManager = AudioDeviceManager::GetAudioDeviceManager();

    auto onLineDevice = deviceManager.FindConnectedDeviceById(device->deviceId_);
    if (onLineDevice == nullptr) {
        AUDIO_ERR_LOG("no online device for type %{public}d and role %{public}d",
            device->deviceType_, device->deviceRole_);
        return;
    }

    if (onLineDevice->pairDeviceDescriptor_ != nullptr) {
        if (usage & MEDIA_OUTPUT_DEVICES) {
            auto simplePairDesc = ConvertDeviceToSimple(onLineDevice->pairDeviceDescriptor_);
            routerInfra.ExcludeDevice(simplePairDesc, MEDIA_INPUT_DEVICES);
            routerInfra.ClearAllSelectSelectDevice(SelectDeviceType::MEDIA_INPUT, simplePairDesc);
        } else if (usage & CALL_OUTPUT_DEVICES) {
            auto simplePairDesc = ConvertDeviceToSimple(onLineDevice->pairDeviceDescriptor_);
            routerInfra.ExcludeDevice(simplePairDesc, CALL_INPUT_DEVICES);
            routerInfra.ClearAllSelectSelectDevice(SelectDeviceType::CALL_INPUT, simplePairDesc);
        }
    }
}


void AudioRouterSelectStrategy::UnexcludeDevices(const std::vector<std::shared_ptr<AudioDeviceDescriptor>> &devices,
    AudioDeviceUsage usage)
{
    auto &routerInfra = AudioRouterInfra::GetInstance();

    routerInfra.Lock();
    for (const auto &device : devices) {
        if (device == nullptr) {
            continue;
        }
        AUDIO_INFO_LOG("unexclude device %{public}d with role %{public}d for usage %{public}d",
            device->deviceType_, device->deviceRole_, usage);
        UnexcludePairDevice(device, usage);
        auto simpleDesc = ConvertDeviceToSimple(device);
        routerInfra.UnexcludeDevice(simpleDesc, usage);
    }
    routerInfra.Unlock();
}

void AudioRouterSelectStrategy::UnexcludePairDevice(const std::shared_ptr<AudioDeviceDescriptor> &device,
    AudioDeviceUsage usage)
{
    auto &routerInfra = AudioRouterInfra::GetInstance();
    auto &deviceManager = AudioDeviceManager::GetAudioDeviceManager();

    auto onLineDevice = deviceManager.FindConnectedDeviceById(device->deviceId_);
    if (onLineDevice == nullptr) {
        AUDIO_ERR_LOG("no online device for type %{public}d and role %{public}d",
            device->deviceType_, device->deviceRole_);
        return;
    }

    if (onLineDevice->pairDeviceDescriptor_ != nullptr) {
        if (usage & MEDIA_OUTPUT_DEVICES) {
            auto simplePairDesc = ConvertDeviceToSimple(onLineDevice->pairDeviceDescriptor_);
            routerInfra.UnexcludeDevice(simplePairDesc, MEDIA_INPUT_DEVICES);
        } else if (usage & CALL_OUTPUT_DEVICES) {
            auto simplePairDesc = ConvertDeviceToSimple(onLineDevice->pairDeviceDescriptor_);
            routerInfra.UnexcludeDevice(simplePairDesc, CALL_INPUT_DEVICES);
        }
    }
}

std::vector<std::shared_ptr<AudioDeviceDescriptor>> AudioRouterSelectStrategy::GetExcludedDevices(
    AudioDeviceUsage usage)
{
    auto &routerInfra = AudioRouterInfra::GetInstance();
    auto excludedDevices = routerInfra.GetExcludedDevices(usage);
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> result;
    for (const auto &simpleDesc : excludedDevices) {
        result.push_back(ConvertSimpleToDevice(simpleDesc));
    }
    return result;
}

bool AudioRouterSelectStrategy::IsDeviceExcluded(const std::shared_ptr<AudioDeviceDescriptor> &device,
    AudioDeviceUsage usage)
{
    auto simpleDesc = ConvertDeviceToSimple(device);
    auto &routerInfra = AudioRouterInfra::GetInstance();
    return routerInfra.IsDeviceExcluded(simpleDesc, usage);
}

bool AudioRouterSelectStrategy::IsSameDeviceDeviceDescriptor(
    std::shared_ptr<AudioDeviceDescriptor> desc1, std::shared_ptr<AudioDeviceDescriptor> desc2)
{
    if (desc1 == nullptr && desc2 == nullptr) {
        return true;
    } else if (desc1 != nullptr && desc2 != nullptr) {
        return desc1->IsSameDeviceDescPtr(desc2);
    }
    return false;
}

void AudioRouterSelectStrategy::SetScoExcluded(bool scoExcluded)
{
    auto &routerInfra = AudioRouterInfra::GetInstance();
    routerInfra.SetScoExcluded(scoExcluded);
}

bool AudioRouterSelectStrategy::GetScoExcluded()
{
    auto &routerInfra = AudioRouterInfra::GetInstance();
    return routerInfra.GetScoExcluded();
}

bool AudioRouterSelectStrategy::IsPreferredDevice(const AudioDeviceDescriptor &desc)
{
    auto simpleDesc = ConvertDeviceToSimple(std::make_shared<AudioDeviceDescriptor>(desc));
    auto &routerInfra = AudioRouterInfra::GetInstance();
    return routerInfra.IsPreferredDevice(simpleDesc);
}

bool AudioRouterSelectStrategy::HasValidSelectDevice(AudioSelectDeviceInfo selectInfo)
{
    auto &routerInfra = AudioRouterInfra::GetInstance();
    return routerInfra.HasValidSelectDevice(selectInfo);
}

AudioDeviceDescriptor AudioRouterSelectStrategy::Get1stCurrentInputDevice(int32_t uid)
{
    auto descs = GetCurrentInputDevice(uid);
    AUDIO_INFO_LOG("uid=%{public}d, deviceId=%{public}d", uid, descs.empty() ? 0 : descs[0]->deviceId_);
    return !descs.empty() && descs.back() ? *descs.back() : AudioDeviceDescriptor();
}

AudioDeviceDescriptor AudioRouterSelectStrategy::Get1stCurrentOutputDevice(int32_t uid)
{
    auto descs = GetCurrentOutputDevice(uid);
    AUDIO_INFO_LOG("uid=%{public}d, deviceId=%{public}d", uid, descs.empty() ? 0 : descs[0]->deviceId_);
    return !descs.empty() && descs.back() ? *descs.back() : AudioDeviceDescriptor();
}

bool AudioRouterSelectStrategy::IsCurrentInputDevice(const int32_t deviceId, int32_t uid)
{
    AUDIO_INFO_LOG("uid=%{public}d, deviceId=%{public}d", uid, deviceId);
    auto descs = GetCurrentInputDevice(uid);
    auto it = find_if(descs.cbegin(), descs.cend(), [deviceId](auto &item) {
        return item && item->deviceId_ == deviceId;
    });
    return it != descs.cend();
}

bool AudioRouterSelectStrategy::IsCurrentOutputDevice(const int32_t deviceId, int32_t uid)
{
    AUDIO_INFO_LOG("uid=%{public}d, deviceId=%{public}d", uid, deviceId);
    auto descs = GetCurrentOutputDevice(uid);
    auto it = find_if(descs.cbegin(), descs.cend(), [deviceId](auto &item) {
        return item && item->deviceId_ == deviceId;
    });
    return it != descs.cend();
}

set<shared_ptr<AudioDeviceDescriptor>> AudioRouterSelectStrategy::FindCurrentOutputDevice(set<DeviceType> &&types)
{
    AUDIO_INFO_LOG("deviceType=%{public}d", types.empty() ? 0 : *types.begin());
    set<shared_ptr<AudioDeviceDescriptor>> result;
    auto ids = AudioRouterInfra::GetInstance().FindCurrentOutputDevice(std::move(types));
    for (auto deviceId: ids) {
        auto desc = AudioDeviceManager::GetAudioDeviceManager().FindConnectedDeviceById(deviceId);
        CHECK_AND_CONTINUE(desc);
        result.insert(desc);
        AUDIO_INFO_LOG("deviceId=%{public}d, deviceType=%{public}d", desc->deviceId_, desc->deviceType_);
    }
    return result;
}

set<shared_ptr<AudioDeviceDescriptor>> AudioRouterSelectStrategy::FindCurrentInputDevice(set<DeviceType> &&types)
{
    AUDIO_INFO_LOG("deviceType=%{public}d", types.empty() ? 0 : *types.begin());
    set<shared_ptr<AudioDeviceDescriptor>> result;
    auto ids = AudioRouterInfra::GetInstance().FindCurrentInputDevice(std::move(types));
    for (auto deviceId: ids) {
        auto desc = AudioDeviceManager::GetAudioDeviceManager().FindConnectedDeviceById(deviceId);
        CHECK_AND_CONTINUE(desc);
        result.insert(desc);
        AUDIO_INFO_LOG("deviceId=%{public}d, deviceType=%{public}d", desc->deviceId_, desc->deviceType_);
    }
    return result;
}

}
}
