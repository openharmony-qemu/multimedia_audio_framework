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

#ifndef DIRECT_PLAYBACK_ENGINE_H
#define DIRECT_PLAYBACK_ENGINE_H

#include <mutex>
#include <atomic>
#include "audio_playback_engine.h"
#include "common/hdi_adapter_info.h"

namespace OHOS {
namespace AudioStandard {
class DirectPlayBackEngine : public AudioPlaybackEngine {
public:
    DirectPlayBackEngine();
    ~DirectPlayBackEngine() override;

    int32_t Init(const AudioDeviceDescriptor &type, bool isVoip) override;
    int32_t Start() override;
    int32_t Stop() override;
    int32_t Pause() override;
    int32_t Flush() override;

    int32_t AddRenderer(const std::shared_ptr<IRendererStream> &stream) override;
    void RemoveRenderer(const std::shared_ptr<IRendererStream> &stream) override;
    bool IsPlaybackEngineRunning() const noexcept override;

    uint64_t GetLatency() noexcept override;

protected:
    void MixStreams() override;

private:
    int32_t InitSink(const AudioStreamInfo &clientStreamInfo);
    int32_t InitSink(uint32_t channel, AudioSampleFormat format, uint32_t rate, AudioChannelLayout layout);
    int32_t StopAudioSink();
    void DoRenderFrame(std::vector<char> &audioBufferConverted, int32_t index, int32_t appUid);
    void DirectCallback(const RenderCallbackType type);
    int32_t RegisterWriteCallback();
    int32_t GetDirectFormatByteSize(AudioSampleFormat format);

private:
    bool isStart_;
    bool isInit_;
    AudioDeviceDescriptor device_ = AudioDeviceDescriptor(AudioDeviceDescriptor::DEVICE_INFO);
    std::atomic<uint32_t> failedCount_;
    uint64_t latency_;
    std::shared_ptr<IRendererStream> stream_;
    uint32_t uChannel_;
    int32_t format_;
    uint32_t uSampleRate_;
};
} // namespace AudioStandard
} // namespace OHOS
#endif
