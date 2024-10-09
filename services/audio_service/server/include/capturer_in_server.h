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

#ifndef CAPTURER_IN_SERVER_H
#define CAPTURER_IN_SERVER_H

#include <mutex>
#include "i_capturer_stream.h"
#include "i_stream_listener.h"
#include "oh_audio_buffer.h"
#include "audio_ring_cache.h"

namespace OHOS {
namespace AudioStandard {
class CapturerInServer : public IStatusCallback, public IReadCallback,
    public std::enable_shared_from_this<CapturerInServer> {
public:
    CapturerInServer(AudioProcessConfig processConfig, std::weak_ptr<IStreamListener> streamListener);
    virtual ~CapturerInServer();
    void OnStatusUpdate(IOperation operation) override;
    int32_t OnReadData(size_t length) override;

    int32_t ResolveBuffer(std::shared_ptr<OHAudioBuffer> &buffer);
    int32_t GetSessionId(uint32_t &sessionId);
    int32_t Start();
    int32_t Pause();
    int32_t Flush();
    int32_t Stop();
    int32_t Release();

    int32_t GetAudioTime(uint64_t &framePos, uint64_t &timestamp);
    int32_t GetLatency(uint64_t &latency);

    int32_t Init();

    int32_t ConfigServerBuffer();
    int32_t InitBufferStatus();
    int32_t UpdateReadIndex();
    BufferDesc DequeueBuffer(size_t length);
    void ReadData(size_t length);
    int32_t DrainAudioBuffer();

    // for inner-cap.
    int32_t UpdatePlaybackCaptureConfig(const AudioPlaybackCaptureConfig &config);
    int32_t UpdatePlaybackCaptureConfigInLegacy(const AudioPlaybackCaptureConfig &config);
    void SetNonInterruptMute(const bool muteFlag);

private:
    int32_t InitCacheBuffer(size_t targetSize);
    bool IsReadDataOverFlow(size_t length, uint64_t currentWriteFrame,
        std::shared_ptr<IStreamListener> stateListener);

    std::mutex statusLock_;
    std::condition_variable statusCv_;
    std::shared_ptr<ICapturerStream> stream_ = nullptr;
    uint32_t streamIndex_ = -1;
    IOperation operation_ = OPERATION_INVALID;
    IStatus status_ = I_STATUS_IDLE;

    bool needCheckBackground_ = false;

    AudioPlaybackCaptureConfig filterConfig_;
    std::weak_ptr<IStreamListener> streamListener_;
    AudioProcessConfig processConfig_;
    size_t totalSizeInFrame_ = 0;
    size_t spanSizeInFrame_ = 0;
    size_t byteSizePerFrame_ = 0;
    size_t spanSizeInBytes_ = 0;
    bool isBufferConfiged_  = false;
    std::atomic<bool> isInited_ = false;
    std::shared_ptr<OHAudioBuffer> audioServerBuffer_ = nullptr;
    int32_t needStart = 0;
    int32_t underflowCount = 0;
    bool resetTime_ = false;
    uint64_t resetTimestamp_ = 0;
    uint32_t overFlowLogFlag_ = 0;
    std::unique_ptr<AudioRingCache> ringCache_ = nullptr;
    size_t cacheSizeInBytes_ = 0;
    std::unique_ptr<uint8_t []> dischargeBuffer_ = nullptr;
    FILE *dumpS2C_ = nullptr; // server to client dump file
    std::string dumpFileName_ = "";
    std::atomic<bool> muteFlag_ = false;
};
} // namespace AudioStandard
} // namespace OHOS
#endif // CAPTURER_IN_SERVER_H
