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

#ifndef HPAE_CAPTURER_STREAM_IMPL_H
#define HPAE_CAPTURER_STREAM_IMPL_H

#include <shared_mutex>
#include "i_capturer_stream.h"

namespace OHOS {
namespace AudioStandard {
class HpaeCapturerStreamImpl : public std::enable_shared_from_this<HpaeCapturerStreamImpl>,
                               public IStreamStatusCallback,
                               public ICapturerStreamCallback,
                               public ICapturerStream {
public:
    HpaeCapturerStreamImpl(AudioProcessConfig processConfig);
    ~HpaeCapturerStreamImpl();
    int32_t InitParams(const std::string &deviceName = "");
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
    void AbortCallback(int32_t abortTimes);
    int32_t OnStreamData(AudioCallBackCapturerStreamInfo &callBackStreamInfo) override;
    void OnStatusUpdate(IOperation operation, uint32_t streamIndex) override;

private:

    uint32_t streamIndex_ = static_cast<uint32_t>(-1); // invalid index

    AudioProcessConfig processConfig_ = {};
    std::weak_ptr<IStatusCallback> statusCallback_;
    std::weak_ptr<IReadCallback> readCallback_;
    State state_ = INVALID;

    size_t byteSizePerFrame_ = 0;
    size_t spanSizeInFrame_ = 0;
    size_t minBufferSize_ = 0;

    size_t totalBytesRead_ = 0;

    FILE *capturerServerDumpFile_ = nullptr;

    // Only for debug
    int32_t abortFlag_ = 0;

    uint32_t capturerId_ = 0;

    std::shared_mutex latencyMutex_;
    uint64_t framesRead_ = 0;
    uint64_t timestamp_ = 0;
    uint64_t latency_ = 0;
};
} // namespace AudioStandard
} // namespace OHOS
#endif // hpae_capturer_stream_impl_H
