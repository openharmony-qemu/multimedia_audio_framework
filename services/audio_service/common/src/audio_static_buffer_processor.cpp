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
#define LOG_TAG "audioStaticBufferProcessor"
#endif

#include "audio_static_buffer_processor.h"

#include "audio_errors.h"
#include "audio_log_utils.h"
#include "volume_tools.h"

namespace OHOS {
namespace AudioStandard {

std::shared_ptr<AudioStaticBufferProcessor> AudioStaticBufferProcessor::CreateInstance(AudioStreamInfo streamInfo,
    std::shared_ptr<OHAudioBufferBase> sharedBuffer)
{
    CHECK_AND_RETURN_RET_LOG(sharedBuffer != nullptr, nullptr, "sharedBuffer is nullptr");
    return std::make_shared<AudioStaticBufferProcessor>(streamInfo, sharedBuffer);
}

int32_t AudioStaticBufferProcessor::ProcessFadeInOut(int8_t *bufferBase, size_t bufferSize,
    AudioStreamInfo streamInfo, bool isFadeOut)
{
    ChannelVolumes mapVols = isFadeOut ? VolumeTools::GetChannelVolumes(streamInfo.channels, 1.0f, 0.0f) :
        VolumeTools::GetChannelVolumes(streamInfo.channels, 0.0f, 1.0f);
    BufferDesc fadeBufferDesc{};
    fadeBufferDesc.buffer = reinterpret_cast<uint8_t *>(bufferBase);
    fadeBufferDesc.bufLength = bufferSize;
    fadeBufferDesc.dataLength = bufferSize;
    int32_t ret = VolumeTools::Process(fadeBufferDesc, streamInfo.format, mapVols);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_OPERATION_FAILED, "VolumeTools::Process failed: %{public}d", ret);
    return SUCCESS;
}

AudioStaticBufferProcessor::AudioStaticBufferProcessor(AudioStreamInfo streamInfo,
    std::shared_ptr<OHAudioBufferBase> sharedBuffer)
{
    sharedBuffer_ = sharedBuffer;
    streamInfo_ = streamInfo;
    audioSpeed_ = std::make_unique<AudioSpeed>(streamInfo.samplingRate, streamInfo.format,
        streamInfo.channels, static_cast<int32_t>(sharedBuffer->GetDataSize()));
}

AudioStaticBufferProcessor::~AudioStaticBufferProcessor()
{
    if (pitchProcessor_ != nullptr) {
        pitchProcessor_->PitchAlgoRelease();
        pitchProcessor_ = nullptr;
    }
}

int32_t AudioStaticBufferProcessor::ProcessBufferSpeed(float speed)
{
    if (isEqual(speed, SPEED_NORMAL)) {
        return SUCCESS;
    }
    
    uint8_t *inputBuffer = sharedBuffer_->GetDataBase();
    int32_t inputSize = static_cast<int32_t>(sharedBuffer_->GetDataSize());
    
    size_t estimatedSize = static_cast<size_t>(inputSize) * MAX_SPEED_BUFFER_FACTOR;
    auto newTempBuffer = std::make_unique<uint8_t[]>(estimatedSize);
    
    audioSpeed_->SetSpeed(speed);
    audioSpeed_->SetPitch(PITCH_DEFAULT);
    
    int32_t outBufferSize = 0;
    if (audioSpeed_->ChangeSpeedFunc(inputBuffer, inputSize,
        newTempBuffer, outBufferSize) == 0) {
        AUDIO_ERR_LOG("process speed error");
        return ERR_OPERATION_FAILED;
    }
    CHECK_AND_RETURN_RET_LOG(outBufferSize != 0, ERR_OPERATION_FAILED, "speed bufferSize is 0");
    
    tempBuffer_ = std::move(newTempBuffer);
    tempBufferSize_ = static_cast<size_t>(outBufferSize);
    return SUCCESS;
}

int32_t AudioStaticBufferProcessor::ProcessBufferPitch(float pitch)
{
    if (isEqual(pitch, PITCH_NORMAL)) {
        return SUCCESS;
    }
    
    CHECK_AND_RETURN_RET_LOG(pitch >= MIN_STREAM_PITCH_LEVEL && pitch <= MAX_STREAM_PITCH_LEVEL,
        ERR_INVALID_PARAM, "Invalid pitch: %{public}f", pitch);
    
    if (pitchProcessor_ == nullptr) {
        pitchProcessor_ = AudioPitchProcessor::CreateInstance(streamInfo_);
        CHECK_AND_RETURN_RET_LOG(pitchProcessor_ != nullptr, ERR_OPERATION_FAILED,
            "Create pitch processor failed");
        
        int32_t ret = pitchProcessor_->PitchAlgoInit();
        if (ret != SUCCESS) {
            AUDIO_INFO_LOG("PitchAlgoInit failed");
            pitchProcessor_ = nullptr;
            return ERR_OPERATION_FAILED;
        }
    }
    
    int32_t ret = pitchProcessor_->SetPitch(pitch);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_OPERATION_FAILED, "SetPitch failed");
    
