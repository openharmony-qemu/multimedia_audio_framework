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
#ifndef AUDIO_SESSION_INFO_H
#define AUDIO_SESSION_INFO_H

namespace OHOS {
namespace AudioStandard {
enum class AudioConcurrencyMode {
    DEFAULT = 0,
    MIX_WITH_OTHERS = 1,
    DUCK_OTHERS = 2,
    PAUSE_OTHERS = 3,
};

struct AudioSessionStrategy {
    AudioConcurrencyMode concurrencyMode;
};

enum class AudioSessionDeactiveReason {
    LOW_PRIORITY = 0, // All audio streams have been interrupted.
    TIMEOUT = 1, // The audio session remains empty for one minute.
};

struct AudioSessionDeactiveEvent {
    AudioSessionDeactiveReason deactiveReason;
};

enum class AudioSessionType {
    DEFAULT = 0,
    MEDIA = 1,
    SONIFICATION = 2,
    CALL = 3,
    VOIP = 4,
    SYSTEM = 5,
    NOTIFICATION = 6,
    DTMF = 7,
    VOICE_ASSISTANT = 8,
};
} // namespace AudioStandard
} // namespace OHOS
#endif // AUDIO_SESSION_INFO_H
