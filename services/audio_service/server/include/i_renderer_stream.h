/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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

#ifndef I_RENDERER_STREAM_H
#define I_RENDERER_STREAM_H

#include "i_stream.h"
#include "audio_stream_info.h"

namespace OHOS {
namespace AudioStandard {
class IWriteCallback {
public:
    virtual int32_t OnWriteData(size_t length) = 0;
    virtual int32_t OnWriteData(int8_t *inputData, size_t requestDataLen) = 0;
    virtual int32_t GetAvailableSize(size_t &length) = 0;
};

class IStreamCallback {
public:
    virtual int32_t OnStreamData(AudioCallBackStreamInfo& callBackStremInfo) = 0;
};

class IRendererStream : public IStream {
public:
    virtual ~IRendererStream() = default;
    virtual int32_t GetStreamFramesWritten(uint64_t &framesWritten) = 0;
    virtual int32_t GetCurrentTimeStamp(uint64_t &timestamp) = 0;
    virtual int32_t GetCurrentPosition(uint64_t &framePosition, uint64_t &timestamp, uint64_t &latency, int32_t base);
    virtual int32_t GetSpeedPosition(uint64_t &framePosition, uint64_t &timestamp, uint64_t &latency, int32_t base)
    {
        return GetCurrentPosition(framePosition, timestamp, latency, base);
    }
    virtual int32_t GetLatency(uint64_t &latency) = 0;
    virtual int32_t SetRate(int32_t rate) = 0;
    virtual int32_t SetAudioEffectMode(int32_t effectMode) = 0;
    virtual int32_t GetAudioEffectMode(int32_t &effectMode) = 0;
    virtual int32_t SetPrivacyType(int32_t privacyType) = 0;
    virtual int32_t GetPrivacyType(int32_t &privacyType) = 0;

    virtual void RegisterWriteCallback(const std::weak_ptr<IWriteCallback> &callback) = 0;
    virtual int32_t GetMinimumBufferSize(size_t &minBufferSize) const = 0;
    virtual void GetByteSizePerFrame(size_t &byteSizePerFrame) const = 0;
    virtual void GetSpanSizePerFrame(size_t &spanSizeInFrame) const = 0;

    virtual int32_t SetOffloadMode(int32_t state, bool isAppBack) = 0;
    virtual int32_t UnsetOffloadMode() = 0;
    virtual int32_t GetOffloadApproximatelyCacheTime(uint64_t &timestamp, uint64_t &paWriteIndex,
        uint64_t &cacheTimeDsp, uint64_t &cacheTimePa) = 0;
    virtual int32_t OffloadSetVolume(float volume) = 0;
    virtual int32_t SetOffloadDataCallbackState(int32_t state) = 0;
    virtual size_t GetWritableSize() = 0;
    virtual int32_t UpdateSpatializationState(bool spatializationEnabled, bool headTrackingEnabled) = 0;
    virtual int32_t UpdateMaxLength(uint32_t maxLength) = 0;

    virtual int32_t Peek(std::vector<char> *audioBuffer, int32_t &index) = 0;
    virtual int32_t ReturnIndex(int32_t index) = 0;
    virtual AudioProcessConfig GetAudioProcessConfig() const noexcept = 0;
    virtual int32_t SetClientVolume(float clientVolume) = 0;
    virtual int32_t SetSpeed(float speed) = 0;
    virtual int32_t SetLoudnessGain(float loudnessGain) = 0;
    virtual void BlockStream() noexcept = 0;
};

struct CaptureInfo {
    std::atomic<bool> isInnerCapEnabled = false;
    std::shared_ptr<IRendererStream> dupStream = nullptr;
};
} // namespace AudioStandard
} // namespace OHOS
#endif // I_RENDERER_STREAM_H
