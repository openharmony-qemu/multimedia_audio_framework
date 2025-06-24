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
#define LOG_TAG "AudioPolicyManagerListenerProxy"
#endif

#include "audio_policy_manager_listener.h"
#include "audio_policy_log.h"

namespace OHOS {
namespace AudioStandard {
AudioPolicyManagerListenerCallback::AudioPolicyManagerListenerCallback(
    const sptr<IStandardAudioPolicyManagerListener> &listener) : listener_(listener)
{
        AUDIO_DEBUG_LOG("AudioPolicyManagerListenerCallback: Instance create");
}

AudioPolicyManagerListenerCallback::~AudioPolicyManagerListenerCallback()
{
    AUDIO_DEBUG_LOG("AudioPolicyManagerListenerCallback: Instance destroy");
}

void AudioPolicyManagerListenerCallback::OnInterrupt(const InterruptEventInternal &interruptEvent)
{
    if (listener_ != nullptr) {
        listener_->OnInterrupt(interruptEvent);
    }
}

void AudioPolicyManagerListenerCallback::OnAvailableDeviceChange(const AudioDeviceUsage usage,
    const DeviceChangeAction &deviceChangeAction)
{
    CHECK_AND_RETURN_LOG(listener_ != nullptr, "listener_ is nullptr");
    listener_->OnAvailableDeviceChange(usage, deviceChangeAction);
}

bool AudioPolicyManagerListenerCallback::OnQueryClientType(const std::string &bundleName, uint32_t uid)
{
    CHECK_AND_RETURN_RET_LOG(listener_ != nullptr, false, "listener_ is nullptr");
    bool ret = false;
    listener_->OnQueryClientType(bundleName, uid, ret);
    return ret;
}

bool AudioPolicyManagerListenerCallback::OnCheckClientInfo(const std::string &bundleName, int32_t &uid, int32_t pid)
{
    CHECK_AND_RETURN_RET_LOG(listener_ != nullptr, false, "listener_ is nullptr");
    bool ret = false;
    listener_->OnCheckClientInfo(bundleName, uid, pid, ret);
    return ret;
}

bool AudioPolicyManagerListenerCallback::OnQueryAllowedPlayback(int32_t uid, int32_t pid)
{
    CHECK_AND_RETURN_RET_LOG(listener_ != nullptr, false, "listener_ is nullptr");
    bool ret = false;
    listener_->OnQueryAllowedPlayback(uid, pid, ret);
    return ret;
}

void AudioPolicyManagerListenerCallback::OnBackgroundMute(const int32_t uid)
{
    CHECK_AND_RETURN_LOG(listener_ != nullptr, "listener_ is nullptr");
    listener_->OnBackgroundMute(uid);
}

bool AudioPolicyManagerListenerCallback::OnQueryBundleNameIsInList(const std::string &bundleName,
    const std::string &listType)
{
    CHECK_AND_RETURN_RET_LOG(listener_ != nullptr, false, "listener_ is nullptr");
    bool ret = false;
    listener_->OnQueryBundleNameIsInList(bundleName, listType, ret);
    return ret;
}
} // namespace AudioStandard
} // namespace OHOS
