/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
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

#ifndef TAIHE_AUDIO_RENDERER_H
#define TAIHE_AUDIO_RENDERER_H

#include <iostream>
#include <map>
#include <queue>

#include "audio_stream_manager.h"
#include "audio_renderer.h"
#include "taihe_audio_enum.h"
#include "taihe_work.h"
#include "taihe_audio_renderer_device_change_callback.h"

namespace ANI::Audio {
using namespace taihe;
using namespace ohos::multimedia::audio;

class AudioRendererImpl {
public:
    AudioRendererImpl();
    explicit AudioRendererImpl(std::shared_ptr<AudioRendererImpl> obj);
    ~AudioRendererImpl() = default;

    static void CreateRendererFailed();
    static std::shared_ptr<AudioRendererImpl> CreateAudioRendererNativeObject();
    static AudioRenderer CreateAudioRendererWrapper(OHOS::AudioStandard::AudioRendererOptions rendererOptions);

#if !defined(ANDROID_PLATFORM) && !defined(IOS_PLATFORM)
    std::shared_ptr<OHOS::AudioStandard::AudioRenderer> audioRenderer_;
#else
    std::unique_ptr<OHOS::AudioStandard::AudioRenderer> audioRenderer_;
#endif
    std::mutex writeCallbackMutex_;
    std::condition_variable writeCallbackCv_;
    bool enqueued_ = false;

    void StartSync();
    int64_t GetAudioTimeSync();
    void DrainSync();
    void FlushSync();
    void PauseSync();
    void StopSync();
    void ReleaseSync();
    int64_t GetBufferSizeSync();
    int64_t GetAudioStreamIdSync();
    void SetVolumeSync(double volume);
    double GetVolume();
    AudioRendererInfo GetRendererInfoSync();
    AudioStreamInfo GetStreamInfoSync();
    void SetInterruptModeSync(InterruptMode mode);
    double GetMinStreamVolumeSync();
    double GetMaxStreamVolumeSync();
    taihe::array<AudioDeviceDescriptor> GetCurrentOutputDevicesSync();
    int64_t GetUnderflowCountSync();
    AudioTimestampInfo GetAudioTimestampInfoSync();
    AudioEffectMode GetAudioEffectModeSync();
    void SetAudioEffectModeSync(AudioEffectMode mode);
    void SetChannelBlendMode(ChannelBlendMode mode);
    void SetVolumeWithRamp(double volume, int32_t duration);
    void SetSpeed(double speed);
    double GetSpeed();
    AudioState GetState();
    void SetSilentModeAndMixWithOthers(bool on);
    bool GetSilentModeAndMixWithOthers();
    void SetDefaultOutputDeviceSync(DeviceType deviceType);
    void SetLoudnessGainSync(double loudnessGain);
    double GetLoudnessGain();
    void SetTargetSync(RenderTarget target);
    RenderTarget GetTarget();

    void OnStateChange(callback_view<void(AudioState)> callback);
    void OnAudioInterrupt(callback_view<void(InterruptEvent const&)> callback);
    void OnPeriodReach(int64_t frame, callback_view<void(int64_t)> callback);
    void OnMarkReach(int64_t frame, callback_view<void(int64_t)> callback);
    void OnOutputDeviceChange(callback_view<void(array_view<AudioDeviceDescriptor>)> callback);
    void OnOutputDeviceChangeWithInfo(callback_view<void(AudioStreamDeviceChangeInfo const&)> callback);
    void OnWriteData(callback_view<AudioDataCallbackResult(array_view<uint8_t>)> callback);

    void OffAudioInterrupt(optional_view<callback<void(InterruptEvent const&)>> callback);
    void OffStateChange(optional_view<callback<void(AudioState)>> callback);
    void OffOutputDeviceChange(optional_view<callback<void(array_view<AudioDeviceDescriptor>)>> callback);
    void OffOutputDeviceChangeWithInfo(optional_view<callback<void(AudioStreamDeviceChangeInfo const&)>> callback);
    void OffPeriodReach(optional_view<callback<void(int64_t)>> callback);
    void OffMarkReach(optional_view<callback<void(int64_t)>> callback);
    void OffWriteData(optional_view<callback<AudioDataCallbackResult(array_view<uint8_t>)>> callback);

