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
#define LOG_TAG "AudioCollaborativeManagerImpl"
#endif

#include "taihe_audio_collaborative_manager.h"
#include "taihe_audio_error.h"
#include "taihe_param_utils.h"
#include "taihe_audio_enum.h"

namespace ANI::Audio {
AudioCollaborativeManagerImpl::AudioCollaborativeManagerImpl() : audioCollaborativeMngr_(nullptr) {}

AudioCollaborativeManagerImpl::AudioCollaborativeManagerImpl(std::shared_ptr<AudioCollaborativeManagerImpl> obj)
    : audioCollaborativeMngr_(nullptr)
{
    if (obj != nullptr) {
        audioCollaborativeMngr_ = obj->audioCollaborativeMngr_;
    }
}

AudioCollaborativeManagerImpl::~AudioCollaborativeManagerImpl() = default;

AudioCollaborativeManager AudioCollaborativeManagerImpl::CreateCollaborativeManagerWrapper()
{
    if (!OHOS::AudioStandard::PermissionUtil::VerifySelfPermission()) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_PERMISSION_DENIED, "No system permission");
        return make_holder<AudioCollaborativeManagerImpl, AudioCollaborativeManager>(nullptr);
    }
    std::shared_ptr<AudioCollaborativeManagerImpl> audioCollaborativeManagerImpl =
        std::make_shared<AudioCollaborativeManagerImpl>();
    if (audioCollaborativeManagerImpl != nullptr) {
        audioCollaborativeManagerImpl->audioCollaborativeMngr_ =
            OHOS::AudioStandard::AudioCollaborativeManager::GetInstance();
        return make_holder<AudioCollaborativeManagerImpl, AudioCollaborativeManager>(audioCollaborativeManagerImpl);
    }
    TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "AudioCollaborativeManagerImpl is nullptr");
    return make_holder<AudioCollaborativeManagerImpl, AudioCollaborativeManager>(nullptr);
}

bool AudioCollaborativeManagerImpl::IsCollaborativePlaybackSupported()
{
    AUDIO_DEBUG_LOG("in");
    bool isCollaborativeSupported = false;
    if (!OHOS::AudioStandard::PermissionUtil::VerifySelfPermission()) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_PERMISSION_DENIED, "No system permission");
        return isCollaborativeSupported;
    }
    if (audioCollaborativeMngr_ == nullptr) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "audioCollaborativeMngr_ is nullptr");
        return isCollaborativeSupported;
    }
    isCollaborativeSupported = audioCollaborativeMngr_->IsCollaborativePlaybackSupported();
    return isCollaborativeSupported;
}

bool AudioCollaborativeManagerImpl::IsCollaborativePlaybackEnabledForDevice(AudioDeviceDescriptor deviceDescriptor)
{
    AUDIO_DEBUG_LOG("in");
    bool isCollaborativeEnabled = false;
    if (!OHOS::AudioStandard::PermissionUtil::VerifySelfPermission()) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_PERMISSION_DENIED, "No system permission");
        return isCollaborativeEnabled;
    }
    bool argTransFlag = true;
    std::shared_ptr<OHOS::AudioStandard::AudioDeviceDescriptor> selectedAudioDevice =
        std::make_shared<OHOS::AudioStandard::AudioDeviceDescriptor>();
    TaiheParamUtils::GetAudioDeviceDescriptor(selectedAudioDevice, argTransFlag, deviceDescriptor);
    if (argTransFlag != true) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERROR_INVALID_PARAM,
            "parameter verification failed: The param of deviceDescriptor must be interface AudioDeviceDescriptor");
        AUDIO_ERR_LOG("invalid parameter");
        return isCollaborativeEnabled;
    }
    if ((selectedAudioDevice->deviceType_ != OHOS::AudioStandard::DeviceType::DEVICE_TYPE_BLUETOOTH_A2DP) ||
        (selectedAudioDevice->connectState_ != OHOS::AudioStandard::ConnectState::CONNECTED)) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERROR_INVALID_PARAM,
            "invalid arguments, device is not A2DP or device is not connected");
        AUDIO_ERR_LOG("invalid parameter");
        return isCollaborativeEnabled;
    }
    if (audioCollaborativeMngr_ == nullptr) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "audioCollaborativeMngr_ is nullptr");
        return isCollaborativeEnabled;
    }
    isCollaborativeEnabled = audioCollaborativeMngr_->IsCollaborativePlaybackEnabledForDevice(selectedAudioDevice);
    return isCollaborativeEnabled;
}

void AudioCollaborativeManagerImpl::SetCollaborativePlaybackEnabledForDeviceSync(
    AudioDeviceDescriptor deviceDescriptor, bool enabled)
{
    if (!OHOS::AudioStandard::PermissionUtil::VerifySelfPermission()) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_PERMISSION_DENIED, "No system permission");
        return;
    }
    std::shared_ptr<OHOS::AudioStandard::AudioDeviceDescriptor> selectedAudioDevice =
        std::make_shared<OHOS::AudioStandard::AudioDeviceDescriptor>();
    bool argTransFlag = true;
    int32_t status = TaiheParamUtils::GetAudioDeviceDescriptor(selectedAudioDevice, argTransFlag, deviceDescriptor);
    if (status != AUDIO_OK) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_INPUT_INVALID,
            "incorrect parameter types: The param of deviceDescriptor must be interface AudioDeviceDescriptor");
        return;
    }
    if (audioCollaborativeMngr_ == nullptr) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "audioCollaborativeMngr_ is nullptr");
        return;
    }
    if (!audioCollaborativeMngr_->IsCollaborativePlaybackSupported()) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_UNAVAILABLE_ON_DEVICE);
        return;
    }
    CHECK_AND_RETURN_LOG(selectedAudioDevice != nullptr, "selectedAudioDevice is nullptr");
    if ((selectedAudioDevice->deviceType_ != OHOS::AudioStandard::DeviceType::DEVICE_TYPE_BLUETOOTH_A2DP) ||
        (selectedAudioDevice->connectState_ != OHOS::AudioStandard::ConnectState::CONNECTED)) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_INVALID_PARAM);
        return;
    }
    int32_t retValue = audioCollaborativeMngr_->SetCollaborativePlaybackEnabledForDevice(selectedAudioDevice, enabled);
    if (retValue == OHOS::AudioStandard::ERR_PERMISSION_DENIED) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_NO_PERMISSION);
        return;
    } else if (retValue != OHOS::AudioStandard::SUCCESS) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM);
        return;
    }
}
} // namespace ANI::Audio
