/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
#define LOG_TAG "AudioSuiteOutputNode"
#endif

#include <vector>
#include "securec.h"
#include "audio_errors.h"
#include "audio_suite_log.h"
#include "audio_suite_node.h"
#include "audio_suite_common.h"
#include "audio_suite_channel.h"
#include "audio_suite_pcm_buffer.h"
#include "audio_suite_output_node.h"

namespace OHOS {
namespace AudioStandard {
namespace AudioSuite {

static constexpr uint32_t DEFAULT_NODE_OUTPUT_NUM = 1;
static constexpr uint32_t AUDIO_SEPARATION_NODE_OUTPUT_NUM = 2;
static constexpr uint32_t NODE_OUTPUT_NUM_MAX = AUDIO_SEPARATION_NODE_OUTPUT_NUM;
static constexpr uint32_t SECONDS_TO_MS = 1000;
static constexpr uint32_t REASAMPLE_QUAILTY = 5;

AudioOutputNode::AudioOutputNode(AudioFormat format) : AudioNode(AudioNodeType::NODE_TYPE_OUTPUT, format),
    preNodeOutputNum_(DEFAULT_NODE_OUTPUT_NUM), cacheBuffer_(NODE_OUTPUT_NUM_MAX)
{
    SetOutDataFormat(format.audioChannelInfo.numChannels,
        format.audioChannelInfo.channelLayout, format.format, format.rate);
    AUDIO_INFO_LOG("AudioOutputNode create nodeId is %{public}u.", GetAudioNodeId());
}

AudioOutputNode::~AudioOutputNode()
{
    DeInit();
    inputStream_.deInit();
    AUDIO_INFO_LOG("AudioOutputNode destroy nodeId: %{public}u.", GetAudioNodeId());
}

int32_t AudioOutputNode::Init()
{
    frameDuration_ = outFormat_.rate == SAMPLE_RATE_11025 ?
        SINGLE_FRAME_DURATION_SAMPLE_RATE_11025 : SINGLE_FRAME_DURATION;
    uint32_t frameDataLen = outFormat_.rate * outFormat_.numChannels *
        AudioSuiteUtil::GetSampleSize(outFormat_.format) * frameDuration_ / SECONDS_TO_MS;
    for (auto& vec : cacheBuffer_) {
        vec.resize(frameDataLen, 0.0f);
    }
    ClearCacheBuffer();

    uint32_t channelOutputLen = outFormat_.rate * outFormat_.numChannels * frameDuration_ / SECONDS_TO_MS;
    channelOutput_.resize(channelOutputLen, 0.0f);
    rateOutput_.resize(channelOutputLen, 0.0f);

    AUDIO_INFO_LOG("frameDataLen = %{public}u, channelOutputLen = %{public}u.", frameDataLen, channelOutputLen);
    return SUCCESS;
}

int32_t AudioOutputNode::DeInit()
{
    ResetResample();
    return SUCCESS;
}

void AudioOutputNode::SetAudioNodeFormat(AudioFormat audioFormat)
{
    AUDIO_INFO_LOG("numChannels:%{public}u, sampleFormat:%{public}u, sampleRate:%{public}d, encodingType:%{public}d",
        audioFormat.audioChannelInfo.numChannels, audioFormat.format, audioFormat.rate, audioFormat.encodingType);
    AudioNode::SetAudioNodeFormat(audioFormat);
    SetOutDataFormat(audioFormat.audioChannelInfo.numChannels,
        audioFormat.audioChannelInfo.channelLayout, audioFormat.format, audioFormat.rate);

    int32_t ret = DeInit();
    CHECK_AND_RETURN_LOG(ret == SUCCESS, "DeInit failed, ret = %{public}d.", ret);

    ret = Init();
    CHECK_AND_RETURN_LOG(ret == SUCCESS, "Init failed, ret = %{public}d.", ret);
}

int32_t AudioOutputNode::Connect(const std::shared_ptr<AudioNode> &preNode)
{
    if (preNode == nullptr) {
        AUDIO_ERR_LOG("Connect failed, preNode is nullptr.");
        return ERR_INVALID_PARAM;
    }

    inputStream_.Connect(preNode->GetSharedInstance(), preNode->GetOutputPort().get());

    if (preNode->GetNodeType() == NODE_TYPE_AUDIO_SEPARATION) {
        preNodeOutputNum_ = AUDIO_SEPARATION_NODE_OUTPUT_NUM;
        AUDIO_ERR_LOG("current output node connect audio separation node.");
    } else {
        preNodeOutputNum_ = DEFAULT_NODE_OUTPUT_NUM;
    }
    return SUCCESS;
}

int32_t AudioOutputNode::DisConnect(const std::shared_ptr<AudioNode> &preNode)
{
    inputStream_.DisConnect(preNode);
    return SUCCESS;
}

int32_t AudioOutputNode::Flush()
{
    ClearCacheBuffer();
    SetAudioNodeDataFinishedFlag(false);
    return SUCCESS;
}

int32_t AudioOutputNode::DoProcess()
{
    std::vector<AudioSuitePcmBuffer *> &outputs = inputStream_.ReadPreOutputData();
    CHECK_AND_RETURN_RET_LOG(outputs.size() == static_cast<size_t>(preNodeOutputNum_), ERROR,
        "outputs size = %{public}zu not equals nodeOutputNum = %{public}d", outputs.size(), preNodeOutputNum_);

    bool allNonNull = std::all_of(outputs.begin(), outputs.end(), [](AudioSuitePcmBuffer *output) {
        return output != nullptr;
    });
    CHECK_AND_RETURN_RET_LOG(allNonNull, ERROR, "Get pre node output data is nullptr.");

    SetAudioNodeDataFinishedFlag(false);
    const std::vector<AudioSuitePcmBuffer *> &inputs = inputStream_.getInputData();
    CHECK_AND_RETURN_RET_LOG(inputs.size() == static_cast<size_t>(preNodeOutputNum_), ERROR,
        "inputStream count = %{public}zu not equals nodeOutputNum = %{public}d.", inputs.size(), preNodeOutputNum_);

    int32_t ret = SUCCESS;
    AudioSuitePcmBuffer *pcmBuffer = nullptr;
    for (int32_t idx = 0; idx < preNodeOutputNum_; idx++) {
        pcmBuffer = inputs[idx];
        CHECK_AND_RETURN_RET_LOG(pcmBuffer != nullptr, ERROR, "Get inputdata fail, pcmBuffer is null.");

        SetInDataFormat(pcmBuffer->GetChannelCount(), pcmBuffer->GetChannelLayout(), SAMPLE_F32LE,
            static_cast<AudioSamplingRate>(pcmBuffer->GetSampleRate()));

        float *inData = pcmBuffer->GetPcmDataBuffer();
        CHECK_AND_RETURN_RET_LOG(inData != nullptr, ERROR, "Get inputdata fail, data is null.");
        ret = FormatConversion(inData, pcmBuffer->GetFrameLen() * sizeof(float),
            cacheBuffer_[idx].data(), cacheBuffer_[idx].size());
        CHECK_AND_RETURN_RET(ret == SUCCESS, ERROR);

        SetAudioNodeDataFinishedFlag(pcmBuffer->GetIsFinished());
    }

    return SUCCESS;
}

int32_t AudioOutputNode::FormatConversion(float *inData, size_t inDataLen, uint8_t *outData, size_t outDataSize)
{
    int32_t ret = SUCCESS;
    if (inFormat_.rate != outFormat_.rate) {
        uint32_t rateDataLen = outFormat_.rate * inFormat_.numChannels * frameDuration_ / SECONDS_TO_MS;
        rateOutput_.resize(rateDataLen);
        float *rateData = rateOutput_.data();

        ret = SetUpResample(inFormat_.rate, outFormat_.rate, inFormat_.numChannels, REASAMPLE_QUAILTY);
        CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERROR, "Set sample convert param failed, ret = %{public}d", ret);

        ret = DoResampleProcess(inData, inDataLen / sizeof(float) / inFormat_.numChannels,
            rateData, rateDataLen / inFormat_.numChannels);
        CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERROR, "process sample convert param failed, ret = %{public}d", ret);
    
