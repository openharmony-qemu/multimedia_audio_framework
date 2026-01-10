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
#include "audio_utils.h"
#include "app_bundle_manager.h"
#include "media_monitor_manager.h"
namespace OHOS {
namespace AudioStandard {
namespace {
    const int32_t VALUE_HUNDRED = 100;
    const std::map<AudioLoopbackReverbPreset, std::string> audioLoopbackReverbPresetMap = {
        {REVERB_PRESET_ORIGINAL, "disable"},
        {REVERB_PRESET_KTV, "ktv"},
        {REVERB_PRESET_THEATER, "theatre"},
        {REVERB_PRESET_CONCERT, "concert"},
    };
    const std::map<AudioLoopbackEqualizerPreset, std::string> audioLoopbackEqualizerPresetMap = {
        {EQUALIZER_PRESET_FLAT, "disable"},
        {EQUALIZER_PRESET_FULL, "full"},
        {EQUALIZER_PRESET_BRIGHT, "bright"},
    };
    static const std::unordered_set<DeviceType> g_deviceMaybeSupportedSet = {
        DEVICE_TYPE_USB_HEADSET,
        DEVICE_TYPE_USB_ARM_HEADSET,
    };
}

std::shared_ptr<AudioLoopback> AudioLoopback::CreateAudioLoopback(AudioLoopbackMode mode, const AppInfo &appInfo)
{
    Security::AccessToken::AccessTokenID tokenId = appInfo.appTokenId;
    tokenId = (tokenId == Security::AccessToken::INVALID_TOKENID) ? IPCSkeleton::GetCallingTokenID() : tokenId;
    int res = Security::AccessToken::AccessTokenKit::VerifyAccessToken(tokenId, MICROPHONE_PERMISSION);
    CHECK_AND_RETURN_RET_LOG(res == Security::AccessToken::PermissionState::PERMISSION_GRANTED, nullptr,
        "Permission denied [tid:%{public}d]", tokenId);
    if (mode == LOOPBACK_HARDWARE) {
        static std::shared_ptr<AudioLoopbackPrivate> instance = std::make_shared<AudioLoopbackPrivate>(mode, appInfo);
        return instance;
    }
    AUDIO_ERR_LOG("Invalid mode");
    return nullptr;
}

void AudioLoopbackPrivate::ReportAudioLoopbackException(const AudioLoopbackReportInfo &info)
{
    std::shared_ptr<Media::MediaMonitor::EventBean> bean = std::make_shared<Media::MediaMonitor::EventBean>(
        Media::MediaMonitor::AUDIO, Media::MediaMonitor::EventId::LOOPBACK_EXCEPTION,
        Media::MediaMonitor::EventType::FAULT_EVENT);
    bean->Add("APP_UID", info.appUid);
    bean->Add("APP_NAME", info.appName);
    bean->Add("RENDER_DEVICE_TYPE", static_cast<int32_t>(info.renderDeviceType));
    bean->Add("CAPTURE_DEVICE_TYPE", static_cast<int32_t>(info.captureDeviceType));
    bean->Add("ERROR_SCOPE", static_cast<int32_t>(info.errScope));
    bean->Add("ERROR_TYPE", static_cast<int32_t>(info.errType));
    Media::MediaMonitor::MediaMonitorManager::GetInstance().WriteLogMsg(bean);
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
    karaokeParams_["Karaoke_reverb_mode"] = "theatre";
    karaokeParams_["Karaoke_eq_mode"] = "full";
    karaokeParams_["Karaoke_volume"] = "50";
    rendererOptions_ = GenerateRendererConfig();
    capturerOptions_ = GenerateCapturerConfig();
    InitStatus();
    GetValidDevice();
}

AudioLoopback::~AudioLoopback() = default;

AudioLoopbackPrivate::~AudioLoopbackPrivate()
{
    AUDIO_INFO_LOG("~AudioLoopbackPrivate");
    std::lock_guard<std::mutex> lock(loopbackMutex_);
    std::unique_lock<std::mutex> stateLock(stateMutex_);
    CHECK_AND_RETURN_LOG(currentState_ == LOOPBACK_STATE_RUNNING, "AudioLoopback not Running");
    currentState_ = LOOPBACK_STATE_DESTROYING;
    stateLock.unlock();
    DestroyAudioLoopbackInner();
}

void AudioLoopbackPrivate::GetValidDevice()
{
    for (auto type : g_deviceMaybeSupportedSet) {
        CHECK_AND_CONTINUE(AudioPolicyManager::GetInstance().IsAudioLoopbackSupported(mode_, type));
        validDevice.insert(type);
    }
}

AudioLoopbackReportInfo AudioLoopbackPrivate::GetReportInfo(AudioLoopbackErrorScope scope,
    AudioLoopbackErrorType type)
{
    AudioLoopbackReportInfo info = {
        .appUid = appInfo_.appUid,
        .appName = AppBundleManager::GetSelfBundleName(appInfo_.appUid),
        .renderDeviceType = activeOutputDevice_,
        .captureDeviceType = activeInputDevice_,
        .errScope = scope,
        .errType = type,
    };
    return info;
}

bool AudioLoopbackPrivate::Enable(bool enable)
{
    Trace trace("AudioLoopbackPrivate::Enable");
    std::lock_guard<std::mutex> lock(loopbackMutex_);
    if (!IsAudioLoopbackSupported()) {
        HILOG_COMM_INFO("[Enable]AudioLoopback not support");
        ReportAudioLoopbackException(GetReportInfo(AudioLoopbackErrorScope::PLATFORM, ERROR_PLATFORM_NOT_SUPPORT));
        return false;
    }
    AUDIO_INFO_LOG("Enable %{public}d, currentState_ %{public}d", enable, currentState_);
    if (enable) {
        CHECK_AND_RETURN_RET_LOG(GetCurrentState() != LOOPBACK_STATE_RUNNING, true, "AudioLoopback already running");
        InitStatus();
        auto ret = CheckDeviceSupport();
        if (ret != DEFAULT_OK) {
            HILOG_COMM_INFO("[Enable]Device not support");
            ReportAudioLoopbackException(GetReportInfo(AudioLoopbackErrorScope::DEVICE, ret));
            return false;
        }
        CreateAudioLoopback();
        currentState_ = LOOPBACK_STATE_PREPARED;
        UpdateStatus();
        if (GetCurrentState() != LOOPBACK_STATE_RUNNING) {
            HILOG_COMM_INFO("[Enable]AudioLoopback Enable failed");
            return false;
        }
    } else {
        std::unique_lock<std::mutex> stateLock(stateMutex_);
        CHECK_AND_RETURN_RET_LOG(currentState_ == LOOPBACK_STATE_RUNNING, true, "AudioLoopback not Running");
        currentState_ = LOOPBACK_STATE_DESTROYING;
        stateLock.unlock();
        DestroyAudioLoopbackInner();
        currentState_ = LOOPBACK_STATE_IDLE;
    }
    return true;
}

void AudioLoopbackPrivate::InitStatus()
{
    currentState_ = LOOPBACK_STATE_IDLE;

    rendererState_ = RENDERER_INVALID;
    isRendererUsb_ = false;
    rendererFastStatus_ = FASTSTATUS_NORMAL;

    capturerState_ = CAPTURER_INVALID;
    isCapturerUsb_ = false;
    capturerFastStatus_ = FASTSTATUS_NORMAL;
}

AudioLoopbackStatus AudioLoopbackPrivate::GetStatus()
{
    Trace trace("AudioLoopbackPrivate::GetStatus");
    std::unique_lock<std::mutex> stateLock(stateMutex_);
    AudioLoopbackStatus status = StateToStatus(currentState_);
    if (status == LOOPBACK_UNAVAILABLE_SCENE || status == LOOPBACK_UNAVAILABLE_DEVICE) {
        currentState_ = LOOPBACK_STATE_IDLE;
    }
    return status;
}

AudioLoopbackStatus AudioLoopbackPrivate::StateToStatus(AudioLoopbackState state)
{
    if (state == LOOPBACK_STATE_RUNNING) {
        return LOOPBACK_AVAILABLE_RUNNING;
    }
    bool ret = CheckDeviceSupport() == DEFAULT_OK;
    if (!ret) {
        return LOOPBACK_UNAVAILABLE_DEVICE;
    }
    if (state == LOOPBACK_STATE_DESTROYED || state == LOOPBACK_STATE_DESTROYING) {
        return LOOPBACK_UNAVAILABLE_SCENE;
    }
    return LOOPBACK_AVAILABLE_IDLE;
}

int32_t AudioLoopbackPrivate::SetVolume(float volume)
{
    Trace trace("AudioLoopbackPrivate::SetVolume");
    if (volume < 0.0 || volume > 1.0) {
        HILOG_COMM_INFO("[SetVolume]with invalid volume");
        return ERR_INVALID_PARAM;
    }
    std::lock_guard<std::mutex> lock(loopbackMutex_);
    karaokeParams_["Karaoke_volume"] = std::to_string(static_cast<int>(volume * VALUE_HUNDRED));
    if (currentState_ == LOOPBACK_STATE_RUNNING) {
        std::string parameters = "Karaoke_volume=" + karaokeParams_["Karaoke_volume"];
        CHECK_AND_RETURN_RET_LOG(SetKaraokeParameters(parameters), ERROR, "SetVolume failed");
    }
    return SUCCESS;
}

bool AudioLoopbackPrivate::SetReverbPreset(AudioLoopbackReverbPreset preset)
{
    Trace trace("AudioLoopbackPrivate::SetReverbPreset");
    std::lock_guard<std::mutex> lock(loopbackMutex_);
    auto it = audioLoopbackReverbPresetMap.find(preset);
    CHECK_AND_RETURN_RET_LOG(it != audioLoopbackReverbPresetMap.end(), false, "preset invalid");
    currentReverbPreset_ = preset;
    karaokeParams_["Karaoke_reverb_mode"] = it->second;
    if (currentState_ == LOOPBACK_STATE_RUNNING) {
        std::string parameters = "Karaoke_reverb_mode=" + karaokeParams_["Karaoke_reverb_mode"];
        CHECK_AND_RETURN_RET_LOG(SetKaraokeParameters(parameters), false, "SetReverbPreset failed");
    }
    return true;
}

AudioLoopbackReverbPreset AudioLoopbackPrivate::GetReverbPreset()
{
    std::lock_guard<std::mutex> lock(loopbackMutex_);
    return currentReverbPreset_;
}

bool AudioLoopbackPrivate::SetEqualizerPreset(AudioLoopbackEqualizerPreset preset)
{
    Trace trace("AudioLoopbackPrivate::SetEqualizerPreset");
    std::lock_guard<std::mutex> lock(loopbackMutex_);
    auto it = audioLoopbackEqualizerPresetMap.find(preset);
    CHECK_AND_RETURN_RET_LOG(it != audioLoopbackEqualizerPresetMap.end(), false, "preset invalid");
    currentEqualizerPreset_ = preset;
    karaokeParams_["Karaoke_eq_mode"] = it->second;
    if (currentState_ == LOOPBACK_STATE_RUNNING) {
        std::string parameters = "Karaoke_eq_mode=" + karaokeParams_["Karaoke_eq_mode"];
        CHECK_AND_RETURN_RET_LOG(SetKaraokeParameters(parameters), false, "SetEqualizerPreset failed");
    }
    return true;
}

AudioLoopbackEqualizerPreset AudioLoopbackPrivate::GetEqualizerPreset()
{
    std::lock_guard<std::mutex> lock(loopbackMutex_);
    return currentEqualizerPreset_;
}

bool AudioLoopbackPrivate::SetKaraokeParameters(const std::string &parameters)
{
    bool ret = AudioPolicyManager::GetInstance().SetKaraokeParameters(activeOutputDevice_, parameters);
    if (!ret) {
        HILOG_COMM_INFO("[SetKaraokeParameters]failed");
    }
    return ret;
}

int32_t AudioLoopbackPrivate::SetAudioLoopbackCallback(const std::shared_ptr<AudioLoopbackCallback> &callback)
{
    std::unique_lock<std::mutex> stateLock(stateMutex_);
    statusCallback_ = callback;
    return SUCCESS;
}

int32_t AudioLoopbackPrivate::RemoveAudioLoopbackCallback()
{
    std::unique_lock<std::mutex> stateLock(stateMutex_);
    statusCallback_ = nullptr;
    return SUCCESS;
}

void AudioLoopbackPrivate::CreateAudioLoopback()
{
    Trace trace("AudioLoopbackPrivate::CreateAudioLoopback");
    audioRenderer_ = AudioRenderer::CreateRenderer(rendererOptions_, appInfo_);
    if (audioRenderer_ == nullptr) {
        HILOG_COMM_INFO("[CreateAudioLoopback]CreateRenderer failed");
        ReportAudioLoopbackException(GetReportInfo(AudioLoopbackErrorScope::STREAM, ERROR_RENDER_CREATE_FAIL));
        return;
    }
    if (!audioRenderer_->IsFastRenderer()) {
        HILOG_COMM_INFO("[CreateAudioLoopback]CreateFastRenderer failed");
        ReportAudioLoopbackException(GetReportInfo(AudioLoopbackErrorScope::STREAM,
            ERROR_RENDER_CREATE_FAST_STREAM_FAIL));
        return;
    }

    audioRenderer_->SetRendererWriteCallback(shared_from_this());
    rendererFastStatus_ = FASTSTATUS_FAST;
    audioCapturer_ = AudioCapturer::CreateCapturer(capturerOptions_, appInfo_);
    if (audioCapturer_ == nullptr) {
        HILOG_COMM_INFO("[CreateAudioLoopback]CreateCapturer failed");
        ReportAudioLoopbackException(GetReportInfo(AudioLoopbackErrorScope::STREAM, ERROR_CAPTURE_CREATE_FAIL));
        return;
    }

    AudioCapturerInfo capturerInfo;
    audioCapturer_->GetCapturerInfo(capturerInfo);
    if (capturerInfo.capturerFlags != STREAM_FLAG_FAST) {
        HILOG_COMM_INFO("[CreateAudioLoopback]CreateFastCapturer failed");
        ReportAudioLoopbackException(GetReportInfo(AudioLoopbackErrorScope::STREAM,
            ERROR_CAPTURE_CREATE_FAST_STREAM_FAIL));
        return;
    }

    audioCapturer_->SetCapturerReadCallback(shared_from_this());
    InitializeCallbacks();
    capturerFastStatus_ = FASTSTATUS_FAST;

    StartAudioLoopback();
}

void AudioLoopbackPrivate::StartAudioLoopback()
{
    if (!audioRenderer_->Start()) {
        HILOG_COMM_INFO("[StartAudioLoopback]audioRenderer Start failed");
        ReportAudioLoopbackException(GetReportInfo(AudioLoopbackErrorScope::STREAM, ERROR_RENDER_START_FAIL));
        return;
    }
    rendererState_ = RENDERER_RUNNING;
    if (!audioCapturer_->Start()) {
        HILOG_COMM_INFO("[StartAudioLoopback]audioCapturer Start failed");
        ReportAudioLoopbackException(GetReportInfo(AudioLoopbackErrorScope::STREAM, ERROR_CAPTURE_START_FAIL));
        return;
    }
    capturerState_ = CAPTURER_RUNNING;
}

void AudioLoopbackPrivate::DisableLoopback()
{
    if (karaokeParams_["Karaoke_enable"] == "enable") {
        karaokeParams_["Karaoke_enable"] = "disable";
        std::string parameters = "Karaoke_enable=" + karaokeParams_["Karaoke_enable"];
        CHECK_AND_RETURN_LOG(SetKaraokeParameters(parameters), "DisableLoopback failed");
    }
}

void AudioLoopbackPrivate::DestroyAudioLoopbackInner()
{
    Trace trace("AudioLoopbackPrivate::DestroyAudioLoopbackInner");
    DisableLoopback();
    if (audioCapturer_) {
        audioCapturer_->Stop();
        audioCapturer_->Release();
        audioCapturer_ = nullptr;
    } else {
        AUDIO_WARNING_LOG("audioCapturer is nullptr");
    }
    if (audioRenderer_) {
        audioRenderer_->Stop();
        audioRenderer_->Release();
        audioRenderer_ = nullptr;
    } else {
        AUDIO_WARNING_LOG("audioRenderer is nullptr");
    }
}

void AudioLoopbackPrivate::DestroyAudioLoopback()
{
    Trace trace("AudioLoopbackPrivate::DestroyAudioLoopback");
    std::lock_guard<std::mutex> lock(loopbackMutex_);
    DestroyAudioLoopbackInner();
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
    Trace trace("AudioLoopbackPrivate::IsAudioLoopbackSupported");
    return !validDevice.empty();
}

AudioLoopbackErrorType AudioLoopbackPrivate::CheckDeviceSupport()
{
    // hifi  usb // enable
    activeOutputDevice_ = AudioPolicyManager::GetInstance().GetActiveOutputDevice();
    activeInputDevice_ = AudioPolicyManager::GetInstance().GetActiveInputDevice();
    CHECK_AND_RETURN_RET_LOG(activeOutputDevice_ == activeInputDevice_, ERROR_DEVICE_MISMATCH,
        "outputdevice dismatch inputdevice, output = %{public}u, input = %{public}u",
        activeOutputDevice_, activeInputDevice_);
    isRendererUsb_ = validDevice.find(activeOutputDevice_) != validDevice.end();
    isCapturerUsb_ = validDevice.find(activeInputDevice_) != validDevice.end();
    CHECK_AND_RETURN_RET_LOG(isRendererUsb_ && isCapturerUsb_, ERROR_DEVICE_NOT_SUPPORT,
        "device not support, output = %{public}u, input = %{public}u",
        activeOutputDevice_, activeInputDevice_);
    return DEFAULT_OK;
}

bool AudioLoopbackPrivate::EnableLoopback()
{
    Trace trace("AudioLoopbackPrivate::EnableLoopback");
    karaokeParams_["Karaoke_enable"] = "enable";
    std::string parameters = "";
    for (auto &param : karaokeParams_) {
        parameters = param.first + "=" + param.second + ";";
        auto ret = SetKaraokeParameters(parameters);
        CHECK_AND_RETURN_RET_LOG(ret, false,
            "EnableLoopback failed");
    }
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
    parent_.isRendererUsb_ = parent_.activeOutputDevice_ == AudioPolicyManager::GetInstance().GetActiveOutputDevice();
    parent_.UpdateStatus();
}

void AudioLoopbackPrivate::RendererCallbackImpl::OnFastStatusChange(FastStatus status)
{
    parent_.rendererFastStatus_ = status;
    parent_.UpdateStatus();
    if (status != FASTSTATUS_FAST) {
        ReportAudioLoopbackException(parent_.GetReportInfo(AudioLoopbackErrorScope::STREAM,
            ERROR_RENDER_FALL_BACK_NORMAL));
    }
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
    parent_.isCapturerUsb_ = parent_.activeInputDevice_ == AudioPolicyManager::GetInstance().GetActiveInputDevice();
    parent_.UpdateStatus();
}

void AudioLoopbackPrivate::CapturerCallbackImpl::OnFastStatusChange(FastStatus status)
{
    parent_.capturerFastStatus_ = status;
    parent_.UpdateStatus();
    if (status != FASTSTATUS_FAST) {
        ReportAudioLoopbackException(parent_.GetReportInfo(AudioLoopbackErrorScope::STREAM,
            ERROR_CAPTURE_FALL_BACK_NORMAL));
    }
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

AudioLoopbackState AudioLoopbackPrivate::GetCurrentState()
{
    std::unique_lock<std::mutex> stateLock(stateMutex_);
    return currentState_;
}

void AudioLoopbackPrivate::UpdateStatus()
{
    Trace trace("AudioLoopbackPrivate::UpdateStatus");
    std::unique_lock<std::mutex> stateLock(stateMutex_);
    CHECK_AND_RETURN(currentState_ == LOOPBACK_STATE_RUNNING || currentState_ == LOOPBACK_STATE_PREPARED);
    AudioLoopbackState oldState = currentState_;
    AudioLoopbackState newState = currentState_;
    const bool isDeviceValid = isRendererUsb_.load() && isCapturerUsb_.load();
    const bool isStateRunning = (rendererState_.load() == RENDERER_RUNNING) &&
        (capturerState_.load() == CAPTURER_RUNNING);
    const bool isFastValid = (rendererFastStatus_.load() == FASTSTATUS_FAST) &&
        (capturerFastStatus_.load() == FASTSTATUS_FAST);
    newState = (isDeviceValid && isStateRunning && isFastValid) ? LOOPBACK_STATE_RUNNING : LOOPBACK_STATE_DESTROYED;

    if (newState == LOOPBACK_STATE_RUNNING) {
        newState = EnableLoopback() ? LOOPBACK_STATE_RUNNING : LOOPBACK_STATE_DESTROYED;
    }
    if (newState != oldState) {
        HILOG_COMM_WARN("[UpdateStatus]UpdateState: %{public}d -> %{public}d", oldState, newState);
        if (newState == LOOPBACK_STATE_DESTROYED) {
            currentState_ = LOOPBACK_STATE_DESTROYING;
            auto self = shared_from_this();
            std::thread([self] {
                CHECK_AND_RETURN(self != nullptr);
                self->DestroyAudioLoopback();
            }).detach();
        }
        currentState_ = newState;
        if (statusCallback_) {
            statusCallback_->OnStatusChange(StateToStatus(currentState_));
        }
    }
}
}  // namespace AudioStandard
}  // namespace OHOS