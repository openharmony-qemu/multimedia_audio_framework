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

#ifndef CABIN_PLAYBACK_ENGINE_H
#define CABIN_PLAYBACK_ENGINE_H

#include <mutex>
#include <atomic>
#include <cstdio>
#include "audio_playback_engine.h"
#include "common/hdi_adapter_info.h"

namespace OHOS {
namespace AudioStandard {
class CabinPlayBackEngine : public AudioPlaybackEngine {
public:
    CabinPlayBackEngine();
    ~CabinPlayBackEngine() override;

    int32_t Init(const AudioDeviceDescriptor &type, bool isVoip) override;
    int32_t Start() override;
    int32_t Stop() override;
    int32_t Pause(bool isStandby = false) override;
    int32_t Flush() override;

    int32_t AddRenderer(const std::shared_ptr<IRendererStream> &stream) override;
    void RemoveRenderer(const std::shared_ptr<IRendererStream> &stream) override;

    uint64_t GetLatency() noexcept override;

private:
    int32_t InitSink(const AudioStreamInfo &clientStreamInfo);
    int32_t InitSink(uint32_t channel, AudioSampleFormat format, uint32_t rate, AudioChannelLayout layout);
    int32_t StopAudioSink();
    void DoRenderFrame(std::vector<char> &audioBufferConverted, int32_t index, int32_t appUid);
    int32_t GetFormatByteSize(AudioSampleFormat format);
    void PollAndWrite();
    void RegisterSinkLatencyFetcherToStreamIfNeeded();
    void RegisterSinkLatencyFetcher(uint32_t renderId);
    void AdjustVolume();
    void StandbySleep();

private:
    bool isVoip_;
    bool isStart_;
    bool isInit_;
    FILE *dump3DA_ = nullptr;
    std::string dumpFileName_ = "";
    AudioDeviceDescriptor device_ = AudioDeviceDescriptor(AudioDeviceDescriptor::DEVICE_INFO);
    uint64_t writeCount_;
    uint64_t fwkSyncTime_;
    uint32_t uChannel_;
    int32_t uformat_;
    uint32_t uSampleRate_;
    uint64_t latency_;
    bool firstSetVolume_;
    std::shared_ptr<IRendererStream> stream_;
    std::function<int32_t (uint32_t &)> sinkLatencyFetcher_;
};
} // namespace AudioStandard
} // namespace OHOS
#endif
