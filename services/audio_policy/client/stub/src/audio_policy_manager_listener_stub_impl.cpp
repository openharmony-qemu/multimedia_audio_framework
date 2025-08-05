/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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
#define LOG_TAG "AudioPolicyManagerListenerStubImpl"
#endif

#include "audio_errors.h"
#include "audio_policy_log.h"
#include "audio_policy_manager_listener_stub_impl.h"
#include "audio_utils.h"

namespace OHOS {
namespace AudioStandard {
AudioPolicyManagerListenerStubImpl::AudioPolicyManagerListenerStubImpl()
{
}

AudioPolicyManagerListenerStubImpl::~AudioPolicyManagerListenerStubImpl()
{
}

int32_t AudioPolicyManagerListenerStubImpl::OnInterrupt(const InterruptEventInternal &interruptEvent)
{
    std::shared_ptr<AudioInterruptCallback> cb = callback_.lock();
    if (cb != nullptr) {
        cb->OnInterrupt(interruptEvent);
    } else {
        AUDIO_WARNING_LOG("AudioPolicyManagerListenerStubImpl: callback_ is nullptr");
    }
    return SUCCESS;
}

int32_t AudioPolicyManagerListenerStubImpl::OnAvailableDeviceChange(uint32_t usage,
    const DeviceChangeAction &deviceChangeAction)
{
    std::shared_ptr<AudioManagerAvailableDeviceChangeCallback> availabledeviceChangedCallback =
        audioAvailableDeviceChangeCallback_.lock();

    CHECK_AND_RETURN_RET_LOG(availabledeviceChangedCallback != nullptr, AUDIO_INVALID_PARAM,
        "OnAvailableDeviceChange: deviceChangeCallback_ or deviceChangeAction is nullptr");

    availabledeviceChangedCallback->OnAvailableDeviceChange(static_cast<AudioDeviceUsage>(usage), deviceChangeAction);
    return SUCCESS;
}

int32_t AudioPolicyManagerListenerStubImpl::OnQueryClientType(const std::string &bundleName, uint32_t uid, bool &ret)
{
    std::shared_ptr<AudioQueryClientTypeCallback> audioQueryClientTypeCallback =
        audioQueryClientTypeCallback_.lock();

    CHECK_AND_RETURN_RET_LOG(audioQueryClientTypeCallback != nullptr, AUDIO_INVALID_PARAM,
        "audioQueryClientTypeCallback_ is nullptr");
    ret = audioQueryClientTypeCallback->OnQueryClientType(bundleName, uid);
    return SUCCESS;
}

int32_t AudioPolicyManagerListenerStubImpl::OnCheckClientInfo(
    const std::string &bundleName, int32_t &uid, int32_t pid, bool &ret)
{
    std::shared_ptr<AudioClientInfoMgrCallback> audioClientInfoMgrCallback = audioClientInfoMgrCallback_.lock();

    CHECK_AND_RETURN_RET_LOG(audioClientInfoMgrCallback != nullptr, AUDIO_INVALID_PARAM,
        "audioClientInfoMgrCallback is nullptr");
    ret = audioClientInfoMgrCallback->OnCheckClientInfo(bundleName, uid, pid);
    return SUCCESS;
}

int32_t AudioPolicyManagerListenerStubImpl::OnCheckVKBInfo(const std::string &bundleName, bool &isValid)
{
    std::shared_ptr<AudioVKBInfoMgrCallback> audioVKBInfoMgrCallback = audioVKBInfoMgrCallback_.lock();

    CHECK_AND_RETURN_RET_LOG(audioVKBInfoMgrCallback != nullptr, false, "audioVKBInfoMgrCallback is nullptr");

    isValid = audioVKBInfoMgrCallback->OnCheckVKBInfo(bundleName);
    return SUCCESS;
}

int32_t AudioPolicyManagerListenerStubImpl::OnQueryAllowedPlayback(int32_t uid, int32_t pid, bool &ret)
{
    std::shared_ptr<AudioQueryAllowedPlaybackCallback> audioQueryAllowedPlaybackCallback =
        audioQueryAllowedPlaybackCallback_.lock();

    CHECK_AND_RETURN_RET_LOG(audioQueryAllowedPlaybackCallback != nullptr, AUDIO_INVALID_PARAM,
        "audioQueryAllowedPlaybackCallback_ is nullptr");
    ret = audioQueryAllowedPlaybackCallback->OnQueryAllowedPlayback(uid, pid);
    return SUCCESS;
}

int32_t AudioPolicyManagerListenerStubImpl::OnBackgroundMute(const int32_t uid)
{
    std::shared_ptr<AudioBackgroundMuteCallback> audioBackgroundMuteCallback =
    audioBackgroundMuteCallback_.lock();

    CHECK_AND_RETURN_RET_LOG(audioBackgroundMuteCallback != nullptr, AUDIO_INVALID_PARAM,
        "audioBackgroundMuteCallback_ is nullptr");

    audioBackgroundMuteCallback->OnBackgroundMute(uid);
    return SUCCESS;
}

int32_t AudioPolicyManagerListenerStubImpl::OnQueryDeviceVolumeBehavior(VolumeBehavior &volumeBehavior)
{
    std::shared_ptr<AudioQueryDeviceVolumeBehaviorCallback> audioQueryDeviceVolumeBehaviorCallback =
        audioQueryDeviceVolumeBehaviorCallback_.lock();

    CHECK_AND_RETURN_RET_LOG(audioQueryDeviceVolumeBehaviorCallback != nullptr, AUDIO_INVALID_PARAM,
        "audioQueryDeviceVolumeBehaviorCallback is nullptr");
    volumeBehavior = audioQueryDeviceVolumeBehaviorCallback->OnQueryDeviceVolumeBehavior();
    AUDIO_INFO_LOG("isReady [%{public}d], isVolumeControlDisabled [%{public}d], databaseVolumeName [%{public}s]",
        volumeBehavior.isReady, volumeBehavior.isVolumeControlDisabled, volumeBehavior.databaseVolumeName.c_str());
    return SUCCESS;
}

int32_t AudioPolicyManagerListenerStubImpl::OnQueryBundleNameIsInList(const std::string &bundleName,
    const std::string &listType, bool &ret)
{
    std::shared_ptr<AudioQueryBundleNameListCallback> audioQueryBundleNameListCallback =
        audioQueryBundleNameListCallback_.lock();

    CHECK_AND_RETURN_RET_LOG(audioQueryBundleNameListCallback != nullptr, AUDIO_INVALID_PARAM,
        "audioQueryBundleNameListCallback_ is nullptr");
    ret = audioQueryBundleNameListCallback->OnQueryBundleNameIsInList(bundleName, listType);
    return SUCCESS;
}

int32_t AudioPolicyManagerListenerStubImpl::OnRouteUpdate(uint32_t routeFlag, const std::string &networkId)
{
    std::shared_ptr<AudioRouteCallback> cb = audioRouteCallback_.lock();
    CHECK_AND_RETURN_RET_LOG(cb != nullptr, AUDIO_INVALID_PARAM, "audioRouteCallback_ is nullptr");
    cb->OnRouteUpdate(routeFlag, networkId);
    return SUCCESS;
}

void AudioPolicyManagerListenerStubImpl::SetAudioRouteCallback(const std::weak_ptr<AudioRouteCallback> &callback)
{
    audioRouteCallback_ = callback;
}

void AudioPolicyManagerListenerStubImpl::SetInterruptCallback(const std::weak_ptr<AudioInterruptCallback> &callback)
{
    callback_ = callback;
}

void AudioPolicyManagerListenerStubImpl::SetAvailableDeviceChangeCallback(
    const std::weak_ptr<AudioManagerAvailableDeviceChangeCallback> &cb)
{
    audioAvailableDeviceChangeCallback_ = cb;
}

void AudioPolicyManagerListenerStubImpl::SetQueryClientTypeCallback(
    const std::weak_ptr<AudioQueryClientTypeCallback> &cb)
{
    audioQueryClientTypeCallback_ = cb;
}

void AudioPolicyManagerListenerStubImpl::SetAudioClientInfoMgrCallback(
    const std::weak_ptr<AudioClientInfoMgrCallback> &cb)
{
    audioClientInfoMgrCallback_ = cb;
}

void AudioPolicyManagerListenerStubImpl::SetAudioVKBInfoMgrCallback(
    const std::weak_ptr<AudioVKBInfoMgrCallback> &cb)
{
    audioVKBInfoMgrCallback_ = cb;
}

void AudioPolicyManagerListenerStubImpl::SetQueryAllowedPlaybackCallback(
    const std::weak_ptr<AudioQueryAllowedPlaybackCallback> &cb)
{
    audioQueryAllowedPlaybackCallback_ = cb;
}

void AudioPolicyManagerListenerStubImpl::SetBackgroundMuteCallback(
    const std::weak_ptr<AudioBackgroundMuteCallback> &cb)
{
    audioBackgroundMuteCallback_ = cb;
}

void AudioPolicyManagerListenerStubImpl::SetQueryBundleNameListCallback(
    const std::weak_ptr<AudioQueryBundleNameListCallback> &cb)
{
    audioQueryBundleNameListCallback_ = cb;
}

void AudioPolicyManagerListenerStubImpl::SetQueryDeviceVolumeBehaviorCallback(
    const std::weak_ptr<AudioQueryDeviceVolumeBehaviorCallback> &cb)
{
    audioQueryDeviceVolumeBehaviorCallback_ = cb;
}
} // namespace AudioStandard
} // namespace OHOS
