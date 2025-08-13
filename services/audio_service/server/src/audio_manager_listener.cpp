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
#define LOG_TAG "AudioManagerListenerCallback"
#endif

#include "audio_system_manager.h"
#include "audio_service_log.h"
#include "audio_manager_listener.h"

namespace OHOS {
namespace AudioStandard {
AudioManagerListenerCallback::AudioManagerListenerCallback(const sptr<IStandardAudioServerManagerListener>& listener)
    : listener_(listener)
{
}

AudioManagerListenerCallback::~AudioManagerListenerCallback()
{
}

void AudioManagerListenerCallback::OnAudioParameterChange(const std::string networkId, const AudioParamKey key,
    const std::string& condition, const std::string& value)
{
    if (listener_ != nullptr) {
        listener_->OnAudioParameterChange(networkId, key, condition, value);
    }
}

void AudioManagerListenerCallback::OnCapturerState(bool isActive)
{
    if (listener_ != nullptr) {
        isFirstOnCapturerStateCallbackSent_ = true;
        listener_->OnCapturerState(isActive);
    }
}

void AudioManagerListenerCallback::OnWakeupClose()
{
    if (listener_ != nullptr) {
        listener_->OnWakeupClose();
    }
}

void AudioManagerListenerCallback::TrigerFirstOnCapturerStateCallback(bool isActive)
{
    if (!isFirstOnCapturerStateCallbackSent_.exchange(true)) {
        OnCapturerState(isActive);
    }
}

void AudioManagerListenerCallback::OnDataTransferStateChange(const int32_t &callbackId,
    const AudioRendererDataTransferStateChangeInfo &info)
{
    if (listener_ != nullptr) {
        listener_->OnDataTransferStateChange(callbackId, info);
    }
}

void AudioManagerListenerCallback::OnMuteStateChange(const int32_t &callbackId,
    const int32_t &uid, const uint32_t &sessionId, const bool &isMuted)
{
    CHECK_AND_RETURN(listener_ != nullptr);
    listener_->OnMuteStateChange(callbackId, uid, sessionId, isMuted);
}
} // namespace AudioStandard
} // namespace OHOS
