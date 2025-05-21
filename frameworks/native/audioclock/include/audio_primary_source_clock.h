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

#ifndef AUDIO_PRIMARY_SOURCE_CLOCK_H
#define AUDIO_PRIMARY_SOURCE_CLOCK_H

#include "audio_source_clock.h"

namespace OHOS {
namespace AudioStandard {

class AudioCapturerSourceClock : public AudioSourceClock {
public:
    ~AudioCapturerSourceClock() override {}
    uint64_t GetTimestamp(uint32_t positionInc) final;
    void CheckAndResetTimestamp(uint64_t &timestamp, uint32_t positionInc);

    uint32_t GetFrameCnt() const;
    void Reset();
    void SetFirstTimestampFromHdi(uint64_t hdiTimestamp);
private:
    uint32_t frameCnt_ = 0;
    uint64_t firstTimeStamp_ = 0;
    uint64_t lastTs_ = 0;
    bool isGetTimeStampFromSystemClock_ = false;
};

} // namespace AudioStandard
} // namespace OHOS

#endif // AUDIO_PRIMARY_SOURCE_CLOCK_H
