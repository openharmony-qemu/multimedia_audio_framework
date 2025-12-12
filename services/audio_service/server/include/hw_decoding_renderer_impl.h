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
#ifndef HW_DECODING_RENDERER_STREAM_IMPL_H
#define HW_DECODING_RENDERER_STREAM_IMPL_H

#include <atomic>
#include <mutex>
#include "i_renderer_stream.h"
#include "sink/i_audio_render_sink.h"

namespace OHOS {
namespace AudioStandard {
class HWDecodingRendererStream : public IRendererStream {
public:
    HWDecodingRendererStream(AudioProcessConfig &processConfig);
    ~HWDecodingRendererStream();
    int32_t Init();
    int32_t Start() override;
    int32_t Pause(bool isStandby = false) override;
    int32_t Flush() override;
    int32_t Drain(bool stopFlag = false) override;
    int32_t Stop() override;
    int32_t Release() override;
    int32_t GetStreamFramesWritten(uint64_t &framesWritten) override;
    int32_t GetCurrentTimeStamp(uint64_t &timestamp) override;
    int32_t GetCurrentPosition(uint64_t &framePosition, uint64_t &timestamp, uint64_t &latency, int32_t base) override;
    int32_t GetLatency(uint64_t &latency) override;
    int32_t SetRate(int32_t rate) override;
    int32_t SetAudioEffectMode(int32_t effectMode) override;
    int32_t GetAudioEffectMode(int32_t &effectMode) override;
    int32_t SetPrivacyType(int32_t privacyType) override;
    int32_t GetPrivacyType(int32_t &privacyType) override;
    int32_t SetSpeed(float speed) override;

    void RegisterStatusCallback(const std::weak_ptr<IStatusCallback> &callback) override;
    void RegisterWriteCallback(const std::weak_ptr<IWriteCallback> &callback) override;
    BufferDesc DequeueBuffer(size_t length) override;
    int32_t EnqueueBuffer(const BufferDesc &bufferDesc) override;
    int32_t GetMinimumBufferSize(size_t &minBufferSize) const override;
    void GetByteSizePerFrame(size_t &byteSizePerFrame) const override;
    void GetSpanSizePerFrame(size_t &spanSizeInFrame) const override;
    void SetStreamIndex(uint32_t index) override;
    uint32_t GetStreamIndex() override;
    // offload
    int32_t SetOffloadMode(int32_t state, bool isAppBack) override;
    int32_t UnsetOffloadMode() override;
    int32_t GetOffloadApproximatelyCacheTime(uint64_t &timestamp, uint64_t &paWriteIndex, uint64_t &cacheTimeDsp,
                                             uint64_t &cacheTimePa) override;
    int32_t OffloadSetVolume() override;
    int32_t SetOffloadDataCallbackState(int32_t state) override;
    size_t GetWritableSize() override;
    // offload end

    int32_t UpdateSpatializationState(bool spatializationEnabled, bool headTrackingEnabled) override;
    int32_t UpdateMaxLength(uint32_t maxLength) override;

    AudioProcessConfig GetAudioProcessConfig() const noexcept override;
    int32_t Peek(std::vector<char> *audioBuffer, int32_t &index) override;
    int32_t ReturnIndex(int32_t index) override;
    int32_t SetClientVolume(float clientVolume) override;
    int32_t SetLoudnessGain(float loudnessGain) override;
    void BlockStream() noexcept override;

    int32_t GetLatencyWithFlag(uint64_t &latency, LatencyFlag flag) override;
    int32_t RegisterSinkLatencyFetcher(const std::function<int32_t (uint32_t &)> &fetcher) override;
private:
    int32_t InitSink(AudioStreamInfo streamInfo);
    int32_t InitBuffer();
    void NotifyOperation(IOperation operation);
private:
    // sink values
    uint32_t renderId_ = 0;
    std::mutex sinkMutex_;
    std::shared_ptr<IAudioRenderSink> sink_ = nullptr;

    // stream values
    AudioProcessConfig processConfig_;
    uint32_t streamIndex_ = 0;
    std::weak_ptr<IStatusCallback> statusCallback_;
    std::weak_ptr<IWriteCallback> writeCallback_;
    int32_t privacyType_ = 0;
    uint64_t writtenFrameCount_ = 0;
    std::mutex sinkLatencyFetcherMutex_;
    std::function<int32_t (uint32_t &)> sinkLatencyFetcher_;

    // buffer values
    std::unique_ptr<uint8_t []> rawBuffer_ = nullptr;
    size_t bufferSize_ = 0;
};
} // namespace AudioStandard
} // namespace OHOS
#endif // HW_DECODING_RENDERER_STREAM_IMPL_H
