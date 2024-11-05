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
#ifndef AUDIO_ASR_H
#define AUDIO_ASR_H
/**
 * ASR noise suppression mode.
 * @enum { number }
 * @syscap SystemCapability.Multimedia.Audio.Capturer
 * @systemapi
 * @since 12
 */
enum class AsrNoiseSuppressionMode {
    /**
     * Bypass noise suppression.
     * @syscap SystemCapability.Multimedia.Audio.Capturer
     * @systemapi
     * @since 12
     */
    BYPASS = 0,
    /**
     * Standard noise suppression.
     * @syscap SystemCapability.Multimedia.Audio.Capturer
     * @systemapi
     * @since 12
     */
    STANDARD = 1,
    /**
     * Near field noise suppression.
     * @syscap SystemCapability.Multimedia.Audio.Capturer
     * @systemapi
     * @since 12
     */
    NEAR_FIELD = 2,
    /**
     * Far field noise suppression.
     * @syscap SystemCapability.Multimedia.Audio.Capturer
     * @systemapi
     * @since 12
     */
    FAR_FIELD = 3,
};

/**
 * ASR AEC mode.
 * @enum { number }
 * @syscap SystemCapability.Multimedia.Audio.Capturer
 * @systemapi
 * @since 12
 */
enum class AsrAecMode {
    /**
     * Bypass AEC.
     * @syscap SystemCapability.Multimedia.Audio.Capturer
     * @systemapi
     * @since 12
     */
    BYPASS = 0,
    /**
     * Using standard AEC.
     * @syscap SystemCapability.Multimedia.Audio.Capturer
     * @systemapi
     * @since 12
     */
    STANDARD = 1,
};

/**
 * ASR Whisper mode.
 * @enum { number }
 * @syscap SystemCapability.Multimedia.Audio.Capturer
 * @systemapi
 * @since 12
 */
enum class AsrWhisperDetectionMode {
    /**
     * Using Bypass WhisperDetection.
     * @syscap SystemCapability.Multimedia.Audio.Capturer
     * @systemapi
     * @since 12
     */
    BYPASS = 0,
    /**
     * Using standard WhisperDetection.
     * @syscap SystemCapability.Multimedia.Audio.Capturer
     * @systemapi
     * @since 12
     */
    STANDARD = 1,
};

/**
 * Asr Voice Control Mode.
 * @enum { number }
 * @syscap SystemCapability.Multimedia.Audio.Capturer
 * @systemapi
 * @since 12
 */
enum class AsrVoiceControlMode {
    /**
     * AUDIO 2 VOICETX.
     * @syscap SystemCapability.Multimedia.Audio.Capturer
     * @systemapi
     * @since 12
     */
    AUDIO_2_VOICETX = 0,
    /**
     * AUDIO MIX 2 VOICETX .
     * @syscap SystemCapability.Multimedia.Audio.Capturer
     * @systemapi
     * @since 12
     */
    AUDIO_MIX_2_VOICETX = 1,
    /**
     * AUDIO 2 VOICE TX EX.
     * @syscap SystemCapability.Multimedia.Audio.Capturer
     * @systemapi
     * @since 12
     */
    AUDIO_2_VOICE_TX_EX = 2,
    /**
     * AUDIO MIX 2 VOICE X EX.
     * @syscap SystemCapability.Multimedia.Audio.Capturer
     * @systemapi
     * @since 12
     */
    AUDIO_MIX_2_VOICE_TX_EX = 3,
    /**
     * AUDIO SUPPRESSION OPPOSITE.
     * @syscap SystemCapability.Multimedia.Audio.Capturer
     * @systemapi
     * @since 13
     */
    AUDIO_SUPPRESSION_OPPOSITE = 4,
    /**
     * AUDIO SUPPRESSION LOCAL.
     * @syscap SystemCapability.Multimedia.Audio.Capturer
     * @systemapi
     * @since 13
     */
    AUDIO_SUPPRESSION_LOCAL = 5,
    /**
     * VOICE TXRX DECREASE.
     * @syscap SystemCapability.Multimedia.Audio.Capturer
     * @systemapi
     * @since 13
     */
    VOICE_TXRX_DECREASE = 6,
};

/**
 * Asr Voice Mute mode.
 * @enum { number }
 * @syscap SystemCapability.Multimedia.Audio.Capturer
 * @systemapi
 * @since 12
 */
enum class AsrVoiceMuteMode {
    /**
     * OUTPUT MUTE.
     * @syscap SystemCapability.Multimedia.Audio.Capturer
     * @systemapi
     * @since 12
     */
    OUTPUT_MUTE = 0,
    /**
     * INPUT MUTE.
     * @syscap SystemCapability.Multimedia.Audio.Capturer
     * @systemapi
     * @since 12
     */
    INPUT_MUTE = 1,
    /**
     * TTS MUTE.
     * @syscap SystemCapability.Multimedia.Audio.Capturer
     * @systemapi
     * @since 12
     */
    TTS_MUTE = 2,
    /**
     * CALL MUTE.
     * @syscap SystemCapability.Multimedia.Audio.Capturer
     * @systemapi
     * @since 12
     */
    CALL_MUTE = 3,
    /**
     * OUTPUT MUTE EX.
     * @syscap SystemCapability.Multimedia.Audio.Capturer
     * @systemapi
     * @since 12
     */
    OUTPUT_MUTE_EX = 4,
};
#endif