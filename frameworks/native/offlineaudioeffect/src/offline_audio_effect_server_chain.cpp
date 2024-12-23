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

#undef LOG_TAG
#define LOG_TAG "OfflineAudioEffectServerChain"

#include "offline_audio_effect_server_chain.h"

#include <algorithm>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cinttypes>
#include <unistd.h>
#include "securec.h"

#include "audio_common_log.h"
#include "audio_errors.h"

using namespace OHOS::AudioStandard;

namespace OHOS {
namespace AudioStandard {
namespace {
    constexpr size_t DEFAULT_BUFFER_SIZE = 7680 * 2;
    constexpr uint32_t SEND_COMMAND_LEN = 10;
    constexpr uint32_t GET_BUFFER_LEN = 10;
    static const std::string LIBNAME = "offline_record_algo";
    static const std::string UUID = "a953e8d6-fb7c-4684-9dc2-78be1a995bb2";
}

struct IEffectModel *OfflineAudioEffectServerChain::model_;
struct EffectInfo OfflineAudioEffectServerChain::info_;

OfflineAudioEffectServerChain::OfflineAudioEffectServerChain(const std::string &chainName) : chainName_(chainName) {}

OfflineAudioEffectServerChain::~OfflineAudioEffectServerChain()
{
    controller_ = nullptr;
    controllerId_ = nullptr;
    serverBufferIn_ = nullptr;
    serverBufferOut_ = nullptr;
}
 
int32_t OfflineAudioEffectServerChain::InitEffectModel()
{
    model_ = IEffectModelGet(false);
    info_.libName = strdup(LIBNAME.c_str());
    info_.effectId = strdup(UUID.c_str());
    info_.ioDirection = 1;
    return SUCCESS;
}

int32_t OfflineAudioEffectServerChain::ReleaseEffectModel()
{
    if (info_.libName != nullptr) {
        free(info_.libName);
    }
    if (info_.effectId != nullptr) {
        free(info_.effectId);
    }
    if (model_ != nullptr) {
        IEffectModelRelease(model_, false);
    }
    return SUCCESS;
}

int32_t OfflineAudioEffectServerChain::GetOfflineAudioEffectChains(std::vector<std::string> &chainNamesVector)
{
    AUDIO_INFO_LOG("GetOfflineAudioEffectChains done");
    return SUCCESS;
}

int32_t OfflineAudioEffectServerChain::Create()
{
    AUDIO_INFO_LOG("Create %{public}s done", chainName_.c_str());
    return SUCCESS;
}

int32_t OfflineAudioEffectServerChain::Configure()
{
    int8_t input[SEND_COMMAND_LEN] = {0};
    int8_t output[GET_BUFFER_LEN] = {0};
    uint32_t replyLen = GET_BUFFER_LEN;

    int32_t ret = controller_->SendCommand(controller_, AUDIO_EFFECT_COMMAND_SET_CONFIG,
        input, SEND_COMMAND_LEN, output, &replyLen);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERROR, "send command AUDIO_EFFECT_COMMAND_SET_CONFIG failed");
    return SUCCESS;
}

int32_t OfflineAudioEffectServerChain::SetParam(AudioStreamInfo inInfo, AudioStreamInfo outInfo)
{
    AUDIO_INFO_LOG("%{public}d %{public}d %{public}hhu %{public}hhu %{public}" PRIu64 "StreamInfo set",
        inInfo.samplingRate, inInfo.encoding, inInfo.format, inInfo.channels, inInfo.channelLayout);
    AUDIO_INFO_LOG("%{public}d %{public}d %{public}hhu %{public}hhu %{public}" PRIu64 "StreamInfo set",
        outInfo.samplingRate, outInfo.encoding, outInfo.format, outInfo.channels, outInfo.channelLayout);
    return SUCCESS;
}

int32_t OfflineAudioEffectServerChain::GetEffectBufferSize(uint32_t &inBufferSize, uint32_t &outBufferSize)
{
    inBufferSize = DEFAULT_BUFFER_SIZE;
    outBufferSize = DEFAULT_BUFFER_SIZE;
    AUDIO_INFO_LOG("GetEffectBufferSize in server done");
    return SUCCESS;
}

int32_t OfflineAudioEffectServerChain::Prepare(const std::shared_ptr<AudioSharedMemory> &bufferIn,
    const std::shared_ptr<AudioSharedMemory> &bufferOut)
{
    serverBufferIn_ = bufferIn;
    serverBufferOut_ = bufferOut;
    AUDIO_INFO_LOG("Prepare in server done");
    return SUCCESS;
}

int32_t OfflineAudioEffectServerChain::Process(uint32_t inSize, uint32_t outSize)
{
    CHECK_AND_RETURN_RET_LOG(inSize <= serverBufferIn_->GetSize() && outSize <= serverBufferOut_->GetSize(),
        ERR_INVALID_PARAM, "inSize %{public}u or outSize %{public}u out of range", inSize, outSize);
    uint8_t *bufIn = serverBufferIn_->GetBase();
    uint8_t *bufOut = serverBufferOut_->GetBase();
    for (uint32_t i = 0; i < outSize; i++) {
        bufOut[i] = bufIn[i] << 1;
    }
    AUDIO_INFO_LOG("Process in server done");
    return SUCCESS;
}

int32_t OfflineAudioEffectServerChain::Release()
{
    AUDIO_INFO_LOG("Release in server success");
    return SUCCESS;
}

int32_t OfflineAudioEffectServerChain::SetDeviceType(const DeviceType deviceType)
{
    return SUCCESS;
}

int32_t OfflineAudioEffectServerChain::SetRenderStreamUsage(const StreamUsage usage)
{
    return SUCCESS;
}

int32_t OfflineAudioEffectServerChain::SetCapturerSourceType(const SourceType sourceType)
{
    return SUCCESS;
}
} // namespace AudioStandard
} // namespace OHOS
