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
#include "audio_bundle_manager.h"
#include "audio_common_utils.h"

namespace OHOS {
namespace AudioStandard {

std::shared_ptr<AudioDeviceSimpleDescriptor> AudioRouterFollowStrategy::GetMediaInputDeviceWhenHasRunningStream(
    SourceType &effectiveSourceType, BluetoothAndNearlinkPreferredRecordCategory &category)
{
    auto &routerInfra = AudioRouterInfra::GetInstance();
    int32_t highestUid = routerInfra.GetHighestInputPriorityApp();
    auto highestPrioritySourceTypes = routerInfra.GetAppRunningSourceTypes(highestUid);
    if (!highestPrioritySourceTypes.empty()) {
        effectiveSourceType = highestPrioritySourceTypes[0];
    }
    category = routerInfra.GetHighestPriorityPreferredInputCategory();
    std::vector<SourceType> checkSourceTypes = {
        SOURCE_TYPE_MIC, SOURCE_TYPE_CAMCORDER, SOURCE_TYPE_LIVE
    };
    bool isSystemSelectActive = routerInfra.HasHighestPriorityRunningSourceType(checkSourceTypes)
        || VolumeUtils::IsPCVolumeEnable();
    return GetMediaInputDeviceByPriority(highestUid, category, isSystemSelectActive);
}

std::shared_ptr<AudioDeviceSimpleDescriptor> AudioRouterFollowStrategy::GetMediaInputDeviceWhenNoRunningStream(
    int32_t uid, BluetoothAndNearlinkPreferredRecordCategory &category, SourceType sourceType)
{
    auto &routerInfra = AudioRouterInfra::GetInstance();
    if (uid == INVALID_UID) {
        auto systemSelectInfo = routerInfra.GetSystemSelectDevice(SelectDeviceType::MEDIA_INPUT);
        return systemSelectInfo.device_;
    }

    category = routerInfra.GetPreferredInputCategory(uid);
    bool isSystemSelectActive = (sourceType == SOURCE_TYPE_MIC) ||
        (sourceType == SOURCE_TYPE_CAMCORDER) || (sourceType == SOURCE_TYPE_LIVE) ||
        VolumeUtils::IsPCVolumeEnable();
    return GetMediaInputDeviceByPriority(uid, category, isSystemSelectActive);
}

std::shared_ptr<AudioDeviceSimpleDescriptor> AudioRouterFollowStrategy::GetMediaInputDeviceByPriority(
    int uid, BluetoothAndNearlinkPreferredRecordCategory &category, bool isSystemSelectActive)
{
    auto &routerInfra = AudioRouterInfra::GetInstance();
    auto appSelectInfo = routerInfra.GetAppSelectDevice(SelectDeviceType::MEDIA_INPUT, uid);
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

    // First check stream-level selection
    if (uid != SYSTEM_UID && ValidStreamId(streamId)) {
        auto streamSelectDevice = routerInfra.GetStreamSelectDevice(uid, streamId);
        if (streamSelectDevice != nullptr) {
            simpleDesc = streamSelectDevice;
            effectiveSourceType = routerInfra.GetStreamSourceType(uid, streamId);
            if (effectiveSourceType == SourceType::SOURCE_TYPE_INVALID) {
                effectiveSourceType = sourceType;
            }
        }
    }

    // If no stream-level selection, proceed with existing logic
    if (simpleDesc == nullptr) {
        auto appSourceTypes = routerInfra.GetAppRunningSourceTypes(uid);
        if (!appSourceTypes.empty()) {
            simpleDesc = GetMediaInputDeviceWhenHasRunningStream(effectiveSourceType, category);
        } else {
            simpleDesc = GetMediaInputDeviceWhenNoRunningStream(uid, category, sourceType);
        }
    }

    if (simpleDesc != nullptr) {
        result = ConvertSimpleToDevice(simpleDesc);
        result = JudgeFinalSelectDevice(result, effectiveSourceType, category);
    }

    AudioRouterInfra::GetInstance().Unlock();
    return result == nullptr ? std::make_shared<AudioDeviceDescriptor>() : result;
}

void AudioRouterFollowStrategy::SetMediaInputDevice(int32_t uid, uint32_t streamId,
    const std::shared_ptr<AudioDeviceDescriptor> &device)
{
    auto simpleDesc = ConvertDeviceToSimple(device);
    if (uid != SYSTEM_UID && ValidStreamId(streamId)) {
        AudioRouterInfra::GetInstance().UpdateStreamSelectDevice(uid, streamId, simpleDesc);
    } else {
        AudioRouterInfra::GetInstance().RefreshSelectDevice(SelectDeviceType::MEDIA_INPUT, uid, simpleDesc);
    }
}

std::shared_ptr<AudioDeviceDescriptor> AudioRouterFollowStrategy::GetMediaOutputDevice(int32_t uid,
    uint32_t streamId)
{
    AudioRouterInfra::GetInstance().Lock();
    auto &routerInfra = AudioRouterInfra::GetInstance();
    int32_t appUid = routerInfra.GetHighestOutputPriorityApp();
    AUDIO_DEBUG_LOG("highestOutputPriorityAppUid: %{public}d", appUid);
    auto appSelectInfo = routerInfra.GetAppSelectDevice(SelectDeviceType::MEDIA_OUTPUT, appUid);
    appSelectInfo = routerInfra.HasRunningOutputStream(appUid) ? appSelectInfo : AudioSelectDeviceInfo();
    auto systemSelectInfo = routerInfra.GetSystemSelectDevice(SelectDeviceType::MEDIA_OUTPUT);
    std::shared_ptr<AudioDeviceSimpleDescriptor> simpleDesc = nullptr;
    if (AudioRouterSelectStrategy::HasValidSelectDevice(appSelectInfo) &&
        AudioRouterSelectStrategy::HasValidSelectDevice(systemSelectInfo)) {
        if (appSelectInfo.selectTime_ >= systemSelectInfo.selectTime_) {
            simpleDesc = appSelectInfo.device_;
        } else {
            simpleDesc = systemSelectInfo.device_;
        }
    } else if (AudioRouterSelectStrategy::HasValidSelectDevice(appSelectInfo)) {
        simpleDesc = appSelectInfo.device_;
    } else if (AudioRouterSelectStrategy::HasValidSelectDevice(systemSelectInfo)) {
        simpleDesc = systemSelectInfo.device_;
    }
    auto result = ConvertSimpleToDevice(simpleDesc);
    AudioRouterInfra::GetInstance().Unlock();
    CHECK_AND_RETURN_RET(result != nullptr, std::make_shared<AudioDeviceDescriptor>());
    return result;
}

void AudioRouterFollowStrategy::SetMediaOutputDevice(int32_t uid, uint32_t streamId,
    const std::shared_ptr<AudioDeviceDescriptor> &device)
{
    AudioRouterInfra::GetInstance().Lock();
    CHECK_AND_RETURN_LOG(device != nullptr, "device is nullptr");
    AUDIO_INFO_LOG("callerUid: %{public}d, deviceType_: %{public}d", uid, device->deviceType_);
    auto simpleDesc = ConvertDeviceToSimple(device);
    CHECK_AND_RETURN_LOG(simpleDesc != nullptr, "simpleDesc is nullptr");
    if (uid != SYSTEM_UID && ValidStreamId(streamId)) {
        AudioRouterInfra::GetInstance().UpdateStreamSelectDevice(uid, streamId, simpleDesc);
    } else {
        AudioRouterInfra::GetInstance().RefreshSelectDevice(SelectDeviceType::MEDIA_OUTPUT, uid, simpleDesc);
    }
    AudioRouterInfra::GetInstance().Unlock();
}

std::shared_ptr<AudioDeviceDescriptor> AudioRouterFollowStrategy::GetCallInputDevice(int32_t uid, uint32_t streamId)
{
    auto systemSelectInfo = AudioRouterInfra::GetInstance().GetSystemSelectDevice(SelectDeviceType::CALL_INPUT);
    auto result = ConvertSimpleToDevice(systemSelectInfo.device_);
    CHECK_AND_RETURN_RET(result != nullptr, std::make_shared<AudioDeviceDescriptor>());
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
            simpleDesc = GetCallOutputDeviceByUid(callOwnerUid);
        } else {
            auto systemSelectInfo = routerInfra.GetSystemSelectDevice(SelectDeviceType::CALL_OUTPUT);
            simpleDesc = systemSelectInfo.device_;
        }
    } else {
        simpleDesc = GetCallOutputDeviceByUid(uid);
    }