    int8_t *inputBuffer = nullptr;
    size_t inputSize = 0;
    
    if (tempBuffer_ != nullptr && tempBufferSize_ != 0) {
        inputBuffer = reinterpret_cast<int8_t*>(tempBuffer_.get());
        inputSize = tempBufferSize_;
    } else {
        inputBuffer = reinterpret_cast<int8_t*>(sharedBuffer_->GetDataBase());
        inputSize = sharedBuffer_->GetDataSize();
    }
    
    uint32_t totalSamples = static_cast<uint32_t>(inputSize / PcmFormatToBits(streamInfo_.format));
    uint32_t samplesPerChannel = totalSamples / streamInfo_.channels;
    uint32_t maxProcessedSamples = samplesPerChannel * MAX_PITCH_BUFFER_FACTOR;
    size_t estimatedOutputSize = maxProcessedSamples * streamInfo_.channels * PcmFormatToBits(streamInfo_.format);
    
    auto newTempBuffer = std::make_unique<uint8_t[]>(estimatedOutputSize);
    
    size_t outputSize = 0;
    ret = pitchProcessor_->ProcessBufferPitch(inputBuffer, inputSize,
        reinterpret_cast<int8_t*>(newTempBuffer.get()), outputSize);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_OPERATION_FAILED, "ProcessBufferPitch failed");
    
    tempBuffer_ = std::move(newTempBuffer);
    tempBufferSize_ = outputSize;
    
    AUDIO_INFO_LOG("ProcessBufferPitch done: pitch=%{public}f, outputSize=%{public}zu", pitch, outputSize);
    return SUCCESS;
}

int32_t AudioStaticBufferProcessor::GetProcessedBuffer(uint8_t **bufferBase, size_t &bufferSize)
{
    if (tempBuffer_ != nullptr && tempBufferSize_ != 0) {
        *bufferBase = tempBuffer_.get();
        bufferSize = tempBufferSize_;
    } else if (processBuffer_ != nullptr && processedBufferSize_ != 0) {
        *bufferBase = processBuffer_.get();
        bufferSize = processedBufferSize_;
    } else {
        CHECK_AND_RETURN_RET_LOG(sharedBuffer_ != nullptr, ERR_NULL_POINTER, "sharedBuffer is nullptr!");
        *bufferBase = sharedBuffer_->GetDataBase();
        bufferSize = sharedBuffer_->GetDataSize();
    }
    return SUCCESS;
}

void AudioStaticBufferProcessor::SaveProcessBuffer()
{
    processBuffer_ = std::move(tempBuffer_);
    processedBufferSize_ = tempBufferSize_;
    tempBuffer_ = nullptr;
    tempBufferSize_ = 0;
}

} // namespace AudioStandard
} // namespace OHOS