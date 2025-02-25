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

#ifndef PLAYBACK_CAPTURER_FILTER_LISTENER_H
#define PLAYBACK_CAPTURER_FILTER_LISTENER_H

#include <cstdint>

#include "audio_info.h"

namespace OHOS {
namespace AudioStandard {
class ICapturerFilterListener {
public:
    virtual ~ICapturerFilterListener() = default;

    // This will be called when a filter is first enabled or changed.
    virtual int32_t OnCapturerFilterChange(uint32_t sessionId, const AudioPlaybackCaptureConfig &newConfig,
        int32_t innerCapId) = 0;

    // This will be called when a filter released.
    virtual int32_t OnCapturerFilterRemove(uint32_t sessionId, int32_t innerCapId) = 0;
};
}  // namespace AudioStandard
}  // namespace OHOS
#endif // PLAYBACK_CAPTURER_FILTER_LISTENER_H