    void DestroyCallbacks();
    void DestroyTaiheCallbacks();

private:
    static void RegisterRendererCallback(std::shared_ptr<uintptr_t> &callback,
        const std::string &cbName, AudioRendererImpl *taiheRenderer);
    static void RegisterRendererDeviceChangeCallback(std::shared_ptr<uintptr_t> &callback,
        AudioRendererImpl *taiheRenderer);
    static void RegisterPositionCallback(int64_t markPosition, std::shared_ptr<uintptr_t> &callback,
        const std::string &cbName, AudioRendererImpl *taiheRenderer);
    static void RegisterPeriodPositionCallback(int64_t frame, std::shared_ptr<uintptr_t> &callback,
        const std::string &cbName, AudioRendererImpl *taiheRenderer);
    static void RegisterRendererOutputDeviceChangeWithInfoCallback(std::shared_ptr<uintptr_t> &callback,
        AudioRendererImpl *taiheRenderer);
    static void RegisterRendererWriteDataCallback(std::shared_ptr<uintptr_t> &callback,
        const std::string &cbName, AudioRendererImpl *taiheRenderer);
    static void UnregisterRendererCallback(std::shared_ptr<uintptr_t> &callback,
        const std::string &cbName, AudioRendererImpl *taiheRenderer);
    static void UnregisterRendererDeviceChangeCallback(std::shared_ptr<uintptr_t> &callback,
        AudioRendererImpl *taiheRenderer);
    static void UnregisterRendererOutputDeviceChangeWithInfoCallback(std::shared_ptr<uintptr_t> &callback,
        AudioRendererImpl *taiheRenderer);
    static void UnregisterPeriodPositionCallback(std::shared_ptr<uintptr_t> &callback,
        const std::string &cbName, AudioRendererImpl *taiheRenderer);
    static void UnregisterPositionCallback(std::shared_ptr<uintptr_t> &callback,
        const std::string &cbName, AudioRendererImpl *taiheRenderer);
    static void UnregisterRendererWriteDataCallback(std::shared_ptr<uintptr_t> &callback,
        AudioRendererImpl *taiheRenderer);

    std::shared_ptr<OHOS::AudioStandard::AudioRendererCallback> callbackTaihe_ = nullptr;
    std::shared_ptr<OHOS::AudioStandard::RendererPositionCallback> positionCbTaihe_ = nullptr;
    std::shared_ptr<OHOS::AudioStandard::RendererPeriodPositionCallback> periodPositionCbTaihe_ = nullptr;
    std::shared_ptr<TaiheAudioRendererDeviceChangeCallback> rendererDeviceChangeCallbackTaihe_ = nullptr;
    std::shared_ptr<TaiheAudioRendererOutputDeviceChangeWithInfoCallback>
        rendererOutputDeviceChangeWithInfoCallbackTaihe_ = nullptr;
    std::shared_ptr<OHOS::AudioStandard::AudioRendererPolicyServiceDiedCallback>
        rendererPolicyServiceDiedCallbackTaihe_ = nullptr;
    std::shared_ptr<OHOS::AudioStandard::AudioRendererWriteCallback> rendererWriteDataCallbackTaihe_ = nullptr;

    std::mutex mutex_;
    static std::mutex createMutex_;
    static int32_t isConstructSuccess_;
    static std::unique_ptr<OHOS::AudioStandard::AudioRendererOptions> sRendererOptions_;

    OHOS::AudioStandard::ContentType contentType_;
    OHOS::AudioStandard::StreamUsage streamUsage_;
};
} // namespace ANI::Audio

#endif // TAIHE_AUDIO_RENDERER_H
