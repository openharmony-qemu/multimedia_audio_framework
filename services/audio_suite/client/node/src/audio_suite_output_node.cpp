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

#include "audio_errors.h"
#include "audio_suite_log.h"
#include "audio_suite_input_node.h"
#include "audio_suite_output_node.h"
#include "audio_suite_info.h"
#include "audio_suite_node.h"
#include "audio_suite_pcm_buffer.h"
#include "audio_errors.h"

namespace OHOS {
namespace AudioStandard {
namespace AudioSuite {

AudioOutputNode::AudioOutputNode(AudioFormat format) : AudioNode(AudioNodeType::NODE_TYPE_OUTPUT, format)
{
    AUDIO_INFO_LOG("AudioOutputNode nodeId is %{public}u, nodeType is %{public}d",
        GetAudioNodeInfo().nodeId, GetAudioNodeInfo().nodeType);
}

AudioOutputNode::~AudioOutputNode()
{
    AUDIO_INFO_LOG("AudioOutputNode NodeId: %{public}u.", GetAudioNodeId());
}

int32_t AudioOutputNode::Connect(const std::shared_ptr<AudioNode> &preNode,
    AudioNodePortType type)
{
    if (preNode == nullptr) {
        AUDIO_ERR_LOG("AudioNode is nullptr.");
        return ERROR;
    }
    inputStream_.Connect(preNode->GetSharedInstance(), preNode->GetOutputPort(type).get());
    return SUCCESS;
}

int32_t AudioOutputNode::Connect(const std::shared_ptr<AudioNode> &preNode)
{
    return ERROR;
}

int32_t AudioOutputNode::DisConnect(const std::shared_ptr<AudioNode> &preNode)
{
    inputStream_.DisConnect(preNode);
    return SUCCESS;
}

int32_t AudioOutputNode::InstallTap(AudioNodePortType portType, std::shared_ptr<SuiteNodeReadTapDataCallback> callback)
{
    AUDIO_INFO_LOG("AudioOutputNode NodeId: %{public}u not support InstallTap opt.", GetAudioNodeId());
    return ERROR;
}

int32_t AudioOutputNode::RemoveTap(AudioNodePortType portType)
{
    AUDIO_INFO_LOG("AudioOutputNode NodeId: %{public}u not support RemoveTap opt.", GetAudioNodeId());
    return ERROR;
}


int32_t AudioOutputNode::DeInit()
{
    inputStream_.deInit();
    return SUCCESS;
}

int32_t AudioOutputNode::Flush()
{
    /* Not implemented yet */
    cacheBuffer_.clear();
    SetAudioNodeDataFinishedFlag(false);
    return SUCCESS;
}

int32_t AudioOutputNode::DoProcess()
{
    AUDIO_INFO_LOG("AudioOutputNode Get data from pre");
    std::vector<AudioSuitePcmBuffer *> &outputVec = inputStream_.ReadPreOutputData();
    if (!outputVec.empty() && outputVec[0] != nullptr) {
        AUDIO_INFO_LOG("AudioOutputNode Get data from pre frameLen:%{public}u,finished:%{public}d",
            outputVec[0]->GetFrameLen(), outputVec[0]->GetIsFinished());
        return SUCCESS;
    };
    AUDIO_INFO_LOG("AudioOutputNode Get data from pre error empty");
    return ERROR;
}

int32_t AudioOutputNode::SetCacheBuffer(std::vector<uint8_t> cacheBuffer)
{
    cacheBuffer_ = cacheBuffer;
    return 0;
}

std::vector<uint8_t> AudioOutputNode::GetCacheBuffer()
{
    return cacheBuffer_;
}

uint8_t* AudioOutputNode::GetProcessedAudioData(size_t &bytes)
{
    const std::vector<AudioSuitePcmBuffer*>& vec = inputStream_.getInputData();

    if (vec.empty()) {
        SetAudioNodeDataFinishedFlag(false);
        AUDIO_ERR_LOG("inputStream is empty");
        return nullptr;
    }

    AudioSuitePcmBuffer *buffer = vec[0];
    if (buffer == nullptr) {
        AUDIO_ERR_LOG("buffer is nullptr.");
        return nullptr;
    }

    float *floatData = buffer->GetPcmDataBuffer();
    size_t frameLen = buffer->GetFrameLen();
    bytes = frameLen * sizeof(float);
    SetAudioNodeDataFinishedFlag(buffer->GetIsFinished());
    AUDIO_INFO_LOG("PCM data frameLen=%{public}zu, bytes=%{public}zu, FinishedFlag: %{public}d",
        frameLen, bytes, GetAudioNodeDataFinishedFlag());

    if (floatData == nullptr || frameLen == 0 || bytes == 0) {
        SetAudioNodeDataFinishedFlag(false);
        return nullptr;
    }

    return reinterpret_cast<uint8_t*>(floatData);
}

int32_t AudioOutputNode::CopyDataFromCache(uint8_t *audioData,
    int32_t frameSize, int32_t &audioDataOffset, bool *finished)
{
    if (cacheBuffer_.size() > static_cast<size_t>(frameSize)) {
        memcpy_s(audioData, frameSize, cacheBuffer_.data(), frameSize);
        cacheBuffer_.erase(cacheBuffer_.begin(), cacheBuffer_.begin() + frameSize);
        audioDataOffset = frameSize;
        *finished = false;
        AUDIO_INFO_LOG("Copying %{public}d bytes from cache, remaining: %{public}zu", frameSize, cacheBuffer_.size());
        return SUCCESS;
    } else if (cacheBuffer_.size() == static_cast<size_t>(frameSize)) {
        memcpy_s(audioData, frameSize, cacheBuffer_.data(), frameSize);
        cacheBuffer_.clear();
        audioDataOffset = frameSize;
        *finished = GetAudioNodeDataFinishedFlag();
        AUDIO_INFO_LOG("Copied exact %{public}d bytes from cache,finished is %{public}d, cache cleared",
            frameSize, *finished);
        return SUCCESS;
    } else {
        size_t cachedSize = cacheBuffer_.size();
        memcpy_s(audioData, frameSize, cacheBuffer_.data(), cachedSize);
        AUDIO_INFO_LOG("Partially copied %{public}zu bytes from cache.", cacheBuffer_.size());
        audioDataOffset = static_cast<int32_t>(cachedSize);
        cacheBuffer_.clear();
        return SUCCESS;
    }
    AUDIO_ERR_LOG("cache data error.");
    return ERROR;
}

int32_t AudioOutputNode::FillRemainingAudioData(
    uint8_t *audioData, int32_t remainingBytes, int32_t *writeDataSize, bool *finished, int32_t frameSize)
{
    if (GetAudioNodeDataFinishedFlag() == true) {
        *finished = true;
        AUDIO_INFO_LOG("All cacheBuffer copy %{public}d bytes to audioData.", *writeDataSize);
        return SUCCESS;
    }
    size_t totalWritten = 0;
    size_t bytesSize = 0;
    while (totalWritten < static_cast<size_t>(remainingBytes)) {
        if (DoProcess() == ERROR) {
            AUDIO_ERR_LOG("Failed to process audio data");
            return ERROR;
        }
        size_t bytes = 0;
        uint8_t* byteData = GetProcessedAudioData(bytes);
        if (bytes == 0 || byteData == nullptr) {
            AUDIO_ERR_LOG("Null or empty processedData");
            return ERROR;
        }

        size_t copySize = std::min(static_cast<size_t>(remainingBytes - totalWritten), bytes);
        memcpy_s(audioData + totalWritten, remainingBytes - totalWritten, byteData, copySize);
        *writeDataSize += copySize;
        totalWritten += copySize;
        bytesSize += bytes;

        if (totalWritten == static_cast<size_t>(remainingBytes)) {
            cacheBuffer_.clear();
            if (bytesSize > static_cast<size_t>(remainingBytes)) {
                cacheBuffer_.assign(byteData + copySize, byteData + bytes);
                *finished = false;
                AUDIO_INFO_LOG("Copying %{public}d bytes to audioData success,"
                    " remain cacheBuffer size: %{public}zu", frameSize, cacheBuffer_.size());
                return SUCCESS;
            }
            if (bytesSize <= static_cast<size_t>(remainingBytes)) {
                *finished = GetAudioNodeDataFinishedFlag();
                AUDIO_INFO_LOG("Copying %{public}d remainingBytes to audioData success, cacheBuffer clear,"
                    " finished: %{public}d", frameSize, *finished);
                return SUCCESS;
            }
        }

        if (GetAudioNodeDataFinishedFlag()) {
            break;
        }
    }
    *finished = GetAudioNodeDataFinishedFlag();
    AUDIO_INFO_LOG("Copydata finished is %{public}d with %{public}d bytes written", *finished, *writeDataSize);
    return SUCCESS;
}

int32_t AudioOutputNode::DoProcess(uint8_t *audioData, int32_t frameSize, int32_t *writeDataSize, bool *finished)
{
    if (finished == nullptr || audioData == nullptr || writeDataSize == nullptr) {
        AUDIO_ERR_LOG("Parameter is nullptr.");
        return ERROR;
    }
    *writeDataSize = 0;
    *finished = false;
    if (GetAudioNodeDataFinishedFlag() && cacheBuffer_.empty()) {
        AUDIO_INFO_LOG("AudioOutputNode finished completed.");
        *finished = true;
        return ERR_NOT_SUPPORTED;
    }

    int32_t audioDataOffset = 0;
    if (!cacheBuffer_.empty()) {
        if (CopyDataFromCache(audioData, frameSize, audioDataOffset, finished) != SUCCESS) {
            return ERROR;
        }
        *writeDataSize = audioDataOffset;
        if (audioDataOffset == frameSize) {
            AUDIO_INFO_LOG("Copying finished.");
            return SUCCESS;
        }
    }

    int32_t remainingBytes = frameSize - audioDataOffset;
    audioData = audioData + audioDataOffset;
    if (remainingBytes > 0) {
        AUDIO_INFO_LOG("RemainFrameSize: %{public}d, audioDataOffset: %{public}d", remainingBytes, audioDataOffset);
        return FillRemainingAudioData(
            audioData,
            remainingBytes,
            writeDataSize,
            finished,
            frameSize);
    }
    AUDIO_ERR_LOG("write error");
    return ERROR;
}

int32_t AudioOutputNode::DoProcess(uint8_t **audioDataArray, int arraySize,
    int32_t requestFrameSize, int32_t *responseSize, bool *finishedFlag)
{
    return ERROR;
}

}  // namespace AudioSuite
}  // namespace AudioStandard
}  // namespace OHOS