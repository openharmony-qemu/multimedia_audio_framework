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

#ifndef OH_AUDIO_STATIC_BUFFER_PROVIDER_H
#define OH_AUDIO_STATIC_BUFFER_PROVIDER_H

#include "audio_utils.h"
#include "oh_audio_buffer.h"
#include "audio_service_log.h"

namespace OHOS {
namespace AudioStandard {

class AudioStaticBufferProvider {
public:
    static std::shared_ptr<AudioStaticBufferProvider> CreateInstance(AudioStreamInfo streamInfo,
        std::shared_ptr<OHAudioBufferBase> sharedBuffer);

    AudioStaticBufferProvider(AudioStreamInfo streamInfo, std::shared_ptr<OHAudioBufferBase> sharedBuffer);
    int32_t GetDataFromStaticBuffer(int8_t *inputData, size_t requestDataLen);
    void SetStaticBufferInfo(const StaticBufferInfo &staticBufferInfo);
    int32_t GetStaticBufferInfo(StaticBufferInfo &staticBufferInfo);
    void SetProcessedBuffer(uint8_t **bufferBase, size_t bufferSize);

    void SetLoopTimes(int64_t times);
    void NeedProcessFadeIn();
    void NeedProcessFadeOut();
    bool IsLoopEnd();
    void ResetStaticPlayPosition();

private:
    int32_t IncreaseCurrentLoopTimes();
    void RefreshBufferStatus();
    int32_t CheckIsValid(int8_t *inputData, size_t offset, size_t requestDataLen, size_t remainSize);
    bool NeedProvideData();
    int32_t ProcessFadeInOutIfNeed(int8_t *inputData, size_t requestDataLen);

private:
    std::shared_ptr<OHAudioBufferBase> sharedBuffer_ = nullptr;
    AudioStreamInfo streamInfo_;
    uint8_t *processedBuffer_ = nullptr;
    size_t processedBufferSize_ = 0;
    int64_t totalLoopTimes_ = 0;
    int64_t currentLoopTimes_ = 0;
    size_t curStaticDataPos_ = 0;
    bool delayRefreshBufferStatus_ = false;
    bool playFinished_ = false;
    bool needFadeIn_ = false;
    bool needFadeOut_ = false;

    std::mutex eventMutex_;
};

} // namespace AudioStandard
} // namespace OHOS
#endif // OH_AUDIO_STATIC_BUFFER_PROVIDER_H