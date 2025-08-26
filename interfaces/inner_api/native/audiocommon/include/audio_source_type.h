/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

#ifndef AUDIO_SOURCE_TYPE_H
#define AUDIO_SOURCE_TYPE_H

/**
* Enumerates the capturer source type
*/
#ifdef __cplusplus
namespace OHOS {
namespace AudioStandard {
#endif
enum SourceType {
    SOURCE_TYPE_INVALID = -1,
    SOURCE_TYPE_MIC,
    SOURCE_TYPE_VOICE_RECOGNITION = 1,
    SOURCE_TYPE_PLAYBACK_CAPTURE = 2,
    SOURCE_TYPE_WAKEUP = 3,
    SOURCE_TYPE_VOICE_CALL = 4,
    SOURCE_TYPE_VOICE_COMMUNICATION = 7,
    SOURCE_TYPE_ULTRASONIC = 8,
    SOURCE_TYPE_VIRTUAL_CAPTURE = 9, // only for voice call
    SOURCE_TYPE_VOICE_MESSAGE = 10,
    SOURCE_TYPE_REMOTE_CAST = 11,
    SOURCE_TYPE_VOICE_TRANSCRIPTION = 12,
    SOURCE_TYPE_CAMCORDER = 13,
    SOURCE_TYPE_UNPROCESSED = 14,
    SOURCE_TYPE_EC = 15,
    SOURCE_TYPE_MIC_REF = 16,
    SOURCE_TYPE_LIVE = 17,
    SOURCE_TYPE_OFFLOAD_CAPTURE = 18,
    SOURCE_TYPE_MAX = SOURCE_TYPE_OFFLOAD_CAPTURE,
};

#ifdef __cplusplus
} // namespace AudioStandard
} // namespace OHOS
#endif
#endif //AUDIO_SOURCE_TYPE_H
