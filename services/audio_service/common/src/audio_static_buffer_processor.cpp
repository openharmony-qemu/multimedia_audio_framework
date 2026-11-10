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

#include <cmath>
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

AudioStaticBufferProcessor::~AudioStaticBufferProcessor() {}

int32_t AudioStaticBufferProcessor::ProcessBufferSpeedAndPitch(float speed, float pitch)
{
    if (isEqual(speed, SPEED_NORMAL) && isEqual(pitch, PITCH_NORMAL)) {
        return SUCCESS;
    }

    uint8_t *inputBuffer = sharedBuffer_->GetDataBase();
    int32_t inputSize = static_cast<int32_t>(sharedBuffer_->GetDataSize());

    int32_t speedBufferFactor = 1;
    if (speed > 0.0f && speed < SPEED_NORMAL) {
        speedBufferFactor = static_cast<int32_t>(std::ceil(1.0f / speed)) + 1;
    }
    size_t estimatedSize = static_cast<size_t>(inputSize) * static_cast<size_t>(speedBufferFactor);
    size_t flushReserve = estimatedSize;
    auto outBuffer = std::make_unique<uint8_t[]>(estimatedSize + flushReserve);

    audioSpeed_->SetMaxSpeedBufferSize(static_cast<int32_t>(estimatedSize));
    audioSpeed_->SetSpeed(speed);
    audioSpeed_->SetPitch(pitch);

    int32_t outBufferSize = 0;
    if (audioSpeed_->ChangeSpeedFunc(inputBuffer, inputSize,
        outBuffer, outBufferSize) == 0) {
        AUDIO_ERR_LOG("process speed and pitch error");
        return ERR_OPERATION_FAILED;
    }
    CHECK_AND_RETURN_RET_LOG(outBufferSize != 0, ERR_OPERATION_FAILED, "speed and pitch bufferSize is 0");

    int32_t flushSize = 0;
    audioSpeed_->FlushSpeedBuffer(outBuffer.get() + outBufferSize,
        static_cast<int32_t>(flushReserve), flushSize);

    tempBuffer_ = std::move(outBuffer);
    tempBufferSize_ = static_cast<size_t>(outBufferSize) + static_cast<size_t>(flushSize);
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