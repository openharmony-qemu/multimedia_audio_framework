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

using namespace ANI::Audio;

namespace ANI::Audio {
AudioStreamManagerImpl::AudioStreamManagerImpl() : audioStreamMngr_(nullptr) {}

AudioStreamManagerImpl::AudioStreamManagerImpl(std::unique_ptr<AudioStreamManagerImpl> obj)
{
    if (obj != nullptr) {
        audioStreamMngr_ = obj->audioStreamMngr_;
        cachedClientId_ = obj->cachedClientId_;
    }
}

AudioStreamManagerImpl::~AudioStreamManagerImpl() = default;

AudioStreamManager AudioStreamManagerImpl::CreateStreamManagerWrapper()
{
    std::unique_ptr<AudioStreamManagerImpl> audioStreamMgrImpl = std::make_unique<AudioStreamManagerImpl>();
    if (audioStreamMgrImpl != nullptr) {
        audioStreamMgrImpl->audioStreamMngr_ = OHOS::AudioStandard::AudioStreamManager::GetInstance();
        audioStreamMgrImpl->cachedClientId_ = getpid();
        return make_holder<AudioStreamManagerImpl, AudioStreamManager>(std::move(audioStreamMgrImpl));
    }
    return make_holder<AudioStreamManagerImpl, AudioStreamManager>(nullptr);
}

array<AudioRendererChangeInfo> AudioStreamManagerImpl::GetCurrentAudioRendererInfoArraySync()
{
    std::vector<AudioRendererChangeInfo> emptyResult;
    if (audioStreamMngr_ == nullptr) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_ILLEGAL_STATE, "audioStreamMngr_ is nullptr");
        return array<AudioRendererChangeInfo>(emptyResult);
    }
    std::vector<std::shared_ptr<OHOS::AudioStandard::AudioRendererChangeInfo>> audioRendererChangeInfos;
    if (audioStreamMngr_->GetCurrentRendererChangeInfos(audioRendererChangeInfos) != AUDIO_OK) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_ILLEGAL_STATE, "GetCurrentRendererChangeInfos failure!");
        return array<AudioRendererChangeInfo>(emptyResult);
    }
    return TaiheParamUtils::SetRendererChangeInfos(audioRendererChangeInfos);
}

array<AudioCapturerChangeInfo> AudioStreamManagerImpl::GetCurrentAudioCapturerInfoArraySync()
{
    std::vector<AudioCapturerChangeInfo> emptyResult;
    if (audioStreamMngr_ == nullptr) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_ILLEGAL_STATE, "audioStreamMngr_ is nullptr");
        return array<AudioCapturerChangeInfo>(emptyResult);
    }
    std::vector<std::shared_ptr<OHOS::AudioStandard::AudioCapturerChangeInfo>> audioCapturerChangeInfos;
    if (audioStreamMngr_->GetCurrentCapturerChangeInfos(audioCapturerChangeInfos) != AUDIO_OK) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_ILLEGAL_STATE, "GetCurrentCapturerChangeInfos failure!");
        return array<AudioCapturerChangeInfo>(emptyResult);
    }
    return TaiheParamUtils::SetCapturerChangeInfos(audioCapturerChangeInfos);
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
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_ILLEGAL_STATE, "AudioStreamManager not initialized");
        return false;
    }
    return audioStreamMngr_->IsStreamActive(TaiheAudioEnum::GetNativeAudioVolumeType(volType));
}

void AudioStreamManagerImpl::OnAudioRendererChange(callback_view<void(array_view<AudioRendererChangeInfo>)> callback)
{
    CHECK_AND_RETURN_RET_LOG(audioStreamMngr_ != nullptr, TaiheAudioError::ThrowErrorAndReturn(
        TAIHE_ERROR_INVALID_PARAM), "audioStreamMngr_ is nullptr");
    auto cacheCallback = TaiheParamUtils::TypeCallback(callback);
    RegisterRendererStateChangeCallback(cacheCallback, RENDERERCHANGE_CALLBACK_NAME, this);
}

