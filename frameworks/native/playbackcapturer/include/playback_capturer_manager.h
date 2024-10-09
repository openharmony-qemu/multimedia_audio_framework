/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef PLAYBACK_CAPTURER_MANAGER_H
#define PLAYBACK_CAPTURER_MANAGER_H

#include <cstdio>
#include <cstdint>
#include <cstddef>
#include <unordered_set>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include "audio_info.h"
#include "playback_capturer_adapter.h"

namespace OHOS {
namespace AudioStandard {
class ICapturerFilterListener {
public:
    virtual ~ICapturerFilterListener() = default;

    // This will be called when a filter is first enabled or changed.
    virtual int32_t OnCapturerFilterChange(uint32_t sessionId, const AudioPlaybackCaptureConfig &newConfig) = 0;

    // This will be called when a filter released.
    virtual int32_t OnCapturerFilterRemove(uint32_t sessionId) = 0;
};

class PlaybackCapturerManager {
public:
    PlaybackCapturerManager();
    ~PlaybackCapturerManager();
    static PlaybackCapturerManager *GetInstance();
    void SetSupportStreamUsage(std::vector<int32_t> usage);
    bool IsStreamSupportInnerCapturer(int32_t streamUsage);
    bool IsPrivacySupportInnerCapturer(int32_t privacyType);
    void SetCaptureSilentState(bool state);
    bool IsCaptureSilently();
    bool GetInnerCapturerState();
    void SetInnerCapturerState(bool state);

    // add for new playback-capturer
    std::vector<StreamUsage> GetDefaultUsages();
    bool RegisterCapturerFilterListener(ICapturerFilterListener *listener);
    int32_t SetPlaybackCapturerFilterInfo(uint32_t sessionId, const AudioPlaybackCaptureConfig &config);
    int32_t RemovePlaybackCapturerFilterInfo(uint32_t sessionId);
private:
    std::mutex setMutex_;
    std::unordered_set<int32_t> supportStreamUsageSet_;
    std::vector<StreamUsage> defaultUsages_ = { STREAM_USAGE_MEDIA, STREAM_USAGE_MUSIC, STREAM_USAGE_MOVIE,
        STREAM_USAGE_GAME, STREAM_USAGE_AUDIOBOOK };
    bool isCaptureSilently_ = false;
    bool isInnerCapturerRunning_ = false;
    ICapturerFilterListener *listener_ = nullptr;
};

}  // namespace AudioStandard
}  // namespace OHOS
#endif // PLAYBACK_CAPTURER_MANAGER_H
