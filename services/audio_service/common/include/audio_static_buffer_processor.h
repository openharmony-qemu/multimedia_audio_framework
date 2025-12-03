/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef OH_AUDIO_STATIC_BUFFER_PROCESSOR_H
#define OH_AUDIO_STATIC_BUFFER_PROCESSOR_H

#include "audio_utils.h"
#include "oh_audio_buffer.h"
#include "audio_service_log.h"
#include "audio_speed.h"

namespace OHOS {
namespace AudioStandard {

static const int32_t MAX_SPEED_BUFFER_FACTOR = 3;

class AudioStaticBufferProcessor
{
public:
    static std::shared_ptr<AudioStaticBufferProcessor> CreateInstance(AudioStreamInfo streamInfo,
        std::shared_ptr<OHAudioBufferBase> sharedBuffer);
    AudioStaticBufferProcessor(AudioStreamInfo streamInfo, std::shared_ptr<OHAudioBufferBase> sharedBuffer);

    int32_t ProcessBuffer(AudioRendererRate renderRate);
    int32_t GetProcessedBuffer(uint8_t *bufferBase, size_t &bufferSize);

private:
    std::unique_ptr<AudioSpeed> audioSpeed_ = nullptr;
    std::unique_ptr<uint8_t[]> speedBuffer_ = nullptr;
    std::shared_ptr<OHAudioBufferBase> sharedBuffer_ = nullptr;
    size_t speedBufferSize_ = 0;
    float curSpeed_ = 1.0f;
};

} // namespace AudioStandard
} // namespace OHOS
#endif // OH_AUDIO_STATIC_BUFFER_PROCESSOR_H