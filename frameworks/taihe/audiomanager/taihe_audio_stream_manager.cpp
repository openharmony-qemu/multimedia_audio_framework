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
#ifndef LOG_TAG
#define LOG_TAG "AudioStreamManagerImpl"
#endif

#include "taihe_audio_stream_manager.h"
#include "taihe_audio_error.h"
#include "taihe_param_utils.h"
#include "taihe_audio_enum.h"

namespace ANI::Audio {
AudioStreamManagerImpl::AudioStreamManagerImpl() : audioStreamMngr_(nullptr) {}

AudioStreamManagerImpl::AudioStreamManagerImpl(std::shared_ptr<AudioStreamManagerImpl> obj)
    : audioStreamMngr_(nullptr)
{
    if (obj != nullptr) {
        audioStreamMngr_ = obj->audioStreamMngr_;
        cachedClientId_ = obj->cachedClientId_;
    }
}

AudioStreamManagerImpl::~AudioStreamManagerImpl() = default;

AudioStreamManager AudioStreamManagerImpl::CreateStreamManagerWrapper()
{
    std::shared_ptr<AudioStreamManagerImpl> audioStreamMgrImpl = std::make_shared<AudioStreamManagerImpl>();
    if (audioStreamMgrImpl != nullptr) {
        audioStreamMgrImpl->audioStreamMngr_ = OHOS::AudioStandard::AudioStreamManager::GetInstance();
        audioStreamMgrImpl->cachedClientId_ = getpid();
        return make_holder<AudioStreamManagerImpl, AudioStreamManager>(audioStreamMgrImpl);
    }
    TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "audioStreamMgrImpl is nullptr");
    return make_holder<AudioStreamManagerImpl, AudioStreamManager>(nullptr);
}

array<AudioRendererChangeInfo> AudioStreamManagerImpl::GetCurrentAudioRendererInfoArraySync()
{
    std::vector<AudioRendererChangeInfo> emptyResult;
    if (audioStreamMngr_ == nullptr) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "audioStreamMngr_ is nullptr");
        return array<AudioRendererChangeInfo>(emptyResult);
    }
    std::vector<std::shared_ptr<OHOS::AudioStandard::AudioRendererChangeInfo>> audioRendererChangeInfos;
    if (audioStreamMngr_->GetCurrentRendererChangeInfos(audioRendererChangeInfos) != AUDIO_OK) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "GetCurrentRendererChangeInfos failure!");
        return array<AudioRendererChangeInfo>(emptyResult);
    }
    return TaiheParamUtils::SetRendererChangeInfos(audioRendererChangeInfos);
}

array<AudioCapturerChangeInfo> AudioStreamManagerImpl::GetCurrentAudioCapturerInfoArraySync()
{
    std::vector<AudioCapturerChangeInfo> emptyResult;
    if (audioStreamMngr_ == nullptr) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "audioStreamMngr_ is nullptr");
        return array<AudioCapturerChangeInfo>(emptyResult);
    }
    std::vector<std::shared_ptr<OHOS::AudioStandard::AudioCapturerChangeInfo>> audioCapturerChangeInfos;
    if (audioStreamMngr_->GetCurrentCapturerChangeInfos(audioCapturerChangeInfos) != AUDIO_OK) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "GetCurrentCapturerChangeInfos failure!");
        return array<AudioCapturerChangeInfo>(emptyResult);
    }
    return TaiheParamUtils::SetCapturerChangeInfos(audioCapturerChangeInfos);
}

array<AudioEffectMode> AudioStreamManagerImpl::GetAudioEffectInfoArraySync(StreamUsage usage)
{
    std::vector<AudioEffectMode> emptyResult;
    int32_t streamUsage = usage.get_value();
    if (!TaiheAudioEnum::IsLegalInputArgumentStreamUsage(streamUsage)) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_INVALID_PARAM,
            "parameter verification failed: The param of usage must be enum StreamUsage");
        AUDIO_ERR_LOG("get streamUsage failed");
        return array<AudioEffectMode>(emptyResult);
    }
    if (audioStreamMngr_ == nullptr) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "audioStreamMngr_ is nullptr");
        return array<AudioEffectMode>(emptyResult);
    }
    OHOS::AudioStandard::AudioSceneEffectInfo audioSceneEffectInfo;
    int32_t ret = audioStreamMngr_->GetEffectInfoArray(audioSceneEffectInfo,
        static_cast<OHOS::AudioStandard::StreamUsage>(streamUsage));
    if (ret != AUDIO_OK) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "GetEffectInfoArray failure!");
        return array<AudioEffectMode>(emptyResult);
    }
    return TaiheParamUtils::SetEffectInfo(audioSceneEffectInfo);
}

