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
#ifndef AUDIO_LOOPBACK_PRIVATE_H
#define AUDIO_LOOPBACK_PRIVATE_H

#include "audio_loopback.h"
#include "audio_renderer.h"
#include "audio_capturer.h"
namespace OHOS {
namespace AudioStandard {

class AudioLoopbackPrivate : public AudioLoopback,
                             public std::enable_shared_from_this<AudioLoopbackPrivate>,
                             public AudioCapturerReadCallback,
                             public AudioRendererWriteCallback {
public:
    explicit AudioLoopbackPrivate(AudioLoopbackMode mode, const AppInfo &appInfo);
    virtual ~AudioLoopbackPrivate();

    bool Enable(bool enable) override;
    AudioLoopbackStatus GetStatus() override;
    int32_t SetVolume(float volume) override;
    int32_t SetAudioLoopbackCallback(const std::shared_ptr<AudioLoopbackCallback> &callback) override;
    int32_t RemoveAudioLoopbackCallback() override;
    void OnWriteData(size_t length) override;
    void OnReadData(size_t length) override;
    
    bool SetReverbPreset(AudioLoopbackReverbPreset preset) override;
    AudioLoopbackReverbPreset GetReverbPreset() override;
    bool SetEqualizerPreset(AudioLoopbackEqualizerPreset preset) override;
    AudioLoopbackEqualizerPreset GetEqualizerPreset() override;

private:

    class RendererCallbackImpl : public AudioRendererCallback,
                                 public AudioRendererOutputDeviceChangeCallback,
                                 public AudioRendererFastStatusChangeCallback {
    public:
        explicit RendererCallbackImpl(AudioLoopbackPrivate &parent);
        void OnInterrupt(const InterruptEvent &interruptEvent) override {}
        void OnStateChange(const RendererState state, const StateChangeCmdType cmdType) override;
        void OnOutputDeviceChange(const AudioDeviceDescriptor &deviceInfo,
            const AudioStreamDeviceChangeReason reason) override;
        void OnFastStatusChange(FastStatus status) override;

    private:
        AudioLoopbackPrivate &parent_;
    };

    class CapturerCallbackImpl : public AudioCapturerCallback,
                                 public AudioCapturerDeviceChangeCallback,
                                 public AudioCapturerFastStatusChangeCallback {
    public:
        explicit CapturerCallbackImpl(AudioLoopbackPrivate &parent);
        void OnInterrupt(const InterruptEvent &interruptEvent) override {}
        void OnStateChange(const CapturerState state) override;
        void OnStateChange(const AudioDeviceDescriptor &deviceInfo) override;
        void OnFastStatusChange(FastStatus status) override;

    private:
        AudioLoopbackPrivate &parent_;
    };
    void InitStatus();
    void InitializeCallbacks();
    void UpdateStatus();
    AudioLoopbackState GetCurrentState();
    AudioRendererOptions GenerateRendererConfig();
    AudioCapturerOptions GenerateCapturerConfig();

    void CreateAudioLoopback();
    void DestroyAudioLoopback();
    void DestroyAudioLoopbackInner();
    bool IsAudioLoopbackSupported();
    bool CheckDeviceSupport();
    bool EnableLoopback();
    void DisableLoopback();
    void StartAudioLoopback();
    AudioLoopbackStatus StateToStatus(AudioLoopbackState state);
    bool SetKaraokeParameters(const std::string &parameters);

    AudioRendererOptions rendererOptions_;
    AudioCapturerOptions capturerOptions_;
    AppInfo appInfo_ = {};
    std::map<std::string, std::string> karaokeParams_ = {};
    std::shared_ptr<AudioRenderer> audioRenderer_;
    std::shared_ptr<AudioCapturer> audioCapturer_;
    std::mutex loopbackMutex_;
    std::mutex stateMutex_;
    std::shared_ptr<AudioLoopbackCallback> statusCallback_;
    AudioLoopbackState currentState_ = LOOPBACK_STATE_IDLE;
    AudioLoopbackMode mode_ = LOOPBACK_HARDWARE;

    std::atomic<RendererState> rendererState_ = RENDERER_INVALID;
    std::atomic<bool> isRendererUsb_ = false;
    std::atomic<FastStatus> rendererFastStatus_ = FASTSTATUS_NORMAL;

    std::atomic<CapturerState> capturerState_ = CAPTURER_INVALID;
    std::atomic<bool> isCapturerUsb_ = false;
    std::atomic<FastStatus> capturerFastStatus_ = FASTSTATUS_NORMAL;
    AudioLoopbackReverbPreset currentReverbPreset_ = REVERB_PRESET_THEATER;
    AudioLoopbackEqualizerPreset currentEqualizerPreset_ = EQUALIZER_PRESET_FULL;
};
}  // namespace AudioStandard
}  // namespace OHOS
#endif // AUDIO_LOOPBACK_PRIVATE_H