    auto result = ConvertSimpleToDevice(simpleDesc);
    AudioRouterInfra::GetInstance().Unlock();
    CHECK_AND_RETURN_RET(result != nullptr, std::make_shared<AudioDeviceDescriptor>());
    return result;
}

std::shared_ptr<AudioDeviceSimpleDescriptor> AudioRouterFollowStrategy::GetCallOutputDeviceByUid(int32_t uid)
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
    uint32_t streamId, const std::shared_ptr<AudioDeviceDescriptor> &device, const std::string caller)
{
    AudioRouterInfra::GetInstance().Lock();
    CHECK_AND_RETURN_LOG(device != nullptr, "device is nullptr");
    auto callerPid = IPCSkeleton::GetCallingPid();
    std::string bundleName = AudioBundleManager::GetBundleNameFromUid(uid);
    AUDIO_INFO_LOG(
        "deviceType: %{public}d callerUid: %{public}d, callerPid: %{public}d, ownerUid:%{public}d,\
        bundle name: %{public}s, caller: %{public}s",
        device->deviceType_, uid, callerPid, AudioSceneManager::GetInstance().GetAudioSceneOwnerUid(),
        bundleName.c_str(), caller.c_str());
    if (device == nullptr || device->deviceType_ == DEVICE_TYPE_NONE) {
        ClearCallOutputDevice(uid, streamId);
    } else {
        RefreshCallOutputDevice(uid, streamId, device);
    }
    AudioRouterInfra::GetInstance().Unlock();
}

