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
    SOURCE_TYPE_MAX = SOURCE_TYPE_VOICE_TRANSCRIPTION
};

typedef enum {
    /** Invalid audio source */
    AUDIO_SOURCE_INVALID = -1,
    /** Default audio source */
    AUDIO_SOURCE_DEFAULT = 0,
    /** Microphone */
    AUDIO_MIC = 1,
    /** Uplink voice */
    AUDIO_VOICE_UPLINK = 2,
    /** Downlink voice */
    AUDIO_VOICE_DOWNLINK = 3,
    /** Voice call */
    AUDIO_VOICE_CALL = 4,
    /** Camcorder */
    AUDIO_CAMCORDER = 5,
    /** Voice recognition */
    AUDIO_VOICE_RECOGNITION = 6,
    /** Voice communication */
    AUDIO_VOICE_COMMUNICATION = 7,
    /** Remote submix */
    AUDIO_REMOTE_SUBMIX = 8,
    /** Unprocessed audio */
    AUDIO_UNPROCESSED = 9,
    /** Voice performance */
    AUDIO_VOICE_PERFORMANCE = 10,
    /** Echo reference */
    AUDIO_ECHO_REFERENCE = 1997,
    /** Radio tuner */
    AUDIO_RADIO_TUNER = 1998,
    /** Hotword */
    AUDIO_HOTWORD = 1999,
    /** Extended remote submix */
    AUDIO_REMOTE_SUBMIX_EXTEND = 10007,
} AudioSourceType;
#ifdef __cplusplus
} // namespace AudioStandard
} // namespace OHOS
#endif
#endif //AUDIO_SOURCE_TYPE_H
