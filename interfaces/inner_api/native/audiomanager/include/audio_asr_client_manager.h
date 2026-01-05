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

#ifndef AUDIO_ASR_CLIENT_MANAGER_H
#define AUDIO_ASR_CLIENT_MANAGER_H
#include "audio_asr.h"
#include "audio_info.h"

namespace OHOS {
namespace AudioStandard {
class AudioAsrClientManager {
public:
    static AudioAsrClientManager &GetInstance();

     /**
     * @brief set audio parameter.
     *
     * @parame key The key of the set audio parameter.
     * @param value The value of the set audio parameter.
     * @since 12
     */
    int32_t SetAsrAecMode(const AsrAecMode asrAecMode);
    
    /**
     * @brief set audio parameter.
     *
     * @parame key The key of the set audio parameter.
     * @param value The value of the set audio parameter.
     * @since 12
     */
    int32_t GetAsrAecMode(AsrAecMode &asrAecMode);

    /**
     * @brief set audio parameter.
     *
     * @parame key The key of the set audio parameter.
     * @param value The value of the set audio parameter.
     * @since 12
     */
    int32_t SetAsrNoiseSuppressionMode(const AsrNoiseSuppressionMode asrNoiseSuppressionMode);

    /**
     * @brief set audio parameter.
     *
     * @parame key The key of the set audio parameter.
     * @param value The value of the set audio parameter.
     * @since 12
     */
    int32_t GetAsrNoiseSuppressionMode(AsrNoiseSuppressionMode &asrNoiseSuppressionMode);

    /**
     * @brief set audio parameter.
     *
     * @parame key The key of the set audio parameter.
     * @param value The value of the set audio parameter.
     * @since 12
     */
    int32_t SetAsrWhisperDetectionMode(const AsrWhisperDetectionMode asrWhisperDetectionMode);

    /**
     * @brief set audio parameter.
     *
     * @parame key The key of the set audio parameter.
     * @param value The value of the set audio parameter.
     * @since 12
     */
    int32_t GetAsrWhisperDetectionMode(AsrWhisperDetectionMode &asrWhisperDetectionMode);

    /**
     * @brief set audio parameter.
     *
     * @parame key The key of the set audio parameter.
     * @param value The value of the set audio parameter.
     * @since 12
     */
    int32_t SetAsrVoiceControlMode(const AsrVoiceControlMode asrVoiceControlMode, bool on);

    /**
     * @brief set audio parameter.
     *
     * @parame key The key of the set audio parameter.
     * @param value The value of the set audio parameter.
     * @since 12
     */
    int32_t SetAsrVoiceMuteMode(const AsrVoiceMuteMode asrVoiceMuteMode, bool on);
private:
    AudioAsrClientManager() = default;
    ~AudioAsrClientManager() = default;
};
} // namespace AudioStandard
} // namespace OHOS
#endif // AUDIO_ASR_CLIENT_MANAGER_H
