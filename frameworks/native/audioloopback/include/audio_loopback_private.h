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

class AudioLoopbackPrivate : public AudioLoopback, public std::enable_shared_from_this<AudioLoopbackPrivate>  {
public:
    bool Enable(bool enable) override;

    AudioLoopbackStatus GetStatus() const override;

    void SetVolume(float volume) override;

    int32_t SetAudioLoopbackCallback(const std::shared_ptr<AudioLoopbackCallback> &callback) override;

    explicit AudioLoopbackPrivate(AudioLoopbackMode mode, const AppInfo &appInfo);

    virtual ~AudioLoopbackPrivate();
private:
    AudioRendererOptions ConfigAudioRendererOptions();
    AudioCapturerOptions ConfigAudioCapturerOptions();

    bool CreateAudioLoopback();
    bool DestroyAudioLoopback();
    bool IsAudioLoopbackSupported();
    bool CheckDeviceSupport();
    int32_t OffKaraoke();
    int32_t SetKaraokeParameters();
    void updateState(AudioLoopbackStatus state);
    AppInfo appInfo_ = {};
    float volume_ = 0.5;
    std::shared_ptr<AudioRenderer> audioRenderer_;
    std::shared_ptr<AudioCapturer> audioCapturer_;
    AudioLoopbackStatus state_ = AVAILABLE_IDLE;
    AudioLoopbackMode mode_;

};
}  // namespace AudioStandard
}  // namespace OHOS
#endif // AUDIO_LOOPBACK_PRIVATE_H
