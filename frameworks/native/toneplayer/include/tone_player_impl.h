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

#ifndef AUDIO_TONEPLAYER_IMPL_H
#define AUDIO_TONEPLAYER_IMPL_H

#include <map>
#include <thread>
#include <mutex>

#include "tone_player.h"
#include "audio_renderer.h"
#include "audio_utils.h"

namespace OHOS {
namespace AudioStandard {
class TonePlayerImpl : public AudioRendererWriteCallback, public AudioRendererCallback, public TonePlayer,
    public std::enable_shared_from_this<TonePlayerImpl> {
public:
    TonePlayerImpl(const std::string cachePath, const AudioRendererInfo &rendererInfo);
    ~TonePlayerImpl();

    // for audio renderer callback
    void OnInterrupt(const InterruptEvent &interruptEvent) override;
    void OnStateChange(const RendererState state, const StateChangeCmdType __attribute__((unused)) cmdType) override;
    void OnWriteData(size_t length) override;

    // for toneplayer
    bool LoadTone(ToneType toneType) override;
    bool StartTone() override;
    bool StopTone() override;
    bool Release() override;

    enum ToneState : uint8_t {
        TONE_IDLE,
        TONE_INIT,
        TONE_STARTING,
        TONE_RUNNING,
        TONE_STOPPING,
        TONE_STOPPED,
        TONE_RELEASED,
    };

private:
    bool InitAudioRenderer();
    bool InitToneWaveInfo();
    bool AudioToneSequenceGen(BufferDesc &bufDesc);
    bool ContinueToneplay(uint32_t sampleCnt, int8_t *audioBuf);
    bool CheckToneStarted(uint32_t sampleCnt, int8_t *audioBuf);
    bool CheckToneStopped();
    void GetCurrentSegmentUpdated();
    bool CheckToneContinuity();
    int32_t GetSamples(uint16_t *freqs, int8_t *buffer, uint32_t samples);

    AudioRendererOptions rendererOptions_ = {};
    std::string cachePath_; // NAPI interface to create AudioRenderer
    std::unique_ptr<AudioRenderer> audioRenderer_;  // Pointer to AudioRenderer used for playback
    bool isRendererInited_ = false;

    std::mutex optMutex_;
    ToneType toneType_ = NUM_TONES;
    int32_t amplitudeType_ = 0;
    uint32_t currSegment_ = 0;  // Current segment index in ToneDescriptor segments[]
    uint32_t currCount_ = 0;  // Current sequence repeat count
    std::shared_ptr<ToneInfo> toneInfo_;  // pointer to active tone Info
    std::shared_ptr<ToneInfo> initialToneInfo_;  // pointer to new active tone Info
    std::vector<int32_t> supportedTones_;

    ToneState toneState_ = TONE_IDLE;

    uint32_t loopCounter_ = 0; // Current tone loopback count
    uint32_t totalSample_ = 0;  // Total no. of tone samples played
    uint32_t nextSegSample_ = 0;  // Position of next segment transition expressed in samples
    uint32_t maxSample_ = 0;  // Maximum number of audio samples played (maximun tone duration)
    uint32_t samplingRate_ = 0;  // Audio Sampling rate
    uint32_t sampleCount_ = 0; // Initial value should be zero before any new Tone renderering

    // to wait for audio rendere callback completion after a change is requested
    FILE *dumpFile_ = nullptr;
    uint32_t processSize_ = 0;  // In audioRenderer, Size of audio blocks generated at a time
};
} // namespace AudioStandard
} // namespace OHOS
#endif /* AUDIO_TONEPLAYER_IMPL_H */
