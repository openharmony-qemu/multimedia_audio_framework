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
#ifndef AUDIO_STREAM_INFO_EXT_H
#define AUDIO_STREAM_INFO_EXT_H

namespace OHOS {
namespace AudioStandard {

const uint32_t CH_MODE_OFFSET = 44;
const uint32_t CH_HOA_ORDNUM_OFFSET = 0;
const uint32_t CH_HOA_COMORD_OFFSET = 8;
const uint32_t CH_HOA_NOR_OFFSET = 12;
const uint64_t CH_MODE_MASK = ((1ULL << 4) - 1ULL) << CH_MODE_OFFSET;
const uint64_t CH_HOA_ORDNUM_MASK = ((1ULL << 8) - 1ULL) << CH_HOA_ORDNUM_OFFSET;
const uint64_t CH_HOA_COMORD_MASK = ((1ULL << 4) - 1ULL) << CH_HOA_COMORD_OFFSET;
const uint64_t CH_HOA_NOR_MASK = ((1ULL << 4) - 1ULL) << CH_HOA_NOR_OFFSET;
const uint32_t SAMPLE_RATE_RESOLUTION_10 = 10;

enum AudioStreamType {
    /**
     * Indicates audio streams default.
     */
    STREAM_DEFAULT = -1,
    /**
     * Indicates audio streams of voices in calls.
     */
    STREAM_VOICE_CALL = 0,
    /**
     * Indicates audio streams for music.
     */
    STREAM_MUSIC = 1,
    /**
     * Indicates audio streams for ringtones.
     */
    STREAM_RING = 2,
    /**
     * Indicates audio streams for media.
     * Deprecated
     */
    STREAM_MEDIA = 3,
    /**
     * Indicates audio streams used for voice assistant and text-to-speech (TTS).
     */
    STREAM_VOICE_ASSISTANT = 4,
    /**
     * Indicates audio streams for system sounds.
     */
    STREAM_SYSTEM = 5,
    /**
     * Indicates audio streams for alarms.
     */
    STREAM_ALARM = 6,
    /**
     * Indicates audio streams for notifications.
     */
    STREAM_NOTIFICATION = 7,
    /**
     * Indicates audio streams for voice calls routed through a connected Bluetooth device.
     * Deprecated
     */
    STREAM_BLUETOOTH_SCO = 8,
    /**
     * Indicates audio streams for enforced audible.
     */
    STREAM_ENFORCED_AUDIBLE = 9,
    /**
     * Indicates audio streams for dual-tone multi-frequency (DTMF) tones.
     */
    STREAM_DTMF = 10,
    /**
     * Indicates audio streams exclusively transmitted through the speaker (text-to-speech) of a device.
     * Deprecated
     */
    STREAM_TTS =  11,
    /**
     * Indicates audio streams used for prompts in terms of accessibility.
     */
    STREAM_ACCESSIBILITY = 12,
    /**
     * Indicates special scene used for recording.
     * Deprecated
     */
    STREAM_RECORDING = 13,
    /**
     * Indicates audio streams for movie.
     * New
     */
    STREAM_MOVIE = 14,
    /**
     * Indicates audio streams for game.
     * New
     */
    STREAM_GAME = 15,
    /**
     * Indicates audio streams for speech.
     * New
     */
    STREAM_SPEECH = 16,
    /**
     * Indicates audio streams for enforced audible.
     * New
     */
    STREAM_SYSTEM_ENFORCED = 17,
    /**
     * Indicates audio streams used for ultrasonic ranging.
     */
    STREAM_ULTRASONIC = 18,
    /**
     * Indicates audio streams for wakeup.
     */
    STREAM_WAKEUP = 19,
    /**
     * Indicates audio streams for voice message.
     */
    STREAM_VOICE_MESSAGE = 20,
    /**
     * Indicates audio streams for navigation.
     */
    STREAM_NAVIGATION = 21,
    /**
     * Indicates audio streams for ForceStop.
     */
    STREAM_INTERNAL_FORCE_STOP = 22,
    /**
     * Indicates audio streams for voice call.
     */
    STREAM_SOURCE_VOICE_CALL = 23,
    /**
     * Indicates audio streams for voip call.
     */
    STREAM_VOICE_COMMUNICATION = 24,
    /**
     * Indicates audio streams for voice ringtones.
     */
    STREAM_VOICE_RING = 25,
    /**
     * Indicates audio streams for voice call assistant.
     */
    STREAM_VOICE_CALL_ASSISTANT = 26,

    /**
     * Indicates audio streams for camcorder.
     */
    STREAM_CAMCORDER = 27,

    /**
     * Indicates audio streams type is APP.
     */
    STREAM_APP = 28,

    /**
     * Indicates the max value of audio stream type (except STREAM_ALL).
     */
    STREAM_TYPE_MAX = STREAM_APP,

    /**
     * Indicates audio streams used for only one volume bar of a device.
     */
    STREAM_ALL = 100,
};
} // namespace AudioStandard
} // namespace OHOS
#endif // AUDIO_STREAM_INFO_ext_H