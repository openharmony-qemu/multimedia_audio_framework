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

#ifndef OFFLINE_STREAM_IN_SERVER_H
#define OFFLINE_STREAM_IN_SERVER_H

#include <memory>

#include "ipc_offline_stream_stub.h"
#ifdef FEATURE_OFFLINE_EFFECT
#include "offline_audio_effect_server_chain.h"
#endif

namespace OHOS {
namespace AudioStandard {
class OfflineStreamInServer : public IpcOfflineStreamStub {
public:
    static sptr<OfflineStreamInServer> GetOfflineStream(int32_t &errCode);
#ifdef FEATURE_OFFLINE_EFFECT
    static int32_t GetOfflineAudioEffectChains(std::vector<std::string> &effectChains);

    int32_t CreateOfflineEffectChain(const std::string &chainName) override;

    int32_t ConfigureOfflineEffectChain(const AudioStreamInfo &inInfo, const AudioStreamInfo &outInfo) override;

    int32_t SetParamOfflineEffectChain(const std::vector<uint8_t> &param) override;

    int32_t PrepareOfflineEffectChain(std::shared_ptr<AudioSharedMemory> &inBuffer,
        std::shared_ptr<AudioSharedMemory> &outBuffer) override;

    int32_t ProcessOfflineEffectChain(uint32_t inSize, uint32_t outSize) override;

    int32_t ReleaseOfflineEffectChain() override;
#endif

    OfflineStreamInServer() = default;
    ~OfflineStreamInServer() = default;
private:
    int32_t AllocSharedMemory(uint32_t inSize, uint32_t outSize);
    std::shared_ptr<AudioSharedMemory> serverBufferIn_;
    std::shared_ptr<AudioSharedMemory> serverBufferOut_;
#ifdef FEATURE_OFFLINE_EFFECT
    std::shared_ptr<OfflineAudioEffectServerChain> effectChain_;
    std::mutex offlineChainMutex_;
#endif
};
} // namespace AudioStandard
} // namespace OHOS
#endif // OFFLINE_STREAM_IN_SERVER_H
