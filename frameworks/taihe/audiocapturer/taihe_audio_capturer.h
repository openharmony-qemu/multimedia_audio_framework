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

#ifndef TAIHE_AUDIO_CAPTURER_H
#define TAIHE_AUDIO_CAPTURER_H

#include <mutex>
#include <map>
#include <list>

#include "audio_capturer.h"
#include "audio_log.h"
#include "taihe_work.h"
#include "taihe_audio_capturer_callback_inner.h"

namespace ANI::Audio {
using namespace taihe;
using namespace ohos::multimedia::audio;

class AudioCapturerImpl {
public:
    AudioCapturerImpl();
    explicit AudioCapturerImpl(std::shared_ptr<AudioCapturerImpl> obj);
    ~AudioCapturerImpl();

    int64_t GetImplPtr();
    std::shared_ptr<OHOS::AudioStandard::AudioCapturer> GetNativePtr();
    static std::shared_ptr<AudioCapturerImpl> CreateAudioCapturerNativeObject();
    static AudioCapturer CreateAudioCapturerWrapper(OHOS::AudioStandard::AudioCapturerOptions capturerOptions);

#if !defined(ANDROID_PLATFORM) && !defined(IOS_PLATFORM)
    std::shared_ptr<OHOS::AudioStandard::AudioCapturer> audioCapturer_;
#else
    std::unique_ptr<OHOS::AudioStandard::AudioCapturer> audioCapturer_;
#endif
    std::mutex readCallbackMutex_;
    std::condition_variable readCallbackCv_;
    std::list<std::shared_ptr<TaiheAudioCapturerCallbackInner>> audioCapturerCallbacks_;
    std::atomic<bool> isFrameCallbackDone_;

    AudioState GetState();
    void StartSync();
    void StopSync();
    void ReleaseSync();
    int64_t GetBufferSizeSync();
    AudioCapturerInfo GetCapturerInfoSync();
    AudioStreamInfo GetStreamInfoSync();
    int64_t GetAudioStreamIdSync();
    int64_t GetAudioTimeSync();
    AudioTimestampInfo GetAudioTimestampInfoSync();
    int64_t GetOverflowCountSync();
    void SetWillMuteWhenInterruptedSync(bool muteWhenInterrupted);
    taihe::array<AudioDeviceDescriptor> GetCurrentInputDevices();
    AudioCapturerChangeInfo GetCurrentAudioCapturerChangeInfo();
    void SetInputDeviceToAccessory();

    void OnAudioInterrupt(callback_view<void(InterruptEvent const&)> callback);
    void OnStateChange(callback_view<void(AudioState)> callback);
    void OnInputDeviceChange(callback_view<void(array_view<AudioDeviceDescriptor>)> callback);
    void OnAudioCapturerChange(callback_view<void(AudioCapturerChangeInfo const&)> callback);
    void OnReadData(callback_view<void(array_view<uint8_t>)> callback);
    void OnPeriodReach(int64_t frame, callback_view<void(int64_t)> callback);
    void OnMarkReach(int64_t frame, callback_view<void(int64_t)> callback);
    void OffStateChange(optional_view<callback<void(AudioState)>> callback);
    void OffAudioInterrupt(optional_view<callback<void(InterruptEvent const&)>> callback);
    void OffInputDeviceChange(optional_view<callback<void(array_view<AudioDeviceDescriptor>)>> callback);
    void OffAudioCapturerChange(optional_view<callback<void(AudioCapturerChangeInfo const&)>> callback);
    void OffReadData(optional_view<callback<void(array_view<uint8_t>)>> callback);
    void OffPeriodReach(optional_view<callback<void(int64_t)>> callback);
    void OffMarkReach(optional_view<callback<void(int64_t)>> callback);

private:
    static void RegisterCapturerCallback(std::shared_ptr<uintptr_t> &callback,
        const std::string &cbName, AudioCapturerImpl *taiheCapturer);
    static void RegisterAudioCapturerDeviceChangeCallback(std::shared_ptr<uintptr_t> &callback,
        const std::string &cbName, AudioCapturerImpl *taiheCapturer);
    static void RegisterAudioCapturerInfoChangeCallback(std::shared_ptr<uintptr_t> &callback,
        const std::string &cbName, AudioCapturerImpl *taiheCapturer);
    static void RegisterCapturerReadDataCallback(std::shared_ptr<uintptr_t> &callback,
        const std::string &cbName, AudioCapturerImpl *taiheCapturer);
    static void RegisterPeriodPositionCallback(int64_t frame, std::shared_ptr<uintptr_t> &callback,
        const std::string &cbName, AudioCapturerImpl *taiheCapturer);
    static void RegisterPositionCallback(int64_t frame, std::shared_ptr<uintptr_t> &callback,
        const std::string &cbName, AudioCapturerImpl *taiheCapturer);

    static void UnregisterCapturerCallback(std::shared_ptr<uintptr_t> &callback,
        const std::string &cbName, AudioCapturerImpl *taiheCapturer);
    static void UnregisterAudioCapturerDeviceChangeCallback(std::shared_ptr<uintptr_t> &callback,
        const std::string &cbName, AudioCapturerImpl *taiheCapturer);
    static void UnregisterAudioCapturerInfoChangeCallback(std::shared_ptr<uintptr_t> &callback,
        const std::string &cbName, AudioCapturerImpl *taiheCapturer);
    static void UnregisterCapturerReadDataCallback(std::shared_ptr<uintptr_t> &callback,
        const std::string &cbName, AudioCapturerImpl *taiheCapturer);
    static void UnregisterCapturerPeriodPositionCallback(std::shared_ptr<uintptr_t> &callback,
        const std::string &cbName, AudioCapturerImpl *taiheCapture);
    static void UnregisterCapturerPositionCallback(std::shared_ptr<uintptr_t> &callback,
        const std::string &cbName, AudioCapturerImpl *taiheCapture);

    static std::unique_ptr<OHOS::AudioStandard::AudioCapturerOptions> sCapturerOptions_;
    static std::mutex createMutex_;
    static int32_t isConstructSuccess_;

    std::shared_ptr<OHOS::AudioStandard::AudioCapturerCallback> callbackTaihe_ = nullptr;
    std::shared_ptr<OHOS::AudioStandard::CapturerPositionCallback> positionCbTaihe_ = nullptr;
    std::shared_ptr<OHOS::AudioStandard::CapturerPeriodPositionCallback> periodPositionCbTaihe_ = nullptr;
    std::shared_ptr<OHOS::AudioStandard::AudioCapturerReadCallback> capturerReadDataCallbackTaihe_ = nullptr;

    OHOS::AudioStandard::SourceType sourceType_;
    std::mutex mutex_;
};
} // namespace ANI::Audio

#endif // TAIHE_AUDIO_CAPTURER_H
