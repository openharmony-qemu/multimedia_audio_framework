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

#include "audio_interrupt_service.h"

namespace OHOS {
namespace AudioStandard {
class AudioZoneUtils {
public:
    static AudioZoneUtils& GetInstance()
    {
        static AudioZoneUtils instance;
        return instance;
    }

    void FilterRemoteAudioInterrupt(const std::shared_ptr<AudioInterruptService> &interruptService,
        std::list<std::pair<AudioInterrupt, AudioFocuState>> &focusList);
private:
    AudioZoneUtils() {}
    ~AudioZoneUtils() {}
};
} // namespace AudioStandard
} // namespace OHOS

#endif // ST_AUDIO_ZONE_UTILS_H