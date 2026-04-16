/*
 * Copyright (C) 2026 Huawei Device Co., Ltd.
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
#define LOG_TAG "AudioDeviceEnhanceManagerImpl"
#endif

#include "taihe_audio_device_enhance_manager.h"

#include "taihe_audio_error.h"
#include "taihe_param_utils.h"
#include "audio_utils.h"
#include "audio_errors.h"

namespace ANI::Audio {
AudioDeviceEnhanceManagerImpl::AudioDeviceEnhanceManagerImpl() : audioDeviceEnhanceMngr_(nullptr) {}

AudioDeviceEnhanceManagerImpl::AudioDeviceEnhanceManagerImpl(std::shared_ptr<AudioDeviceEnhanceManagerImpl> obj)
    : audioDeviceEnhanceMngr_(nullptr)
{
    if (obj != nullptr) {
        audioDeviceEnhanceMngr_ = obj->audioDeviceEnhanceMngr_;
    }
}

AudioDeviceEnhanceManagerImpl::~AudioDeviceEnhanceManagerImpl() = default;

AudioDeviceEnhanceManager AudioDeviceEnhanceManagerImpl::CreateDeviceEnhanceManagerWrapper()
{
    auto audioDeviceEnhanceManagerImpl = std::make_shared<AudioDeviceEnhanceManagerImpl>();
    if (audioDeviceEnhanceManagerImpl != nullptr) {
        audioDeviceEnhanceManagerImpl->audioDeviceEnhanceMngr_ =
            &OHOS::AudioStandard::AudioDeviceEnhanceManager::GetInstance();
        return make_holder<AudioDeviceEnhanceManagerImpl, AudioDeviceEnhanceManager>(audioDeviceEnhanceManagerImpl);
    }
    TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "AudioDeviceEnhanceManagerImpl is nullptr");
    return make_holder<AudioDeviceEnhanceManagerImpl, AudioDeviceEnhanceManager>(nullptr);
}

bool AudioDeviceEnhanceManagerImpl::IsEnhancedRoutingSupported()
{
    bool supported = false;
    if (audioDeviceEnhanceMngr_ == nullptr) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "audioDeviceEnhanceMngr_ is nullptr");
        return supported;
    }
    int32_t ret = audioDeviceEnhanceMngr_->IsEnhancedRoutingSupported(supported);
    if (ret != OHOS::AudioStandard::SUCCESS) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "IsEnhancedRoutingSupported failed");
        return false;
    }
    return supported;
}

void AudioDeviceEnhanceManagerImpl::SelectOutputDeviceSync(AudioDeviceDescriptor deviceDescriptor)
{
    std::shared_ptr<OHOS::AudioStandard::AudioDeviceDescriptor> selectedAudioDevice =
        std::make_shared<OHOS::AudioStandard::AudioDeviceDescriptor>();
    bool argTransFlag = true;
    int32_t status = TaiheParamUtils::GetAudioDeviceDescriptor(selectedAudioDevice, argTransFlag, deviceDescriptor);
    if (status != AUDIO_OK || !argTransFlag) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_INVALID_PARAM, "select output device failed");
        return;
    }
    CHECK_AND_RETURN_LOG(audioDeviceEnhanceMngr_ != nullptr, "audioDeviceEnhanceMngr_ is nullptr");
    int32_t ret = audioDeviceEnhanceMngr_->SelectOutputDevice(selectedAudioDevice);
    if (ret != OHOS::AudioStandard::SUCCESS) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "SelectOutputDevice failed");
    }
}

void AudioDeviceEnhanceManagerImpl::SelectOutputDeviceForAudioRendererSync(
    weak::AudioRenderer renderer, AudioDeviceDescriptor deviceDescriptor)
{
    AudioRendererImpl *taiheRenderer = reinterpret_cast<AudioRendererImpl *>(renderer->GetImplPtr());
    if (taiheRenderer == nullptr) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_INVALID_PARAM, "renderer impl is nullptr");
        return;
    }
    if (taiheRenderer->audioRenderer_ == nullptr) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_INVALID_PARAM, "renderer native object is nullptr");
        return;
    }

    std::shared_ptr<OHOS::AudioStandard::AudioDeviceDescriptor> selectedAudioDevice =
        std::make_shared<OHOS::AudioStandard::AudioDeviceDescriptor>();
    bool argTransFlag = true;
    int32_t status = TaiheParamUtils::GetAudioDeviceDescriptor(selectedAudioDevice, argTransFlag, deviceDescriptor);
    if (status != AUDIO_OK || !argTransFlag) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_INVALID_PARAM, "select output device failed");
        return;
    }
    CHECK_AND_RETURN_LOG(audioDeviceEnhanceMngr_ != nullptr, "audioDeviceEnhanceMngr_ is nullptr");
    int32_t ret = audioDeviceEnhanceMngr_->SelectOutputDeviceForAudioRenderer(
        taiheRenderer->audioRenderer_, selectedAudioDevice);
    if (ret != OHOS::AudioStandard::SUCCESS) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "SelectOutputDeviceForAudioRenderer failed");
    }
}

void AudioDeviceEnhanceManagerImpl::SelectInputDeviceSync(AudioDeviceDescriptor deviceDescriptor)
{
    std::shared_ptr<OHOS::AudioStandard::AudioDeviceDescriptor> selectedAudioDevice =
        std::make_shared<OHOS::AudioStandard::AudioDeviceDescriptor>();
    bool argTransFlag = true;
    int32_t status = TaiheParamUtils::GetAudioDeviceDescriptor(selectedAudioDevice, argTransFlag, deviceDescriptor);
    if (status != AUDIO_OK || !argTransFlag) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_INVALID_PARAM, "select input device failed");
        return;
    }
    CHECK_AND_RETURN_LOG(audioDeviceEnhanceMngr_ != nullptr, "audioDeviceEnhanceMngr_ is nullptr");
    int32_t ret = audioDeviceEnhanceMngr_->SelectInputDevice(selectedAudioDevice);
    if (ret != OHOS::AudioStandard::SUCCESS) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "SelectInputDevice failed");
    }
}

void AudioDeviceEnhanceManagerImpl::SelectInputDeviceForAudioCapturerSync(
    weak::AudioCapturer capturer, AudioDeviceDescriptor deviceDescriptor)
{
    AudioCapturerImpl *taiheCapturer = reinterpret_cast<AudioCapturerImpl *>(capturer->GetImplPtr());
    if (taiheCapturer == nullptr) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_INVALID_PARAM, "capturer impl is nullptr");
        return;
    }
    if (taiheCapturer->audioCapturer_ == nullptr) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_INVALID_PARAM, "capturer native object is nullptr");
        return;
    }

    std::shared_ptr<OHOS::AudioStandard::AudioDeviceDescriptor> selectedAudioDevice =
        std::make_shared<OHOS::AudioStandard::AudioDeviceDescriptor>();
    bool argTransFlag = true;
    int32_t status = TaiheParamUtils::GetAudioDeviceDescriptor(selectedAudioDevice, argTransFlag, deviceDescriptor);
    if (status != AUDIO_OK || !argTransFlag) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_INVALID_PARAM, "select input device failed");
        return;
    }
    CHECK_AND_RETURN_LOG(audioDeviceEnhanceMngr_ != nullptr, "audioDeviceEnhanceMngr_ is nullptr");
    int32_t ret = audioDeviceEnhanceMngr_->SelectInputDeviceForAudioCapturer(
        taiheCapturer->audioCapturer_, selectedAudioDevice);
    if (ret != OHOS::AudioStandard::SUCCESS) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "SelectInputDeviceForAudioCapturer failed");
    }
}
} // namespace ANI::Audio
