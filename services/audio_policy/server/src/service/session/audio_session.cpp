/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#undef LOG_TAG
#define LOG_TAG "AudioSession"

#include "audio_session.h"

#include "audio_policy_log.h"
#include "audio_errors.h"
#include "audio_session_state_monitor.h"

namespace OHOS {
namespace AudioStandard {
AudioSession::AudioSession(const int32_t callerPid, const AudioSessionStrategy &strategy,
    const std::shared_ptr<AudioSessionStateMonitor> audioSessionStateMonitor)
{
    AUDIO_INFO_LOG("AudioSession()");
    callerPid_ = callerPid;
    strategy_ = strategy;
    audioSessionStateMonitor_ = audioSessionStateMonitor;
    state_ = AudioSessionState::SESSION_NEW;
}

AudioSession::~AudioSession()
{
    AUDIO_ERR_LOG("~AudioSession()");
}

int32_t AudioSession::Activate()
{
    std::lock_guard<std::mutex> lock(sessionMutex_);
    state_ = AudioSessionState::SESSION_ACTIVE;
    AUDIO_INFO_LOG("Audio session state change: pid %{public}d, state %{public}d",
        callerPid_, static_cast<int32_t>(state_));
    return SUCCESS;
}

int32_t AudioSession::Deactivate()
{
    std::lock_guard<std::mutex> lock(sessionMutex_);
    state_ = AudioSessionState::SESSION_DEACTIVE;
    interruptMap_.clear();
    AUDIO_INFO_LOG("Audio session state change: pid %{public}d, state %{public}d",
        callerPid_, static_cast<int32_t>(state_));
    return SUCCESS;
}

AudioSessionState AudioSession::GetSessionState()
{
    std::lock_guard<std::mutex> lock(sessionMutex_);
    AUDIO_INFO_LOG("pid %{public}d, state %{public}d", callerPid_, static_cast<int32_t>(state_));
    return state_;
}

void AudioSession::SetSessionStrategy(const AudioSessionStrategy strategy)
{
    std::lock_guard<std::mutex> lock(sessionMutex_);
    strategy_ = strategy;
}

AudioSessionStrategy AudioSession::GetSessionStrategy()
{
    std::lock_guard<std::mutex> lock(sessionMutex_);
    AUDIO_INFO_LOG("GetSessionStrategy: pid %{public}d, strategy_.concurrencyMode %{public}d",
        callerPid_, static_cast<int32_t>(strategy_.concurrencyMode));
    return strategy_;
}

int32_t AudioSession::AddAudioInterrpt(const std::pair<AudioInterrupt, AudioFocuState> interruptPair)
{
    uint32_t streamId = interruptPair.first.streamId;
    AUDIO_INFO_LOG("AddAudioInterrpt: streamId %{public}u", streamId);

    std::lock_guard<std::mutex> lock(sessionMutex_);
    if (interruptMap_.count(streamId) != 0) {
        AUDIO_WARNING_LOG("The streamId has been added. The old interrupt will be coverd.");
    }
    interruptMap_[streamId] = interruptPair;
    auto monitor = audioSessionStateMonitor_.lock();
    if (monitor != nullptr) {
        monitor->StopMonitor(callerPid_);
    }
    return SUCCESS;
}

int32_t AudioSession::RemoveAudioInterrpt(const std::pair<AudioInterrupt, AudioFocuState> interruptPair)
{
    uint32_t streamId = interruptPair.first.streamId;
    AUDIO_INFO_LOG("RemoveAudioInterrpt: streamId %{public}u", streamId);

    std::lock_guard<std::mutex> lock(sessionMutex_);
    if (interruptMap_.count(streamId) == 0) {
        AUDIO_WARNING_LOG("The streamId has been removed.");
        return SUCCESS;
    }
    interruptMap_.erase(streamId);

    auto monitor = audioSessionStateMonitor_.lock();
    if (interruptMap_.empty() && monitor != nullptr) {
        monitor->StartMonitor(callerPid_);
    }
    return SUCCESS;
}

int32_t AudioSession::RemoveAudioInterrptByStreamId(const uint32_t &streamId)
{
    AUDIO_INFO_LOG("RemoveAudioInterrptByStreamId: streamId %{public}u", streamId);

    std::lock_guard<std::mutex> lock(sessionMutex_);
    if (interruptMap_.count(streamId) == 0) {
        AUDIO_WARNING_LOG("The streamId has been removed.");
        return SUCCESS;
    }
    interruptMap_.erase(streamId);

    auto monitor = audioSessionStateMonitor_.lock();
    if (interruptMap_.empty() && monitor != nullptr) {
        monitor->StartMonitor(callerPid_);
    }

    return SUCCESS;
}

bool AudioSession::IsAudioSessionEmpty()
{
    std::lock_guard<std::mutex> lock(sessionMutex_);
    return interruptMap_.size() == 0;
}

bool AudioSession::IsAudioRendererEmpty()
{
    std::lock_guard<std::mutex> lock(sessionMutex_);
    for (const auto &iter : interruptMap_) {
        if (iter.second.first.audioFocusType.streamType != STREAM_DEFAULT) {
            return false;
        }
    }
    return true;
}
} // namespace AudioStandard
} // namespace OHOS
