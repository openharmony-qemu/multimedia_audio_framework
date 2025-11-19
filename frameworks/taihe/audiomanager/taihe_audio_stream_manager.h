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

#ifndef TAIHE_AUDIO_STREAM_MANAGER_H
#define TAIHE_AUDIO_STREAM_MANAGER_H

#include "audio_stream_manager.h"
#include "taihe_audio_renderer_state_callback.h"
#include "taihe_audio_capturer_state_callback.h"

namespace ANI::Audio {
using namespace taihe;
using namespace ohos::multimedia::audio;

const std::string RENDERERCHANGE_CALLBACK_NAME = "audioRendererChange";
const std::string CAPTURERCHANGE_CALLBACK_NAME = "audioCapturerChange";

class AudioStreamManagerImpl {
public:
    AudioStreamManagerImpl();
    explicit AudioStreamManagerImpl(std::shared_ptr<AudioStreamManagerImpl> obj);
    ~AudioStreamManagerImpl();

    static AudioStreamManager CreateStreamManagerWrapper();

    array<AudioRendererChangeInfo> GetCurrentAudioRendererInfoArraySync();
    array<AudioCapturerChangeInfo> GetCurrentAudioCapturerInfoArraySync();
    array<AudioEffectMode> GetAudioEffectInfoArraySync(StreamUsage usage);
    bool IsActiveSync(AudioVolumeType volumeType);
    bool IsStreamActive(StreamUsage streamUsage);
    bool IsAcousticEchoCancelerSupported(SourceType sourceType);
    bool IsRecordingAvailable(AudioCapturerInfo capturerInfo);
    bool IsAudioLoopbackSupported(AudioLoopbackMode mode);
    void OnAudioRendererChange(callback_view<void(array_view<AudioRendererChangeInfo>)> callback);
    void OnAudioCapturerChange(callback_view<void(array_view<AudioCapturerChangeInfo>)> callback);
    void OffAudioRendererChange(optional_view<callback<void(array_view<AudioRendererChangeInfo>)>> callback);
    void OffAudioCapturerChange(optional_view<callback<void(array_view<AudioCapturerChangeInfo>)>> callback);

private:
    static void RegisterRendererStateChangeCallback(std::shared_ptr<uintptr_t> &callback, const std::string &cbName,
        AudioStreamManagerImpl *taiheStreamManager);
    static void RegisterCapturerStateChangeCallback(std::shared_ptr<uintptr_t> &callback, const std::string &cbName,
        AudioStreamManagerImpl *taiheStreamManager);
    static void UnregisterRendererChangeCallback(std::shared_ptr<uintptr_t> &callback,
        AudioStreamManagerImpl *taiheStreamManager);
    static void UnregisterCapturerChangeCallback(std::shared_ptr<uintptr_t> &callback,
        AudioStreamManagerImpl *taiheStreamManager);

    OHOS::AudioStandard::AudioStreamManager *audioStreamMngr_;
    int32_t cachedClientId_ = -1;

    std::shared_ptr<OHOS::AudioStandard::AudioRendererStateChangeCallback> rendererStateCallback_ = nullptr;
    std::shared_ptr<OHOS::AudioStandard::AudioCapturerStateChangeCallback> capturerStateCallback_ = nullptr;
    std::mutex mutex_;
};
} // namespace ANI::Audio

#endif // TAIHE_AUDIO_STREAM_MANAGER_H