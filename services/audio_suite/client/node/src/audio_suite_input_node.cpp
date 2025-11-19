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
#define LOG_TAG "AudioSuiteInputNode"
#endif

#include <cinttypes>
#include "audio_errors.h"
#include "audio_suite_log.h"
#include "audio_suite_node.h"
#include "audio_suite_input_node.h"
#include "audio_suite_pcm_buffer.h"

namespace OHOS {
namespace AudioStandard {
namespace AudioSuite {

static constexpr uint32_t REQUEST_DATA_TRY_COUNTS = 3;
static constexpr uint32_t CACHE_FRAME_DEFAULT_LEN = 3;
static constexpr uint32_t CACHE_FRAME_SAMPLE_RATE_11025_LEN = 4;

AudioInputNode::AudioInputNode(AudioFormat format) : AudioNode(AudioNodeType::NODE_TYPE_INPUT, format)
{
    AUDIO_INFO_LOG("numChannels:%{public}u, channelLayout:%{public}" PRIu64 "sampleFormat:%{public}u,"
        "sampleRate:%{public}d, encodingType:%{public}d", format.audioChannelInfo.numChannels,
        format.audioChannelInfo.channelLayout, format.format, format.rate, format.encodingType);
}

AudioInputNode::~AudioInputNode()
{
    AUDIO_INFO_LOG("AudioInputNode::~AudioInputNode");
}

int32_t AudioInputNode::Init()
{
    if (outputStream_ == nullptr) {
        outputStream_ = std::make_shared<OutputPort<AudioSuitePcmBuffer*>>(GetSharedInstance());
        CHECK_AND_RETURN_RET_LOG(outputStream_ != nullptr, ERR_INVALID_OPERATION, "Create OutputPort is null");
    }
    uint32_t doubleFrame = 2;
    PcmBufferFormat inPcmFormat = GetAudioNodeInPcmFormat();
    if (GetAudioNodeFormat().rate == AudioSamplingRate::SAMPLE_RATE_11025) {
        inPcmData_.ResizePcmBuffer(inPcmFormat, PCM_DATA_DURATION_40_MS);
        inPcmFormat.sampleRate = SAMPLE_RATE_16000;
        inPcmFormat.sampleFormat = SAMPLE_F32LE;
        outPcmData_.ResizePcmBuffer(inPcmFormat);
        singleRequestSize_ = outPcmData_.GetDataSize() * doubleFrame;
    } else {
        inPcmData_.ResizePcmBuffer(inPcmFormat);
        outPcmData_.ResizePcmBuffer(inPcmFormat);
        singleRequestSize_ = inPcmData_.GetDataSize();
    }

    uint32_t frames = GetAudioNodeFormat().rate == AudioSamplingRate::SAMPLE_RATE_11025 ?
        CACHE_FRAME_SAMPLE_RATE_11025_LEN : CACHE_FRAME_DEFAULT_LEN;
    cachedBuffer_.ResizeBuffer(outPcmData_.GetDataSize() * frames);
    return SUCCESS;
}

int32_t AudioInputNode::DeInit()
{
    Flush();
    return SUCCESS;
}

int32_t AudioInputNode::Flush()
{
    AUDIO_INFO_LOG("AudioInputNode::Flush");
    cachedBuffer_.ClearBuffer();
    SetAudioNodeDataFinishedFlag(false);
    convert_.Reset();
    outputStream_->resetResampleCfg();
    return SUCCESS;
}

int32_t AudioInputNode::Connect(const std::shared_ptr<AudioNode>& preNode)
{
    AUDIO_ERR_LOG("AudioInputNode::Connect not support opt");
    return ERROR;
}

int32_t AudioInputNode::DisConnect(const std::shared_ptr<AudioNode>& preNode)
{
    AUDIO_ERR_LOG("AudioInputNode::DisConnect not support opt");
    return ERROR;
}

std::shared_ptr<OutputPort<AudioSuitePcmBuffer*>> AudioInputNode::GetOutputPort()
{
    return outputStream_;
}

int32_t AudioInputNode::DoProcess()
{
    CHECK_AND_RETURN_RET(outputStream_ != nullptr, ERR_INVALID_PARAM, "outputStream is null");
    CHECK_AND_RETURN_RET(GetDataFromUser() == SUCCESS, ERR_WRITE_FAILED, "Get data from user fail");
    CHECK_AND_RETURN_RET(GeneratePushBuffer() == SUCCESS, ERR_WRITE_FAILED, "Get data from buffer fail");
    return SUCCESS;
}

int32_t AudioInputNode::SetRequestDataCallback(std::shared_ptr<InputNodeRequestDataCallBack> callback)
{
    CHECK_AND_RETURN_RET(callback != nullptr, ERR_INVALID_PARAM,
        "AudioInputNode::SetRequestDataCallback callback is null");
    reqDataCallback_ = callback;
    return SUCCESS;
}

bool AudioInputNode::IsSetReadDataCallback()
{
    return reqDataCallback_ != nullptr;
}

void AudioInputNode::SetAudioNodeFormat(AudioFormat audioFormat)
{
    AUDIO_INFO_LOG("AudioInputNode::SetAudioNodeFormat, numChannels:%{public}u,"
        "channelLayout:%{public}" PRIu64 "sampleFormat:%{public}u, sampleRate:%{public}d, encodingType:%{public}d",
        audioFormat.audioChannelInfo.numChannels, audioFormat.audioChannelInfo.channelLayout,
        audioFormat.format, audioFormat.rate, audioFormat.encodingType);

    AudioNode::SetAudioNodeFormat(audioFormat);
    DeInit();
    Init();
}

int32_t AudioInputNode::GetDataFromUser()
{
    CHECK_AND_RETURN_RET_LOG(reqDataCallback_ != nullptr, ERR_INVALID_PARAM, "reqDataCallback is null");
    if (GetAudioNodeDataFinishedFlag()) {
        AUDIO_INFO_LOG("GetDataFromUser already finish");
        return SUCCESS;
    }

    uint32_t needSize = cachedBuffer_.GetRestSpace();
    if (needSize < singleRequestSize_) {
        return SUCCESS;
    }

    uint32_t times = needSize / singleRequestSize_;
    uint32_t curTryCounts = 0;
    bool isFinished = false;
    while ((times > 0) && !isFinished) {
        uint32_t singleGetSize = inPcmDataGetSize_;
        if (singleGetSize == 0) {
            inPcmData_.Reset();
        }
        while ((singleGetSize < inPcmData_.GetDataSize()) && (curTryCounts < REQUEST_DATA_TRY_COUNTS) && !isFinished) {
            int32_t getSize = reqDataCallback_->OnRequestDataCallBack(
                inPcmData_.GetPcmData() + singleGetSize, inPcmData_.GetDataSize() - singleGetSize, &isFinished);

            ++curTryCounts;
            CHECK_AND_RETURN_RET_LOG((getSize > 0) &&
                (static_cast<uint32_t>(getSize) <= inPcmData_.GetDataSize() - singleGetSize),
                ERROR, "OnRequestDataCallBack error, ret = %{public}d", singleGetSize);
            singleGetSize += static_cast<uint32_t>(getSize);
        }

        if ((singleGetSize == inPcmData_.GetDataSize()) || isFinished) {
            AudioSuitePcmBuffer *ConverPcmData = convert_.Process(&inPcmData_, outPcmData_.GetPcmBufferFormat());
            CHECK_AND_RETURN_RET_LOG(ConverPcmData != nullptr, ERR_INVALID_PARAM, "convert pcm format fail");

            int32_t ret = cachedBuffer_.PushData(ConverPcmData->GetPcmData(), ConverPcmData->GetDataSize());
            AUDIO_INFO_LOG("222 GetDataFromUser needSize:%{public}u, inPcmDataSize:%{public}u, times:%{public}u.",
                needSize, ConverPcmData->GetDataSize(), times);

            CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "Push data to cache fail, ret = %{public}d", ret);
            inPcmDataGetSize_ = 0;
        } else {
            inPcmDataGetSize_ = singleGetSize;
            break;
        }
        times--;
    }

    SetAudioNodeDataFinishedFlag(isFinished);
    return SUCCESS;
}

int32_t AudioInputNode::GeneratePushBuffer()
{
    CHECK_AND_RETURN_RET_LOG(cachedBuffer_.GetSize() >= outPcmData_.GetDataSize(), ERROR, "cachedBuffer not data");

    int32_t ret = cachedBuffer_.GetData(outPcmData_.GetPcmData(), outPcmData_.GetDataSize());
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "Get data from cachedBuffer fail");

    outPcmData_.SetIsFinished(GetAudioNodeDataFinishedFlag() && (cachedBuffer_.GetSize() == 0));
    outputStream_->WriteDataToOutput(&outPcmData_);
    return SUCCESS;
}

}
}
}