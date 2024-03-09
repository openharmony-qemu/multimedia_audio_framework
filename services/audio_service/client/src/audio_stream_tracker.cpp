/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "audio_stream_tracker.h"
#include "audio_policy_manager.h"

namespace OHOS {
namespace AudioStandard {
AudioStreamTracker::AudioStreamTracker(AudioMode mode, int32_t clientUid)
{
    AUDIO_DEBUG_LOG("AudioStreamtracker:CTOR");
    eMode_ = mode;
    clientUid_ = clientUid;
    state_ = INVALID;
}

AudioStreamTracker::~AudioStreamTracker() {}

void AudioStreamTracker::RegisterTracker(const AudioRegisterTrackerInfo &registerTrackerInfo,
    const std::shared_ptr<AudioClientTracker> &clientTrackerObj)
{
    AUDIO_DEBUG_LOG("Register tracker entered");
    AudioStreamChangeInfo streamChangeInfo;

    state_ = registerTrackerInfo.state;

    if (eMode_ == AUDIO_MODE_PLAYBACK) {
        streamChangeInfo.audioRendererChangeInfo.clientUID = clientUid_;
        streamChangeInfo.audioRendererChangeInfo.sessionId = registerTrackerInfo.sessionId;
        streamChangeInfo.audioRendererChangeInfo.clientPid = registerTrackerInfo.clientPid;
        streamChangeInfo.audioRendererChangeInfo.rendererState = static_cast<RendererState>(registerTrackerInfo.state);
        streamChangeInfo.audioRendererChangeInfo.rendererInfo = registerTrackerInfo.rendererInfo;
        streamChangeInfo.audioRendererChangeInfo.outputDeviceInfo.deviceRole = OUTPUT_DEVICE;
        streamChangeInfo.audioRendererChangeInfo.channelCount = registerTrackerInfo.channelCount;
    } else {
        streamChangeInfo.audioCapturerChangeInfo.clientUID = clientUid_;
        streamChangeInfo.audioCapturerChangeInfo.sessionId = registerTrackerInfo.sessionId;
        streamChangeInfo.audioCapturerChangeInfo.clientPid = registerTrackerInfo.clientPid;
        streamChangeInfo.audioCapturerChangeInfo.capturerState = static_cast<CapturerState>(registerTrackerInfo.state);
        streamChangeInfo.audioCapturerChangeInfo.capturerInfo = registerTrackerInfo.capturerInfo;
        streamChangeInfo.audioCapturerChangeInfo.inputDeviceInfo.deviceRole = INPUT_DEVICE;
        streamChangeInfo.audioCapturerChangeInfo.appTokenId = registerTrackerInfo.appTokenId;
    }
    AudioPolicyManager::GetInstance().RegisterTracker(eMode_, streamChangeInfo, clientTrackerObj);
}

void AudioStreamTracker::UpdateTracker(const int32_t sessionId, const State state, const int32_t clientPid,
    const AudioRendererInfo &rendererInfo, const AudioCapturerInfo &capturerInfo)
{
    AUDIO_DEBUG_LOG("Update tracker entered");
    AudioStreamChangeInfo streamChangeInfo;

    if (state_ == INVALID || state_ == state) {
        AUDIO_DEBUG_LOG("Update tracker is called in wrong state/same state");
        return;
    }

    state_ = state;
    if (eMode_ == AUDIO_MODE_PLAYBACK) {
        streamChangeInfo.audioRendererChangeInfo.clientUID = clientUid_;
        streamChangeInfo.audioRendererChangeInfo.sessionId = sessionId;
        streamChangeInfo.audioRendererChangeInfo.clientPid = clientPid;
        streamChangeInfo.audioRendererChangeInfo.rendererState = static_cast<RendererState>(state);
        streamChangeInfo.audioRendererChangeInfo.rendererInfo = rendererInfo;
    } else {
        streamChangeInfo.audioCapturerChangeInfo.clientUID = clientUid_;
        streamChangeInfo.audioCapturerChangeInfo.sessionId = sessionId;
        streamChangeInfo.audioCapturerChangeInfo.clientPid = clientPid;
        streamChangeInfo.audioCapturerChangeInfo.capturerState = static_cast<CapturerState>(state);
        streamChangeInfo.audioCapturerChangeInfo.capturerInfo = capturerInfo;
    }
    AudioPolicyManager::GetInstance().UpdateTracker(eMode_, streamChangeInfo);
}
} // namespace AudioStandard
} // namespace OHOS
