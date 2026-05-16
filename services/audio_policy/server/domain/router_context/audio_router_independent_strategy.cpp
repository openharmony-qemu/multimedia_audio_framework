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
#define LOG_TAG "AudioRouterIndependentStrategy"
#endif

#include "audio_router_independent_strategy.h"
#include "audio_router_infra.h"
#include "audio_policy_log.h"

namespace OHOS {
namespace AudioStandard {

std::shared_ptr<AudioDeviceDescriptor> AudioRouterIndependentStrategy::GetMediaInputDevice(int32_t uid,
    uint32_t streamId, SourceType sourceType)
{
    AudioRouterInfra::GetInstance().Lock();

    auto &routerInfra = AudioRouterInfra::GetInstance();

    std::shared_ptr<AudioDeviceSimpleDescriptor> simpleDesc = nullptr;
    SourceType effectiveSourceType = sourceType;
    BluetoothAndNearlinkPreferredRecordCategory category =
        BluetoothAndNearlinkPreferredRecordCategory::PREFERRED_NONE;

    if (routerInfra.IsSystemUid(uid)) {
        auto systemSelectInfo = routerInfra.GetSystemSelectDevice(SelectDeviceType::MEDIA_INPUT);
        simpleDesc = systemSelectInfo.device_;
    } else {
        auto selectResult = GetSelectDeviceByPriority(uid, streamId, SelectDeviceType::MEDIA_INPUT);
        simpleDesc = selectResult.device_;
        category = routerInfra.GetPreferredInputCategory(uid);
        auto appRunningSourceTypes = routerInfra.GetAppRunningSourceTypes(uid);

        if (selectResult.source_ == SelectDeviceSource::STREAM) {
            effectiveSourceType = routerInfra.GetStreamSourceType(uid, streamId);
        } else if (selectResult.source_ == SelectDeviceSource::APP) {
            effectiveSourceType = appRunningSourceTypes.empty() ? SourceType::SOURCE_TYPE_INVALID :
                appRunningSourceTypes[0];
        }

        if (simpleDesc == nullptr) {
            simpleDesc = ConvertDeviceToSimple(GetPreferDevice(category));
            effectiveSourceType = appRunningSourceTypes.empty() ? SourceType::SOURCE_TYPE_INVALID :
                appRunningSourceTypes[0];
        }
    }

    std::shared_ptr<AudioDeviceDescriptor> result = nullptr;
    if (simpleDesc != nullptr) {
        result = ConvertSimpleToDevice(simpleDesc);
        effectiveSourceType = effectiveSourceType == SourceType::SOURCE_TYPE_INVALID ?
            sourceType : effectiveSourceType;
        result = JudgeFinalSelectDevice(result, effectiveSourceType, category);
    }

    AudioRouterInfra::GetInstance().Unlock();
    return result == nullptr ? std::make_shared<AudioDeviceDescriptor>() : result;
}

void AudioRouterIndependentStrategy::SetMediaInputDevice(int32_t uid, uint32_t streamId,
    const std::shared_ptr<AudioDeviceDescriptor> &device)
{
    auto simpleDesc = ConvertDeviceToSimple(device);
    if (uid != SYSTEM_UID && ValidStreamId(streamId)) {
        AudioRouterInfra::GetInstance().UpdateStreamSelectDevice(uid, streamId, simpleDesc);
    } else {
        AudioRouterInfra::GetInstance().RefreshSelectDevice(SelectDeviceType::MEDIA_INPUT, uid, simpleDesc);
        AudioRouterInfra::GetInstance().RefreshSelectDevice(SelectDeviceType::CALL_INPUT, uid, simpleDesc);
    }
}

std::shared_ptr<AudioDeviceDescriptor> AudioRouterIndependentStrategy::GetMediaOutputDevice(int32_t uid,
    uint32_t streamId)
{
    AudioRouterInfra::GetInstance().Lock();

    auto simpleDesc = GetSelectDevice(uid, streamId, SelectDeviceType::MEDIA_OUTPUT);
    auto result = ConvertSimpleToDevice(simpleDesc);

    AudioRouterInfra::GetInstance().Unlock();
    return GetFinalOutputDeviceForMedia(result);
}

void AudioRouterIndependentStrategy::SetMediaOutputDevice(int32_t uid, uint32_t streamId,
    const std::shared_ptr<AudioDeviceDescriptor> &device)
{
    auto simpleDesc = ConvertDeviceToSimple(device);
    AudioRouterInfra::GetInstance().ClearWirelessSelectDevice(simpleDesc);
    if (uid != SYSTEM_UID && ValidStreamId(streamId)) {
        AudioRouterInfra::GetInstance().UpdateStreamSelectDevice(uid, streamId, simpleDesc);
    } else {
        AudioRouterInfra::GetInstance().RefreshSelectDevice(SelectDeviceType::MEDIA_OUTPUT, uid, simpleDesc);
        AudioRouterInfra::GetInstance().RefreshSelectDevice(SelectDeviceType::CALL_OUTPUT, uid, simpleDesc);
    }
}

std::shared_ptr<AudioDeviceDescriptor> AudioRouterIndependentStrategy::GetCallInputDevice(int32_t uid,
    uint32_t streamId)
{
    AudioRouterInfra::GetInstance().Lock();

    auto simpleDesc = GetSelectDevice(uid, streamId, SelectDeviceType::CALL_INPUT);
    auto result = ConvertSimpleToDevice(simpleDesc);

    AudioRouterInfra::GetInstance().Unlock();
    return result;
}

void AudioRouterIndependentStrategy::SetCallInputDevice(int32_t uid,
    uint32_t streamId, const std::shared_ptr<AudioDeviceDescriptor> &device)
{
    SetMediaInputDevice(uid, streamId, device);
}

std::shared_ptr<AudioDeviceDescriptor> AudioRouterIndependentStrategy::GetCallOutputDevice(int32_t uid,
    uint32_t streamId)
{
    AudioRouterInfra::GetInstance().Lock();

    auto simpleDesc = GetSelectDevice(uid, streamId, SelectDeviceType::CALL_OUTPUT);
    auto result = ConvertSimpleToDevice(simpleDesc);

    AudioRouterInfra::GetInstance().Unlock();
    return GetFinalOutputDeviceForCall(result);
}

void AudioRouterIndependentStrategy::SetCallOutputDevice(int32_t uid,
    uint32_t streamId, const std::shared_ptr<AudioDeviceDescriptor> &device, const std::string caller)
{
    SetMediaOutputDevice(uid, streamId, device);
}

void AudioRouterIndependentStrategy::UpdateCurrentOutputDevice(int32_t uid,
    const std::vector<std::shared_ptr<AudioDeviceDescriptor>> &devices)
{
    uid = uid == INVALID_UID ? SYSTEM_UID : uid;
    AUDIO_INFO_LOG("uid=%{public}d, deviceId=%{public}d", uid,
        !devices.empty() && devices[0] ? devices[0]->deviceId_ : 0);
    auto simpleDevices = ConvertDeviceToSimple(devices);
    AudioRouterInfra::GetInstance().UpdateCurrentOutputDevice(uid, simpleDevices);
}

std::vector<std::shared_ptr<AudioDeviceDescriptor>> AudioRouterIndependentStrategy::GetCurrentOutputDevice(int32_t uid)
{
    uid = uid == INVALID_UID ? SYSTEM_UID : uid;
    auto simpleDescs = AudioRouterInfra::GetInstance().GetCurrentOutputDevice(uid);
    AUDIO_INFO_LOG("uid=%{public}d, deviceId=%{public}d", uid,
        !simpleDescs.empty() && simpleDescs[0] ? simpleDescs[0]->deviceId_ : 0);
    return ConvertSimpleToDevice(simpleDescs);
}

void AudioRouterIndependentStrategy::UpdateCurrentInputDevice(int32_t uid,
    const std::vector<std::shared_ptr<AudioDeviceDescriptor>> &devices)
{
    uid = uid == INVALID_UID ? SYSTEM_UID : uid;
    AUDIO_INFO_LOG("uid=%{public}d, deviceId=%{public}d", uid,
        !devices.empty() && devices[0] ? devices[0]->deviceId_ : 0);
    auto simpleDevices = ConvertDeviceToSimple(devices);
    AudioRouterInfra::GetInstance().UpdateCurrentInputDevice(uid, simpleDevices);
}

std::vector<std::shared_ptr<AudioDeviceDescriptor>> AudioRouterIndependentStrategy::GetCurrentInputDevice(int32_t uid)
{
    uid = uid == INVALID_UID ? SYSTEM_UID : uid;
    auto simpleDescs = AudioRouterInfra::GetInstance().GetCurrentInputDevice(uid);
    AUDIO_INFO_LOG("uid=%{public}d, deviceId=%{public}d", uid,
        !simpleDescs.empty() && simpleDescs[0] ? simpleDescs[0]->deviceId_ : 0);
    return ConvertSimpleToDevice(simpleDescs);
}

AudioRouterIndependentStrategy::SelectDeviceResult AudioRouterIndependentStrategy::GetSelectDeviceByPriority(
    int32_t uid, uint32_t streamId, SelectDeviceType type)
{
    auto& routerInfra = AudioRouterInfra::GetInstance();
    SelectDeviceResult result;
    result.device_ = nullptr;
    result.source_ = SelectDeviceSource::NONE;

    if (ValidStreamId(streamId)) {
        auto streamSelectDevice = routerInfra.GetStreamSelectDevice(uid, streamId);
        if (streamSelectDevice != nullptr) {
            result.device_ = streamSelectDevice;
            result.source_ = SelectDeviceSource::STREAM;
        }
    }

    if (result.device_ == nullptr) {
        auto appSelectInfo = routerInfra.GetAppSelectDevice(type, uid);
        if (appSelectInfo.device_ != nullptr) {
            result.device_ = appSelectInfo.device_;
            result.source_ = SelectDeviceSource::APP;
        }
    }

    if (result.device_ == nullptr) {
        auto systemSelectInfo = routerInfra.GetSystemSelectDevice(type);
        if (systemSelectInfo.device_ != nullptr) {
            result.device_ = systemSelectInfo.device_;
            result.source_ = SelectDeviceSource::SYSTEM;
        }
    }

    return result;
}

std::shared_ptr<AudioDeviceSimpleDescriptor> AudioRouterIndependentStrategy::GetSelectDevice(
    int32_t uid, uint32_t streamId, SelectDeviceType type)
{
    auto& routerInfra = AudioRouterInfra::GetInstance();

    std::shared_ptr<AudioDeviceSimpleDescriptor> simpleDesc = nullptr;

    if (routerInfra.IsSystemUid(uid)) {
        auto systemSelectInfo = routerInfra.GetSystemSelectDevice(type);
        simpleDesc = systemSelectInfo.device_;
    } else {
        auto selectResult = GetSelectDeviceByPriority(uid, streamId, type);
        simpleDesc = selectResult.device_;
    }
    return simpleDesc;
}

}
}
