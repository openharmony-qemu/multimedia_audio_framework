/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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
#define LOG_TAG "AudioOffloadStream"
#endif

#include "audio_offload_stream.h"

#include "audio_policy_log.h"
#include "audio_server_proxy.h"

namespace OHOS {
namespace AudioStandard {

const int32_t UID_DAUDIO = 3055;

uint32_t AudioOffloadStream::GetOffloadSessionId(OffloadAdapter offloadAdapter)
{
    return offloadSessionIdMap_[offloadAdapter];
}

void AudioOffloadStream::SetOffloadStatus(OffloadAdapter offloadAdapter, uint32_t sessionId)
{
    std::lock_guard<std::mutex> lock(offloadMutex_);

    for (auto &iter : offloadSessionIdMap_) {
        if (iter.second == sessionId) {
            // Find target sessionId, means offload status is already been set or the stream is moved from
            // one offload pipe to another offload pipe, map record should be reset.
            iter.second = NO_OFFLOAD_STREAM_SESSIONID;
        }
    }

    CHECK_AND_RETURN_LOG(offloadAdapter < OFFLOAD_IN_ADAPTER_SIZE, "Invalid offload adapter");
    AUDIO_INFO_LOG("Set offload session: %{public}u", sessionId);
    offloadSessionIdMap_[offloadAdapter] = sessionId;
    SetOffloadStatusInternal(sessionId, offloadAdapter);
}

void AudioOffloadStream::UnsetOffloadStatus(uint32_t sessionId)
{
    std::lock_guard<std::mutex> lock(offloadMutex_);

    for (auto &iter : offloadSessionIdMap_) {
        if (iter.second == sessionId) {
            iter.second = NO_OFFLOAD_STREAM_SESSIONID;
            CHECK_AND_CONTINUE_LOG(iter.first < OFFLOAD_IN_ADAPTER_SIZE, "Invalid offload adapter");
            UnsetOffloadStatusInternal(sessionId, iter.first);
        }
    }
}

void AudioOffloadStream::UpdateOffloadStatusFromUpdateTracker(uint32_t sessionId, RendererState state)
{
    std::lock_guard<std::mutex> lock(offloadMutex_);

    for (auto &iter : offloadSessionIdMap_) {
        if (iter.second == sessionId) {
            if (state == RENDERER_RUNNING) {
                AudioServerProxy::GetInstance().SetOffloadModeProxy(
                    sessionId, static_cast<int32_t>(currentPowerState_), false);
            }
        }
    }
}

void AudioOffloadStream::HandlePowerStateChanged(PowerMgr::PowerState state)
{
    if (currentPowerState_ == state) {
        return;
    }
    currentPowerState_ = state;

    std::lock_guard<std::mutex> lock(offloadMutex_);
    for (auto &iter : offloadSessionIdMap_) {
        if (iter.second != NO_OFFLOAD_STREAM_SESSIONID) {
            // Only update offload power state, other actions already done before
            AudioServerProxy::GetInstance().SetOffloadModeProxy(
                iter.second, static_cast<int32_t>(currentPowerState_), false);
        }
    }
}

// must be called with offloadMutex_ lock
void AudioOffloadStream::SetOffloadStatusInternal(uint32_t sessionId, OffloadAdapter offloadAdapter)
{
    AUDIO_INFO_LOG("Set offload enable for stream: %{public}u", sessionId);

    // Offload stream need:
    // 1) Set offload enabled and current power state to renderer stream in audioservice
    // 2) Set offload stream sessionId to volume module
    // 3) Update pipe type in stream collector
    audioPolicyManager_.SetOffloadSessionId(sessionId, offloadAdapter);
    AudioServerProxy::GetInstance().SetOffloadModeProxy(
        sessionId, static_cast<int32_t>(currentPowerState_), false);
    streamCollector_.UpdateRendererPipeInfo(sessionId, PIPE_TYPE_OFFLOAD);
}

// must be called with offloadMutex_ lock
void AudioOffloadStream::UnsetOffloadStatusInternal(uint32_t sessionId, OffloadAdapter offloadAdapter)
{
    AUDIO_INFO_LOG("Unset offload enable for stream: %{public}u", sessionId);

    AudioServerProxy::GetInstance().UnsetOffloadModeProxy(sessionId);
    audioPolicyManager_.ResetOffloadSessionId(offloadAdapter);
    streamCollector_.UpdateRendererPipeInfo(sessionId, PIPE_TYPE_NORMAL_OUT);
}

void AudioOffloadStream::Dump(std::string &dumpString)
{
    std::lock_guard<std::mutex> lock(offloadMutex_);
    dumpString += ("\n");
    dumpString += ("OffloadSessionIdPrimary: " + std::to_string(offloadSessionIdMap_[OFFLOAD_IN_PRIMARY]) + "\n");
    dumpString += ("OffloadSessionIdRemote: " + std::to_string(offloadSessionIdMap_[OFFLOAD_IN_REMOTE]) + "\n");
}

std::vector<SinkInput> AudioOffloadStream::FilterSinkInputs(int32_t sessionId, std::vector<SinkInput> sinkInputs)
{
    // find sink-input id with audioRendererFilter
    std::vector<SinkInput> targetSinkInputs = {};

    for (size_t i = 0; i < sinkInputs.size(); i++) {
        CHECK_AND_CONTINUE_LOG(sinkInputs[i].uid != UID_DAUDIO,
            "Find sink-input with daudio[%{public}d]", sinkInputs[i].pid);
        CHECK_AND_CONTINUE_LOG(sinkInputs[i].streamType != STREAM_DEFAULT,
            "Sink-input[%{public}zu] of effect sink, don't move", i);
        if (sessionId == sinkInputs[i].streamId) {
            targetSinkInputs.push_back(sinkInputs[i]);
        }
    }
    return targetSinkInputs;
}
} // namespace AudioStandard
} // namespace OHOS
