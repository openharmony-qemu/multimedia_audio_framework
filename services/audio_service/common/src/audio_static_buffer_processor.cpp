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

std::shared_ptr<AudioStaticBufferProcessor> AudioStaticBufferProcessor::CreateInstance(AudioStreamInfo streamInfo)
{
    return std::make_shared<AudioStaticBufferProcessor>(streamInfo);
}

AudioStaticBufferProcessor::AudioStaticBufferProcessor(AudioStreamInfo streamInfo)
{
    if (audioSpeed_ == nullptr) {
        audioSpeed_ = std::make_unique<AudioSpeed>(streamInfo.samplingRate, streamInfo.format, streamInfo.channels);
    }
}

int32_t AudioStaticBufferProcessor::ProcessBuffer(std::shared_ptr<OHAudioBufferBase> sharedBuffer)
{
    float speed = ConvertAudioRenderRateToSpeed(sharedBuffer->GetStaticRenderRate());
    CHECK_AND_RETURN_RET(!isEqual(speed, curSpeed_), SUCCESS);
    if (isEqual(speed, SPEED_NORMAL)) {
        speedBuffer_ = sharedBuffer->GetDataBase();
        speedBufferSize_ = sharedBuffer->GetDataSize();
        curSpeed_ = speed;
        sharedBuffer->SetProcessedBuffer(speedBuffer_, speedBufferSize_);
        return SUCCESS;
    }

    if (speedBuffer_ != nullptr) {
        speedBuffer_ = std::make_unique<uint8_t[]>(sharedBuffer->GetDataSize() * MAX_SPEED_BUFFER_FACTOR);
    }

    int32_t outBufferSize = 0;
    if (audioSpeed_->ChangeSpeedFunc(sharedBuffer->GetDataBase(), sharedBuffer->GetDataSize(),
        speedBuffer_, outBufferSize) == 0) {
        AUDIO_ERR_LOG("process speed error");
        return ERR_OPERATION_FAILED;
    }
    CHECK_AND_RETURN_RET_LOG(outBufferSize != 0, ERR_OPERATION_FAILED, "speed bufferSize is 0");
    AUDIO_INFO_LOG("WJJ outBufferSize %{public}zu", outBufferSize);

    speedBuffer_ = speedBuffer.get();
    speedBufferSize_ = static_cast<size_t>(outBufferSize);
    curSpeed_ = speed;
    sharedBuffer->SetProcessedBuffer(speedBuffer_, speedBufferSize_);
    return SUCCESS;
}
} // namespace AudioStandard
} // namespace OHOS