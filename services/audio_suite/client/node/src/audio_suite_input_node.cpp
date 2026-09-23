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
    if (GetAudioNodeFormat().encodingType == ENCODING_METADATA) {
        AUDIO_INFO_LOG("encodingType is metaData not init pcm buffer.");
        return SUCCESS;
    }

    InitFormatConverters();
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
        singleRequestSize_ = outPcmData_.GetDataSize();
    }
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
    nextNodeNeedDataLength_ = 0;
    return SUCCESS;
}

int32_t AudioInputNode::InitCacheBuffer(uint32_t needDataLength)
{
    int32_t ret = outPcmData_.ResizePcmBuffer(outPcmData_.GetPcmBufferFormat(), needDataLength);
    CHECK_AND_RETURN_RET(ret == SUCCESS, ERROR, "outPcmData Failed to initialize storage.");

    if (outPcmData_.GetDataSize() > singleRequestSize_) {
        singleRequestSize_ = outPcmData_.GetDataSize();
    }
    ret = cachedBuffer_.ResizeBuffer(singleRequestSize_);
    CHECK_AND_RETURN_RET(ret == SUCCESS, ERROR, "cachedBuffer ResizeBuffer failed");

    uint32_t inDuration = needDataLength >= inPcmData_.GetDataDuration() ?
        needDataLength : inPcmData_.GetDataDuration();
    ret = inPcmData_.ResizePcmBuffer(inPcmData_.GetPcmBufferFormat(), inDuration);
    CHECK_AND_RETURN_RET(ret == SUCCESS, ERROR, "inPcmData Failed to initialize storage.");
    AUDIO_INFO_LOG("cachedBufferSize:%{public}u, singleRequestSize_:%{public}u, "
        "inPcmDataSize:%{public}u, outPcmDataSize:%{public}u",
        cachedBuffer_.GetRestSpace(), singleRequestSize_, inPcmData_.GetDataSize(), outPcmData_.GetDataSize());

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

int32_t AudioInputNode::DoProcess(uint32_t needDataLength)
{
    if (nextNodeNeedDataLength_ != needDataLength) {
        int32_t ret = InitCacheBuffer(needDataLength);
        CHECK_AND_RETURN_RET(ret == SUCCESS, ERROR, "Failed to initialize storage.");
        nextNodeNeedDataLength_ = needDataLength;
        AUDIO_INFO_LOG("AudioInputNode nextNodeNeedDataLength_:%{public}u", nextNodeNeedDataLength_);
    }
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
    return reqDataCallback_ != nullptr || reqMetaDataCallback_ != nullptr;
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

    auto nextNodes = nextNodes_;
    for (auto& weakNextNode : nextNodes) {
        auto nextNode = weakNextNode.lock();
        if (nextNode != nullptr) {
            if (nextNode->IsSystemNodeType()) {
                nextNode->UpdateInputFormat(audioFormat);
            }
        }
    }
}

int32_t AudioInputNode::GetDataFromUser()
{
    CHECK_AND_RETURN_RET_LOG(reqDataCallback_ != nullptr, ERR_INVALID_PARAM, "reqDataCallback is null");
    if (GetAudioNodeDataFinishedFlag()) {
        AUDIO_INFO_LOG("GetDataFromUser already finish");
        return SUCCESS;
    }

    uint32_t needSize = cachedBuffer_.GetRestSpace();
    AUDIO_DEBUG_LOG("needSize:%{public}u, singleRequestSize_:%{public}u.", needSize, singleRequestSize_);
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
            CHECK_AND_RETURN_RET_LOG((getSize >= 0) &&
                (static_cast<uint32_t>(getSize) <= inPcmData_.GetDataSize() - singleGetSize),
                ERROR, "OnRequestDataCallBack error, ret = %{public}d", singleGetSize);
            singleGetSize += static_cast<uint32_t>(getSize);
            AUDIO_DEBUG_LOG("RequestData getSize:%{public}d, singleGetSize:%{public}u.", getSize, singleGetSize);
        }

        if ((singleGetSize == inPcmData_.GetDataSize()) || isFinished) {
            AudioSuitePcmBuffer *ConverPcmData = inputConverter_.Process(&inPcmData_, outPcmData_.GetPcmBufferFormat());
            CHECK_AND_RETURN_RET_LOG(ConverPcmData != nullptr, ERR_INVALID_PARAM, "convert pcm format fail");

            int32_t ret = cachedBuffer_.PushData(ConverPcmData->GetPcmData(), ConverPcmData->GetDataSize());
            AUDIO_DEBUG_LOG("GetDataFromUser needSize:%{public}u, inPcmDataSize:%{public}u, times:%{public}u.",
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
    CHECK_AND_RETURN_RET_LOG(cachedBuffer_.GetSize() >= outPcmData_.GetDataSize(),
        ERROR,
        "cachedBuffer not data%{public}d",
        cachedBuffer_.GetSize());

    int32_t ret = cachedBuffer_.GetData(outPcmData_.GetPcmData(), outPcmData_.GetDataSize());

    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "Get data from cachedBuffer fail");

    outPcmData_.SetIsFinished(GetAudioNodeDataFinishedFlag() && (cachedBuffer_.GetSize() == 0));
    WriteOutputData(&outPcmData_);
    return SUCCESS;
}

void AudioInputNode::SetRequestMetaDataCallback(std::shared_ptr<InputNodeRequestMetaDataCallBack> callback)
{
    reqMetaDataCallback_ = callback;
}

int32_t AudioInputNode::DoProcess(AudioMetaFrame &metaFrame,
    int32_t *responseAudioSize, int32_t *responseMetaSize, bool *finishedFlag)
{
    CHECK_AND_RETURN_RET_LOG(responseAudioSize != nullptr && responseMetaSize != nullptr && finishedFlag != nullptr,
        ERR_INVALID_PARAM, "DoProcess output param is nullptr");
    CHECK_AND_RETURN_RET_LOG(!GetAudioNodeDataFinishedFlag(), ERR_NOT_SUPPORTED,
        "meta data inputNode data finished, nodeType:%{public}d, nodeId:%{public}u", GetNodeType(), GetAudioNodeId());
    CHECK_AND_RETURN_RET_LOG(GetAudioNodeFormat().encodingType == ENCODING_METADATA, ERR_NOT_SUPPORTED,
        "currentNode encodingType node metaData, nodeType:%{public}d, nodeId:%{public}u, encodingType:%{public}d",
        GetNodeType(), GetAudioNodeId(), GetAudioNodeFormat().encodingType);
    CHECK_AND_RETURN_RET_LOG(reqMetaDataCallback_ != nullptr, ERR_NOT_SUPPORTED,
        "currentNode reqMetaDataCallback not set, nodeType:%{public}d, nodeId:%{public}u, encodingType:%{public}d",
        GetNodeType(), GetAudioNodeId(), GetAudioNodeFormat().encodingType);

    *responseAudioSize = reqMetaDataCallback_->OnRequestDataCallBack(metaFrame, responseMetaSize, finishedFlag);
    AUDIO_INFO_LOG("OnRequestMetaDataCallBack  responseAudioSize:%{public}d, responseMetaSize:%{public}d",
        *responseAudioSize, *responseMetaSize);
    SetAudioNodeDataFinishedFlag(*finishedFlag);
    return SUCCESS;
}

}
}
}