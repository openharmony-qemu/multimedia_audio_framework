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
#define LOG_TAG "AudioRouterFollowStrategy"
#endif

#include "audio_router_follow_strategy.h"
#include "audio_router_infra.h"
#include "audio_policy_log.h"
#include "audio_scene_manager.h"
#include "audio_info.h"

namespace OHOS {
namespace AudioStandard {

std::shared_ptr<AudioDeviceSimpleDescriptor> AudioRouterFollowStrategy::GetMediaInputDeviceWhenHasRunningStream(
    SourceType &effectiveSourceType, BluetoothAndNearlinkPreferredRecordCategory &category)
{
    auto &routerInfra = AudioRouterInfra::GetInstance();
    auto highestPrioritySourceTypes = routerInfra.GetHighestPriorityRunningSourceTypes();
    if (!highestPrioritySourceTypes.empty()) {
        effectiveSourceType = highestPrioritySourceTypes[0];
    }
    category = routerInfra.GetHighestPriorityPreferredInputCategory();
    std::vector<SourceType> checkSourceTypes = {
        SOURCE_TYPE_MIC, SOURCE_TYPE_CAMCORDER, SOURCE_TYPE_LIVE
    };
    bool isSystemSelectActive = routerInfra.HasHighestPriorityRunningSourceType(checkSourceTypes);

    auto appSelectInfo = routerInfra.GetHighestPrioritySelectDevice(SelectDeviceType::MEDIA_INPUT);
    auto systemSelectInfo = routerInfra.GetSystemSelectDevice(SelectDeviceType::MEDIA_INPUT);

    std::shared_ptr<AudioDeviceSimpleDescriptor> simpleDesc = nullptr;

    if (appSelectInfo.device_ != nullptr && systemSelectInfo.device_ != nullptr) {
        if (appSelectInfo.selectTime_ > systemSelectInfo.selectTime_ || !isSystemSelectActive) {
            simpleDesc = appSelectInfo.device_;
        } else {
            simpleDesc = systemSelectInfo.device_;
            category = BluetoothAndNearlinkPreferredRecordCategory::PREFERRED_NONE;
        }
    } else if (appSelectInfo.device_ != nullptr) {
        simpleDesc = appSelectInfo.device_;
    } else if (systemSelectInfo.device_ != nullptr && isSystemSelectActive) {
        simpleDesc = systemSelectInfo.device_;
        category = BluetoothAndNearlinkPreferredRecordCategory::PREFERRED_NONE;
    } else {
        simpleDesc = ConvertDeviceToSimple(GetPreferDevice(category));
    }

    return simpleDesc;
}

std::shared_ptr<AudioDeviceSimpleDescriptor> AudioRouterFollowStrategy::GetMediaInputDeviceWhenNoRunningStream(
    int32_t uid, BluetoothAndNearlinkPreferredRecordCategory &category)
{
    auto &routerInfra = AudioRouterInfra::GetInstance();
    category = routerInfra.GetPreferredInputCategory(uid);

    auto appSelectInfo = routerInfra.GetAppSelectDevice(SelectDeviceType::MEDIA_INPUT, uid);

    std::shared_ptr<AudioDeviceSimpleDescriptor> simpleDesc = nullptr;

    if (appSelectInfo.device_ != nullptr) {
        simpleDesc = appSelectInfo.device_;
    } else {
        simpleDesc = ConvertDeviceToSimple(GetPreferDevice(category));
    }

    return simpleDesc;
}

std::shared_ptr<AudioDeviceDescriptor> AudioRouterFollowStrategy::GetMediaInputDevice(int32_t uid,
    uint32_t streamId, SourceType sourceType)
{
    AudioRouterInfra::GetInstance().Lock();

    auto& routerInfra = AudioRouterInfra::GetInstance();

    std::shared_ptr<AudioDeviceDescriptor> result = nullptr;
    std::shared_ptr<AudioDeviceSimpleDescriptor> simpleDesc = nullptr;
    SourceType effectiveSourceType = sourceType;
    BluetoothAndNearlinkPreferredRecordCategory category =
        BluetoothAndNearlinkPreferredRecordCategory::PREFERRED_NONE;

    if (!routerInfra.GetHighestPriorityRunningSourceTypes().empty()) {
        simpleDesc = GetMediaInputDeviceWhenHasRunningStream(effectiveSourceType, category);
    } else {
        simpleDesc = GetMediaInputDeviceWhenNoRunningStream(uid, category);
    }

    if (simpleDesc != nullptr) {
        result = ConvertSimpleToDevice(simpleDesc);
        result = JudgeFinalSelectDevice(result, effectiveSourceType, category);
    }

    AudioRouterInfra::GetInstance().Unlock();
    return result;
}

void AudioRouterFollowStrategy::SetMediaInputDevice(int32_t uid, uint32_t streamId,
    const std::shared_ptr<AudioDeviceDescriptor> &device)
{
    auto simpleDesc = ConvertDeviceToSimple(device);
    if (uid != SYSTEM_UID && streamId != 0) {
        AudioRouterInfra::GetInstance().UpdateStreamSelectDevice(uid, streamId, simpleDesc);
    } else {
        AudioRouterInfra::GetInstance().RefreshSelectDevice(SelectDeviceType::MEDIA_INPUT, uid, simpleDesc);
    }
}

std::shared_ptr<AudioDeviceDescriptor> AudioRouterFollowStrategy::GetMediaOutputDevice(int32_t uid,
    uint32_t streamId, StreamUsage streamUsage)
{
    auto systemSelectInfo = AudioRouterInfra::GetInstance().GetSystemSelectDevice(SelectDeviceType::MEDIA_OUTPUT);
    auto result = ConvertSimpleToDevice(systemSelectInfo.device_);
    return result;
}

void AudioRouterFollowStrategy::SetMediaOutputDevice(int32_t uid, uint32_t streamId,
    const std::shared_ptr<AudioDeviceDescriptor> &device)
{
    auto simpleDesc = ConvertDeviceToSimple(device);
    if (uid != SYSTEM_UID && streamId != 0) {
        AudioRouterInfra::GetInstance().UpdateStreamSelectDevice(uid, streamId, simpleDesc);
    } else {
        AudioRouterInfra::GetInstance().RefreshSelectDevice(SelectDeviceType::MEDIA_OUTPUT, SYSTEM_UID, simpleDesc);
    }
}

std::shared_ptr<AudioDeviceDescriptor> AudioRouterFollowStrategy::GetCallInputDevice(int32_t uid, uint32_t streamId)
{
    auto systemSelectInfo = AudioRouterInfra::GetInstance().GetSystemSelectDevice(SelectDeviceType::CALL_INPUT);
    auto result = ConvertSimpleToDevice(systemSelectInfo.device_);
    return result;
}

void AudioRouterFollowStrategy::SetCallInputDevice(int32_t uid, uint32_t streamId,
    const std::shared_ptr<AudioDeviceDescriptor> &device)
{
    auto simpleDesc = ConvertDeviceToSimple(device);
    AudioRouterInfra::GetInstance().RefreshSelectDevice(SelectDeviceType::CALL_INPUT, SYSTEM_UID, simpleDesc);
}

std::shared_ptr<AudioDeviceDescriptor> AudioRouterFollowStrategy::GetCallOutputDevice(int32_t uid, uint32_t streamId)
{
    AudioRouterInfra::GetInstance().Lock();
    auto& routerInfra = AudioRouterInfra::GetInstance();
    
    std::shared_ptr<AudioDeviceSimpleDescriptor> simpleDesc = nullptr;
    
    if (uid == INVALID_UID) {
        if (AudioSceneManager::GetInstance().IsInAudioCallScene()) {
            auto callOwnerUid = AudioSceneManager::GetInstance().GetAudioSceneOwnerUid();
            simpleDesc = GetCallLatestOutputDevice(callOwnerUid);
        } else {
            auto systemSelectInfo = routerInfra.GetSystemSelectDevice(SelectDeviceType::CALL_OUTPUT);
            simpleDesc = systemSelectInfo.device_;
        }
    } else {
        simpleDesc = GetCallLatestOutputDevice(uid);
    }
    
    auto result = ConvertSimpleToDevice(simpleDesc);
    AudioRouterInfra::GetInstance().Unlock();
    return result;
}

std::shared_ptr<AudioDeviceSimpleDescriptor> AudioRouterFollowStrategy::GetCallLatestOutputDevice(int32_t uid)
{
    std::shared_ptr<AudioDeviceSimpleDescriptor> simpleDesc = nullptr;
    auto &routerInfra = AudioRouterInfra::GetInstance();

    auto appSelectInfo = routerInfra.GetAppSelectDevice(SelectDeviceType::CALL_OUTPUT, uid);
    if (appSelectInfo.device_ != nullptr) {
        simpleDesc = appSelectInfo.device_;
    } else {
        auto systemSelectInfo = routerInfra.GetSystemSelectDevice(SelectDeviceType::CALL_OUTPUT);
        simpleDesc = systemSelectInfo.device_;
    }
    return simpleDesc;
}

void AudioRouterFollowStrategy::SetCallOutputDevice(int32_t uid,
    uint32_t streamId, const std::shared_ptr<AudioDeviceDescriptor> &device)
{
    AudioRouterInfra::GetInstance().Lock();
    auto &routerInfra = AudioRouterInfra::GetInstance();

    if (device == nullptr || device->deviceType_ == DEVICE_TYPE_NONE) {
        ClearCallOutputDevice(uid, streamId);
    } else {
        RefreshCallOutputDevice(uid, streamId, device);
    }

    AudioRouterInfra::GetInstance().Unlock();
}

void AudioRouterFollowStrategy::ClearCallOutputDevice(int32_t uid, uint32_t streamId)
{
    auto &routerInfra = AudioRouterInfra::GetInstance();
    auto callOwnerUid = AudioSceneManager::GetInstance().GetAudioSceneOwnerUid();
    if (uid == CLEAR_UID) {
        routerInfra.ClearAllSelectSelectDevice(SelectDeviceType::CALL_OUTPUT);
    } else if (uid == SYSTEM_UID || uid == callOwnerUid) {
        routerInfra.RefreshSelectDevice(SelectDeviceType::CALL_OUTPUT, callOwnerUid, nullptr);
        routerInfra.RefreshSelectDevice(SelectDeviceType::CALL_OUTPUT, SYSTEM_UID, nullptr);
    } else {
        routerInfra.RefreshSelectDevice(SelectDeviceType::CALL_OUTPUT, uid, nullptr);
    }
}
void AudioRouterFollowStrategy::RefreshCallOutputDevice(int32_t uid, uint32_t streamId,
    const std::shared_ptr<AudioDeviceDescriptor> &device)
{
    auto simpleDesc = ConvertDeviceToSimple(device);
    auto &routerInfra = AudioRouterInfra::GetInstance();
    if (uid == SYSTEM_UID) {
        if (AudioSceneManager::GetInstance().IsInAudioCallScene()) {
            auto callOwnerUid = AudioSceneManager::GetInstance().GetAudioSceneOwnerUid();
            routerInfra.ClearAllSelectSelectDevice(SelectDeviceType::CALL_OUTPUT);
            routerInfra.RefreshSelectDevice(SelectDeviceType::CALL_OUTPUT, callOwnerUid, simpleDesc);
        } else {
            routerInfra.ClearAllSelectSelectDevice(SelectDeviceType::CALL_OUTPUT);
        }
    }

    routerInfra.RefreshSelectDevice(SelectDeviceType::CALL_OUTPUT, uid, simpleDesc);
}

}
}
