/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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
#define LOG_TAG "AudioRecordingManagerImpl"
#endif

#include "taihe_audio_record_controller.h"

#include <map>
#include "audio_errors.h"
#include "audio_log.h"
#include "audio_utils.h"
#include "taihe_audio_error.h"
#include "taihe_audio_enum.h"
#include "taihe_param_utils.h"
#include "taihe_audio_record_controller_callback.h"

namespace ANI::Audio {
static const std::map<int32_t, int32_t> ERR_MAP = {
    {OHOS::AudioStandard::ERR_SYSTEM_PERMISSION_DENIED, TAIHE_ERR_PERMISSION_DENIED},
    {OHOS::AudioStandard::ERR_INVALID_PARAM, TAIHE_ERR_INVALID_PARAM},
    {OHOS::AudioStandard::ERROR, TAIHE_ERR_UNSUPPORTED},
    {-1, TAIHE_ERR_UNSUPPORTED},
};

static const std::map<int32_t, std::string> ERR_INFO_MAP = {
    {OHOS::AudioStandard::ERR_SYSTEM_PERMISSION_DENIED, "Caller is not a system application."},
    {OHOS::AudioStandard::ERR_INVALID_PARAM, "Parameter verification failed."},
    {OHOS::AudioStandard::ERROR, "Operation not allowed."},
    {-1, "Operation not allowed."},
};

static int32_t GetResInt(int32_t errNum)
{
    auto it = ERR_MAP.find(errNum);
    int32_t errInt = TAIHE_ERR_UNSUPPORTED;
    if (it != ERR_MAP.end()) {
        errInt = it->second;
    } else {
        AUDIO_ERR_LOG("err not found.");
    }
    return errInt;
}

static std::string GetResStr(int32_t errNum)
{
    auto it = ERR_INFO_MAP.find(errNum);
    std::string errStr = "Operation not allowed.";
    if (it != ERR_INFO_MAP.end()) {
        errStr = it->second;
    } else {
        AUDIO_ERR_LOG("err not found.");
    }
    return errStr;
}

AudioRecordingManagerImpl::AudioRecordingManagerImpl() : audioMngr_(nullptr) {}

AudioRecordingManagerImpl::AudioRecordingManagerImpl(std::shared_ptr<AudioRecordingManagerImpl> obj)
    : audioMngr_(nullptr)
{
    if (obj != nullptr) {
        audioMngr_ = obj->audioMngr_;
    }
}

AudioRecordingManagerImpl::~AudioRecordingManagerImpl() = default;

AudioRecordingManager AudioRecordingManagerImpl::CreateRecordingManagerWrapper()
{
    std::shared_ptr<AudioRecordingManagerImpl> impl = std::make_shared<AudioRecordingManagerImpl>();
    if (impl == nullptr) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "impl is nullptr");
        return make_holder<AudioRecordingManagerImpl, AudioRecordingManager>(nullptr);
    }

    impl->audioMngr_ = OHOS::AudioStandard::AudioSystemManager::GetInstance();
    if (impl->audioMngr_ == nullptr) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "audioMngr_ is nullptr");
        return make_holder<AudioRecordingManagerImpl, AudioRecordingManager>(nullptr);
    }
    return make_holder<AudioRecordingManagerImpl, AudioRecordingManager>(impl);
}

void AudioRecordingManagerImpl::EnableSystemRecordControllerPromise(bool show,
    SystemRecordControllerConfig const &config)
{
    return EnableSystemRecordControllerSync(show, config);
}

void AudioRecordingManagerImpl::EnableSystemRecordControllerSync(bool show,
    SystemRecordControllerConfig const &config)
{
    if (audioMngr_ == nullptr) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "audioMngr_ is nullptr");
        return;
    }
    int32_t sourceTypeValue = config.sourceType.get_value();
    if (!TaiheAudioEnum::IsLegalInputArgumentRecordControllerSourceType(sourceTypeValue)) {
        AUDIO_ERR_LOG("Invalid sourceType: %{public}d", sourceTypeValue);
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_INVALID_PARAM,
            "parameter verification failed: The param of sourceType must be SOURCE_TYPE_MIC, "
            "SOURCE_TYPE_CAMCORDER, or SOURCE_TYPE_LIVE");
        return;
    }
    OHOS::AudioStandard::SystemRecordControllerConfig nativeConfig;
    nativeConfig.sourceType = static_cast<OHOS::AudioStandard::SourceType>(sourceTypeValue);
    int32_t ret = audioMngr_->EnableSystemRecordController(show, nativeConfig);
    if (ret != OHOS::AudioStandard::SUCCESS) {
        AUDIO_ERR_LOG("EnableSystemRecordController failed, ret=%{public}d", ret);
        TaiheAudioError::ThrowErrorAndReturn(GetResInt(ret), GetResStr(ret));
        return;
    }
}

