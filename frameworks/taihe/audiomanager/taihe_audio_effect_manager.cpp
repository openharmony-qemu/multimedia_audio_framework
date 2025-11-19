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
#define LOG_TAG "TaiheAudioEffectMgr"
#endif

#include "taihe_audio_effect_manager.h"
#include "taihe_audio_error.h"
#include "taihe_param_utils.h"
#include "taihe_audio_enum.h"
#include "audio_errors.h"

namespace ANI::Audio {
using namespace OHOS::HiviewDFX;

AudioEffectManagerImpl::AudioEffectManagerImpl() : audioEffectMngr_(nullptr) {}

AudioEffectManagerImpl::AudioEffectManagerImpl(OHOS::AudioStandard::AudioEffectManager *audioEffectMngr)
    : audioEffectMngr_(nullptr)
{
    cachedClientId_ = getpid();
    if (audioEffectMngr != nullptr) {
        audioEffectMngr_ = audioEffectMngr;
    }
}

AudioEffectManagerImpl::~AudioEffectManagerImpl() = default;

AudioEffectManager AudioEffectManagerImpl::CreateEffectManagerWrapper()
{
    auto *audioEffectMngr = OHOS::AudioStandard::AudioEffectManager::GetInstance();
    if (audioEffectMngr == nullptr) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "Failed to get AudioEffectManager instance");
        return make_holder<AudioEffectManagerImpl, AudioEffectManager>(nullptr);
    }
    return make_holder<AudioEffectManagerImpl, AudioEffectManager>(audioEffectMngr);
}

array<AudioEffectProperty> AudioEffectManagerImpl::GetSupportedAudioEffectProperty()
{
    std::vector<AudioEffectProperty> emptyResult;
    if (audioEffectMngr_ == nullptr) {
        AUDIO_ERR_LOG("audioEffectMngr_ is nullptr");
        TaiheAudioError::ThrowError(TAIHE_ERR_SYSTEM, "incorrect parameter types: The type of options must be empty");
        return array<AudioEffectProperty>(emptyResult);
    }

    OHOS::AudioStandard::AudioEffectPropertyArrayV3 propertyArray = {};
    int32_t result = audioEffectMngr_->GetSupportedAudioEffectProperty(propertyArray);
    if (result != AUDIO_OK) {
        AUDIO_ERR_LOG("get audio enhance property failure! %{public}d", result);
        TaiheAudioError::ThrowError(result, "interface operation failed");
        return array<AudioEffectProperty>(emptyResult);
    }
    return TaiheParamUtils::ToTaiheEffectPropertyArray(propertyArray);
}

void AudioEffectManagerImpl::SetAudioEffectProperty(array_view<AudioEffectProperty> propertyArray)
{
    if (audioEffectMngr_ == nullptr) {
        AUDIO_ERR_LOG("audioEffectMngr_ is nullptr");
        TaiheAudioError::ThrowError(TAIHE_ERR_INPUT_INVALID,
            "parameter verification failed: mandatory parameters are left unspecified");
        return;
    }

    OHOS::AudioStandard::AudioEffectPropertyArrayV3 innerPropertyArray = {};
    int32_t result = TaiheParamUtils::GetEffectPropertyArray(innerPropertyArray, propertyArray);
    if (result != AUDIO_OK || innerPropertyArray.property.size() <= 0) {
        AUDIO_ERR_LOG("GetEffectPropertyArray failed or arguments error");
        TaiheAudioError::ThrowError(TAIHE_ERR_INVALID_PARAM,
            "parameter verification failed: mandatory parameters are left unspecified");
        return;
    }

    result = audioEffectMngr_->SetAudioEffectProperty(innerPropertyArray);
    if (result != AUDIO_OK) {
        AUDIO_ERR_LOG("set audio effect property failure! %{public}d", result);
        TaiheAudioError::ThrowError(result, "interface operation failed");
        return;
    }
}

array<AudioEffectProperty> AudioEffectManagerImpl::GetAudioEffectProperty()
{
    std::vector<AudioEffectProperty> emptyResult;
    if (audioEffectMngr_ == nullptr) {
        AUDIO_ERR_LOG("audioEffectMngr_ is nullptr");
        TaiheAudioError::ThrowError(TAIHE_ERR_SYSTEM, "incorrect parameter types: The type of options must be empty");
        return array<AudioEffectProperty>(emptyResult);
    }

    OHOS::AudioStandard::AudioEffectPropertyArrayV3 propertyArray = {};
    int32_t result = audioEffectMngr_->GetAudioEffectProperty(propertyArray);
    if (result != AUDIO_OK) {
        AUDIO_ERR_LOG("get audio enhance property failure! %{public}d", result);
        TaiheAudioError::ThrowError(TAIHE_ERR_SYSTEM, "interface operation failed");
        return array<AudioEffectProperty>(emptyResult);
    }
    return TaiheParamUtils::ToTaiheEffectPropertyArray(propertyArray);
}
} // namespace ANI::Audio