        inData = rateData;
        inDataLen = rateDataLen * sizeof(float);
    }

    if (inFormat_.numChannels != outFormat_.numChannels) {
        AudioChannelInfo inChannel = {inFormat_.channelLayout, inFormat_.numChannels};
        AudioChannelInfo outChannel = {outFormat_.channelLayout, outFormat_.numChannels};
        ret = SetChannelConvertProcessParam(inChannel, outChannel, inFormat_.format, true);
        CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERROR, "Set channel convert param failed, ret = %{public}d", ret);

        float *channelData = channelOutput_.data();
        size_t channelDataLen = channelOutput_.size() * sizeof(float);
        ret = ChannelConvertProcess(inDataLen / sizeof(float) / inFormat_.numChannels,
            inData, inDataLen, channelData, channelDataLen);
        CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERROR, "process channel convert param failed, ret = %{public}d", ret);
        inData = channelData;
        inDataLen = channelDataLen;
    }

    if (inFormat_.format != outFormat_.format) {
        ConvertFromFloat(outFormat_.format, inDataLen / sizeof(float), inData, static_cast<void *>(outData));
    } else {
        errno_t err = memcpy_s(outData, outDataSize, inData, inDataLen);
        CHECK_AND_RETURN_RET_LOG(err == EOK, ERROR, "memcpy_s failed, ret = %{public}d.", err);
    }
    bufferUsedOffset_ = 0;

    return SUCCESS;
}

int32_t AudioOutputNode::DoProcess(uint8_t *audioData, int32_t frameSize, int32_t *writeDataSize, bool *finished)
{
    uint8_t *audioDataArray[] = { audioData };
    return DoProcess(audioDataArray, DEFAULT_NODE_OUTPUT_NUM, frameSize, writeDataSize, finished);
}

