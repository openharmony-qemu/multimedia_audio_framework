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

#include "offline_stream_in_client.h"
#include "audio_info.h"
#include "oh_audio_buffer.h"

#include <mutex>
#include <shared_mutex>
#include <vector>

namespace OHOS {
namespace AudioStandard {
class OfflineAudioEffectChain {
public:
    OfflineAudioEffectChain(const OfflineAudioEffectChain&) = delete;
    OfflineAudioEffectChain& operator=(const OfflineAudioEffectChain&) = delete;
    /**
     * @brief 
     *
     * @return 
     * @since 12
     */
    int32_t Configure(const AudioStreamInfo &inInfo, const AudioStreamInfo &outInfo);

    /**
     * @brief 
     *
     * @return 
     * @since 12
     */
    int32_t Prepare();

    /**
     * @brief 
     *
     * @return 
     * @since 12
     */
    int32_t GetEffectBufferSize(uint32_t &inBufferSize, uint32_t &outBufferSize);

    /**
     * @brief 
     *
     * @return 
     * @since 12
     */
    int32_t Process(uint8_t *inBuffer, int32_t inSize, uint8_t *outBuffer, int32_t outSize);

    /**
     * @brief 
     *
     * @return 
     * @since 12
     */
    void Release();
private:
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
    friend class OfflineAudioEffectManager;
};

class OfflineAudioEffectManager {
public:
    static std::shared_ptr<OfflineAudioEffectManager> GetInstance();

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
