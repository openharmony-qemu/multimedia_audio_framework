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
#ifndef LOG_TAG
#define LOG_TAG "OfflineAudioEffectManager"
#endif

#include "offline_audio_effect_manager.h"

#include "audio_errors.h"
#include "audio_service_log.h"

#include <securec.h>

namespace OHOS {
namespace AudioStandard {
std::shared_ptr<OfflineAudioEffectManager> OfflineAudioEffectManager::GetInstance()
{
    static std::shared_ptr<OfflineAudioEffectManager> manager = std::make_shared<OfflineAudioEffectManager>();
    return manager;
}

std::vector<std::string> OfflineAudioEffectManager::GetOfflineAudioEffectChains()
{
    std::vector<std::string> effectChains{};
    int32_t ret = OfflineStreamInClient::GetOfflineAudioEffectChains(effectChains);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, effectChains, "Get chains failed, errcode is %{public}d", ret);
    return effectChains;
}

std::unique_ptr<OfflineAudioEffectChain> OfflineAudioEffectManager::CreateOfflineAudioEffectChain(
    const std::string &chainName)
{
    std::unique_ptr<OfflineAudioEffectChain> chain = std::make_unique<OfflineAudioEffectChain>(chainName);
    int32_t ret = chain->InitIpcChain();
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, nullptr, "create OfflineEffectChain failed, errcode is %{public}d", ret);
    return chain;
}

OfflineAudioEffectChain::OfflineAudioEffectChain(const std::string &effectChainName)
{
    chainName_ = effectChainName;
    offlineStreamInClient_ = OfflineStreamInClient::Create();
    AUDIO_INFO_LOG("%{public}s offline effect chain created!", chainName_.c_str());
}

OfflineAudioEffectChain::~OfflineAudioEffectChain()
{
    Release();
}

int32_t OfflineAudioEffectChain::InitIpcChain()
{
    CHECK_AND_RETURN_RET_LOG(offlineStreamInClient_, ERR_ILLEGAL_STATE, "offline stream is null!");
    return offlineStreamInClient_->CreateOfflineEffectChain(chainName_);
}

int32_t OfflineAudioEffectChain::Configure(const AudioStreamInfo &inInfo, const AudioStreamInfo &outInfo)
{
    CHECK_AND_RETURN_RET_LOG(offlineStreamInClient_, ERR_ILLEGAL_STATE, "offline stream is null!");
    return offlineStreamInClient_->ConfigureOfflineEffectChain(inInfo, outInfo);
}

int32_t OfflineAudioEffectChain::GetEffectBufferSize(uint32_t &inBufferSize, uint32_t &outBufferSize)
{
    std::shared_lock<std::shared_mutex> lock(bufferMutex_);
    CHECK_AND_RETURN_RET_LOG(clientBufferIn_ && clientBufferOut_, ERR_ILLEGAL_STATE, "buffer not prepared");
    inBufferSize = clientBufferIn_->GetSize();
    outBufferSize = clientBufferOut_->GetSize();
    return SUCCESS;
}

int32_t OfflineAudioEffectChain::Prepare()
{
    CHECK_AND_RETURN_RET_LOG(offlineStreamInClient_, ERR_ILLEGAL_STATE, "offline stream is null!");
    std::lock_guard<std::shared_mutex> lock(bufferMutex_);
    int32_t ret = offlineStreamInClient_->PrepareOfflineEffectChain(clientBufferIn_, clientBufferOut_);
    inBufferBase_ = clientBufferIn_->GetBase();
    outBufferBase_ = clientBufferOut_->GetBase();
    return ret;
}

int32_t OfflineAudioEffectChain::Process(uint8_t *inBuffer, int32_t inSize, uint8_t *outBuffer, int32_t outSize)
{
    CHECK_AND_RETURN_RET_LOG(offlineStreamInClient_, ERR_ILLEGAL_STATE, "offline stream is null!");
    std::lock_guard<std::shared_mutex> lock(bufferMutex_);
    CHECK_AND_RETURN_RET_LOG(inBufferBase_ && outBufferBase_ && clientBufferIn_ && clientBufferOut_,
        ERR_ILLEGAL_STATE, "buffer not prepared");
    int32_t inBufferSize = clientBufferIn_->GetSize();
    int32_t outBufferSize = clientBufferOut_->GetSize();
    CHECK_AND_RETURN_RET_LOG(inSize > 0 && inSize <= inBufferSize && outSize > 0 && outSize <= outBufferSize,
        ERR_INVALID_PARAM, "buffer size invalid");
    CHECK_AND_RETURN_RET_LOG(inBuffer && outBuffer, ERR_INVALID_PARAM, "buffer ptr invalid");
    int32_t ret = memcpy_s(inBufferBase_, inBufferSize, inBuffer, inSize);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_OPERATION_FAILED, "memcpy inbuffer failed");
    ret = offlineStreamInClient_->ProcessOfflineEffectChain(inSize, outSize);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_OPERATION_FAILED, "process effect failed");
    ret = memcpy_s(outBuffer, outSize, outBufferBase_, outSize);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_OPERATION_FAILED, "memcpy outBuffer failed");
    return SUCCESS;
}

void OfflineAudioEffectChain::Release()
{
    std::lock_guard<std::shared_mutex> lock(bufferMutex_);
    clientBufferIn_ = nullptr;
    clientBufferOut_ = nullptr;
    inBufferBase_ = nullptr;
    outBufferBase_ = nullptr;
    if (offlineStreamInClient_ != nullptr) {
        offlineStreamInClient_->ReleaseOfflineEffectChain();
        offlineStreamInClient_ = nullptr;
    }
}
} // namespace AudioStandard
} // namespace OHOS
