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

int32_t AudioStaticBufferProcessor::ProcessBuffer()
{
    float speed = ConvertAudioRenderRateToSpeed(sharedBuffer_->GetStaticRenderRate());

    if (isEqual(speed, SPEED_NORMAL)) {
        curSpeed_ = speed;
        sharedBuffer_->SetProcessedBuffer(sharedBuffer_->GetDataBase(), sharedBuffer_->GetDataSize());
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
    AUDIO_INFO_LOG("WJJ outBufferSize %{public}d", outBufferSize);

    speedBufferSize_ = static_cast<size_t>(outBufferSize);
    curSpeed_ = speed;
    sharedBuffer_->SetProcessedBuffer(speedBuffer_.get(), speedBufferSize_);
    return SUCCESS;
}
} // namespace AudioStandard
} // namespace OHOS