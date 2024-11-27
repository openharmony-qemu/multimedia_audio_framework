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

#ifndef ST_OFFLINE_AUDIO_EFFECT_MANAGER_H
#define ST_OFFLINE_AUDIO_EFFECT_MANAGER_H

#include <mutex>
#include <shared_mutex>
#include <vector>

#include "audio_info.h"
#include "offline_stream_in_client.h"
#include "oh_audio_buffer.h"

namespace OHOS {
namespace AudioStandard {
class OfflineAudioEffectChain {
public:
    OfflineAudioEffectChain(const OfflineAudioEffectChain&) = delete;
    OfflineAudioEffectChain& operator=(const OfflineAudioEffectChain&) = delete;
    /**
     * @brief Configure the audio stream information
     *
     * @param inInfo Input audio stream information
     * @param outInfo Output audio stream information
     * @return The result of the config, 0 for success, other for error code
     * @since 15
     */
    int32_t Configure(const AudioStreamInfo &inInfo, const AudioStreamInfo &outInfo);

    /**
     * @brief Prepare the offline audio effect chain
     *
     * @return The result of the preparation, 0 for success, other for error code
     * @since 15
     */
    int32_t Prepare();

    /**
     * @brief Get the size of the audio effect buffer
     *
     * @param inBufferSize Size of the input buffer
     * @param outBufferSize Size of the output buffer
     * @return The result of the retrieval, 0 for success, other for error code
     * @since 15
     */
    int32_t GetEffectBufferSize(uint32_t &inBufferSize, uint32_t &outBufferSize);

    /**
     * @brief Process the audio data
     *
     * @param inBuffer Input audio data buffer
     * @param inSize Size of the input audio data
     * @param outBuffer Output audio data buffer
     * @param outSize Size of the output audio data
     * @return The result of processing, 0 for success, other for error code
     * @since 15
     */
    int32_t Process(uint8_t *inBuffer, int32_t inSize, uint8_t *outBuffer, int32_t outSize);

    /**
     * @brief Release the resources of the audio effect chain
     *
     * @since 15
     */
    void Release();
private:
    friend class OfflineAudioEffectManager;

    OfflineAudioEffectChain(const std::string &effectChainName);
    ~OfflineAudioEffectChain();
    int32_t InitIpcChain();
    std::string chainName_;
    std::shared_ptr<OfflineStreamInClient> offlineStreamInClient_;
    std::shared_ptr<AudioSharedMemory> clientBufferIn_;
    std::shared_ptr<AudioSharedMemory> clientBufferOut_;
    uint8_t *inBufferBase_;
    uint8_t *outBufferBase_;
    std::shared_mutex bufferMutex_;
};

class OfflineAudioEffectManager {
public:
    /**
     * @brief 
     *
     * @return Returns all offline audio effect chains avalible.
     * @since 12
     */
    std::vector<std::string> GetOfflineAudioEffectChains();

    /**
     * @brief 
     *
     * @return Returns offload audio effect chain with chainName provided, nullptr if failed
     * @since 12
     */
    std::unique_ptr<OfflineAudioEffectChain> CreateOfflineAudioEffectChain(const std::string &chainName);
};
} // namespace AudioStandard
} // namespace OHOS
#endif // ST_AUDIO_SPATIALIZATION_MANAGER_H
