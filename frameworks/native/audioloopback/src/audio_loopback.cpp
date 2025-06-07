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
#ifndef LOG_TAG
#define LOG_TAG "AudioLoopback"
#endif

#include "audio_loopback_private.h"
#include "audio_manager_log.h"
#include "audio_errors.h"

namespace OHOS {
namespace AudioStandard {

AudioLoopback::~AudioLoopback() = default;

AudioLoopbackPrivate::~AudioLoopbackPrivate()
{
    AUDIO_INFO_LOG("~AudioLoopbackPrivate");
}

std::shared_ptr<AudioLoopback> CreateAudioLoopback(AudioLoopbackMode mode, const AppInfo &appInfo)
{
    return std::make_shared<AudioLoopbackPrivate>(mode, appInfo);
}

AudioLoopbackPrivate::AudioLoopbackPrivate(AudioLoopbackMode mode, const AppInfo &appInfo)
{
    appInfo_ = appInfo;
    if (!(appInfo_.appPid)) {
        appInfo_.appPid = getpid();
    }

    if (appInfo_.appUid < 0) {
        appInfo_.appUid = static_cast<int32_t>(getuid());
    }
    mode_ = mode;
}

AudioRendererOptions AudioLoopbackPrivate::ConfigAudioRendererOptions()
{
    AudioRendererOptions rendererOptions = {
        {
            AudioSamplingRate::SAMPLE_RATE_48000,
            AudioEncodingType::ENCODING_PCM,
            AudioSampleFormat::SAMPLE_S16LE,
            AudioChannel::STEREO,
        },
        {
            ContentType::CONTENT_TYPE_MUSIC,
            StreamUsage::STREAM_USAGE_MUSIC,
            STREAM_FLAG_FAST,
        }
    };
    return rendererOptions;
}

AudioCapturerOptions AudioLoopbackPrivate::ConfigAudioCapturerOptions()
{
    AudioCapturerOptions capturerOptions = {
        {
            AudioSamplingRate::SAMPLE_RATE_48000,
            AudioEncodingType::ENCODING_PCM,
            AudioSampleFormat::SAMPLE_S16LE,
            AudioChannel::STEREO,
        },
        {
            SourceType::SOURCE_TYPE_MIC,
            STREAM_FLAG_FAST,
        }
    };
    return capturerOptions;
}

bool AudioLoopbackPrivate::IsAudioLoopbackSupported()
{
    return true;
}

bool AudioLoopbackPrivate::CheckDeviceSupport()
{
    return true;
}

int32_t AudioLoopbackPrivate::SetKaraokeParameters()
{
    std::string parameters = "Karaoke_enable=enable;Karaoke_reverb_mode=ktv;Karaoke_eq_mode=full;Karaoke_volume=" + std::to_string(static_cast<int>(volume_ * 50));
    return SUCCESS;
}

int32_t AudioLoopbackPrivate::OffKaraoke()
{
    std::string parameters = "Karaoke_enable=disable;";
    return SUCCESS;
}
void AudioLoopbackPrivate::updateState(AudioLoopbackStatus state)
{
    state_ = state;
}

bool AudioLoopbackPrivate::CreateAudioLoopback()
{
    CHECK_AND_RETURN_RET_LOG(IsAudioLoopbackSupported(), false, "AudioLoopback mode not supported");
    CHECK_AND_RETURN_RET_LOG(CheckDeviceSupport(), false, "Device not supported");
    AudioRendererOptions rendererOptions = ConfigAudioRendererOptions();
    audioRenderer_ = AudioRenderer::CreateRenderer(rendererOptions, appInfo_);
    if (audioRenderer_ == nullptr || !audioRenderer_->IsFastRenderer()) {
        AUDIO_ERR_LOG("CreateRenderer failed");
        return false;
    }

    AudioCapturerOptions capturerOptions = ConfigAudioCapturerOptions();
    audioCapturer_ = AudioCapturer::CreateCapturer(capturerOptions, appInfo_);
    AudioCapturerInfo capturerInfo;
    audioCapturer_->GetCapturerInfo(capturerInfo);
    if (audioCapturer_ == nullptr || capturerInfo.capturerFlags != STREAM_FLAG_FAST) {
        AUDIO_ERR_LOG("CreateCapturer failed");
        return false;
    }

    bool ret = audioRenderer_->Start();
    if(!ret) {
        AUDIO_ERR_LOG("audioRenderer Start failed");
        return false;
    }

    ret = audioCapturer_->Start();
    if(!ret) {
        AUDIO_ERR_LOG("audioCapturer Start failed");
        return false;
    }
    SetKaraokeParameters();
    updateState(AVAILABLE_RUNNING);
    return true;
}

bool AudioLoopbackPrivate::DestroyAudioLoopback()
{
    bool ret = audioCapturer_->Stop();
    if(!ret) {
        AUDIO_ERR_LOG("audioCapturer Stop failed");
        return false;
    }
    ret = audioRenderer_->Stop();
    if(!ret) {
        AUDIO_ERR_LOG("audioRenderer Stop failed");
        return false;
    }
    ret = audioCapturer_->Release();
    if(!ret) {
        AUDIO_ERR_LOG("audioCapturer Release failed");
        return false;
    }
    ret = audioRenderer_->Release();
    if(!ret) {
        AUDIO_ERR_LOG("audioRenderer Release failed");
        return false;
    }
    OffKaraoke();
    updateState(AVAILABLE_IDLE);
    return true;
}

bool AudioLoopbackPrivate::Enable(bool enable)
{
    return enable ? CreateAudioLoopback() : DestroyAudioLoopback();
}


AudioLoopbackStatus AudioLoopbackPrivate::GetStatus() const
{
    return state_;
}

void AudioLoopbackPrivate::SetVolume(float volume)
{
    volume_ = volume;
}

int32_t AudioLoopbackPrivate::SetAudioLoopbackCallback(const std::shared_ptr<AudioLoopbackCallback> &callback)
{
    return SUCCESS;
}
}  // namespace AudioStandard
}  // namespace OHOS