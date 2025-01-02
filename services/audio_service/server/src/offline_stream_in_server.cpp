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
#define LOG_TAG "OfflineStreamInServer"
#endif

#include "offline_stream_in_server.h"
#include "audio_service_log.h"
#include "audio_errors.h"

namespace OHOS {
namespace AudioStandard {
namespace {
static const std::string OFFLINE_SERVER_BUFFER_IN = "offline_server_buffer_in";
static const std::string OFFLINE_SERVER_BUFFER_OUT = "offline_server_buffer_out";
static constexpr int32_t MAXIMUM_BUFFER_SIZE = 1000000; // 1,000,000
}
// static method
sptr<OfflineStreamInServer> OfflineStreamInServer::GetOfflineStream(int32_t &errCode)
{
    sptr<OfflineStreamInServer> streamInServer = sptr<OfflineStreamInServer>::MakeSptr();
    CHECK_AND_RETURN_RET_LOG(streamInServer, nullptr, "Create OfflineStream failed, errCode: %{public}d", errCode);
    return streamInServer;
}

// static method
#ifdef FEATURE_OFFLINE_EFFECT
int32_t OfflineStreamInServer::GetOfflineAudioEffectChains(std::vector<std::string> &effectChains)
{
    return OfflineAudioEffectServerChain::GetOfflineAudioEffectChains(effectChains);
}

int32_t OfflineStreamInServer::CreateOfflineEffectChain(const std::string &chainName)
{
    effectChain_ = std::make_shared<OfflineAudioEffectServerChain>(chainName);
    return effectChain_->Create();
}

int32_t OfflineStreamInServer::ConfigureOfflineEffectChain(const AudioStreamInfo &inInfo,
    const AudioStreamInfo &outInfo)
{
    CHECK_AND_RETURN_RET_LOG(effectChain_, ERR_ILLEGAL_STATE, "effectChain not init");
    return effectChain_->SetParam(inInfo, outInfo);
}

int32_t OfflineStreamInServer::PrepareOfflineEffectChain(std::shared_ptr<AudioSharedMemory> &inBuffer,
    std::shared_ptr<AudioSharedMemory> &outBuffer)
{
    if (serverBufferIn_ == nullptr || serverBufferOut_ == nullptr) {
        uint32_t inSize = 0;
        uint32_t outSize = 0;
        effectChain_->GetEffectBufferSize(inSize, outSize);
        CHECK_AND_RETURN_RET_LOG(AllocSharedMemory(inSize, outSize) == SUCCESS, ERR_OPERATION_FAILED,
            "AllocSharedMemory failed");
    }
    inBuffer = serverBufferIn_;
    outBuffer = serverBufferOut_;
    return effectChain_->Prepare(serverBufferIn_, serverBufferOut_);
}

int32_t OfflineStreamInServer::ProcessOfflineEffectChain(uint32_t inSize, uint32_t outSize)
{
    CHECK_AND_RETURN_RET_LOG(effectChain_, ERR_ILLEGAL_STATE, "effectChain not init");
    return effectChain_->Process(inSize, outSize);
}

void OfflineStreamInServer::ReleaseOfflineEffectChain()
{
    CHECK_AND_RETURN_LOG(effectChain_, "effectChain not init");
    effectChain_->Release();
}
#endif

int32_t OfflineStreamInServer::AllocSharedMemory(uint32_t inSize, uint32_t outSize)
{
    CHECK_AND_RETURN_RET_LOG(inSize < MAXIMUM_BUFFER_SIZE && outSize < MAXIMUM_BUFFER_SIZE,
        ERR_INVALID_PARAM, "alloc %{public}u inBuf or %{public}u outBuf out of range", inSize, outSize);
    serverBufferIn_ = AudioSharedMemory::CreateFormLocal(inSize, OFFLINE_SERVER_BUFFER_IN);
    CHECK_AND_RETURN_RET_LOG(serverBufferIn_ != nullptr, ERR_OPERATION_FAILED, "serverBufferIn_ mmap failed.");
    serverBufferOut_ = AudioSharedMemory::CreateFormLocal(outSize, OFFLINE_SERVER_BUFFER_OUT);
    CHECK_AND_RETURN_RET_LOG(serverBufferOut_ != nullptr, ERR_OPERATION_FAILED, "serverBufferOut_ mmap failed.");
    return SUCCESS;
}
} // namespace AudioStandard
} // namespace OHOS
