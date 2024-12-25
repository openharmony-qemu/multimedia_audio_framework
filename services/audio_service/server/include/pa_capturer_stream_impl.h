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

#ifndef PA_CAPTURER_STREAM_IMPL_H
#define PA_CAPTURER_STREAM_IMPL_H

#include <pulse/pulseaudio.h>
#include "i_capturer_stream.h"

namespace OHOS {
namespace AudioStandard {
class PaCapturerStreamImpl : public std::enable_shared_from_this<PaCapturerStreamImpl>, public ICapturerStream {
public:
    PaCapturerStreamImpl(pa_stream *paStream, AudioProcessConfig processConfig, pa_threaded_mainloop *mainloop);
    ~PaCapturerStreamImpl();
    int32_t InitParams();
    int32_t Start() override;
    int32_t Pause(bool isStandby = false) override;
    int32_t Flush() override;
    int32_t Drain(bool stopFlag = false) override { return 0; };
    int32_t Stop() override;
    int32_t Release() override;
    int32_t GetStreamFramesRead(uint64_t &framesRead) override;
    int32_t GetCurrentTimeStamp(uint64_t &timestamp) override;
    int32_t GetLatency(uint64_t &latency) override;

    void RegisterStatusCallback(const std::weak_ptr<IStatusCallback> &callback) override;
    void RegisterReadCallback(const std::weak_ptr<IReadCallback> &callback) override;
    BufferDesc DequeueBuffer(size_t length) override;
    int32_t EnqueueBuffer(const BufferDesc &bufferDesc) override;
    int32_t GetMinimumBufferSize(size_t &minBufferSize) const override;
    void GetByteSizePerFrame(size_t &byteSizePerFrame) const override;
    void GetSpanSizePerFrame(size_t &spanSizeInFrame) const override;
    void SetStreamIndex(uint32_t index) override;
    uint32_t GetStreamIndex() override;
    int32_t DropBuffer() override;

private:
    static void PAStreamReadCb(pa_stream *stream, size_t length, void *userdata);
    static void PAStreamMovedCb(pa_stream *stream, void *userdata);
    static void PAStreamUnderFlowCb(pa_stream *stream, void *userdata);
    static void PAStreamSetStartedCb(pa_stream *stream, void *userdata);
    static void PAStreamStartSuccessCb(pa_stream *stream, int32_t success, void *userdata);
    static void PAStreamPauseSuccessCb(pa_stream *stream, int32_t success, void *userdata);
    static void PAStreamFlushSuccessCb(pa_stream *stream, int32_t success, void *userdata);
    static void PAStreamStopSuccessCb(pa_stream *stream, int32_t success, void *userdata);
    static void PAStreamUpdateTimingInfoSuccessCb(pa_stream *stream, int32_t success, void *userdata);

    uint32_t streamIndex_ = static_cast<uint32_t>(-1); // invalid index

    pa_stream *paStream_ = nullptr;
    AudioProcessConfig processConfig_ = {};
    std::weak_ptr<IStatusCallback> statusCallback_;
    std::weak_ptr<IReadCallback> readCallback_;
    std::mutex streamImplLock_;
    int32_t streamCmdStatus_ = 0;
    int32_t streamFlushStatus_ = 0;
    State state_ = INVALID;
    uint32_t underFlowCount_ = 0;
    pa_threaded_mainloop *mainloop_ = nullptr;

    size_t byteSizePerFrame_ = 0;
    size_t spanSizeInFrame_ = 0;
    size_t minBufferSize_ = 0;

    size_t totalBytesRead_ = 0;

    FILE *capturerServerDumpFile_ = nullptr;
    bool releasedFlag_ = false;
};
} // namespace AudioStandard
} // namespace OHOS
#endif // PA_CAPTURER_STREAM_IMPL_H
