/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#ifndef ST_AUDIO_ZONE_UTILS_H
#define ST_AUDIO_ZONE_UTILS_H

#include <list>
#include <mutex>
#include <vector>
#include "audio_interrupt_service.h"

namespace OHOS {
namespace AudioStandard {
enum class AudioFocusFilterType {
    FILTER_REMOTE_FOCUS,
    FILTER_LOCAL_FOCUS,
};

class AudioZoneFocusFilter {
public:
    static AudioZoneFocusFilter& GetInstance()
    {
        static AudioZoneFocusFilter instance;
        return instance;
    }

    void UpdateLocalSessionIds(const std::vector<uint32_t> &sessionIds);
    void FilterAudioInterrupt(AudioFocusFilterType filterType,
        std::list<std::pair<AudioInterrupt, AudioFocuState>> &focusList,
        const std::shared_ptr<AudioInterruptService> &interruptService);
private:
    AudioZoneFocusFilter() {}
    ~AudioZoneFocusFilter() {}
    std::vector<uint32_t> GetLocalSessionIds();

    std::vector<uint32_t> localSessionIds_;
    std::mutex localSessionIdsMutex_;
};
} // namespace AudioStandard
} // namespace OHOS

#endif // ST_AUDIO_ZONE_UTILS_H