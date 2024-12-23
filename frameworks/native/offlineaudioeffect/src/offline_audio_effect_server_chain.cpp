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
#include <cinttypes>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <unistd.h>
#include "securec.h"

#include "audio_common_log.h"
#include "audio_errors.h"
#include "audio_utils.h"

using namespace OHOS::AudioStandard;

namespace OHOS {
namespace AudioStandard {
namespace {
constexpr uint32_t MAX_DESCRIPTOR_NUM = 20;
constexpr uint32_t MAX_CMD_LEN = 10;
constexpr uint32_t MAX_REPLY_LEN = 10;
constexpr uint32_t MAX_TIME_INTERVAL_MS = 160; // ms
// key for effectName, value for (libName, effectId)
static std::map<std::string, std::pair<std::string, std::string>> g_chainName2infoMap;
static std::mutex g_chainMutex;
}

template<typename T>
static inline void FreeIfNotNull(T*& ptr)
{
    if (ptr != nullptr) {
        free(ptr);
        ptr = nullptr;
    }
}

static inline int32_t GetByteSize(AudioSampleFormat format)
{
    static const std::unordered_map<AudioSampleFormat, int32_t> sizeMap = {
        {SAMPLE_U8, 1},
        {SAMPLE_S16LE, 2},
        {SAMPLE_S24LE, 3},
        {SAMPLE_S32LE, 4},
        {SAMPLE_F32LE, 4}
    };

    auto it = sizeMap.find(format);
    return (it != sizeMap.end()) ? it->second : 2;  // Default size is 2
}

OfflineAudioEffectServerChain::OfflineAudioEffectServerChain(const std::string &chainName) : chainName_(chainName) {}

OfflineAudioEffectServerChain::~OfflineAudioEffectServerChain()
{
    if (controller_) {
        Release();
    }
    FreeIfNotNull(controllerId_.libName);
    FreeIfNotNull(controllerId_.effectId);
    serverBufferIn_ = nullptr;
    serverBufferOut_ = nullptr;
    DumpFileUtil::CloseDumpFile(&dumpFileIn_);
    DumpFileUtil::CloseDumpFile(&dumpFileOut_);
}

static struct IEffectModel *InitEffectModel()
{
    static struct IEffectModel *effectModel = IEffectModelGet(false);
    return effectModel;
}

static void InitControllerDescriptor()
{
    std::lock_guard<std::mutex> maplock(g_chainMutex);
    if (g_chainName2infoMap.size() > 0) {
        return;
    }
    static uint32_t descsLen = MAX_DESCRIPTOR_NUM;
    auto model = InitEffectModel();
    CHECK_AND_RETURN_LOG(model, "get all descriptor failed, effectmodel is nullptr");
    struct EffectControllerDescriptor descs[MAX_DESCRIPTOR_NUM];
    int32_t ret = model->GetAllEffectDescriptors(model, descs, &descsLen);
    CHECK_AND_RETURN_LOG(model, "get all descriptor failed, errCode is %{public}d", ret);
    for (uint32_t i = 0; i < descsLen; i++) {
        g_chainName2infoMap.insert_or_assign(descs[i].effectName,
            std::make_pair(std::string(descs[i].libName), std::string(descs[i].effectId)));
        FreeIfNotNull(descs[i].effectName);
        FreeIfNotNull(descs[i].libName);
        FreeIfNotNull(descs[i].effectId);
        FreeIfNotNull(descs[i].supplier);
    }
}

void OfflineAudioEffectServerChain::InitDump()
{
    static uint32_t chainId = 0;
    std::string dumpFileName = "OfflineEffectServer";
    std::string dumpFileInName = dumpFileName + "_" + std::to_string(chainId) + "_In.pcm";
    std::string dumpFileOutName = dumpFileName + "_" + std::to_string(chainId) + "_Out.pcm";
    DumpFileUtil::OpenDumpFile(DUMP_SERVER_PARA, dumpFileInName, &dumpFileIn_);
    DumpFileUtil::OpenDumpFile(DUMP_SERVER_PARA, dumpFileOutName, &dumpFileOut_);
    chainId++;
}

int32_t OfflineAudioEffectServerChain::GetOfflineAudioEffectChains(std::vector<std::string> &chainNamesVector)
{
    InitControllerDescriptor();
    std::lock_guard<std::mutex> maplock(g_chainMutex);
    for (auto item : g_chainName2infoMap) {
        chainNamesVector.emplace_back(item.first);
    }
    AUDIO_INFO_LOG("GetOfflineAudioEffectChains done");
    return SUCCESS;
}

int32_t OfflineAudioEffectServerChain::Create()
{
    InitControllerDescriptor();
    std::lock_guard<std::mutex> maplock(g_chainMutex);
    auto mapIter = g_chainName2infoMap.find(chainName_);
    CHECK_AND_RETURN_RET_LOG(mapIter != g_chainName2infoMap.end(), ERROR,
        "create failed, no chain named %{public}s", chainName_.c_str());
    auto model = InitEffectModel();
    CHECK_AND_RETURN_RET_LOG(model, ERROR, "create failed, effectmodel is nullptr");

    // second.first for libName, second.second for effectId, 1 for ioDirection
    // do not need to release char* in info
    struct EffectInfo info = {&mapIter->second.first[0], &mapIter->second.second[0], 1};

    int32_t ret = model->CreateEffectController(model, &info, &controller_, &controllerId_);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERROR,
        "create %{public}s effect controller failed, errCode is %{public}d", chainName_.c_str(), ret);

