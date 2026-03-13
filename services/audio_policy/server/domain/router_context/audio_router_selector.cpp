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
#define LOG_TAG "AudioRouterSelector"
#endif

#include "audio_router_selector.h"
#include "audio_router_infra.h"
#include "audio_router_follow_strategy.h"
#include "audio_router_independent_strategy.h"
#include "audio_device_manager.h"
#include "audio_policy_log.h"

namespace OHOS {
namespace AudioStandard {

int32_t AudioRouterSelector::DeviceParamsCheck(DeviceRole role, 
    const std::vector<std::shared_ptr<AudioDeviceDescriptor>>& devices)
{
    if (devices.empty()) {
        AUDIO_ERR_LOG("devices is empty");
        return ERR_INVALID_PARAM;
    }

    for (const auto& device : devices) {
        if (device == nullptr) {
            AUDIO_ERR_LOG("device is nullptr");
            return ERR_INVALID_PARAM;
        }
    }

    return SUCCESS;
}

int32_t AudioRouterSelector::RefreshVirtualDevice(std::shared_ptr<AudioDeviceDescriptor>& desc)
{
    if (desc == nullptr) {
        AUDIO_ERR_LOG("desc is nullptr");
        return ERR_INVALID_PARAM;
    }

    bool isVirtualDevice = AudioDeviceManager::GetAudioDeviceManager().IsVirtualConnectedDevice(desc);
    if (isVirtualDevice) {
        desc->connectState_ = CONNECT_STATE_CONNECTED;
    }

    return SUCCESS;
}

int32_t AudioRouterSelector::SetPreferredDevice(AudioScene scene, SourceType sourceType,
    const std::shared_ptr<AudioDeviceDescriptor>& desc)
{
    if (desc == nullptr) {
        AUDIO_ERR_LOG("desc is nullptr");
        return ERR_INVALID_PARAM;
    }

    auto simpleDesc = GetSelectStrategy()->ConvertDeviceToSimple(desc);
    AudioRouterInfra::GetInstance().RefreshSelectDevice(SelectDeviceType::MEDIA_INPUT, 0, simpleDesc);

    return SUCCESS;
}

void AudioRouterSelector::NotifyUserSelectionEventForInput(const std::shared_ptr<AudioDeviceDescriptor>& desc,
    SourceType sourceType)
{
    AUDIO_INFO_LOG("Notify user selection event for input, deviceType: %{public}d, sourceType: %{public}d",
        desc->deviceType_, sourceType);
}

void AudioRouterSelector::WriteSelectInputSysEvents(const std::shared_ptr<AudioDeviceDescriptor>& desc,
    SourceType sourceType, AudioScene scene)
{
    AUDIO_INFO_LOG("Write select input sys events, deviceType: %{public}d, sourceType: %{public}d, scene: %{public}d",
        desc->deviceType_, sourceType, scene);
}

int32_t AudioRouterSelector::SelectInputDevice(std::shared_ptr<AudioDeviceDescriptor> selectedDesc)
{
    if (selectedDesc == nullptr) {
        AUDIO_ERR_LOG("selectedDesc is nullptr");
        return ERR_INVALID_PARAM;
    }

    AUDIO_INFO_LOG("SelectInputDevice called, deviceType: %{public}d, deviceRole: %{public}d",
        selectedDesc->deviceType_, selectedDesc->deviceRole_);

    int32_t res = DeviceParamsCheck(INPUT_DEVICE, {selectedDesc});
    CHECK_AND_RETURN_RET(res == SUCCESS, res);

    auto ret = RefreshVirtualDevice(selectedDesc);
    CHECK_AND_RETURN_RET(ret == SUCCESS, ret);

    SourceType sourceType = SOURCE_TYPE_MIC;
    if (selectedDesc->deviceRole_ == INPUT_DEVICE) {
        sourceType = selectedDesc->deviceType_ == DEVICE_TYPE_MIC ? SOURCE_TYPE_MIC : SOURCE_TYPE_VOICE_COMMUNICATION;
    }

    auto scene = AUDIO_SCENE_DEFAULT;
    if (selectedDesc->deviceRole_ == INPUT_DEVICE) {
        scene = AUDIO_SCENE_RECORD;
    }

    auto setRet = SetPreferredDevice(scene, sourceType, selectedDesc);
    CHECK_AND_RETURN_RET(setRet == SUCCESS, setRet);

    NotifyUserSelectionEventForInput(selectedDesc, sourceType);
    WriteSelectInputSysEvents(selectedDesc, sourceType, scene);

    return SUCCESS;
}

std::shared_ptr<AudioRouterSelectStrategy> AudioRouterSelector::GetSelectStrategy()
{
    static std::shared_ptr<AudioRouterSelectStrategy> strategy = nullptr;
    if (strategy == nullptr) {
        strategy = std::make_shared<AudioRouterFollowStrategy>();
    }
    return strategy;
}

} 
} 