int32_t AudioOutputNode::DoProcess(uint8_t **audioDataArray, int32_t arraySize,
    int32_t requestFrameSize, int32_t *responseSize, bool *finishedFlag)
{
    CHECK_AND_RETURN_RET(DoProcessParamCheck(
        audioDataArray, arraySize, requestFrameSize, responseSize, finishedFlag) == SUCCESS, ERR_INVALID_PARAM);

    if (GetAudioNodeDataFinishedFlag() && CacheBufferEmpty()) {
        AUDIO_ERR_LOG("multi ouput finished completed.");
        return ERR_NOT_SUPPORTED;
    }

    int32_t writeDataSize = 0;
    int32_t remainRequestSize = requestFrameSize;
    do {
        if (!CacheBufferEmpty()) {
            int32_t copySize = std::min(remainRequestSize, GetCacheBufferDataLen());
            for (int32_t idx = 0; idx < arraySize; idx++) {
                errno_t err = memcpy_s(audioDataArray[idx] + writeDataSize, remainRequestSize,
                    GetCacheBufferData(idx), copySize);
                CHECK_AND_RETURN_RET_LOG(err == EOK, ERROR, "memcpy_s failed, ret = %{public}d.", err);
            }
            remainRequestSize -= copySize;
            writeDataSize += copySize;
            UpdateUsedOffset(copySize);
        }

        if (!CacheBufferEmpty()) {
            *responseSize = writeDataSize;
            *finishedFlag = false;
            return SUCCESS;
        }

        if (GetAudioNodeDataFinishedFlag()) {
            *responseSize = writeDataSize;
            *finishedFlag = true;
            return SUCCESS;
        }

        if (writeDataSize == requestFrameSize) {
            *responseSize = writeDataSize;
            *finishedFlag = false;
            return SUCCESS;
        }

        CHECK_AND_RETURN_RET_LOG(DoProcess() == SUCCESS, ERROR, "Get data from pre node failed.");
    } while (writeDataSize < requestFrameSize);

    AUDIO_ERR_LOG("write data failed, writeDataSize = %{public}d.", writeDataSize);
    return ERROR;
}

int32_t AudioOutputNode::DoProcessParamCheck(uint8_t **audioDataArray, int32_t arraySize,
    int32_t requestFrameSize, int32_t *responseSize, bool *finishedFlag)
{
    if ((audioDataArray == nullptr) || (responseSize == nullptr) || (finishedFlag == nullptr)) {
        AUDIO_ERR_LOG("check output param failed, parame is nullptr.");
        return ERR_INVALID_PARAM;
    }

    if (arraySize != preNodeOutputNum_) {
        AUDIO_ERR_LOG("arraySize = %{public}d not equal outputNum = %{public}d.", arraySize, preNodeOutputNum_);
        return ERR_INVALID_PARAM;
    }

    for (int32_t idx = 0; idx < arraySize; idx++) {
        if (audioDataArray[idx] == nullptr) {
            AUDIO_ERR_LOG("the %{public}d output is nullptr.", idx);
            return ERR_INVALID_PARAM;
        }
    }

    if (requestFrameSize <= 0) {
        AUDIO_ERR_LOG("requesetFrameSize = %{public}d invalid.", requestFrameSize);
        return ERR_INVALID_PARAM;
    }

    return SUCCESS;
}

void AudioOutputNode::SetInDataFormat(
    uint32_t channels, AudioChannelLayout layout, AudioSampleFormat sample, uint32_t rate)
{
    inFormat_.numChannels = channels;
    inFormat_.channelLayout = layout;
    inFormat_.format = sample;
    inFormat_.rate = rate;
}

void AudioOutputNode::SetOutDataFormat(
    uint32_t channels, AudioChannelLayout layout, AudioSampleFormat sample, uint32_t rate)
{
    outFormat_.numChannels = channels;
    outFormat_.channelLayout = layout;
    outFormat_.format = sample;
    outFormat_.rate = rate;
}

bool AudioOutputNode::CacheBufferEmpty()
{
    return cacheBuffer_[0].size() <= bufferUsedOffset_;
}

void AudioOutputNode::UpdateUsedOffset(size_t bytesConsumed)
{
    bufferUsedOffset_ += bytesConsumed;
}

void AudioOutputNode::ClearCacheBuffer()
{
    bufferUsedOffset_ = cacheBuffer_[0].size();
}

int32_t AudioOutputNode::GetCacheBufferDataLen()
{
    if (bufferUsedOffset_ >= cacheBuffer_[0].size()) {
        return 0;
    }

    return cacheBuffer_[0].size() - bufferUsedOffset_;
}

uint8_t *AudioOutputNode::GetCacheBufferData(size_t idx)
{
    return cacheBuffer_[idx].data() + bufferUsedOffset_;
}

}  // namespace AudioSuite
}  // namespace AudioStandard
}  // namespace OHOS