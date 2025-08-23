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
    INVALID = -1,
    DEFAULT = 0,
    MIX_WITH_OTHERS = 1,
    DUCK_OTHERS = 2,
    PAUSE_OTHERS = 3,
    SILENT = 4,
    STANDALONE = 5,
};

struct AudioSessionStrategy {
    mutable AudioConcurrencyMode concurrencyMode;
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

/**
 * Audio session scene.
 * @since 20
 */
enum class AudioSessionScene {
    /**
     * Invalid audio session scene.
     * @since 20
     */
    INVALID = -1,
    /**
     * Scene for media.
     * @since 20
     */
    MEDIA = 0,
    /**
     * Scene for game.
     * @since 20
     */
    GAME = 1,
    /**
     * Scene for voice communication.
     * @since 20
     */
    VOICE_COMMUNICATION = 2,
};

/**
 * Enumerates the session state change hints.
 * @since 20
 */
enum class AudioSessionStateChangeHint {
    /**
     * Invalid audio session state change hint.
     * @since 20
     */
    INVALID = -1,
    /**
     * Resume the playback.
     * @since 20
     */
    RESUME = 0,

    /**
     * Paused/Pause the playback.
     * @since 20
     */
    PAUSE = 1,

    /**
     * Stopped/Stop the playback due to focus priority.
     * @since 20
     */
    STOP = 2,

    /**
     * Stopped/Stop the playback due to no audio stream for a long time.
     * @since 20
     */
    TIME_OUT_STOP = 3,

    /**
     * Ducked the playback. (In ducking, the audio volume is reduced, but not silenced.)
     * @since 20
     */
    DUCK = 4,

    /**
     * Unducked the playback.
     * @since 20
     */
    UNDUCK = 5,
};

/**
 * Audio session state changed event.
 * @since 20
 */
struct AudioSessionStateChangedEvent {
    /**
     * Audio session state changed hints.
     * @since 20
     */
    AudioSessionStateChangeHint stateChangeHint;
};
} // namespace AudioStandard
} // namespace OHOS
#endif // AUDIO_SESSION_INFO_H
