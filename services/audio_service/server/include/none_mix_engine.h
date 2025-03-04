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
#ifndef NONE_MIX_ENGINE_H
#define NONE_MIX_ENGINE_H
#include <mutex>
#include <atomic>
#include "audio_playback_engine.h"

namespace OHOS {
namespace AudioStandard {
class NoneMixEngine : public AudioPlaybackEngine {
public:
    NoneMixEngine();
    ~NoneMixEngine() override;

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
    void AdjustVoipVolume();

private:
    void StandbySleep();
    int32_t InitSink(const AudioStreamInfo &clientStreamInfo);
    int32_t InitSink(uint32_t channel, AudioSampleFormat format, uint32_t rate);
    int32_t SwitchSink(const AudioStreamInfo &streamInfo, bool isVoip);
    void PauseAsync();
    int32_t StopAudioSink();
    void DoFadeinOut(bool isFadeOut, char* buffer, size_t bufferSize);
    void DoRenderFrame(std::vector<char> &audioBufferConverted, int32_t index, int32_t appUid);

    AudioSamplingRate GetDirectSampleRate(AudioSamplingRate sampleRate);
    AudioSamplingRate GetDirectVoipSampleRate(AudioSamplingRate sampleRate);
    AudioSampleFormat GetDirectDeviceFormat(AudioSampleFormat format);
    AudioSampleFormat GetDirectVoipDeviceFormat(AudioSampleFormat format);
    int32_t GetDirectFormatByteSize(AudioSampleFormat format);

    void GetTargetSinkStreamInfo(const AudioStreamInfo &clientStreamInfo, uint32_t &targetSampleRate,
        uint32_t &targetChannel, AudioSampleFormat &targetFormat, bool &isVoip);

private:
    bool isVoip_;
    bool isStart_;
    bool isInit_;
    AudioDeviceDescriptor device_ = AudioDeviceDescriptor(AudioDeviceDescriptor::DEVICE_INFO);
    std::atomic<uint32_t> failedCount_;
    uint64_t writeCount_;
    uint64_t fwkSyncTime_;
    uint64_t latency_;
    std::shared_ptr<IRendererStream> stream_;

    std::mutex startMutex;

    std::mutex fadingMutex_;
    std::condition_variable cvFading_;
    std::atomic<bool> startFadein_;
    std::atomic<bool> startFadeout_;
    uint32_t uChannel_;
    int32_t uFormat_;
    uint32_t uSampleRate_;
    bool firstSetVolume_;
};
} // namespace AudioStandard
} // namespace OHOS
#endif
