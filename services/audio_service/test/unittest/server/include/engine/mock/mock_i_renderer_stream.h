/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#ifndef MOCK_I_RENDERER_STREAM_H
#define MOCK_I_RENDERER_STREAM_H

#include <gmock/gmock.h>
#include "i_renderer_stream.h"

namespace OHOS {
namespace AudioStandard {

class MockIRendererStream : public IRendererStream {
public:
    // IStream methods
    MOCK_METHOD(void, SetStreamIndex, (uint32_t index), (override));
    MOCK_METHOD(uint32_t, GetStreamIndex, (), (override));
    MOCK_METHOD(int32_t, Start, (), (override));
    MOCK_METHOD(int32_t, Pause, (bool isStandby), (override));
    MOCK_METHOD(int32_t, Flush, (), (override));
    MOCK_METHOD(int32_t, Drain, (bool stopFlag), (override));
    MOCK_METHOD(int32_t, Stop, (), (override));
    MOCK_METHOD(int32_t, Release, (), (override));
    MOCK_METHOD(void, RegisterStatusCallback, (const std::weak_ptr<IStatusCallback>& callback), (override));
    MOCK_METHOD(BufferDesc, DequeueBuffer, (size_t length), (override));
    MOCK_METHOD(int32_t, EnqueueBuffer, (const BufferDesc& bufferDesc), (override));

    // IRendererStream methods
    MOCK_METHOD(int32_t, GetStreamFramesWritten, (uint64_t& framesWritten), (override));
    MOCK_METHOD(int32_t, GetCurrentTimeStamp, (uint64_t& timestamp), (override));
    MOCK_METHOD(int32_t, GetCurrentPosition,
                    (uint64_t& framePosition, uint64_t& timestamp, uint64_t& latency, int32_t base), (override));
    MOCK_METHOD(int32_t, GetSpeedPosition,
                    (uint64_t& framePosition, uint64_t& timestamp, uint64_t& latency, int32_t base), (override));
    MOCK_METHOD(int32_t, GetLatency, (uint64_t& latency), (override));
    MOCK_METHOD(int32_t, SetRate, (int32_t rate), (override));
    MOCK_METHOD(int32_t, SetAudioEffectMode, (int32_t effectMode), (override));
    MOCK_METHOD(int32_t, GetAudioEffectMode, (int32_t& effectMode), (override));
    MOCK_METHOD(int32_t, SetPrivacyType, (int32_t privacyType), (override));
    MOCK_METHOD(int32_t, GetPrivacyType, (int32_t& privacyType), (override));
    MOCK_METHOD(void, RegisterWriteCallback, (const std::weak_ptr<IWriteCallback>& callback), (override));
    MOCK_METHOD(int32_t, GetMinimumBufferSize, (size_t& minBufferSize), (const, override));
    MOCK_METHOD(void, GetByteSizePerFrame, (size_t& byteSizePerFrame), (const, override));
    MOCK_METHOD(void, GetSpanSizePerFrame, (size_t& spanSizeInFrame), (const, override));
    MOCK_METHOD(void, AbortCallback, (int32_t abortTimes), (override));
    MOCK_METHOD(int32_t, SetOffloadMode, (int32_t state, bool isAppBack), (override));
    MOCK_METHOD(int32_t, UnsetOffloadMode, (), (override));
    MOCK_METHOD(int32_t, GetOffloadApproximatelyCacheTime,
        (uint64_t& timestamp, uint64_t& paWriteIndex, uint64_t& cacheTimeDsp, uint64_t& cacheTimePa), (override));
    MOCK_METHOD(int32_t, OffloadSetVolume, (), (override));
    MOCK_METHOD(int32_t, SetOffloadDataCallbackState, (int32_t state), (override));
    MOCK_METHOD(size_t, GetWritableSize, (), (override));
    MOCK_METHOD(int32_t, UpdateSpatializationState, (bool spatializationEnabled, bool headTrackingEnabled), (override));
    MOCK_METHOD(int32_t, UpdateMaxLength, (uint32_t maxLength), (override));
    MOCK_METHOD(int32_t, Peek, (std::vector<char>* audioBuffer, int32_t& index), (override));
    MOCK_METHOD(int32_t, ReturnIndex, (int32_t index), (override));
    MOCK_METHOD(AudioProcessConfig, GetAudioProcessConfig, (), (const, noexcept, override));
    MOCK_METHOD(int32_t, SetClientVolume, (float clientVolume), (override));
    MOCK_METHOD(int32_t, SetSpeed, (float speed), (override));
    MOCK_METHOD(int32_t, SetLoudnessGain, (float loudnessGain), (override));
    MOCK_METHOD(void, BlockStream, (), (noexcept, override));
    MOCK_METHOD(void, SetSendDataEnabled, (bool enabled), (override));
    MOCK_METHOD(int32_t, GetLatencyWithFlag, (uint64_t& latency, LatencyFlag flag), (override));
};

} // namespace AudioStandard
} // namespace OHOS

#endif // MOCK_I_RENDERER_STREAM_H