bool AudioStreamManagerImpl::IsActiveSync(AudioVolumeType volumeType)
{
    int32_t volType = volumeType.get_value();
    if (!TaiheAudioEnum::IsLegalInputArgumentVolType(volType)) {
        AUDIO_ERR_LOG("Invalid volumeType: %{public}d", volType);
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERROR_INVALID_PARAM,
            "parameter verification failed: The param of volumeType must be enum AudioVolumeType");
        return false;
    }
    if (audioStreamMngr_ == nullptr) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "audioStreamMngr_ is nullptr");
        return false;
    }
    return audioStreamMngr_->IsStreamActive(TaiheAudioEnum::GetNativeAudioVolumeType(volType));
}

bool AudioStreamManagerImpl::IsStreamActive(StreamUsage streamUsage)
{
    int32_t usage = streamUsage.get_value();
    if (!TaiheAudioEnum::IsLegalInputArgumentStreamUsage(usage)) {
        AUDIO_ERR_LOG("get streamUsage failed: %{public}d", usage);
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERROR_INVALID_PARAM,
            "parameter verification failed: The param of streamUsage must be enum StreamUsage");
        return false;
    }
    if (audioStreamMngr_ == nullptr) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "audioStreamMngr_ is nullptr");
        return false;
    }
    return audioStreamMngr_->IsStreamActiveByStreamUsage(TaiheAudioEnum::GetNativeStreamUsage(usage));
}

bool AudioStreamManagerImpl::IsAcousticEchoCancelerSupported(SourceType sourceType)
{
    int32_t type = sourceType.get_value();
    if (!TaiheAudioEnum::IsValidSourceType(type)) {
        AUDIO_ERR_LOG("get sourceType failed: %{public}d", type);
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERROR_INVALID_PARAM,
            "parameter verification failed: The param of sourceType must be enum SourceType");
        return false;
    }
    if (audioStreamMngr_ == nullptr) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "audioStreamMngr_ is nullptr");
        return false;
    }
    return audioStreamMngr_->IsAcousticEchoCancelerSupported(static_cast<OHOS::AudioStandard::SourceType>(type));
}

bool AudioStreamManagerImpl::IsRecordingAvailable(AudioCapturerInfo capturerInfo)
{
    OHOS::AudioStandard::AudioCapturerInfo innerCapturerInfo;
    int32_t ret = TaiheParamUtils::GetAudioCapturerInfo(innerCapturerInfo, capturerInfo);
    if (ret != AUDIO_OK || innerCapturerInfo.sourceType == OHOS::AudioStandard::SourceType::SOURCE_TYPE_INVALID) {
        AUDIO_ERR_LOG("get audioCapturerChangeInfo failed");
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_INVALID_PARAM, "parameter verification failed");
        return false;
    }
    if (audioStreamMngr_ == nullptr) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "audioStreamMngr_ is nullptr");
        return false;
    }
    return audioStreamMngr_->IsCapturerFocusAvailable(innerCapturerInfo);
}

bool AudioStreamManagerImpl::IsAudioLoopbackSupported(AudioLoopbackMode mode)
{
    int32_t loopbackMode = mode.get_value();
    if (!TaiheAudioEnum::IsLegalInputArgumentAudioLoopbackMode(loopbackMode)) {
        AUDIO_ERR_LOG("get loopbackMode failed: %{public}d", loopbackMode);
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERROR_INVALID_PARAM,
            "parameter verification failed: The param of loopbackMode must be enum AudioLoopbackMode");
        return false;
    }
    if (audioStreamMngr_ == nullptr) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "audioStreamMngr_ is nullptr");
        return false;
    }
    return audioStreamMngr_->IsAudioLoopbackSupported(
        static_cast<OHOS::AudioStandard::AudioLoopbackMode>(loopbackMode));
}