void AudioRecordingManagerImpl::OnSystemRecordControllerEnabledChange(
    callback_view<void(SystemRecordControllerChangeInfo const&)> callback)
{
    if (!OHOS::AudioStandard::PermissionUtil::VerifySelfPermission()) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_PERMISSION_DENIED, "No system permission");
        return;
    }
    auto cacheCallback = TaiheParamUtils::TypeCallback(callback);
    RegisterRecordControllerEnabledChangeCallback(cacheCallback, this);
}

void AudioRecordingManagerImpl::RegisterRecordControllerEnabledChangeCallback(
    std::shared_ptr<uintptr_t> &callback, AudioRecordingManagerImpl *recordControllerImpl)
{
    CHECK_AND_RETURN_RET_LOG(recordControllerImpl != nullptr,
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_NO_MEMORY), "recordControllerImpl is nullptr");
    std::lock_guard<std::mutex> lock(recordControllerImpl->mutex_);
    CHECK_AND_RETURN_RET_LOG(recordControllerImpl->audioMngr_ != nullptr,
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_NO_MEMORY), "audioMngr_ is nullptr");

    if (recordControllerImpl->recordControllerChangeCallbackTaihe_ == nullptr) {
        auto recordControllerCallback = std::make_shared<TaiheAudioRecordControllerCallback>();
        CHECK_AND_RETURN_LOG(recordControllerCallback != nullptr, "no memory");

        int32_t ret = recordControllerImpl->audioMngr_->SetRecordControllerEnabledChangeCallback(
            recordControllerCallback);
        if (ret != OHOS::AudioStandard::SUCCESS) {
            AUDIO_ERR_LOG("SetRecordControllerEnabledChangeCallback Failed %{public}d", ret);
            TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "Registering callback failed");
            return;
        }
        recordControllerImpl->recordControllerChangeCallbackTaihe_ = recordControllerCallback;
    }

    std::shared_ptr<TaiheAudioRecordControllerCallback> cb =
        std::static_pointer_cast<TaiheAudioRecordControllerCallback>(
            recordControllerImpl->recordControllerChangeCallbackTaihe_);
    CHECK_AND_RETURN_LOG(cb != nullptr, "static_pointer_cast failed");
    cb->SaveCallbackReference(RECORD_CONTROLLER_ENABLED_CHANGE_CALLBACK_NAME, callback);
}

void AudioRecordingManagerImpl::OffSystemRecordControllerEnabledChange(
    optional_view<callback<void(SystemRecordControllerChangeInfo const&)>> callback)
{
    if (!OHOS::AudioStandard::PermissionUtil::VerifySelfPermission()) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_PERMISSION_DENIED, "No system permission");
        return;
    }
    std::shared_ptr<uintptr_t> cacheCallback = nullptr;
    if (callback.has_value()) {
        cacheCallback = TaiheParamUtils::TypeCallback(callback.value());
    }
    UnregisterRecordControllerEnabledChangeCallback(cacheCallback, this);
}

void AudioRecordingManagerImpl::UnregisterRecordControllerEnabledChangeCallback(
    std::shared_ptr<uintptr_t> &callback, AudioRecordingManagerImpl *recordControllerImpl)
{
    CHECK_AND_RETURN_RET_LOG(recordControllerImpl != nullptr,
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_NO_MEMORY), "recordControllerImpl is nullptr");
    std::lock_guard<std::mutex> lock(recordControllerImpl->mutex_);
    CHECK_AND_RETURN_RET_LOG(recordControllerImpl->audioMngr_ != nullptr,
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_NO_MEMORY), "audioMngr_ is nullptr");
    CHECK_AND_RETURN_RET_LOG(recordControllerImpl->recordControllerChangeCallbackTaihe_ != nullptr,
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM),
        "recordControllerChangeCallbackTaihe_ is nullptr");

    std::shared_ptr<TaiheAudioRecordControllerCallback> cb =
        std::static_pointer_cast<TaiheAudioRecordControllerCallback>(
            recordControllerImpl->recordControllerChangeCallbackTaihe_);
    CHECK_AND_RETURN_LOG(cb != nullptr, "static_pointer_cast failed");

    if (callback != nullptr) {
        CHECK_AND_RETURN_LOG(cb->ContainSameJsCallbackInner(
            RECORD_CONTROLLER_ENABLED_CHANGE_CALLBACK_NAME, callback),
            "The callback to be unregistered is not the same as the registered callback");
    }

    cb->RemoveCallbackReference(RECORD_CONTROLLER_ENABLED_CHANGE_CALLBACK_NAME, callback);
    int32_t ret = recordControllerImpl->audioMngr_->UnsetRecordControllerEnabledChangeCallback(
        recordControllerImpl->recordControllerChangeCallbackTaihe_);
    CHECK_AND_RETURN_LOG(ret == OHOS::AudioStandard::SUCCESS,
        "UnsetRecordControllerEnabledChangeCallback Failed");
    recordControllerImpl->recordControllerChangeCallbackTaihe_ = nullptr;
}
} // namespace ANI::Audio
