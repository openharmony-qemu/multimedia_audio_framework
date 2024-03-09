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
 
#ifndef AUDIO_STREAM_TRACKER_H
#define AUDIO_STREAM_TRACKER_H

#include "audio_info.h"
#include "audio_log.h"
#include "audio_stream_manager.h"

namespace OHOS {
namespace AudioStandard {
class AudioStreamTracker {
public:
    AudioStreamTracker(AudioMode mode, int32_t clientUid);
    virtual ~AudioStreamTracker();
    void RegisterTracker(const AudioRegisterTrackerInfo &registerTrackerInfo,
        const std::shared_ptr<AudioClientTracker> &clientTrackerObj);
    void UpdateTracker(const int32_t sessionId, const State state, const int32_t clientPid,
        const AudioRendererInfo &rendererInfo, const AudioCapturerInfo &capturerInfo);

private:
    int32_t clientUid_ = -1;
    AudioMode eMode_; // to determine renderer or capturer
    State state_;
};
} // namespace AudioStandard
} // namespace OHOS
#endif // AUDIO_STREAM_TRACKER_H