    int8_t input[MAX_CMD_LEN] = {0};
    int8_t output[MAX_REPLY_LEN] = {0};
    uint32_t replyLen = MAX_REPLY_LEN;

    std::lock_guard<std::mutex> lock(offlineChainMutex_);
    CHECK_AND_RETURN_RET_LOG(controller_, ERROR, "enable failed, controller is nullptr");
    ret = controller_->SendCommand(controller_, AUDIO_EFFECT_COMMAND_ENABLE,
        static_cast<int8_t *>(input), MAX_CMD_LEN, output, &replyLen);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERROR,
        "%{public}s effect COMMAND_ENABLE failed, errCode is %{public}d", chainName_.c_str(), ret);

    InitDump();

    AUDIO_INFO_LOG("Create %{public}s done", chainName_.c_str());
    return SUCCESS;
}

int32_t OfflineAudioEffectServerChain::SetParam(AudioStreamInfo inInfo, AudioStreamInfo outInfo)
{
    AUDIO_INFO_LOG("%{public}d %{public}d %{public}hhu %{public}hhu %{public}" PRIu64 " InStreamInfo set",
        inInfo.samplingRate, inInfo.encoding, inInfo.format, inInfo.channels, inInfo.channelLayout);
    AUDIO_INFO_LOG("%{public}d %{public}d %{public}hhu %{public}hhu %{public}" PRIu64 " OutStreamInfo set",
        outInfo.samplingRate, outInfo.encoding, outInfo.format, outInfo.channels, outInfo.channelLayout);

    offlineConfig_.inputCfg = {inInfo.samplingRate, inInfo.channels, inInfo.format};
    offlineConfig_.outputCfg = {outInfo.samplingRate, outInfo.channels, outInfo.format};

    int8_t output[MAX_REPLY_LEN] = {0};
    uint32_t replyLen = MAX_REPLY_LEN;

    std::lock_guard<std::mutex> lock(offlineChainMutex_);
    CHECK_AND_RETURN_RET_LOG(controller_, ERROR, "configure failed, controller is nullptr");
    int32_t ret = controller_->SendCommand(controller_, AUDIO_EFFECT_COMMAND_SET_CONFIG,
        reinterpret_cast<int8_t *>(&offlineConfig_), sizeof(offlineConfig_), output, &replyLen);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERROR,
        "%{public}s effect COMMAND_SET_CONFIG failed, errCode is %{public}d", chainName_.c_str(), ret);

    inBufferSize_ = static_cast<uint32_t>(GetByteSize(inInfo.format)) * inInfo.samplingRate * inInfo.channels *
        MAX_TIME_INTERVAL_MS / AUDIO_MS_PER_SECOND;
    outBufferSize_ = static_cast<uint32_t>(GetByteSize(outInfo.format)) * outInfo.samplingRate * outInfo.channels *
        MAX_TIME_INTERVAL_MS / AUDIO_MS_PER_SECOND;
    return SUCCESS;
}

