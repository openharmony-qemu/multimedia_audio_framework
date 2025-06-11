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
#include "audio_stream_info.h"
#include "audio_policy_manager.h"
#include "securec.h"
#include "access_token.h"
#include "accesstoken_kit.h"
#include "ipc_skeleton.h"
namespace OHOS {
namespace AudioStandard {
namespace {
    const int32_t VALUE_HUNDRED = 100;
}
std::shared_ptr<AudioLoopback> AudioLoopback::CreateAudioLoopback(AudioLoopbackMode mode, const AppInfo &appInfo)
{
    Security::AccessToken::AccessTokenID tokenId = appInfo.appTokenId;
    if (tokenId == Security::AccessToken::INVALID_TOKENID) {
        tokenId = IPCSkeleton::GetCallingTokenID();
    }

    int res = Security::AccessToken::AccessTokenKit::VerifyAccessToken(tokenId, MICROPHONE_PERMISSION);
    CHECK_AND_RETURN_RET_LOG(res == Security::AccessToken::PermissionState::PERMISSION_GRANTED,
        nullptr, "Permission denied [tid:%{public}d]", tokenId);
    return std::make_shared<AudioLoopbackPrivate>(mode, appInfo);
}

AudioLoopbackPrivate::AudioLoopbackPrivate(AudioLoopbackMode mode, const AppInfo &appInfo)
{
    appInfo_ = appInfo;
    if (appInfo_.appPid == 0) {
        appInfo_.appPid = getpid();
    }

    if (appInfo_.appUid < 0) {
        appInfo_.appUid = static_cast<int32_t>(getuid());
    }
    mode_ = mode;
    karaokeParams_["Karaoke_enable"] = "disable";
    karaokeParams_["Karaoke_reverb_mode"] = "ktv";
    karaokeParams_["Karaoke_eq_mode"] = "full";
    karaokeParams_["Karaoke_volume"] = "50";
    rendererOptions_ = GenerateRendererConfig();
    capturerOptions_ = GenerateCapturerConfig();
    InitStatus();
}

AudioLoopback::~AudioLoopback() = default;

AudioLoopbackPrivate::~AudioLoopbackPrivate()
{
    AUDIO_INFO_LOG("~AudioLoopbackPrivate");
    if (currentStatus_ == AVAILABLE_RUNNING) {
        DestroyAudioLoopback();
    }
}

bool AudioLoopbackPrivate::Enable(bool enable)
{
    AUDIO_INFO_LOG("Enable %{public}d, currentStatus_ %{public}d", enable, currentStatus_);
    if (enable) {
        CHECK_AND_RETURN_RET_LOG(currentStatus_ != AVAILABLE_RUNNING, false, "AudioLoopback already running");
        InitStatus();
        bool ret = IsAudioLoopbackSupported() && CheckDeviceSupport() && CreateAudioLoopback();
        if (!ret) {
            AUDIO_ERR_LOG("Create AudioLoopback failed");
            DestroyAudioLoopback();
        }
        isStarted_ = true;
        UpdateStatus();
        CHECK_AND_RETURN_RET_LOG(currentStatus_ == AVAILABLE_RUNNING, false, "AudioLoopback Enable failed");
    } else {
        CHECK_AND_RETURN_RET_LOG(currentStatus_ == AVAILABLE_RUNNING, true, "AudioLoopback not Running");
        DestroyAudioLoopback();
    }
    return true;
}

void AudioLoopbackPrivate::InitStatus()
{
    isStarted_ = false;
    currentStatus_ = AVAILABLE_IDLE;

    rendererState_ = RENDERER_INVALID;
    isRendererUsb_ = false;
    rendererFastStatus_ = FASTSTATUS_NORMAL;

    capturerState_ = CAPTURER_INVALID;
    isCapturerUsb_ = false;
    capturerFastStatus_ = FASTSTATUS_NORMAL;
}

AudioLoopbackStatus AudioLoopbackPrivate::GetStatus()
{
    if (currentStatus_ == AVAILABLE_RUNNING) {
        return currentStatus_;
    }
    bool ret = CheckDeviceSupport();
    if (!ret) {
        return UNAVAILABLE_DEVICE;
    }
    if (currentStatus_ == UNAVAILABLE_SCENE) {
        currentStatus_ = AVAILABLE_IDLE;
        return UNAVAILABLE_SCENE;
    }
    currentStatus_ = AVAILABLE_IDLE;
    return currentStatus_;
}

int32_t AudioLoopbackPrivate::SetVolume(float volume)
{
    if (volume < 0.0 || volume > 1.0) {
        AUDIO_ERR_LOG("SetVolume with invalid volume %{public}f", volume);
        return ERR_INVALID_PARAM;
    }
    karaokeParams_["Karaoke_volume"] = std::to_string(static_cast<int>(volume * VALUE_HUNDRED));
    if (currentStatus_ == AVAILABLE_RUNNING) {
        std::string parameters = "Karaoke_volume=" + karaokeParams_["Karaoke_volume"];
        CHECK_AND_RETURN_RET_LOG(AudioPolicyManager::GetInstance().SetKaraokeParameters(parameters), ERROR,
            "SetVolume failed");
    }
    return SUCCESS;
}

int32_t AudioLoopbackPrivate::SetAudioLoopbackCallback(const std::shared_ptr<AudioLoopbackCallback> &callback)
{
    std::lock_guard<std::mutex> lock(mutex_);
    statusCallback_ = callback;
    return SUCCESS;
}

int32_t AudioLoopbackPrivate::RemoveAudioLoopbackCallback()
{
    std::lock_guard<std::mutex> lock(mutex_);
    statusCallback_ = nullptr;
    return SUCCESS;
}

bool AudioLoopbackPrivate::CreateAudioLoopback()
{
    audioRenderer_ = AudioRenderer::CreateRenderer(rendererOptions_, appInfo_);
    if (audioRenderer_ == nullptr || !audioRenderer_->IsFastRenderer()) {
        AUDIO_ERR_LOG("CreateRenderer failed");
        return false;
    }
    audioRenderer_->SetRendererWriteCallback(shared_from_this());
    rendererFastStatus_ = FASTSTATUS_FAST;
    audioCapturer_ = AudioCapturer::CreateCapturer(capturerOptions_, appInfo_);
    if(audioCapturer_ == nullptr) {
        AUDIO_ERR_LOG("CreateCapturer failed");
        return false;
    }
    AudioCapturerInfo capturerInfo;
    audioCapturer_->GetCapturerInfo(capturerInfo);
    if (capturerInfo.capturerFlags != STREAM_FLAG_FAST) {
        AUDIO_ERR_LOG("CreateCapturer failed");
        return false;
    }
    audioCapturer_->SetCapturerReadCallback(shared_from_this());
    InitializeCallbacks();
    capturerFastStatus_ = FASTSTATUS_FAST;
    bool ret = audioRenderer_->Start();
    if (!ret) {
        AUDIO_ERR_LOG("audioRenderer Start failed");
        return false;
    }
    rendererState_ = RENDERER_RUNNING;
    ret = audioCapturer_->Start();
    if (!ret) {
        AUDIO_ERR_LOG("audioCapturer Start failed");
        return false;
    }
    capturerState_ = CAPTURER_RUNNING;
    return true;
}

void AudioLoopbackPrivate::DisableLoopback()
 {
    if (currentStatus_ == AVAILABLE_RUNNING) {
        karaokeParams_["Karaoke_enable"] = "disable";
        std::string parameters = "Karaoke_enable=" + karaokeParams_["Karaoke_enable"];
        CHECK_AND_RETURN_LOG(AudioPolicyManager::GetInstance().SetKaraokeParameters(parameters),
            "DisableLoopback failed");
    }
 }

void AudioLoopbackPrivate::DestroyAudioLoopback()
{
    isStarted_ = false;
    bool ret = true;
    DisableLoopback();
    currentStatus_ = AVAILABLE_IDLE;
    if (audioCapturer_) {
        ret = audioCapturer_->Stop();
        if (!ret) {
            AUDIO_ERR_LOG("audioCapturer Stop failed");
        }
        ret = audioCapturer_->Release();
        if (!ret) {
            AUDIO_ERR_LOG("audioCapturer Release failed");
        }
        AUDIO_INFO_LOG("audioCapturer Release success");
        audioCapturer_ = nullptr;
    } else {
         AUDIO_ERR_LOG("audioCapturer is nullptr");
    }
    if (audioRenderer_) {
        ret = audioRenderer_->Stop();
        if (!ret) {
            AUDIO_ERR_LOG("audioRenderer Stop failed");
        }
        ret = audioRenderer_->Release();
        if (!ret) {
            AUDIO_ERR_LOG("audioRenderer Release failed");
        }
        AUDIO_INFO_LOG("audioRenderer Release success");
        audioRenderer_ = nullptr;
    } else {
         AUDIO_ERR_LOG("audioRenderer is nullptr");
    }
}

AudioRendererOptions AudioLoopbackPrivate::GenerateRendererConfig()
{
    AudioRendererOptions rendererOptions = {};
    rendererOptions.streamInfo.samplingRate = AudioSamplingRate::SAMPLE_RATE_48000;
    rendererOptions.streamInfo.encoding = AudioEncodingType::ENCODING_PCM;
    rendererOptions.streamInfo.format = AudioSampleFormat::SAMPLE_S16LE;
    rendererOptions.streamInfo.channels = AudioChannel::STEREO;
    rendererOptions.rendererInfo.contentType = ContentType::CONTENT_TYPE_MUSIC;
    rendererOptions.rendererInfo.streamUsage = StreamUsage::STREAM_USAGE_MUSIC;
    rendererOptions.rendererInfo.rendererFlags = STREAM_FLAG_FAST;
    rendererOptions.rendererInfo.isLoopback = true;
    rendererOptions.rendererInfo.loopbackMode = mode_;
    return rendererOptions;
}

AudioCapturerOptions AudioLoopbackPrivate::GenerateCapturerConfig()
{
    AudioCapturerOptions capturerOptions = {};
    capturerOptions.streamInfo.samplingRate = AudioSamplingRate::SAMPLE_RATE_48000;
    capturerOptions.streamInfo.encoding = AudioEncodingType::ENCODING_PCM;
    capturerOptions.streamInfo.format = AudioSampleFormat::SAMPLE_S16LE;
    capturerOptions.streamInfo.channels = AudioChannel::STEREO;
    capturerOptions.capturerInfo.sourceType = SourceType::SOURCE_TYPE_MIC;
    capturerOptions.capturerInfo.capturerFlags = STREAM_FLAG_FAST;
    capturerOptions.capturerInfo.isLoopback = true;
    capturerOptions.capturerInfo.loopbackMode = mode_;
    return capturerOptions;
}

bool AudioLoopbackPrivate::IsAudioLoopbackSupported()
{
    return true;
}

bool AudioLoopbackPrivate::CheckDeviceSupport()
{
    isRendererUsb_ = AudioPolicyManager::GetInstance().GetActiveOutputDevice() == DEVICE_TYPE_USB_HEADSET;
    isCapturerUsb_ = AudioPolicyManager::GetInstance().GetActiveInputDevice() == DEVICE_TYPE_USB_HEADSET;
    return isRendererUsb_ && isCapturerUsb_;
}

bool AudioLoopbackPrivate::SetKaraokeParameters()
{
    std::string parameters = "";
    for (auto &param : karaokeParams_) {
        parameters += param.first + "=" + param.second + ";";
    }
    CHECK_AND_RETURN_RET_LOG(AudioPolicyManager::GetInstance().SetKaraokeParameters(parameters), false,
        "SetKaraokeParameters failed");
    return true;
}

void AudioLoopbackPrivate::OnReadData(size_t length)
{
    CHECK_AND_RETURN_LOG(audioCapturer_ != nullptr, "audioCapturer is nullptr");
    BufferDesc bufDesc;
    int32_t ret = audioCapturer_->GetBufferDesc(bufDesc);
    CHECK_AND_RETURN_LOG(ret == SUCCESS, "get bufDesc failed, bufLength=%{public}zu, dataLength=%{public}zu",
        bufDesc.bufLength, bufDesc.dataLength);
}

void AudioLoopbackPrivate::OnWriteData(size_t length)
{
    CHECK_AND_RETURN_LOG(audioRenderer_ != nullptr, "audioRenderer is nullptr");
    BufferDesc bufDesc;
    audioRenderer_->GetBufferDesc(bufDesc);
    memset_s((void*)bufDesc.buffer, bufDesc.bufLength, 0, bufDesc.bufLength);
    audioRenderer_->Enqueue(bufDesc);
}

AudioLoopbackPrivate::RendererCallbackImpl::RendererCallbackImpl(AudioLoopbackPrivate &parent)
    : parent_(parent) {}

void AudioLoopbackPrivate::RendererCallbackImpl::OnStateChange(
    const RendererState state, const StateChangeCmdType __attribute__((unused)) cmdType)
{
    parent_.rendererState_ = state;
    parent_.UpdateStatus();
}

void AudioLoopbackPrivate::RendererCallbackImpl::OnOutputDeviceChange(const AudioDeviceDescriptor &deviceInfo,
    const AudioStreamDeviceChangeReason __attribute__((unused)) reason)
{
    parent_.isRendererUsb_ = (deviceInfo.deviceType_ == DEVICE_TYPE_USB_HEADSET);
    parent_.UpdateStatus();
}

void AudioLoopbackPrivate::RendererCallbackImpl::OnFastStatusChange(FastStatus status)
{
    parent_.rendererFastStatus_ = status;
    parent_.UpdateStatus();
}

AudioLoopbackPrivate::CapturerCallbackImpl::CapturerCallbackImpl(AudioLoopbackPrivate &parent)
    : parent_(parent) {}

void AudioLoopbackPrivate::CapturerCallbackImpl::OnStateChange(const CapturerState state)
{
    parent_.capturerState_ = state;
    parent_.UpdateStatus();
}

void AudioLoopbackPrivate::CapturerCallbackImpl::OnStateChange(const AudioDeviceDescriptor &deviceInfo)
{
    parent_.isCapturerUsb_ = (deviceInfo.deviceType_ == DEVICE_TYPE_USB_HEADSET);
    parent_.UpdateStatus();
}

void AudioLoopbackPrivate::CapturerCallbackImpl::OnFastStatusChange(FastStatus status)
{
    parent_.capturerFastStatus_ = status;
    parent_.UpdateStatus();
}

void AudioLoopbackPrivate::InitializeCallbacks()
{
    auto rendererCallback = std::make_shared<RendererCallbackImpl>(*this);
    audioRenderer_->SetRendererCallback(rendererCallback);
    audioRenderer_->RegisterOutputDeviceChangeWithInfoCallback(rendererCallback);
    audioRenderer_->SetFastStatusChangeCallback(rendererCallback);

    auto capturerCallback = std::make_shared<CapturerCallbackImpl>(*this);
    audioCapturer_->SetCapturerCallback(capturerCallback);
    audioCapturer_->SetAudioCapturerDeviceChangeCallback(capturerCallback);
    audioCapturer_->SetFastStatusChangeCallback(capturerCallback);
}

void AudioLoopbackPrivate::UpdateStatus()
{
    CHECK_AND_RETURN(isStarted_);
    AudioLoopbackStatus oldStatus = currentStatus_;
    AudioLoopbackStatus newStatus = currentStatus_;
    const bool isDeviceValid = isRendererUsb_ && isCapturerUsb_;

    if (!isDeviceValid) {
        newStatus = UNAVAILABLE_DEVICE;
    } else {
        const bool isStateRunning = (rendererState_ == RENDERER_RUNNING) && (capturerState_ == CAPTURER_RUNNING);
        const bool isFastValid = (rendererFastStatus_ == FASTSTATUS_FAST) && (capturerFastStatus_ == FASTSTATUS_FAST);
        newStatus = (isStateRunning && isFastValid) ? AVAILABLE_RUNNING : UNAVAILABLE_SCENE;
    }

    if (newStatus == AVAILABLE_RUNNING) {
        karaokeParams_["Karaoke_enable"] = "enable";
        currentStatus_ = AVAILABLE_RUNNING;
        newStatus = SetKaraokeParameters() ? AVAILABLE_RUNNING : UNAVAILABLE_SCENE;
    }
    if (newStatus != oldStatus) {
        AUDIO_INFO_LOG("UpdateStatus: %{public}d -> %{public}d", oldStatus, newStatus);
        if (currentStatus_ == AVAILABLE_RUNNING && newStatus != AVAILABLE_RUNNING) {
            DestroyAudioLoopback();
        }
        currentStatus_ = newStatus;
        if (statusCallback_) {
            statusCallback_->OnStatusChange(currentStatus_);
        }
    }
}
}  // namespace AudioStandard
}  // namespace OHOS