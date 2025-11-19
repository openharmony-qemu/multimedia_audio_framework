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

AudioOutputNode::AudioOutputNode(AudioFormat format)
    : AudioNode(AudioNodeType::NODE_TYPE_OUTPUT, format),
      preNodeOutputNum_(DEFAULT_NODE_OUTPUT_NUM)
{
    AUDIO_INFO_LOG("AudioOutputNode create nodeId is %{public}u.", GetAudioNodeId());
}

AudioOutputNode::~AudioOutputNode()
{
    DeInit();
    inputStream_.deInit();
    AUDIO_INFO_LOG("AudioOutputNode destroy nodeId: %{public}u.", GetAudioNodeId());
}

void AudioOutputNode::SetAudioNodeFormat(AudioFormat audioFormat)
{
    AUDIO_INFO_LOG("numChannels:%{public}u, sampleFormat:%{public}u, sampleRate:%{public}d, encodingType:%{public}d",
        audioFormat.audioChannelInfo.numChannels, audioFormat.format, audioFormat.rate, audioFormat.encodingType);
    AudioNode::SetAudioNodeFormat(audioFormat);
}

int32_t AudioOutputNode::Connect(const std::shared_ptr<AudioNode> &preNode)
{
    if (preNode == nullptr) {
        AUDIO_INFO_LOG("Connect failed, preNode is nullptr.");
        return ERR_INVALID_PARAM;
    }

    inputStream_.Connect(preNode->GetSharedInstance(), preNode->GetOutputPort().get());

    if (preNode->GetNodeType() == NODE_TYPE_AUDIO_SEPARATION) {
        preNodeOutputNum_ = AUDIO_SEPARATION_NODE_OUTPUT_NUM;
        AUDIO_INFO_LOG("current output node connect audio separation node.");
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
    std::vector<AudioSuitePcmBuffer *> &inputs = inputStream_.ReadPreOutputData(GetAudioNodeInPcmFormat(), true);

    outputs_.clear();
    outputs_.insert(outputs_.end(), inputs.begin(), inputs.end());
    CHECK_AND_RETURN_RET_LOG(outputs_.size() == static_cast<size_t>(preNodeOutputNum_), ERROR,
        "outputs size = %{public}zu not equals nodeOutputNum = %{public}d", outputs_.size(), preNodeOutputNum_);

    bool allNonNull = std::all_of(outputs_.begin(), outputs_.end(), [](AudioSuitePcmBuffer *output) {
        return output != nullptr;
    });
    CHECK_AND_RETURN_RET_LOG(allNonNull, ERROR, "Get pre node output data is nullptr.");
    SetAudioNodeDataFinishedFlag(inputs[0]->GetIsFinished());
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
                uint8_t *data =  GetCacheBufferData(idx);
                CHECK_AND_RETURN_RET_LOG(data != nullptr, ERROR, "Get data from pcmbuffer failed.");
                errno_t err = memcpy_s(audioDataArray[idx] + writeDataSize, remainRequestSize, data, copySize);
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

bool AudioOutputNode::CacheBufferEmpty()
{
    if (outputs_.empty() || (outputs_[0] == nullptr)) {
        return true;
    }

    return outputs_[0]->GetDataSize() <= bufferUsedOffset_;
}

void AudioOutputNode::UpdateUsedOffset(size_t bytesConsumed)
{
    bufferUsedOffset_ += bytesConsumed;
}

void AudioOutputNode::ClearCacheBuffer()
{
    if (outputs_.empty() || (outputs_[0] == nullptr)) {
        bufferUsedOffset_ = 0;
    } else {
        bufferUsedOffset_ = outputs_[0]->GetDataSize();
    }
}

int32_t AudioOutputNode::GetCacheBufferDataLen()
{
    if (outputs_.empty() || (outputs_[0] == nullptr)) {
        return 0;
    }

    if (bufferUsedOffset_ >= outputs_[0]->GetDataSize()) {
        return 0;
    }

    return outputs_[0]->GetDataSize() - bufferUsedOffset_;
}

uint8_t *AudioOutputNode::GetCacheBufferData(size_t idx)
{
    if (outputs_.size() < idx) {
        return nullptr;
    }

    if (outputs_[idx] == nullptr) {
        return nullptr;
    }

    return outputs_[idx]->GetPcmData() + bufferUsedOffset_;
}

}  // namespace AudioSuite
}  // namespace AudioStandard
}  // namespace OHOS