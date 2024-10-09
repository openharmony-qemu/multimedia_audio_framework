/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#ifndef AUDIO_TONEPLAYER_H
#define AUDIO_TONEPLAYER_H

#include <memory>

#include "audio_info.h"
#include "timestamp.h"

namespace OHOS {
namespace AudioStandard {
/**
 * @brief Defines information about ToneType Enum
 */
    // This enum must be kept consistant with constants in TonePlayer
enum ToneType {
    // DTMF tones ITU-T Recommendation Q.23
    TONE_TYPE_DIAL_0 = 0,
    TONE_TYPE_DIAL_1,
    TONE_TYPE_DIAL_2,
    TONE_TYPE_DIAL_3,
    TONE_TYPE_DIAL_4,
    TONE_TYPE_DIAL_5,
    TONE_TYPE_DIAL_6,
    TONE_TYPE_DIAL_7,
    TONE_TYPE_DIAL_8,
    TONE_TYPE_DIAL_9,
    TONE_TYPE_DIAL_S,
    TONE_TYPE_DIAL_P,
    TONE_TYPE_DIAL_A,
    TONE_TYPE_DIAL_B,
    TONE_TYPE_DIAL_C,
    TONE_TYPE_DIAL_D,

    // Call supervisory tones: 3GPP TS 22.001 (CEPT)
    TONE_TYPE_COMMON_SUPERVISORY_DIAL = 100,
    FIRST_SUPERVISORY_TONE = TONE_TYPE_COMMON_SUPERVISORY_DIAL,
    TONE_TYPE_COMMON_SUPERVISORY_BUSY = 101,
    TONE_TYPE_COMMON_SUPERVISORY_CONGESTION = 102,
    TONE_TYPE_COMMON_SUPERVISORY_RADIO_ACK = 103,
    TONE_TYPE_COMMON_SUPERVISORY_RADIO_NOT_AVAILABLE = 104,
    TONE_TYPE_COMMON_SUPERVISORY_CALL_WAITING = 106,
    TONE_TYPE_COMMON_SUPERVISORY_RINGTONE = 107,
    LAST_SUPERVISORY_TONE = TONE_TYPE_COMMON_SUPERVISORY_RINGTONE,

    // Proprietary tones: 3GPP TS 31.111
    TONE_TYPE_COMMON_PROPRIETARY_BEEP = 200,
    TONE_TYPE_COMMON_PROPRIETARY_ACK = 201,
    TONE_TYPE_COMMON_PROPRIETARY_PROMPT = 203,
    TONE_TYPE_COMMON_PROPRIETARY_DOUBLE_BEEP = 204,

    NUM_TONES,
    NUM_SUP_TONES = LAST_SUPERVISORY_TONE - FIRST_SUPERVISORY_TONE + 1
    };
class TonePlayer {
public:

    /**
     * @brief create tonePlayer instance.
     *
     * @param cachePath Application cache path
     * @param rendererInfo Indicates information about audio renderer. For details, see
     * {@link AudioRendererInfo}.
     * @return Returns unique pointer to the TonePlayer object
     * @since 9
    */
    static std::shared_ptr<TonePlayer> Create(const std::string cachePath, const AudioRendererInfo &rendererInfo);

    /**
     * @brief create tonePlayer instance.
     *
     * @param rendererInfo Indicates information about audio renderer. For details, see
     * {@link AudioRendererInfo}.
     * @return Returns unique pointer to the TonePlayer object
     * @since 9
    */
    static std::shared_ptr<TonePlayer> Create(const AudioRendererInfo &rendererInfo);

    /**
     * @brief Load audio tonePlayer.
     *
     * @return Returns <b>true</b> if the tonePlayer is successfully started; returns <b>false</b> otherwise.
     * @since 9
     */
    virtual bool LoadTone(ToneType toneType) = 0;

    /**
     * @brief Starts audio tonePlayer.
     *
     * @return Returns <b>true</b> if the tonePlayer is successfully started; returns <b>false</b> otherwise.
     * @since 9
     */
    virtual bool StartTone() = 0;

    /**
     * @brief Stop audio tonePlayer.
     *
     * @return Returns <b>true</b> if the tonePlayer is successfully started; returns <b>false</b> otherwise.
     * @since 9
     */
    virtual bool StopTone() = 0;

    /**
     * @brief Release audio tonePlayer.
     *
     * @return Returns <b>true</b> if the tonePlayer is successfully started; returns <b>false</b> otherwise.
     * @since 9
     */
    virtual bool Release() = 0;
    virtual ~TonePlayer() = default;
};
}  // namespace AudioStandard
}  // namespace OHOS
#endif  // AUDIO_TONEPLAYER_H
