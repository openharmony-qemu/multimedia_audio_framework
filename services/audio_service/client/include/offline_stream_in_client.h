/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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
#ifndef OFFLINE_STREAM_IN_CLIENT_H
#define OFFLINE_STREAM_IN_CLIENT_H

#include <cstdint>
#include <string>
#include <vector>

#include "audio_shared_memory.h"
#include "ipc_offline_stream.h"

namespace OHOS {
namespace AudioStandard {
class OfflineStreamInClient {
public:
    static int32_t GetOfflineAudioEffectChains(std::vector<std::string> &effectChains);
    static std::shared_ptr<OfflineStreamInClient> Create();

    OfflineStreamInClient(const sptr<IpcOfflineStream> &ipcProxy);
    ~OfflineStreamInClient() = default;

    /**
     * @brief Create the offline audio effect chain
     *
     * @param chainName Audio effect chain name
     * @return The result of construction, 0 for success, other for error code
     * @since 15
     */
    int32_t CreateOfflineEffectChain(const std::string &chainName);

    /**
     * @brief Configure the offline audio effect chain
     *
     * @param inInfo Input audio stream information
     * @param outInfo Output audio stream information
     * @return The result of configuration, 0 for success, other for error code
     * @since 15
     */
    int32_t ConfigureOfflineEffectChain(const AudioStreamInfo &inInfo, const AudioStreamInfo &outInfo);

    /**
     * @brief Prepare the offline audio effect chain
     *
     * @param bufIn Input audio sharedmemory buffer
     * @param bufOut Output audio sharedmemory buffer
     * @return The result of preparation, 0 for success, other for error code
     * @since 15
     */
    int32_t PrepareOfflineEffectChain(std::shared_ptr<AudioSharedMemory> &bufIn,
        std::shared_ptr<AudioSharedMemory> &bufOut);

    /**
     * @brief Process the offline audio effect chain
     *
     * @param inSize Size of input audio data in sharedmemory
     * @param outSize Size of output audio data in sharedmemory
     * @return The result of processing, 0 for success, other for error code
     * @since 15
     */
    int32_t ProcessOfflineEffectChain(uint32_t inSize, uint32_t outSize);

    /**
     * @brief Release the offline audio effect chain
     *
     * @since 15
     */
    void ReleaseOfflineEffectChain();
private:
    sptr<IpcOfflineStream> streamProxy_ = nullptr;
};
} // namespace AudioStandard
} // namespace OHOS
#endif // OFFLINE_STREAM_IN_CLIENT_H
