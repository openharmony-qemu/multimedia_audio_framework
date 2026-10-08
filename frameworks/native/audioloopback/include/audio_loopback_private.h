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
enum class AudioLoopbackErrorScope {
    DEFAULT = 0,
    PLATFORM,
    DEVICE,
    STREAM,
};

enum AudioLoopbackErrorType {
    ERROR_PLATFORM_NOT_SUPPORT = -100,
    ERROR_DEVICE_NOT_SUPPORT = -200,
    ERROR_DEVICE_MISMATCH = -201,
    ERROR_DEVICE_RECORD_PERMISSION_DENIED = -202,
    ERROR_RENDER_CREATE_FAIL = -300,
    ERROR_RENDER_CREATE_FAST_STREAM_FAIL = -301,
    ERROR_RENDER_START_FAIL = -302,
    ERROR_RENDER_FALL_BACK_NORMAL = -303,
    ERROR_CAPTURE_CREATE_FAIL = -304,
    ERROR_CAPTURE_CREATE_FAST_STREAM_FAIL = -305,
    ERROR_CAPTURE_START_FAIL = -306,
    ERROR_CAPTURE_FALL_BACK_NORMAL = -307,
    DEFAULT_OK = 0,
};

struct AudioLoopbackReportInfo {
    int32_t appUid = INVALID_UID;
    std::string appName;
    DeviceType renderDeviceType = DEVICE_TYPE_NONE;
    DeviceType captureDeviceType = DEVICE_TYPE_NONE;
    AudioLoopbackErrorScope errScope = AudioLoopbackErrorScope::PLATFORM;
    AudioLoopbackErrorType errType = DEFAULT_OK;
};

class AudioLoopbackPrivate : public AudioLoopback,
                             public std::enable_shared_from_this<AudioLoopbackPrivate>,
                             public AudioCapturerReadCallback,
                             public AudioRendererWriteCallback {
public:
    explicit AudioLoopbackPrivate(AudioLoopbackMode mode, const AppInfo &appInfo, bool isGlobal = false);
    virtual ~AudioLoopbackPrivate();

    bool Enable(bool enable) override;
    AudioLoopbackStatus GetStatus() override;
    int32_t SetVolume(float volume) override;
    float GetVolume() override;
    int32_t SetAudioLoopbackCallback(const std::shared_ptr<AudioLoopbackCallback> &callback) override;
    int32_t RemoveAudioLoopbackCallback() override;
    void OnWriteData(size_t length) override;
    void OnReadData(size_t length) override;
    
    bool SetReverbPreset(AudioLoopbackReverbPreset preset) override;
    AudioLoopbackReverbPreset GetReverbPreset() override;
    bool SetEqualizerPreset(AudioLoopbackEqualizerPreset preset) override;
    AudioLoopbackEqualizerPreset GetEqualizerPreset() override;
    int32_t GetLoopbackDebugInfo(AudioLoopbackDebugInfo &debugInfo) const override;

    std::vector<AudioDevicePair> GetSupportedDevicePairs() override;

    AudioDevicePair GetPreferredDevicePair() override;

    static void ReportAudioLoopbackException(const AudioLoopbackReportInfo &info);
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
    // Skip the destroy when the session was re-enabled after this destroy was
    // scheduled (generation mismatch), so a stale destroy cannot tear down the
    // streams rebuilt by a later Enable(true).
    void DestroyAudioLoopback(uint32_t generation);
    void DestroyAudioLoopbackInner();
    // Single-field helpers guarded by karaokeParamsMutex_ (leaf lock).
    void SetKaraokeParam(KaraokeParamType paramType, int32_t value);
    int32_t GetKaraokeParam(KaraokeParamType paramType) const;
    bool IsAudioLoopbackSupported();
    AudioLoopbackErrorType CheckDeviceSupport();
    bool EnableLoopback();
    void DisableLoopback();
    void StartAudioLoopback();
    AudioLoopbackStatus StateToStatus(AudioLoopbackState state);
    bool SetKaraokeParameters(KaraokeParamType paramType, int32_t value);
    AudioLoopbackReportInfo GetReportInfo(AudioLoopbackErrorScope scope, AudioLoopbackErrorType type);
    void GetValidDevice();
    void ReportUsingDuration(uint64_t usingDuration);

    bool isGlobal_ = false;
    uint64_t usingDuration_ = 0;
    AudioRendererOptions rendererOptions_;
    AudioCapturerOptions capturerOptions_;
    AppInfo appInfo_ = {};
    std::map<KaraokeParamType, int32_t> karaokeParams_ = {};
    // Sole guard of karaokeParams_. Must stay a LEAF lock: EnableLoopback runs
    // under stateMutex_ (service callback thread) and inside loopbackMutex_
    // (Enable(true) -> UpdateStatus -> EnableLoopback), so holding either outer
    // lock here, or taking an outer lock while holding this one, deadlocks.
    mutable std::mutex karaokeParamsMutex_;
    std::shared_ptr<AudioRenderer> audioRenderer_;
    std::shared_ptr<AudioCapturer> audioCapturer_;
    // Sole guard of the two stream members above. Must stay a LEAF lock taken
    // only for member copy/move-out: Stop/Release wait for the data callbacks
    // (OnReadData/OnWriteData) to return and those take this mutex, so no code
    // path may hold it across a blocking call. Guarding them with
    // loopbackMutex_ instead deadlocked the stop path (it is held across the
    // teardown by Enable and the destructor).
    mutable std::mutex streamMutex_;
    mutable std::mutex loopbackMutex_;
    mutable std::mutex stateMutex_;
    // Bumped by every Enable(true) rebuild; detached destroys carry the
    // generation they were scheduled for and skip themselves when it changed.
    std::atomic<uint32_t> destroyGeneration_ = 0;
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

    DeviceType activeOutputDevice_ = DEVICE_TYPE_NONE;
    DeviceType activeInputDevice_ = DEVICE_TYPE_NONE;
    std::unordered_set<DeviceType> validDevice;
};
}  // namespace AudioStandard
}  // namespace OHOS
#endif // AUDIO_LOOPBACK_PRIVATE_H