void AudioStreamManagerImpl::OnAudioRendererChange(callback_view<void(array_view<AudioRendererChangeInfo>)> callback)
{
    auto cacheCallback = TaiheParamUtils::TypeCallback(callback);
    RegisterRendererStateChangeCallback(cacheCallback, RENDERERCHANGE_CALLBACK_NAME, this);
}

void AudioStreamManagerImpl::OnAudioCapturerChange(callback_view<void(array_view<AudioCapturerChangeInfo>)> callback)
{
    auto cacheCallback = TaiheParamUtils::TypeCallback(callback);
    RegisterCapturerStateChangeCallback(cacheCallback, CAPTURERCHANGE_CALLBACK_NAME, this);
}

void AudioStreamManagerImpl::OffAudioRendererChange(
    optional_view<callback<void(array_view<AudioRendererChangeInfo>)>> callback)
{
    std::shared_ptr<uintptr_t> cacheCallback = nullptr;
    if (callback.has_value()) {
        cacheCallback = TaiheParamUtils::TypeCallback(callback.value());
    }
    UnregisterRendererChangeCallback(cacheCallback, this);
}

void AudioStreamManagerImpl::OffAudioCapturerChange(optional_view<callback<void(array_view<AudioCapturerChangeInfo>)>>
    callback)
{
    CHECK_AND_RETURN_RET_LOG(audioStreamMngr_ != nullptr, TaiheAudioError::ThrowErrorAndReturn(
        TAIHE_ERROR_INVALID_PARAM), "audioStreamMngr_ is nullptr");
    std::shared_ptr<uintptr_t> cacheCallback = nullptr;
    if (callback.has_value()) {
        cacheCallback = TaiheParamUtils::TypeCallback(callback.value());
    }
    UnregisterCapturerChangeCallback(cacheCallback, this);
}

void AudioStreamManagerImpl::RegisterRendererStateChangeCallback(std::shared_ptr<uintptr_t> &callback,
    const std::string &cbName, AudioStreamManagerImpl *taiheStreamManager)
{
    CHECK_AND_RETURN_LOG((taiheStreamManager != nullptr) &&
        (taiheStreamManager->audioStreamMngr_ != nullptr), "Failed to retrieve stream mgr taihe instance.");
    std::lock_guard<std::mutex> lock(taiheStreamManager->mutex_);
    if (!taiheStreamManager->rendererStateCallback_) {
        taiheStreamManager->rendererStateCallback_ = std::make_shared<TaiheAudioRendererStateCallback>();
        CHECK_AND_RETURN_RET_LOG(taiheStreamManager->rendererStateCallback_ != nullptr,
            TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_NO_MEMORY),
            "AudioStreamManagerImpl: Memory Allocation Failed !!");

        int32_t ret = taiheStreamManager->audioStreamMngr_->RegisterAudioRendererEventListener(
            taiheStreamManager->cachedClientId_, taiheStreamManager->rendererStateCallback_);
        CHECK_AND_RETURN_RET_LOG(ret == OHOS::AudioStandard::SUCCESS,
            TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM),
            "AudioStreamManagerImpl: Registering of Renderer State Change Callback Failed");
    }

    std::shared_ptr<TaiheAudioRendererStateCallback> cb =
        std::static_pointer_cast<TaiheAudioRendererStateCallback>(taiheStreamManager->rendererStateCallback_);
    CHECK_AND_RETURN_LOG(cb != nullptr, "cb is nullptr");
    cb->SaveCallbackReference(cbName, callback);
    AUDIO_INFO_LOG("RegisterRendererStateChangeCallback is successful");
}

