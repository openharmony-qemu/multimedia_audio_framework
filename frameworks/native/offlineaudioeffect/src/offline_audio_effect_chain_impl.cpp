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
#ifndef LOG_TAG
#define LOG_TAG "OfflineAudioEffectManager"
#endif

#include "offline_audio_effect_chain_impl.h"

#include <securec.h>

#include "audio_errors.h"
#include "audio_service_log.h"
#include "audio_utils.h"

namespace OHOS {
namespace AudioStandard {
static constexpr size_t MAX_PARAM_SIZE = 1000;
OfflineAudioEffectChainImpl::OfflineAudioEffectChainImpl(const std::string &effectChainName)
{
    chainName_ = effectChainName;
    offlineStreamInClient_ = OfflineStreamInClient::Create();
    AUDIO_INFO_LOG("%{public}s offline effect chain created!", chainName_.c_str());
}

OfflineAudioEffectChainImpl::~OfflineAudioEffectChainImpl()
{
    Release();
}

void OfflineAudioEffectChainImpl::InitDump()
{
    static uint32_t chainId = 0;
    std::string dumpFileName = "OfflineEffectClient";
    std::string dumpFileInName = dumpFileName + "_" + std::to_string(chainId) + "_In.pcm";
    std::string dumpFileOutName = dumpFileName + "_" + std::to_string(chainId) + "_Out.pcm";
    DumpFileUtil::OpenDumpFile(DumpFileUtil::DUMP_CLIENT_PARA, dumpFileInName, &dumpFileIn_);
    DumpFileUtil::OpenDumpFile(DumpFileUtil::DUMP_CLIENT_PARA, dumpFileOutName, &dumpFileOut_);
    chainId++;
}

int32_t OfflineAudioEffectChainImpl::CreateEffectChain()
{
    std::lock_guard<std::mutex> lock(streamClientMutex_);
    CHECK_AND_RETURN_RET_LOG(offlineStreamInClient_, ERR_ILLEGAL_STATE, "offline stream is null!");
    int32_t ret = offlineStreamInClient_->CreateOfflineEffectChain(chainName_);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "InitIpcChainFailed!");
    InitDump();
    return SUCCESS;
}

int32_t OfflineAudioEffectChainImpl::Configure(const AudioStreamInfo &inInfo, const AudioStreamInfo &outInfo)
{
    CHECK_AND_RETURN_RET_LOG(CheckSupportedParams(inInfo) == SUCCESS, ERR_INVALID_PARAM, "inInfo do not support");
    CHECK_AND_RETURN_RET_LOG(CheckSupportedParams(outInfo) == SUCCESS, ERR_INVALID_PARAM, "outInfo do not support");
    std::lock_guard<std::mutex> lock(streamClientMutex_);
    CHECK_AND_RETURN_RET_LOG(offlineStreamInClient_, ERR_ILLEGAL_STATE, "offline stream is null!");
    return offlineStreamInClient_->ConfigureOfflineEffectChain(inInfo, outInfo);
}

int32_t OfflineAudioEffectChainImpl::SetParam(const std::vector<uint8_t> &param)
{
    CHECK_AND_RETURN_RET_LOG(param.size() <= MAX_PARAM_SIZE, ERR_INVALID_PARAM, "param size overflow");
    std::lock_guard<std::mutex> lock(streamClientMutex_);
    CHECK_AND_RETURN_RET_LOG(offlineStreamInClient_, ERR_ILLEGAL_STATE, "offline stream is null!");
    return offlineStreamInClient_->SetParamOfflineEffectChain(param);
}

int32_t OfflineAudioEffectChainImpl::GetEffectBufferSize(uint32_t &inBufferSize, uint32_t &outBufferSize)
{
    std::lock_guard<std::mutex> lock(streamClientMutex_);
    CHECK_AND_RETURN_RET_LOG(clientBufferIn_ && clientBufferOut_, ERR_ILLEGAL_STATE, "buffer not prepared");
    inBufferSize = clientBufferIn_->GetSize();
    outBufferSize = clientBufferOut_->GetSize();
    return SUCCESS;
}

int32_t OfflineAudioEffectChainImpl::Prepare()
{
    std::lock_guard<std::mutex> lock(streamClientMutex_);
    CHECK_AND_RETURN_RET_LOG(offlineStreamInClient_, ERR_ILLEGAL_STATE, "offline stream is null!");
    int32_t ret = offlineStreamInClient_->PrepareOfflineEffectChain(clientBufferIn_, clientBufferOut_);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_OPERATION_FAILED, "prepare failed, errCode is %{public}d", ret);
    CHECK_AND_RETURN_RET_LOG(clientBufferIn_ && clientBufferOut_, ERR_ILLEGAL_STATE, "buffer not prepared");
    inBufferBase_ = clientBufferIn_->GetBase();
    outBufferBase_ = clientBufferOut_->GetBase();
    return ret;
}

int32_t OfflineAudioEffectChainImpl::Process(uint8_t *inBuffer, int32_t inSize, uint8_t *outBuffer, int32_t outSize)
{
    std::lock_guard<std::mutex> lock(streamClientMutex_);
    CHECK_AND_RETURN_RET_LOG(offlineStreamInClient_, ERR_ILLEGAL_STATE, "offline stream is null!");
    CHECK_AND_RETURN_RET_LOG(inBufferBase_ && outBufferBase_ && clientBufferIn_ && clientBufferOut_,
        ERR_ILLEGAL_STATE, "buffer not prepared");
    int32_t inBufferSize = static_cast<int32_t>(clientBufferIn_->GetSize());
    int32_t outBufferSize = static_cast<int32_t>(clientBufferOut_->GetSize());
    CHECK_AND_RETURN_RET_LOG(inSize > 0 && inSize <= inBufferSize && outSize > 0 && outSize <= outBufferSize,
        ERR_INVALID_PARAM, "buffer size invalid");
    CHECK_AND_RETURN_RET_LOG(inBuffer && outBuffer, ERR_INVALID_PARAM, "buffer ptr invalid");

    DumpFileUtil::WriteDumpFile(dumpFileIn_, inBufferBase_, inSize);

    int32_t ret = memcpy_s(inBufferBase_, inBufferSize, inBuffer, inSize);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_OPERATION_FAILED, "memcpy inbuffer failed");
    ret = offlineStreamInClient_->ProcessOfflineEffectChain(inSize, outSize);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_OPERATION_FAILED, "process effect failed");
    ret = memcpy_s(outBuffer, outSize, outBufferBase_, outSize);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_OPERATION_FAILED, "memcpy outBuffer failed");

    DumpFileUtil::WriteDumpFile(dumpFileOut_, outBufferBase_, outSize);
    return SUCCESS;
}

void OfflineAudioEffectChainImpl::Release()
{
    std::lock_guard<std::mutex> lock(streamClientMutex_);
    if (offlineStreamInClient_ != nullptr) {
        offlineStreamInClient_->ReleaseOfflineEffectChain();
        offlineStreamInClient_ = nullptr;
    }
    inBufferBase_ = nullptr;
    outBufferBase_ = nullptr;
    clientBufferIn_ = nullptr;
    clientBufferOut_ = nullptr;
    DumpFileUtil::CloseDumpFile(&dumpFileIn_);
    DumpFileUtil::CloseDumpFile(&dumpFileOut_);
}
} // namespace AudioStandard
} // namespace OHOS
