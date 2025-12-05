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

namespace OHOS {
namespace AudioStandard {

std::shared_ptr<AudioStaticBufferProcessor> AudioStaticBufferProcessor::CreateInstance(AudioStreamInfo streamInfo,
    std::shared_ptr<OHAudioBufferBase> sharedBuffer)
{
    CHECK_AND_RETURN_RET_LOG(sharedBuffer != nullptr, nullptr, "sharedBuffer is nullptr");
    return std::make_shared<AudioStaticBufferProcessor>(streamInfo, sharedBuffer);
}

AudioStaticBufferProcessor::AudioStaticBufferProcessor(AudioStreamInfo streamInfo,
    std::shared_ptr<OHAudioBufferBase> sharedBuffer)
{
    sharedBuffer_ = sharedBuffer;
    if (audioSpeed_ == nullptr) {
        audioSpeed_ = std::make_unique<AudioSpeed>(streamInfo.samplingRate, streamInfo.format,
            streamInfo.channels, static_cast<int32_t>(sharedBuffer->GetDataSize()));
    }
}

int32_t AudioStaticBufferProcessor::ProcessBuffer(AudioRendererRate renderRate)
{
    float speed = ConvertAudioRenderRateToSpeed(renderRate);
    if (isEqual(speed, SPEED_NORMAL)) {
        curSpeed_ = speed;
        return SUCCESS;
    }

    if (speed == curSpeed_) {
        return SUCCESS;
    }
    speedBuffer_ = std::make_unique<uint8_t[]>(sharedBuffer_->GetDataSize() * MAX_SPEED_BUFFER_FACTOR);
    audioSpeed_->SetSpeed(speed);
    audioSpeed_->SetPitch(speed);

    int32_t outBufferSize = 0;
    if (audioSpeed_->ChangeSpeedFunc(sharedBuffer_->GetDataBase(), sharedBuffer_->GetDataSize(),
        speedBuffer_, outBufferSize) == 0) {
        AUDIO_ERR_LOG("process speed error");
        return ERR_OPERATION_FAILED;
    }
    CHECK_AND_RETURN_RET_LOG(outBufferSize != 0, ERR_OPERATION_FAILED, "speed bufferSize is 0");

    speedBufferSize_ = static_cast<size_t>(outBufferSize);
    curSpeed_ = speed;
    return SUCCESS;
}

int32_t AudioStaticBufferProcessor::GetProcessedBuffer(uint8_t **bufferBase, size_t &bufferSize)
{
    if (speedBuffer_ != nullptr && speedBufferSize_ != 0) {
        AUDIO_INFO_LOG("Use %{public}f speed processed buffer!", curSpeed_);
        *bufferBase = speedBuffer_.get();
        bufferSize = speedBufferSize_;
    } else {
        AUDIO_INFO_LOG("Use original buffer!");
        CHECK_AND_RETURN_RET_LOG(sharedBuffer_ != nullptr, ERR_NULL_POINTER, "sharedBuffer is nullptr!");
        *bufferBase = sharedBuffer_->GetDataBase();
        bufferSize = sharedBuffer_->GetDataSize();
    }
    return SUCCESS;
}

} // namespace AudioStandard
} // namespace OHOS