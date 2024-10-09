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
#define LOG_TAG "AudioEnhanceChain"
#endif

#include "audio_enhance_chain.h"

#include <chrono>

#include "securec.h"
#include "audio_effect_log.h"
#include "audio_errors.h"

namespace OHOS {
namespace AudioStandard {
constexpr uint32_t MAX_BATCH_LEN = 20; // Number of input channel, ec(8) + onBoardMic(4) + headsetMic(2) + buffer(6)
constexpr uint32_t MAX_BYTE_LEN_PER_FRAME = 19200;
constexpr uint32_t MAX_BIT_DEPTH = 8;

static int32_t GetOneFrameInputData(EnhanceBufferAttr *enhanceBufferAttr, std::vector<uint8_t> &input,
    uint32_t inputLen)
{
    CHECK_AND_RETURN_RET_LOG(enhanceBufferAttr->bitDepth != 0 &&
        enhanceBufferAttr->bitDepth < MAX_BIT_DEPTH, ERROR, "bitDepth is invalid");
    CHECK_AND_RETURN_RET_LOG(enhanceBufferAttr->byteLenPerFrame != 0 &&
        enhanceBufferAttr->byteLenPerFrame < MAX_BYTE_LEN_PER_FRAME, ERROR, "byteLenPerFrame is out of range");
    CHECK_AND_RETURN_RET_LOG(enhanceBufferAttr->batchLen != 0 &&
        enhanceBufferAttr->batchLen < MAX_BATCH_LEN, ERROR, "batchLen is out of range");

    input.reserve(inputLen);
    std::vector<std::vector<uint8_t>> cache;
    if (enhanceBufferAttr->batchLen > 0) {
        cache.resize(enhanceBufferAttr->batchLen);
    }
    for (auto &it : cache) {
        it.resize(enhanceBufferAttr->byteLenPerFrame);
    }

    int32_t ret = 0;
    // ref channel
    for (uint32_t j = 0; j < enhanceBufferAttr->refNum; ++j) {
        ret = memset_s(cache[j].data(), cache[j].size(), 0, cache[j].size());
        CHECK_AND_RETURN_RET_LOG(ret == 0, ERROR, "memset error in ref channel memcpy");
    }
    uint32_t index = 0;
    for (uint32_t i = 0; i < enhanceBufferAttr->byteLenPerFrame / enhanceBufferAttr->bitDepth; ++i) {
        // mic channel
        for (uint32_t j = enhanceBufferAttr->refNum; j < enhanceBufferAttr->batchLen; ++j) {
            ret = memcpy_s(&cache[j][i * enhanceBufferAttr->bitDepth], enhanceBufferAttr->bitDepth,
                enhanceBufferAttr->input + index, enhanceBufferAttr->bitDepth);
            CHECK_AND_RETURN_RET_LOG(ret == 0, ERROR, "memcpy error in mic channel memcpy");
            index += enhanceBufferAttr->bitDepth;
        }
    }
    for (uint32_t i = 0; i < enhanceBufferAttr->batchLen; ++i) {
        input.insert(input.end(), cache[i].begin(), cache[i].end());
    }
    return SUCCESS;
}

AudioEnhanceChain::AudioEnhanceChain(const std::string &scene)
{
    setConfigFlag_ = false;
    sceneType_ = scene;
    enhanceMode_ = "EFFECT_DEFAULT";
}

AudioEnhanceChain::~AudioEnhanceChain()
{
    ReleaseEnhanceChain();
}

void AudioEnhanceChain::SetEnhanceMode(const std::string &mode)
{
    enhanceMode_ = mode;
}

void AudioEnhanceChain::ReleaseEnhanceChain()
{
    for (uint32_t i = 0; i < standByEnhanceHandles_.size() && i < enhanceLibHandles_.size(); i++) {
        if (!enhanceLibHandles_[i]) {
            continue;
        }
        if (!standByEnhanceHandles_[i]) {
            continue;
        }
        if (!enhanceLibHandles_[i]->releaseEffect) {
            continue;
        }
        enhanceLibHandles_[i]->releaseEffect(standByEnhanceHandles_[i]);
    }
    standByEnhanceHandles_.clear();
    enhanceLibHandles_.clear();
}

void AudioEnhanceChain::AddEnhanceHandle(AudioEffectHandle handle, AudioEffectLibrary *libHandle)
{
    std::lock_guard<std::mutex> lock(chainMutex_);
    int32_t ret = 0;
    AudioEffectTransInfo cmdInfo = {};
    AudioEffectTransInfo replyInfo = {};

    ret = (*handle)->command(handle, EFFECT_CMD_INIT, &cmdInfo, &replyInfo);
    CHECK_AND_RETURN_LOG(ret == 0, "[%{public}s] with [%{public}s], either one of libs EFFECT_CMD_INIT fail",
        sceneType_.c_str(), enhanceMode_.c_str());

    standByEnhanceHandles_.emplace_back(handle);
    enhanceLibHandles_.emplace_back(libHandle);
}

void AudioEnhanceChain::SetHandleConfig(AudioEffectHandle handle, DataDescription desc)
{
    int32_t ret = 0;
    AudioEffectTransInfo cmdInfo = {};
    AudioEffectTransInfo replyInfo = {};
    cmdInfo.data = static_cast<void *>(&desc);
    cmdInfo.size = sizeof(desc);
    
    ret = (*handle)->command(handle, EFFECT_CMD_SET_CONFIG, &cmdInfo, &replyInfo);
    CHECK_AND_RETURN_LOG(ret == 0, "[%{publc}s] with mode [%{public}s], either one of libs \
        EFFECT_CMD_SET_CONFIG fai", sceneType_.c_str(), enhanceMode_.c_str());
    setConfigFlag_ = true;
}

int32_t AudioEnhanceChain::ApplyEnhanceChain(EnhanceBufferAttr *enhanceBufferAttr)
{
    CHECK_AND_RETURN_RET_LOG(enhanceBufferAttr != nullptr, ERROR, "enhance buffer is null");
    
    enhanceBufferAttr->refNum = REF_CHANNEL_NUM_MAP.find(sceneType_)->second;
    enhanceBufferAttr->batchLen = enhanceBufferAttr->refNum + enhanceBufferAttr->micNum;
    uint32_t inputLen = enhanceBufferAttr->byteLenPerFrame * enhanceBufferAttr->batchLen;
    uint32_t outputLen = enhanceBufferAttr->byteLenPerFrame * enhanceBufferAttr->outNum;
    std::vector<uint8_t> input;
    std::vector<uint8_t> output;
    output.resize(outputLen);

    if (IsEmptyEnhanceHandles()) {
        AUDIO_ERR_LOG("audioEnhanceChain->standByEnhanceHandles is empty");
        CHECK_AND_RETURN_RET_LOG(memcpy_s(enhanceBufferAttr->output, outputLen, enhanceBufferAttr->input,
            outputLen) == 0, ERROR, "memcpy error in IsEmptyEnhanceHandles");
        return ERROR;
    }
    if (GetOneFrameInputData(enhanceBufferAttr, input, inputLen) != SUCCESS) {
        AUDIO_ERR_LOG("GetOneFrameInputData failed");
        CHECK_AND_RETURN_RET_LOG(memcpy_s(enhanceBufferAttr->output, outputLen, enhanceBufferAttr->input,
            outputLen) == 0, ERROR, "memcpy error in GetOneFrameInputData");
        return ERROR;
    }
    AudioBuffer audioBufIn_ = {};
    AudioBuffer audioBufOut_ = {};
    audioBufIn_.frameLength = inputLen;
    audioBufOut_.frameLength = outputLen;
    audioBufIn_.raw = static_cast<void *>(input.data());
    audioBufOut_.raw = static_cast<void *>(output.data());

    {
        std::lock_guard<std::mutex> lock(chainMutex_);
        for (AudioEffectHandle handle : standByEnhanceHandles_) {
            int32_t ret = 0;
            if (!setConfigFlag_) {
                DataDescription desc = DataDescription(enhanceBufferAttr->frameLength, enhanceBufferAttr->sampleRate,
                    enhanceBufferAttr->dataFormat, enhanceBufferAttr->micNum, enhanceBufferAttr->refNum,
                    enhanceBufferAttr->outNum);
                SetHandleConfig(handle, desc);
            }
            ret = (*handle)->process(handle, &audioBufIn_, &audioBufOut_);
            CHECK_AND_CONTINUE_LOG(ret == 0, "[%{publc}s] with mode [%{public}s], either one of libs process fail",
                sceneType_.c_str(), enhanceMode_.c_str());
        }
        CHECK_AND_RETURN_RET_LOG(memcpy_s(enhanceBufferAttr->output, outputLen, audioBufOut_.raw, outputLen) == 0,
            ERROR, "memcpy error in audioBufOut_ to enhanceBufferAttr->output");
        return SUCCESS;
    }
}

bool AudioEnhanceChain::IsEmptyEnhanceHandles()
{
    std::lock_guard<std::mutex> lock(chainMutex_);
    return standByEnhanceHandles_.size() == 0;
}

} // namespace AudioStandard
} // namespace OHOS