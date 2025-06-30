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

#ifndef CAPTURER_CLOCK_MANAGER_H
#define CAPTURER_CLOCK_MANAGER_H

#include "capturer_clock.h"
#include "audio_source_clock.h"

namespace OHOS {
namespace AudioStandard {

class CapturerClockManager {
public:
    static CapturerClockManager &GetInstance(void);

    std::shared_ptr<CapturerClock> CreateCapturerClock(uint32_t sessionId, uint32_t sampleRate);
    void DeleteCapturerClock(uint32_t sessionId);
    std::shared_ptr<CapturerClock> GetCapturerClock(uint32_t sessionId);

    bool RegisterAudioSourceClock(uint32_t captureId, std::shared_ptr<AudioSourceClock> clock);
    void DeleteAudioSourceClock(uint32_t captureId);
    std::shared_ptr<AudioSourceClock> GetAudioSourceClock(uint32_t captureId);
private:
    std::map<uint32_t, std::shared_ptr<CapturerClock>> capturerClockPool_;
    std::map<uint32_t, std::shared_ptr<AudioSourceClock>> audioSrcClockPool_;
    std::mutex clockPoolMtx_;
};

} // namespace AudioStandard
} // namespace OHOS

#endif // CAPTURER_CLOCK_MANAGER_H