void AudioStreamManagerImpl::RegisterCapturerStateChangeCallback(std::shared_ptr<uintptr_t> &callback,
    const std::string &cbName, AudioStreamManagerImpl *taiheStreamManager)
{
    CHECK_AND_RETURN_LOG((taiheStreamManager != nullptr) &&
        (taiheStreamManager->audioStreamMngr_ != nullptr), "Failed to retrieve stream mgr taihe instance.");
    std::lock_guard<std::mutex> lock(taiheStreamManager->mutex_);
    if (!taiheStreamManager->capturerStateCallback_) {
        taiheStreamManager->capturerStateCallback_ = std::make_shared<TaiheAudioCapturerStateCallback>();
        CHECK_AND_RETURN_RET_LOG(taiheStreamManager->capturerStateCallback_ != nullptr,
            TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_NO_MEMORY),
            "AudioStreamManagerImpl: Memory Allocation Failed !!");

        int32_t ret = taiheStreamManager->audioStreamMngr_->RegisterAudioCapturerEventListener(
            taiheStreamManager->cachedClientId_, taiheStreamManager->capturerStateCallback_);
        CHECK_AND_RETURN_RET_LOG(ret == OHOS::AudioStandard::SUCCESS,
            TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM),
            "AudioStreamManagerImpl: Registering of Capturer State Change Callback Failed");
    }

    std::shared_ptr<TaiheAudioCapturerStateCallback> cb =
        std::static_pointer_cast<TaiheAudioCapturerStateCallback>(taiheStreamManager->capturerStateCallback_);
    CHECK_AND_RETURN_LOG(cb != nullptr, "cb is nullptr");
    cb->SaveCallbackReference(cbName, callback);
    AUDIO_INFO_LOG("RegisterRendererStateChangeCallback is successful");
}

void AudioStreamManagerImpl::UnregisterRendererChangeCallback(std::shared_ptr<uintptr_t> &callback,
    AudioStreamManagerImpl *taiheStreamManager)
{
    CHECK_AND_RETURN_LOG((taiheStreamManager != nullptr) &&
        (taiheStreamManager->audioStreamMngr_ != nullptr), "Failed to retrieve stream mgr taihe instance.");
    std::lock_guard<std::mutex> lock(taiheStreamManager->mutex_);
    CHECK_AND_RETURN_LOG(taiheStreamManager->rendererStateCallback_ != nullptr,
        "rendererStateCallback_ is nullptr");
    std::shared_ptr<TaiheAudioRendererStateCallback> cb =
        std::static_pointer_cast<TaiheAudioRendererStateCallback>(taiheStreamManager->rendererStateCallback_);
    CHECK_AND_RETURN_LOG(cb != nullptr, "cb is nullptr");
    if (callback) {
        CHECK_AND_RETURN_LOG(cb->IsSameCallback(callback),
            "The callback need to be unregistered is not the same as the registered callback");
    }
    int32_t ret = taiheStreamManager->audioStreamMngr_->
        UnregisterAudioRendererEventListener(taiheStreamManager->cachedClientId_);
    CHECK_AND_RETURN_LOG(ret == OHOS::AudioStandard::SUCCESS, "Unregister renderer state change callback failed");
    cb->RemoveCallbackReference(callback);
    taiheStreamManager->rendererStateCallback_.reset();
}

void AudioStreamManagerImpl::UnregisterCapturerChangeCallback(std::shared_ptr<uintptr_t> &callback,
    AudioStreamManagerImpl *taiheStreamManager)
{
    CHECK_AND_RETURN_LOG((taiheStreamManager != nullptr) &&
        (taiheStreamManager->audioStreamMngr_ != nullptr), "Failed to retrieve stream mgr taihe instance.");
    std::lock_guard<std::mutex> lock(taiheStreamManager->mutex_);
    CHECK_AND_RETURN_LOG(taiheStreamManager->capturerStateCallback_ != nullptr,
        "capturerStateCallback_ is nullptr");
    std::shared_ptr<TaiheAudioCapturerStateCallback> cb =
        std::static_pointer_cast<TaiheAudioCapturerStateCallback>(taiheStreamManager->capturerStateCallback_);
    CHECK_AND_RETURN_LOG(cb != nullptr, "cb is nullptr");
    if (callback) {
        CHECK_AND_RETURN_LOG(cb->IsSameCallback(callback),
            "The callback need to be unregistered is not the same as the registered callback");
    }
    int32_t ret = taiheStreamManager->audioStreamMngr_->
        UnregisterAudioCapturerEventListener(taiheStreamManager->cachedClientId_);
    CHECK_AND_RETURN_LOG(ret == OHOS::AudioStandard::SUCCESS, "Unregister capturer state change callback failed");
    cb->RemoveCallbackReference(callback);
    taiheStreamManager->capturerStateCallback_.reset();
}
} // namespace ANI::Audio