int32_t OfflineAudioEffectServerChain::GetEffectBufferSize(uint32_t &inBufferSize, uint32_t &outBufferSize)
{
    CHECK_AND_RETURN_RET_LOG(inBufferSize_ != 0, ERROR, "inBufferSize_ do not init");
    CHECK_AND_RETURN_RET_LOG(outBufferSize_ != 0, ERROR, "inBufferSize_ do not init");
    inBufferSize = inBufferSize_;
    outBufferSize = outBufferSize_;
    AUDIO_INFO_LOG("GetEffectBufferSize in server done, inBufferSize_:%{public}u inBufferSize_:%{public}u",
        inBufferSize_, outBufferSize_);
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
    CHECK_AND_RETURN_RET_LOG(serverBufferIn_ && serverBufferIn_->GetBase(), ERROR, "serverBufferIn_ is nullptr");
    CHECK_AND_RETURN_RET_LOG(serverBufferOut_ && serverBufferOut_->GetBase(), ERROR, "serverBufferOut_ is nullptr");

    CHECK_AND_RETURN_RET_LOG(inSize <= inBufferSize_, ERROR,
        "inSize %{public}u > serverInBufferSize %{public}u", inSize, inBufferSize_);
    CHECK_AND_RETURN_RET_LOG(outSize <= outBufferSize_, ERROR,
        "outSize %{public}u > serverOutBufferSize %{public}u", outSize, outBufferSize_);

    DumpFileUtil::WriteDumpFile(dumpFileIn_, serverBufferIn_->GetBase(), inSize);

    struct AudioEffectBuffer input;
    struct AudioEffectBuffer output;

    input = {static_cast<int32_t>(inSize) / GetFormatByteSize(offlineConfig_.inputCfg.format),
        GetFormatByteSize(offlineConfig_.inputCfg.format),
        reinterpret_cast<int8_t *>(serverBufferIn_->GetBase()), static_cast<int32_t>(inSize)};
    output = {};

    std::lock_guard<std::mutex> lock(offlineChainMutex_);
    CHECK_AND_RETURN_RET_LOG(controller_, ERROR, "process failed, controller is nullptr");
    int32_t ret = controller_->EffectProcess(controller_, &input, &output);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERROR, "EffectProcess failed ret:%{public}d", ret);
    ret = memcpy_s(reinterpret_cast<int8_t *>(serverBufferOut_->GetBase()), outSize,
        output.rawData, output.frameCount * GetFormatByteSize(offlineConfig_.outputCfg.format));
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERROR, "memcpy failed, ret:%{public}d", ret);
    FreeIfNotNull(output.rawData);

    DumpFileUtil::WriteDumpFile(dumpFileOut_, serverBufferOut_->GetBase(), outSize);
    return SUCCESS;
}

int32_t OfflineAudioEffectServerChain::Release()
{
    auto model = InitEffectModel();
    CHECK_AND_RETURN_RET_LOG(model, ERROR, "model is nullptr");

    std::lock_guard<std::mutex> lock(offlineChainMutex_);
    int32_t ret = model->DestroyEffectController(model, &controllerId_);
    controller_ = nullptr;
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERROR,
        "chainId:%{public}s release failed, errCode is %{public}d", controllerId_.effectId, ret);

    AUDIO_INFO_LOG("Release in server success");
    return SUCCESS;
}
} // namespace AudioStandard
} // namespace OHOS
