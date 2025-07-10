/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#ifndef PRO_RENDERER_STREAM_IMPL_H
#define PRO_RENDERER_STREAM_IMPL_H

#include <atomic>
#include <queue>
#include <mutex>
#include <condition_variable>
#include "i_renderer_stream.h"
#include "audio_resample.h"
#include "linear_pos_time_model.h"
#include "audio_down_mix_stereo.h"
#include "audio_common_converter.h"

namespace OHOS {
namespace AudioStandard {
class ProRendererStreamImpl : public IRendererStream {
public:
    ProRendererStreamImpl(AudioProcessConfig processConfig, bool isDirect);
    ~ProRendererStreamImpl();
    int32_t InitParams();
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
    int32_t OffloadSetVolume(float volume) override;
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

private:
    bool GetAudioTime(uint64_t &framePos, int64_t &sec, int64_t &nanoSec);
    AudioSamplingRate GetDirectSampleRate(AudioSamplingRate sampleRate) const noexcept;
    AudioSampleFormat GetDirectFormat(AudioSampleFormat format) const noexcept;
    void ConvertSrcToFloat(const BufferDesc &bufferDesc);
    void ConvertFloatToDes(int32_t writeIndex);
    void GetStreamVolume();
    void PopSinkBuffer(std::vector<char> *audioBuffer, int32_t &index);
    int32_t PopWriteBufferIndex();
    void SetOffloadDisable();
    void InitBasicInfo(const AudioStreamInfo &streamInfo);

private:
    bool isDirect_;
    bool isNeedResample_;
    bool isNeedMcr_;
    bool isBlock_;
    bool isDrain_;
    bool isFirstFrame_;
    int32_t privacyType_;
    int32_t renderRate_;
    uint32_t streamIndex_; // invalid index
    uint32_t currentRate_;
    uint32_t desSamplingRate_;
    AudioSampleFormat desFormat_;
    size_t byteSizePerFrame_;
    size_t spanSizeInFrame_;
    size_t totalBytesWritten_;
    size_t sinkBytesWritten_;
    size_t minBufferSize_;
    std::atomic<IStatus> status_;
    std::weak_ptr<IStatusCallback> statusCallback_;
    std::weak_ptr<IWriteCallback> writeCallback_;
    std::vector<float> resampleSrcBuffer;
    std::vector<float> resampleDesBuffer;
    std::vector<std::vector<char>> sinkBuffer_;
    std::shared_ptr<AudioResample> resample_;
    std::queue<int32_t> readQueue_;
    std::queue<int32_t> writeQueue_;
    LinearPosTimeModel handleTimeModel_;
    AudioProcessConfig processConfig_;
    std::unique_ptr<AudioDownMixStereo> downMixer_;
    BufferBaseInfo bufferInfo_;

    std::mutex firstFrameMutex;
    std::mutex enqueueMutex;
    std::mutex peekMutex;
    std::condition_variable firstFrameSync_;
    std::condition_variable drainSync_;
    FILE *dumpFile_;

    std::atomic<bool> isFirstNoUnderrunFrame_ = false;
};
} // namespace AudioStandard
} // namespace OHOS
#endif
