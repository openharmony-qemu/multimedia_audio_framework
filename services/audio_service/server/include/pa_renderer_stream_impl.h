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

#ifndef PA_RENDERER_STREAM_IMPL_H
#define PA_RENDERER_STREAM_IMPL_H

#include <pulse/pulseaudio.h>
#include <mutex>
#include "i_renderer_stream.h"

namespace OHOS {
namespace AudioStandard {
class PaRendererStreamImpl : public IRendererStream {
public:
    PaRendererStreamImpl(pa_stream *paStream, AudioProcessConfig processConfig, pa_threaded_mainloop *mainloop);
    ~PaRendererStreamImpl();
    int32_t InitParams();
    int32_t Start() override;
    int32_t Pause() override;
    int32_t Flush() override;
    int32_t Drain() override;
    int32_t Stop() override;
    int32_t Release() override;
    int32_t GetStreamFramesWritten(uint64_t &framesWritten) override;
    int32_t GetCurrentTimeStamp(uint64_t &timeStamp) override;
    int32_t GetLatency(uint64_t &latency) override;
    int32_t SetRate(int32_t rate) override;
    int32_t SetLowPowerVolume(float volume) override;
    int32_t GetLowPowerVolume(float &powerVolume) override;
    int32_t SetAudioEffectMode(int32_t effectMode) override;
    int32_t GetAudioEffectMode(int32_t &effectMode) override;
    int32_t SetPrivacyType(int32_t privacyType) override;
    int32_t GetPrivacyType(int32_t &privacyType) override;

    void RegisterStatusCallback(const std::weak_ptr<IStatusCallback> &callback) override;
    void RegisterWriteCallback(const std::weak_ptr<IWriteCallback> &callback) override;
    BufferDesc DequeueBuffer(size_t length) override;
    int32_t EnqueueBuffer(const BufferDesc &bufferDesc) override;
    int32_t GetMinimumBufferSize(size_t &minBufferSize) const override;
    void GetByteSizePerFrame(size_t &byteSizePerFrame) const override;
    void GetSpanSizePerFrame(size_t &spanSizeInFrame) const override;
    void SetStreamIndex(uint32_t index) override;
    uint32_t GetStreamIndex() override;
    void AbortCallback(int32_t abortTimes) override;
    // offload
    int32_t SetOffloadMode(int32_t state, bool isAppBack) override;
    int32_t UnsetOffloadMode() override;
    int32_t GetOffloadApproximatelyCacheTime(uint64_t &timeStamp, uint64_t &paWriteIndex,
        uint64_t &cacheTimeDsp, uint64_t &cacheTimePa) override;
    int32_t OffloadSetVolume(float volume) override;
    size_t GetWritableSize() override;
    // offload end

private:
    static void PAStreamWriteCb(pa_stream *stream, size_t length, void *userdata);
    static void PAStreamMovedCb(pa_stream *stream, void *userdata);
    static void PAStreamUnderFlowCb(pa_stream *stream, void *userdata);
    static void PAStreamSetStartedCb(pa_stream *stream, void *userdata);
    static void PAStreamStartSuccessCb(pa_stream *stream, int32_t success, void *userdata);
    static void PAStreamPauseSuccessCb(pa_stream *stream, int32_t success, void *userdata);
    static void PAStreamFlushSuccessCb(pa_stream *stream, int32_t success, void *userdata);
    static void PAStreamDrainSuccessCb(pa_stream *stream, int32_t success, void *userdata);
    static void PAStreamDrainInStopCb(pa_stream *stream, int32_t success, void *userdata);
    static void PAStreamAsyncStopSuccessCb(pa_stream *stream, int32_t success, void *userdata);

    const std::string GetEffectModeName(int32_t effectMode);
    const std::string GetEffectSceneName(AudioStreamType audioType);
    // offload
    int32_t OffloadGetPresentationPosition(uint64_t& frames, int64_t& timeSec, int64_t& timeNanoSec);
    int32_t OffloadSetBufferSize(uint32_t sizeMs);
    void SyncOffloadMode();
    int32_t OffloadUpdatePolicy(AudioOffloadType statePolicy, bool force);
    void ResetOffload();
    int32_t OffloadUpdatePolicyInWrite();
    // offload end

    uint32_t streamIndex_ = static_cast<uint32_t>(-1); // invalid index

    pa_stream *paStream_ = nullptr;
    uint32_t sinkInputIndex_ = 0;
    AudioProcessConfig processConfig_;
    std::weak_ptr<IStatusCallback> statusCallback_;
    std::weak_ptr<IWriteCallback> writeCallback_;
    std::mutex rendererStreamLock_;
    int32_t streamCmdStatus_;
    int32_t streamDrainStatus_;
    int32_t streamFlushStatus_;
    State state_;
    uint32_t underFlowCount_;
    bool isDrain_ = false;
    pa_threaded_mainloop *mainloop_;

    size_t byteSizePerFrame_ = 0;
    size_t spanSizeInFrame_ = 0;
    size_t minBufferSize_ = 0;

    size_t totalBytesWritten_ = 0;
    uint32_t sinkLatencyInMsec_ {0};
    int32_t renderRate_;
    int32_t effectMode_ = -1;
    std::string effectSceneName_ = "SCENE_MUSIC";
    int32_t privacyType_ = 0;

    float powerVolumeFactor_ = 1.0f;

    static constexpr float MAX_STREAM_VOLUME_LEVEL = 1.0f;
    static constexpr float MIN_STREAM_VOLUME_LEVEL = 0.0f;
    // Only for debug
    int32_t abortFlag_ = 0;
    // offload
    bool offloadEnable_ = false;
    int64_t offloadTsOffset_ = 0;
    uint64_t offloadTsLast_ = 0;
    AudioOffloadType offloadStatePolicy_ = OFFLOAD_DEFAULT;
    AudioOffloadType offloadNextStateTargetPolicy_ = OFFLOAD_DEFAULT;
    time_t lastOffloadUpdateFinishTime_ = 0;
    FILE *dumpFile_ = nullptr;
    // offload end
};
} // namespace AudioStandard
} // namespace OHOS
#endif // PA_RENDERER_STREAM_IMPL_H
