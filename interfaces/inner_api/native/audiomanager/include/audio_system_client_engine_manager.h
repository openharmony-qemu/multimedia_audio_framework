/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#ifndef AUDIO_SYSTEM_CLIENT_ENGINE_MANAGER_H
#define AUDIO_SYSTEM_CLIENT_ENGINE_MANAGER_H

#include "audio_info.h"

namespace OHOS {
namespace AudioStandard {
class AudioSystemClientEngineManager {
public:
    static AudioSystemClientEngineManager &GetInstance();

    /**
     * @brief set audio parameter.
     *
     * @parame key The key of the set audio parameter.
     * @param value The value of the set audio parameter.
     * @since 12
     */
    int32_t IsWhispering();

    /**
     * @brief Get audio parameter.
     *
     * @param mainKey Main key of audio parameters to be obtained.
     * @param subKeys subKeys of audio parameters to be obtained.
     * @param result value of sub key parameters.
     * @return Returns {@link SUCCESS} if the setting is successful; returns an error code defined
     * @since 11
     */
    int32_t GetExtraParameters(const std::string &mainKey,
        const std::vector<std::string> &subKeys, std::vector<std::pair<std::string, std::string>> &result);

    /**
     * @brief Set audio parameters.
     *
     * @param key The main key of the set audio parameter.
     * @param kvpairs The pairs with sub keys and values of the set audio parameter.
     * @return Returns {@link SUCCESS} if the setting is successful; returns an error code defined
     * @since 11
     */
    int32_t SetExtraParameters(const std::string &key,
        const std::vector<std::pair<std::string, std::string>> &kvpairs);

    /**
     * @brief Get transaction Id.
     *
     * @param deviceType device type.
     * @param deviceRole device role.
     * @return Returns transaction Id.
     * @since 9
     */
    uint64_t GetTransactionId(DeviceType deviceType, DeviceRole deviceRole);

    /**
     * @brief Set mono audio state
     *
     * @param monoState mono state
     * @since 8
     */
    void SetAudioMonoState(bool monoState);

    /**
     * @brief Set audio balance value
     *
     * @param balanceValue balance value
     * @since 8
     */
    void SetAudioBalanceValue(float balanceValue);

      /**
     * @brief Set render whitelist.
     *
     * @param list render whitelist.
     * @return Returns {@link SUCCESS} if the operation is successfully.
     * @return Returns {@link ERR_ILLEGAL_STATE} if the server is not available.
     * @return Returns {@link ERR_INVALID_PARAM} if the sessionId is not exist.
     */
    int32_t SetRenderWhitelist(std::vector<std::string> list);

#ifdef HAS_FEATURE_INNERCAPTURER
    /**
     * @brief check capture limit
     *
     * @param AudioPlaybackCaptureConfig inner capture filter info
     * @param innerCapId unique identifier of inner capture
     * @return Returns {@link SUCCESS} if the operation is successfully.
     * @test
     */
    int32_t CheckCaptureLimit(const AudioPlaybackCaptureConfig &config, int32_t &innerCapId);

    /**
     * @brief release capture limit
     *
     * @param innerCapId unique identifier of inner capture
     * @return Returns {@link SUCCESS} if the operation is successfully.
     * @test
     */
    int32_t ReleaseCaptureLimit(int32_t innerCapId);
#endif

    int32_t GenerateSessionId(uint32_t &sessionId);

    /**
     * @brief get the effect algorithmic latency value for a specified audio stream.
     *
     * @param sessionId the session ID value for the stream
     * @return Returns the effect algorithmic latency in ms.
     * @since 12
     */
    uint32_t GetEffectLatency(const std::string &sessionId);

    /**
     * @brief Set foreground list.
     *
     * @param list The foreground list.
     * @return Returns {@link SUCCESS} if the operation is successfully.
     * @return Returns {@link ERR_ILLEGAL_STATE} if the server is not available.
     * @return Returns {@link ERR_INVALID_PARAM} if the sessionId is not exist.
     */
    int32_t SetForegroundList(std::vector<std::string> list);

    /**
     * @brief Get standby state.
     *
     * @param sessionId Specifies which stream to be check.
     * @param isStandby true means the stream is in standby status.
     * @param enterStandbyTime Specifies when the stream enter standby status, in MONOTONIC time.
     * @return Returns {@link SUCCESS} if the operation is successfully.
     * @return Returns {@link ERR_ILLEGAL_STATE} if the server is not available.
     * @return Returns {@link ERR_INVALID_PARAM} if the sessionId is not exist.
     */
    int32_t GetStandbyStatus(uint32_t sessionId, bool &isStandby, int64_t &enterStandbyTime);
    
    /**
     * @brief Get audio parameter.
     *
     * @param key Key of audio parameters to be obtained.
     * @return Returns the value of the obtained audio parameter
     * @since 9
     */
    const std::string GetAudioParameter(const std::string key);

    /**
     * @brief set audio parameter.
     *
     * @param key The key of the set audio parameter.
     * @param value The value of the set audio parameter.
     * @since 9
     */
    void SetAudioParameter(const std::string &key, const std::string &value);
private:
    AudioSystemClientEngineManager() = default;
    ~AudioSystemClientEngineManager() = default;
};
} // namespace AudioStandard
} // namespace OHOS
#endif // AUDIO_SYSTEM_CLIENT_ENGINE_MANAGER_H
