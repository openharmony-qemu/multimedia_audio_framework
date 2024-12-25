
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

#ifndef OFFLINE_AUDIO_EFFECT_CHAIN_IMPL_H
#define OFFLINE_AUDIO_EFFECT_CHAIN_IMPL_H

#include <mutex>
#include <shared_mutex>

#include "offline_audio_effect_manager.h"
#include "oh_audio_buffer.h"

namespace OHOS {
namespace AudioStandard {
class OfflineAudioEffectChainImpl : public OfflineAudioEffectChain {
public:
    int32_t Configure(const AudioStreamInfo &inInfo, const AudioStreamInfo &outInfo) override;

    int32_t Prepare() override;

    int32_t GetEffectBufferSize(uint32_t &inBufferSize, uint32_t &outBufferSize) override;

    int32_t Process(uint8_t *inBuffer, int32_t inSize, uint8_t *outBuffer, int32_t outSize) override;

    void Release() override;

    OfflineAudioEffectChainImpl(const std::string &chainName);

    ~OfflineAudioEffectChainImpl();

    int32_t CreateEffectChain();
private:
    void InitDump();

    std::string chainName_;
    std::shared_ptr<OfflineStreamInClient> offlineStreamInClient_ = nullptr;
    std::shared_ptr<AudioSharedMemory> clientBufferIn_ = nullptr;
    std::shared_ptr<AudioSharedMemory> clientBufferOut_ = nullptr;
    uint8_t *inBufferBase_ = nullptr;
    uint8_t *outBufferBase_ = nullptr;
    std::mutex streamClientMutex_;
    FILE *dumpFileIn_ = nullptr;
    FILE *dumpFileOut_ = nullptr;
};
} // namespace AudioStandard
} // namespace OHOS
#endif // OFFLINE_AUDIO_EFFECT_CHAIN_IMPL_H