void AudioRouterFollowStrategy::UpdateCurrentOutputDevice(int32_t uid,
    const std::vector<std::shared_ptr<AudioDeviceDescriptor>> &devices)
{
    AUDIO_INFO_LOG("uid=%{public}d, deviceId=%{public}d", uid,
        !devices.empty() && devices[0] ? devices[0]->deviceId_ : 0);
    auto simpleDevices = ConvertDeviceToSimple(devices);
    AudioRouterInfra::GetInstance().UpdateCurrentOutputDevice(SYSTEM_UID, simpleDevices);
}

std::vector<std::shared_ptr<AudioDeviceDescriptor>> AudioRouterFollowStrategy::GetCurrentOutputDevice(int32_t uid)
{
    auto simpleDescs = AudioRouterInfra::GetInstance().GetCurrentOutputDevice(SYSTEM_UID);
    AUDIO_INFO_LOG("uid=%{public}d, deviceId=%{public}d", uid,
        !simpleDescs.empty() && simpleDescs[0] ? simpleDescs[0]->deviceId_ : 0);
    return ConvertSimpleToDevice(simpleDescs);
}

void AudioRouterFollowStrategy::UpdateCurrentInputDevice(int32_t uid,
    const std::vector<std::shared_ptr<AudioDeviceDescriptor>> &devices)
{
    AUDIO_INFO_LOG("uid=%{public}d, deviceId=%{public}d", uid,
        !devices.empty() && devices[0] ? devices[0]->deviceId_ : 0);
    auto simpleDevices = ConvertDeviceToSimple(devices);
    AudioRouterInfra::GetInstance().UpdateCurrentInputDevice(SYSTEM_UID, simpleDevices);
}

std::vector<std::shared_ptr<AudioDeviceDescriptor>> AudioRouterFollowStrategy::GetCurrentInputDevice(int32_t uid)
{
    auto simpleDescs = AudioRouterInfra::GetInstance().GetCurrentInputDevice(SYSTEM_UID);
    AUDIO_INFO_LOG("uid=%{public}d, deviceId=%{public}d", uid,
        !simpleDescs.empty() && simpleDescs[0] ? simpleDescs[0]->deviceId_ : 0);
    return ConvertSimpleToDevice(simpleDescs);
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

std::shared_ptr<AudioDeviceDescriptor> AudioRouterFollowStrategy::GetMediaDefaultOutputDevice(
    int32_t uid, uint32_t streamId)
{
    AudioRouterInfra::GetInstance().Lock();
    auto &routerInfra = AudioRouterInfra::GetInstance();
    std::shared_ptr<AudioDeviceSimpleDescriptor> simpleDesc = nullptr;

    if (streamId == INVALID_STREAM_ID ||
        routerInfra.GetStreamStreamUsage(uid, streamId) != StreamUsage::STREAM_USAGE_INVALID) {
        simpleDesc = routerInfra.GetLatestStreamSelectDevice(SelectDeviceType::MEDIA_OUTPUT);
    } else {
        simpleDesc = routerInfra.GetStreamDefaultDevice(uid, streamId, SelectDeviceType::MEDIA_OUTPUT);
    }

    AudioRouterInfra::GetInstance().Unlock();
    return ConvertSimpleToDevice(simpleDesc);
}

std::shared_ptr<AudioDeviceDescriptor> AudioRouterFollowStrategy::GetCallDefaultOutputDevice(
    int32_t uid, uint32_t streamId)
{
    AudioRouterInfra::GetInstance().Lock();
    auto &routerInfra = AudioRouterInfra::GetInstance();
    std::shared_ptr<AudioDeviceSimpleDescriptor> simpleDesc = nullptr;

    if (AudioSceneManager::GetInstance().IsInAudioCallScene()) {
        auto callOwnerUid = AudioSceneManager::GetInstance().GetAudioSceneOwnerUid();
        simpleDesc = routerInfra.GetFirstRunningStreamDefaultDevice(callOwnerUid,
            SelectDeviceType::CALL_OUTPUT);
    } else {
        simpleDesc = routerInfra.GetStreamDefaultDevice(uid, streamId, SelectDeviceType::CALL_OUTPUT);
    }

    AudioRouterInfra::GetInstance().Unlock();
    return ConvertSimpleToDevice(simpleDesc);
}
}
}