void AudioStreamManagerImpl::OnAudioCapturerChange(callback_view<void(array_view<AudioCapturerChangeInfo>)> callback)
{
    CHECK_AND_RETURN_RET_LOG(audioStreamMngr_ != nullptr, TaiheAudioError::ThrowErrorAndReturn(
        TAIHE_ERROR_INVALID_PARAM), "audioStreamMngr_ is nullptr");
    auto cacheCallback = TaiheParamUtils::TypeCallback(callback);
    RegisterCapturerStateChangeCallback(cacheCallback, CAPTURERCHANGE_CALLBACK_NAME, this);
}

void AudioStreamManagerImpl::OffAudioRendererChange(
    optional_view<callback<void(array_view<AudioRendererChangeInfo>)>> callback)
{
    CHECK_AND_RETURN_RET_LOG(audioStreamMngr_ != nullptr, TaiheAudioError::ThrowErrorAndReturn(
        TAIHE_ERROR_INVALID_PARAM), "audioStreamMngr_ is nullptr");
    std::shared_ptr<uintptr_t> cacheCallback;
    if (callback.has_value()) {
        cacheCallback = TaiheParamUtils::TypeCallback(callback.value());
    }
    UnregisterRendererStateChangeCallback(cacheCallback, this);
}

void AudioStreamManagerImpl::OffAudioCapturerChange(optional_view<callback<void(array_view<AudioCapturerChangeInfo>)>>
    callback)
{
    CHECK_AND_RETURN_RET_LOG(audioStreamMngr_ != nullptr, TaiheAudioError::ThrowErrorAndReturn(
        TAIHE_ERROR_INVALID_PARAM), "audioStreamMngr_ is nullptr");
    std::shared_ptr<uintptr_t> cacheCallback;
    if (callback.has_value()) {
        cacheCallback = TaiheParamUtils::TypeCallback(callback.value());
    }
    UnregisterCapturerStateChangeCallback(cacheCallback, this);
}

void AudioStreamManagerImpl::RegisterRendererStateChangeCallback(std::shared_ptr<uintptr_t> &callback,
    const std::string &cbName, AudioStreamManagerImpl *taiheStreamManager)
{
    if (!taiheStreamManager->rendererStateCallback_) {
        taiheStreamManager->rendererStateCallback_ = std::make_shared<TaiheAudioRendererStateCallback>(get_env());
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
    cb->SaveCallbackReference(cbName, callback);
    AUDIO_INFO_LOG("RegisterRendererStateChangeCallback is successful");
}

void AudioStreamManagerImpl::RegisterCapturerStateChangeCallback(std::shared_ptr<uintptr_t> &callback,
    const std::string &cbName, AudioStreamManagerImpl *taiheStreamManager)
{
    if (!taiheStreamManager->capturerStateCallback_) {
        taiheStreamManager->capturerStateCallback_ = std::make_shared<TaiheAudioCapturerStateCallback>(get_env());
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
    cb->SaveCallbackReference(cbName, callback);
    AUDIO_INFO_LOG("RegisterRendererStateChangeCallback is successful");
}

void AudioStreamManagerImpl::UnregisterRendererStateChangeCallback(std::shared_ptr<uintptr_t> &callback,
    AudioStreamManagerImpl *taiheStreamManager)
{
    CHECK_AND_RETURN_LOG(taiheStreamManager->rendererStateCallback_ != nullptr,
        "rendererStateCallback_ is nullptr");
    std::shared_ptr<TaiheAudioRendererStateCallback> cb =
        std::static_pointer_cast<TaiheAudioRendererStateCallback>(taiheStreamManager->rendererStateCallback_);
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

void AudioStreamManagerImpl::UnregisterCapturerStateChangeCallback(std::shared_ptr<uintptr_t> &callback,
    AudioStreamManagerImpl *taiheStreamManager)
{
    CHECK_AND_RETURN_LOG(taiheStreamManager->capturerStateCallback_ != nullptr,
        "capturerStateChangeCallbackNapi is nullptr");
    std::shared_ptr<TaiheAudioCapturerStateCallback> cb =
        std::static_pointer_cast<TaiheAudioCapturerStateCallback>(taiheStreamManager->capturerStateCallback_);
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