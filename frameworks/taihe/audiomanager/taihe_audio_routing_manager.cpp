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
#define LOG_TAG "AudioRoutingManagerImpl"
#endif

#if !defined(IOS_PLATFORM) && !defined(ANDROID_PLATFORM)
#include "parameters.h"
#endif
#include "taihe_audio_routing_manager.h"
#include "taihe_audio_error.h"
#include "taihe_param_utils.h"
#include "taihe_audio_enum.h"
#include "taihe_audio_manager_callbacks.h"
#include "taihe_audio_rounting_available_devicechange_callback.h"
#include "taihe_audio_routing_manager_callbacks.h"

namespace ANI::Audio {
AudioRoutingManagerImpl::AudioRoutingManagerImpl() : audioMngr_(nullptr) {}

AudioRoutingManagerImpl::AudioRoutingManagerImpl(std::shared_ptr<AudioRoutingManagerImpl> obj)
    : audioMngr_(nullptr)
{
    if (obj != nullptr) {
        audioMngr_ = obj->audioMngr_;
        audioRoutingMngr_ = obj->audioRoutingMngr_;
    }
}

AudioRoutingManagerImpl::~AudioRoutingManagerImpl()
{
    AUDIO_DEBUG_LOG("AudioRoutingManagerImpl::~AudioRoutingManagerImpl()");
}

AudioRoutingManager AudioRoutingManagerImpl::CreateRoutingManagerWrapper()
{
    std::shared_ptr<AudioRoutingManagerImpl> audioRoutingMgrImpl = std::make_shared<AudioRoutingManagerImpl>();
    if (audioRoutingMgrImpl != nullptr) {
        audioRoutingMgrImpl->audioMngr_ = OHOS::AudioStandard::AudioSystemManager::GetInstance();
        audioRoutingMgrImpl->audioRoutingMngr_ = OHOS::AudioStandard::AudioRoutingManager::GetInstance();
        return make_holder<AudioRoutingManagerImpl, AudioRoutingManager>(audioRoutingMgrImpl);
    }
    TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "audioRoutingMgrImpl is nullptr");
    return make_holder<AudioRoutingManagerImpl, AudioRoutingManager>(nullptr);
}

array<AudioDeviceDescriptor> AudioRoutingManagerImpl::GetAvailableDevices(DeviceUsage usage)
{
    std::vector<AudioDeviceDescriptor> emptyResult;
    int32_t deviceUsage = usage.get_value();
    if (!TaiheAudioEnum::IsLegalDeviceUsage(deviceUsage)) {
        AUDIO_ERR_LOG("Invalid deviceUsage type: %{public}d", deviceUsage);
        TaiheAudioError::ThrowError(TAIHE_ERROR_INVALID_PARAM);
        return array<AudioDeviceDescriptor>(emptyResult);
    }
    OHOS::AudioStandard::AudioDeviceUsage audioDevUsage =
        static_cast<OHOS::AudioStandard::AudioDeviceUsage>(deviceUsage);

    if (audioRoutingMngr_ == nullptr) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "audioRoutingMngr_ is nullptr");
        return array<AudioDeviceDescriptor>(emptyResult);
    }
    std::vector<std::shared_ptr<OHOS::AudioStandard::AudioDeviceDescriptor>> availableDescs =
        audioRoutingMngr_->GetAvailableDevices(audioDevUsage);
    return TaiheParamUtils::SetDeviceDescriptors(availableDescs);
}

array<AudioDeviceDescriptor> AudioRoutingManagerImpl::GetExcludedDevices(DeviceUsage usage)
{
    std::vector<AudioDeviceDescriptor> emptyResult;
    int32_t deviceUsage = usage.get_value();
    OHOS::AudioStandard::AudioDeviceUsage audioDevUsage =
        static_cast<OHOS::AudioStandard::AudioDeviceUsage>(deviceUsage);

    if (audioMngr_ == nullptr) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "audioMngr_ is nullptr");
        return array<AudioDeviceDescriptor>(emptyResult);
    }
    std::vector<std::shared_ptr<OHOS::AudioStandard::AudioDeviceDescriptor>> excludedDevices =
        audioMngr_->GetExcludedDevices(audioDevUsage);
    return TaiheParamUtils::SetDeviceDescriptors(excludedDevices);
}

void AudioRoutingManagerImpl::ExcludeOutputDevicesSync(DeviceUsage usage, array_view<AudioDeviceDescriptor> devices)
{
    int32_t deviceUsage = usage.get_value();
    OHOS::AudioStandard::AudioDeviceUsage audioDevUsage =
        static_cast<OHOS::AudioStandard::AudioDeviceUsage>(deviceUsage);
    std::vector<std::shared_ptr<OHOS::AudioStandard::AudioDeviceDescriptor>> deviceDescriptors;
    bool bArgTransFlag = true;
    TaiheParamUtils::GetAudioDeviceDescriptorVector(deviceDescriptors, bArgTransFlag, devices);

    if (audioMngr_ == nullptr) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "audioMngr_ is nullptr");
        return;
    }
    if (audioRoutingMngr_ == nullptr) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "audioRoutingMngr_ is nullptr");
        return;
    }
    if (audioMngr_->ExcludeOutputDevices(audioDevUsage, deviceDescriptors) != OHOS::AudioStandard::SUCCESS) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "SelectOutputDevice failed");
    }
}

void AudioRoutingManagerImpl::UnexcludeOutputDevicesWithUsageAndDevices(DeviceUsage usage,
    array_view<AudioDeviceDescriptor> devices)
{
    int32_t deviceUsage = usage.get_value();
    OHOS::AudioStandard::AudioDeviceUsage audioDevUsage =
        static_cast<OHOS::AudioStandard::AudioDeviceUsage>(deviceUsage);
    std::vector<std::shared_ptr<OHOS::AudioStandard::AudioDeviceDescriptor>> deviceDescriptors;
    bool bArgTransFlag = true;
    TaiheParamUtils::GetAudioDeviceDescriptorVector(deviceDescriptors, bArgTransFlag, devices);

    if (audioMngr_ == nullptr) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "audioMngr_ is nullptr");
        return;
    }
    if (audioRoutingMngr_ == nullptr) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "audioRoutingMngr_ is nullptr");
        return;
    }
    if (audioMngr_->UnexcludeOutputDevices(audioDevUsage, deviceDescriptors) != OHOS::AudioStandard::SUCCESS) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "SelectOutputDevice failed");
    }
}

void AudioRoutingManagerImpl::UnexcludeOutputDevicesWithUsage(DeviceUsage usage)
{
    int32_t deviceUsage = usage.get_value();
    OHOS::AudioStandard::AudioDeviceUsage audioDevUsage =
        static_cast<OHOS::AudioStandard::AudioDeviceUsage>(deviceUsage);

    if (audioMngr_ == nullptr) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "audioMngr_ is nullptr");
        return;
    }
    if (audioRoutingMngr_ == nullptr) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "audioRoutingMngr_ is nullptr");
        return;
    }
    if (audioMngr_->UnexcludeOutputDevices(audioDevUsage) != OHOS::AudioStandard::SUCCESS) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "SelectOutputDevice failed");
    }
}

array<AudioDeviceDescriptor> AudioRoutingManagerImpl::GetPreferredOutputDeviceForRendererInfoSync(
    AudioRendererInfo const &rendererInfo)
{
    std::vector<AudioDeviceDescriptor> emptyResult;
    OHOS::AudioStandard::AudioRendererInfo innerRendererInfo;
    if (TaiheParamUtils::GetRendererInfo(innerRendererInfo, rendererInfo) != AUDIO_OK) {
        TaiheAudioError::ThrowError(TAIHE_ERR_INPUT_INVALID,
            "Incorrect parameter types: The type of rendererInfo must be interface AudioRendererInfo");
        return array<AudioDeviceDescriptor>(emptyResult);
    }
    if (innerRendererInfo.streamUsage == OHOS::AudioStandard::StreamUsage::STREAM_USAGE_INVALID) {
        TaiheAudioError::ThrowError(TAIHE_ERR_INVALID_PARAM,
            "Parameter verification failed. Your usage in AudioRendererInfo is invalid.");
        return array<AudioDeviceDescriptor>(emptyResult);
    }

    if (audioMngr_ == nullptr) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "audioMngr_ is nullptr");
        return array<AudioDeviceDescriptor>(emptyResult);
    }
    if (audioRoutingMngr_ == nullptr) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "audioRoutingMngr_ is nullptr");
        return array<AudioDeviceDescriptor>(emptyResult);
    }
    std::vector<std::shared_ptr<OHOS::AudioStandard::AudioDeviceDescriptor>> outDeviceDescriptors;
    audioRoutingMngr_->GetPreferredOutputDeviceForRendererInfo(innerRendererInfo, outDeviceDescriptors);
    return TaiheParamUtils::SetDeviceDescriptors(outDeviceDescriptors);
}

array<AudioDeviceDescriptor> AudioRoutingManagerImpl::GetPreferredInputDeviceForCapturerInfoSync(
    AudioCapturerInfo const &capturerInfo)
{
    std::vector<AudioDeviceDescriptor> emptyResult;
    OHOS::AudioStandard::AudioCapturerInfo innerCapturerInfo;
    if (TaiheParamUtils::GetAudioCapturerInfo(innerCapturerInfo, capturerInfo) != AUDIO_OK ||
        innerCapturerInfo.sourceType == OHOS::AudioStandard::SourceType::SOURCE_TYPE_INVALID) {
        TaiheAudioError::ThrowError(TAIHE_ERR_INVALID_PARAM,
            "Parameter verification failed. You source in AudioCapturerInfo is invalid.");
        AUDIO_ERR_LOG("sourceType invalid");
        return array<AudioDeviceDescriptor>(emptyResult);
    }

    if (audioMngr_ == nullptr) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "audioMngr_ is nullptr");
        return array<AudioDeviceDescriptor>(emptyResult);
    }
    if (audioRoutingMngr_ == nullptr) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "audioRoutingMngr_ is nullptr");
        return array<AudioDeviceDescriptor>(emptyResult);
    }
    std::vector<std::shared_ptr<OHOS::AudioStandard::AudioDeviceDescriptor>> inDeviceDescriptors;
    audioRoutingMngr_->GetPreferredInputDeviceForCapturerInfo(innerCapturerInfo, inDeviceDescriptors);
    return TaiheParamUtils::SetDeviceDescriptors(inDeviceDescriptors);
}

void AudioRoutingManagerImpl::SelectOutputDeviceSync(array_view<AudioDeviceDescriptor> outputAudioDevices)
{
    std::vector<std::shared_ptr<OHOS::AudioStandard::AudioDeviceDescriptor>> deviceDescriptors;
    bool bArgTransFlag = true;
    TaiheParamUtils::GetAudioDeviceDescriptorVector(deviceDescriptors, bArgTransFlag, outputAudioDevices);
    if (!bArgTransFlag) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_UNSUPPORTED, "select output device failed");
        return;
    }

    if (audioMngr_ == nullptr) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "audioMngr_ is nullptr");
        return;
    }
    if (audioRoutingMngr_ == nullptr) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "audioRoutingMngr_ is nullptr");
        return;
    }
    if (audioMngr_->SelectOutputDevice(deviceDescriptors) != OHOS::AudioStandard::SUCCESS) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "SelectOutputDevice failed");
    }
}

void AudioRoutingManagerImpl::SelectOutputDeviceByFilterSync(AudioRendererFilter const &filter,
    array_view<AudioDeviceDescriptor> outputAudioDevices)
{
    OHOS::sptr<OHOS::AudioStandard::AudioRendererFilter> audioRendererFilter;
    bool bArgTransFlag = true;
    TaiheParamUtils::GetAudioRendererFilter(audioRendererFilter, bArgTransFlag, filter);

    std::vector<std::shared_ptr<OHOS::AudioStandard::AudioDeviceDescriptor>> deviceDescriptors;
    TaiheParamUtils::GetAudioDeviceDescriptorVector(deviceDescriptors, bArgTransFlag, outputAudioDevices);
    if (!bArgTransFlag) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_UNSUPPORTED, "select output device by filter failed");
        return;
    }

    if (audioMngr_ == nullptr) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "audioMngr_ is nullptr");
        return;
    }
    if (audioRoutingMngr_ == nullptr) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "audioRoutingMngr_ is nullptr");
        return;
    }
    if (audioMngr_->SelectOutputDevice(audioRendererFilter, deviceDescriptors) != OHOS::AudioStandard::SUCCESS) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "SelectOutputDeviceByFilter failed");
    }
}

array<AudioDeviceDescriptor> AudioRoutingManagerImpl::GetPreferredInputDeviceByFilter(AudioCapturerFilter const &filter)
{
    std::vector<AudioDeviceDescriptor> emptyResult;
    OHOS::sptr<OHOS::AudioStandard::AudioCapturerFilter> audioCapturerFilter;
    int32_t status = TaiheParamUtils::GetAudioCapturerFilter(audioCapturerFilter, filter);
    if (status != AUDIO_OK) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_INVALID_PARAM,
            "Parameter verification failed. AudioCapturerFilter abnormal.");
        return array<AudioDeviceDescriptor>(emptyResult);
    }
    if (audioMngr_ == nullptr) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "audioMngr_ is nullptr");
        return array<AudioDeviceDescriptor>(emptyResult);
    }
    std::vector<std::shared_ptr<OHOS::AudioStandard::AudioDeviceDescriptor>> deviceDescriptors =
        audioMngr_->GetInputDevice(audioCapturerFilter);
    return TaiheParamUtils::SetDeviceDescriptors(deviceDescriptors);
}

array<AudioDeviceDescriptor> AudioRoutingManagerImpl::GetPreferredOutputDeviceByFilter(
    AudioRendererFilter const &filter)
{
    std::vector<AudioDeviceDescriptor> emptyResult;
    OHOS::sptr<OHOS::AudioStandard::AudioRendererFilter> audioRendererFilter;
    bool bArgTransFlag = true;
    int32_t status = TaiheParamUtils::GetAudioRendererFilter(audioRendererFilter, bArgTransFlag, filter);
    if (status != AUDIO_OK) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_INVALID_PARAM,
            "Parameter verification failed. Your usage in AudioRendererFilter is invalid.");
        return array<AudioDeviceDescriptor>(emptyResult);
    }
    if (audioMngr_ == nullptr) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "audioMngr_ is nullptr");
        return array<AudioDeviceDescriptor>(emptyResult);
    }
    std::vector<std::shared_ptr<OHOS::AudioStandard::AudioDeviceDescriptor>> deviceDescriptors =
        audioMngr_->GetOutputDevice(audioRendererFilter);
    return TaiheParamUtils::SetDeviceDescriptors(deviceDescriptors);
}

void AudioRoutingManagerImpl::SelectInputDeviceByFilterSync(AudioCapturerFilter const &filter,
    array_view<AudioDeviceDescriptor> inputAudioDevices)
{
    OHOS::sptr<OHOS::AudioStandard::AudioCapturerFilter> audioCapturerFilter;
    int32_t status = TaiheParamUtils::GetAudioCapturerFilter(audioCapturerFilter, filter);
    if (status != AUDIO_OK) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_INVALID_PARAM,
            "Parameter verification failed. AudioCapturerFilter abnormal.");
        return;
    }
    std::vector<std::shared_ptr<OHOS::AudioStandard::AudioDeviceDescriptor>> deviceDescriptors;
    bool bArgTransFlag = true;
    TaiheParamUtils::GetAudioDeviceDescriptorVector(deviceDescriptors, bArgTransFlag, inputAudioDevices);
    if (!bArgTransFlag) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_UNSUPPORTED);
        return;
    }
    if (audioMngr_ == nullptr) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "audioMngr_ is nullptr");
        return;
    }
    int32_t intValue = audioMngr_->SelectInputDevice(audioCapturerFilter, deviceDescriptors);
    if (intValue != OHOS::AudioStandard::SUCCESS) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "SelectInputDevice failed");
        return;
    }
    return;
}

#if !defined(IOS_PLATFORM) && !defined(ANDROID_PLATFORM)
bool AudioRoutingManagerImpl::IsMicBlockDetectionSupportedSync()
{
    bool supported = OHOS::system::GetBoolParameter("const.multimedia.audio.mic_block_detection", false);
    if (supported == true) {
    AUDIO_INFO_LOG("mic block detection supported");
    } else {
    AUDIO_ERR_LOG("mic block detection is not supported");
    }
    return supported;
}
#endif

void AudioRoutingManagerImpl::SelectInputDeviceSync(array_view<AudioDeviceDescriptor> inputAudioDevices)
{
    std::vector<std::shared_ptr<OHOS::AudioStandard::AudioDeviceDescriptor>> deviceDescriptors;
    bool bArgTransFlag = true;
    TaiheParamUtils::GetAudioDeviceDescriptorVector(deviceDescriptors, bArgTransFlag, inputAudioDevices);
    if (!bArgTransFlag) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_INVALID_PARAM, "select input device failed");
        return;
    }

    if (audioMngr_ == nullptr) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "audioMngr_ is nullptr");
        return;
    }
    if (audioRoutingMngr_ == nullptr) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "audioRoutingMngr_ is nullptr");
        return;
    }
    if (audioMngr_->SelectInputDevice(deviceDescriptors) != OHOS::AudioStandard::SUCCESS) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "SelectInputDevice failed");
    }
}

array<AudioDeviceDescriptor> AudioRoutingManagerImpl::GetDevicesSync(DeviceFlag deviceFlag)
{
    std::vector<AudioDeviceDescriptor> emptyResult;
    if (!TaiheAudioEnum::IsLegalInputArgumentDeviceFlag(deviceFlag)) {
        AUDIO_ERR_LOG("deviceFlag invalid");
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERROR_INVALID_PARAM,
            "Parameter verification failed: The param of deviceFlag must be enum DeviceFlag");
        return array<AudioDeviceDescriptor>(emptyResult);
    }

    if (audioMngr_ == nullptr) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "audioMngr_ is nullptr");
        return array<AudioDeviceDescriptor>(emptyResult);
    }
    if (audioRoutingMngr_ == nullptr) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "audioRoutingMngr_ is nullptr");
        return array<AudioDeviceDescriptor>(emptyResult);
    }
    OHOS::AudioStandard::DeviceFlag nativeFlag = static_cast<OHOS::AudioStandard::DeviceFlag>(deviceFlag.get_value());
    std::vector<std::shared_ptr<OHOS::AudioStandard::AudioDeviceDescriptor>> deviceDescriptors =
        audioMngr_->GetDevices(nativeFlag);
    return TaiheParamUtils::SetDeviceDescriptors(deviceDescriptors);
}

void AudioRoutingManagerImpl::SetCommunicationDeviceSync(CommunicationDeviceType deviceType, bool active)
{
    if (!TaiheAudioEnum::IsLegalInputArgumentCommunicationDeviceType(deviceType)) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_INVALID_PARAM,
            "Parameter verification failed: The param of deviceType must be enum CommunicationDeviceType");
        AUDIO_ERR_LOG("get deviceType failed");
        return;
    }
    if (audioMngr_ == nullptr) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "audioMngr_ is nullptr");
        return;
    }
    if (audioRoutingMngr_ == nullptr) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "audioRoutingMngr_ is nullptr");
        return;
    }
    OHOS::AudioStandard::DeviceType nativeType = static_cast<OHOS::AudioStandard::DeviceType>(deviceType.get_value());
    if (audioMngr_->SetDeviceActive(nativeType, active) != OHOS::AudioStandard::SUCCESS) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "SetDeviceActive failed");
    }
}

bool AudioRoutingManagerImpl::IsCommunicationDeviceActiveSync(CommunicationDeviceType deviceType)
{
    if (!TaiheAudioEnum::IsLegalInputArgumentActiveDeviceType(deviceType)) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_INVALID_PARAM,
            "Parameter verification failed: The param of deviceType must be enum CommunicationDeviceType");
        AUDIO_ERR_LOG("get deviceType failed");
        return false;
    }
    if (audioMngr_ == nullptr) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "audioMngr_ is nullptr");
        return false;
    }
    if (audioRoutingMngr_ == nullptr) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "audioRoutingMngr_ is nullptr");
        return false;
    }
    OHOS::AudioStandard::DeviceType nativeType = static_cast<OHOS::AudioStandard::DeviceType>(deviceType.get_value());
    bool isActive = audioMngr_->IsDeviceActive(nativeType);
    return isActive;
}

void AudioRoutingManagerImpl::OnPreferredInputDeviceChangeForCapturerInfo(AudioCapturerInfo const &capturerInfo,
    callback_view<void(array_view<AudioDeviceDescriptor>)> callback)
{
    auto cacheCallback = TaiheParamUtils::TypeCallback(callback);
    RegisterPreferredInputDeviceChangeCallback(capturerInfo, cacheCallback, PREFERRED_INPUT_DEVICE_CALLBACK_NAME, this);
}

void AudioRoutingManagerImpl::RegisterPreferredInputDeviceChangeCallback(AudioCapturerInfo const &capturerInfo,
    std::shared_ptr<uintptr_t> &callback, const std::string &cbName, AudioRoutingManagerImpl *audioRoutingManagerImpl)
{
    if ((audioRoutingManagerImpl == nullptr) || (audioRoutingManagerImpl->audioMngr_ == nullptr) ||
        (audioRoutingManagerImpl->audioRoutingMngr_ == nullptr)) {
        AUDIO_ERR_LOG("AudioRoutingManagerImpl::Failed to retrieve stream mgr taihe instance.");
        return;
    }
    CHECK_AND_RETURN_LOG(GetTaihePrefInputDeviceChangeCb(callback, audioRoutingManagerImpl) == nullptr,
        "Do not allow duplicate registration of the same callback");

    OHOS::AudioStandard::AudioCapturerInfo captureInfo;
    TaiheParamUtils::GetAudioCapturerInfo(captureInfo, capturerInfo);

    CHECK_AND_RETURN_RET_LOG(captureInfo.sourceType != OHOS::AudioStandard::SourceType::SOURCE_TYPE_INVALID,
        TaiheAudioError::ThrowError(TAIHE_ERR_INVALID_PARAM,
        "Parameter verification failed. Your source in AudioCapturerInfo is invalid."), "invalid sourceType");

    std::shared_ptr<TaiheAudioPreferredInputDeviceChangeCallback> cb =
        std::make_shared<TaiheAudioPreferredInputDeviceChangeCallback>();
    CHECK_AND_RETURN_LOG(cb != nullptr, "Memory allocation failed!!");

    cb->SaveCallbackReference(callback);

    int32_t ret = audioRoutingManagerImpl->audioRoutingMngr_->SetPreferredInputDeviceChangeCallback(
        captureInfo, cb);
    CHECK_AND_RETURN_RET_LOG(ret == OHOS::AudioStandard::SUCCESS, TaiheAudioError::ThrowError(ret),
        "Registering Preferred Input Device Change Callback Failed %{public}d", ret);

    AddPreferredInputDeviceChangeCallback(audioRoutingManagerImpl, cb);
}

void AudioRoutingManagerImpl::AddPreferredInputDeviceChangeCallback(AudioRoutingManagerImpl *audioRoutingManagerImpl,
    std::shared_ptr<TaiheAudioPreferredInputDeviceChangeCallback> cb)
{
    CHECK_AND_RETURN_LOG(audioRoutingManagerImpl != nullptr, "audioRoutingManagerImpl is nullptr");
    std::lock_guard<std::mutex> lock(audioRoutingManagerImpl->preferredInputDeviceMutex_);
    audioRoutingManagerImpl->preferredInputDeviceCallbacks_.push_back(cb);
}

void AudioRoutingManagerImpl::OffPreferredInputDeviceChangeForCapturerInfo(
    optional_view<callback<void(array_view<AudioDeviceDescriptor>)>> callback)
{
    std::shared_ptr<uintptr_t> cacheCallback = nullptr;
    if (callback.has_value()) {
        cacheCallback = TaiheParamUtils::TypeCallback(callback.value());
    }
    UnregisterPreferredInputDeviceChangeCallback(cacheCallback, this);
}

void AudioRoutingManagerImpl::OffPreferOutputDeviceChangeForRendererInfo(
    optional_view<callback<void(array_view<AudioDeviceDescriptor>)>> callback)
{
    std::shared_ptr<uintptr_t> cacheCallback = nullptr;
    if (callback.has_value()) {
        cacheCallback = TaiheParamUtils::TypeCallback(callback.value());
    }
    UnregisterPreferredOutputDeviceChangeCallback(cacheCallback, this);
}

void AudioRoutingManagerImpl::UnregisterPreferredInputDeviceChangeCallback(std::shared_ptr<uintptr_t> &callback,
    AudioRoutingManagerImpl *audioRoutingManagerImpl)
{
    if ((audioRoutingManagerImpl == nullptr) || (audioRoutingManagerImpl->audioMngr_ == nullptr) ||
        (audioRoutingManagerImpl->audioRoutingMngr_ == nullptr)) {
        AUDIO_ERR_LOG("AudioRoutingManagerImpl::Failed to retrieve stream mgr taihe instance.");
        return;
    }
    if (callback != nullptr) {
        std::shared_ptr<TaiheAudioPreferredInputDeviceChangeCallback> cb =
            GetTaihePrefInputDeviceChangeCb(callback, audioRoutingManagerImpl);
        CHECK_AND_RETURN_LOG(cb != nullptr, "TaiheAudioPreferredInputDeviceChangeCallback is nullptr");
        int32_t ret = audioRoutingManagerImpl->audioRoutingMngr_->UnsetPreferredInputDeviceChangeCallback(cb);
        CHECK_AND_RETURN_LOG(ret == OHOS::AudioStandard::SUCCESS, "UnsetPreferredInputDeviceChangeCallback Failed");

        RemovePreferredInputDeviceChangeCallback(audioRoutingManagerImpl, cb);
        return;
    }

    RemoveAllPrefInputDeviceChangeCallback(audioRoutingManagerImpl);
}
void AudioRoutingManagerImpl::UnregisterPreferredOutputDeviceChangeCallback(std::shared_ptr<uintptr_t> &callback,
    AudioRoutingManagerImpl *audioRoutingManagerImpl)
{
    if ((audioRoutingManagerImpl == nullptr) || (audioRoutingManagerImpl->audioMngr_ == nullptr) ||
        (audioRoutingManagerImpl->audioRoutingMngr_ == nullptr)) {
        AUDIO_ERR_LOG("AudioRoutingManagerImpl::Failed to retrieve stream mgr taihe instance.");
        return;
    }
    if (callback != nullptr) {
        std::shared_ptr<TaiheAudioPreferredOutputDeviceChangeCallback> cb =
            GetTaihePrefOutputDeviceChangeCb(callback, audioRoutingManagerImpl);
        CHECK_AND_RETURN_LOG(cb != nullptr, "TaiheAudioPreferredOutputDeviceChangeCallback is nullptr");
        int32_t ret = audioRoutingManagerImpl->audioRoutingMngr_->UnsetPreferredOutputDeviceChangeCallback(cb);
        CHECK_AND_RETURN_LOG(ret == OHOS::AudioStandard::SUCCESS, "UnsetPreferredOutputDeviceChangeCallback Failed");

        RemovePreferredOutputDeviceChangeCallback(audioRoutingManagerImpl, cb);
        return;
    }

    RemoveAllPrefOutputDeviceChangeCallback(audioRoutingManagerImpl);
}

std::shared_ptr<TaiheAudioPreferredInputDeviceChangeCallback> AudioRoutingManagerImpl::GetTaihePrefInputDeviceChangeCb(
    std::shared_ptr<uintptr_t> &callback, AudioRoutingManagerImpl *audioRoutingManagerImpl)
{
    CHECK_AND_RETURN_RET_LOG(audioRoutingManagerImpl != nullptr, nullptr, "audioRoutingManagerImpl is nullptr");
    std::lock_guard<std::mutex> lock(audioRoutingManagerImpl->preferredInputDeviceMutex_);
    std::shared_ptr<TaiheAudioPreferredInputDeviceChangeCallback> cb = nullptr;
    for (auto &iter : audioRoutingManagerImpl->preferredInputDeviceCallbacks_) {
        if (iter == nullptr) {
            AUDIO_ERR_LOG("iter is null");
            continue;
        }
        if (iter->ContainSameJsCallback(callback)) {
            cb = iter;
        }
    }
    return cb;
}

void AudioRoutingManagerImpl::RemovePreferredInputDeviceChangeCallback(
    AudioRoutingManagerImpl *audioRoutingManagerImpl, std::shared_ptr<TaiheAudioPreferredInputDeviceChangeCallback> cb)
{
    CHECK_AND_RETURN_LOG(audioRoutingManagerImpl != nullptr, "audioRoutingManagerImpl is nullptr");
    std::lock_guard<std::mutex> lock(audioRoutingManagerImpl->preferredInputDeviceMutex_);
    audioRoutingManagerImpl->preferredInputDeviceCallbacks_.remove(cb);
}

void AudioRoutingManagerImpl::RemovePreferredOutputDeviceChangeCallback(
    AudioRoutingManagerImpl *audioRoutingManagerImpl, std::shared_ptr<TaiheAudioPreferredOutputDeviceChangeCallback> cb)
{
    CHECK_AND_RETURN_LOG(audioRoutingManagerImpl != nullptr, "audioRoutingManagerImpl is nullptr");
    std::lock_guard<std::mutex> lock(audioRoutingManagerImpl->preferredOutputDeviceMutex_);
    audioRoutingManagerImpl->preferredOutputDeviceCallbacks_.remove(cb);
}

void AudioRoutingManagerImpl::RemoveAllPrefInputDeviceChangeCallback(AudioRoutingManagerImpl *audioRoutingManagerImpl)
{
    CHECK_AND_RETURN_LOG(audioRoutingManagerImpl != nullptr, "audioRoutingManagerImpl is nullptr");
    CHECK_AND_RETURN_LOG(audioRoutingManagerImpl->audioRoutingMngr_ != nullptr,
        "audioRoutingManagerImpl->audioRoutingMngr_ is nullptr");
    std::lock_guard<std::mutex> lock(audioRoutingManagerImpl->preferredInputDeviceMutex_);
    for (auto &iter : audioRoutingManagerImpl->preferredInputDeviceCallbacks_) {
        int32_t ret = audioRoutingManagerImpl->audioRoutingMngr_->UnsetPreferredInputDeviceChangeCallback(iter);
        CHECK_AND_RETURN_RET_LOG(ret == OHOS::AudioStandard::SUCCESS, TaiheAudioError::ThrowError(ret),
            "Unset one of preferred input device change callback failed!");
    }
    audioRoutingManagerImpl->preferredInputDeviceCallbacks_.clear();
}

void AudioRoutingManagerImpl::RemoveAllPrefOutputDeviceChangeCallback(AudioRoutingManagerImpl *audioRoutingManagerImpl)
{
    CHECK_AND_RETURN_LOG(audioRoutingManagerImpl != nullptr, "audioRoutingManagerImpl is nullptr");
    CHECK_AND_RETURN_LOG(audioRoutingManagerImpl->audioRoutingMngr_ != nullptr,
        "audioRoutingManagerImpl->audioRoutingMngr_ is nullptr");
    std::lock_guard<std::mutex> lock(audioRoutingManagerImpl->preferredOutputDeviceMutex_);
    for (auto &iter : audioRoutingManagerImpl->preferredOutputDeviceCallbacks_) {
        int32_t ret = audioRoutingManagerImpl->audioRoutingMngr_->UnsetPreferredOutputDeviceChangeCallback(iter);
        CHECK_AND_RETURN_RET_LOG(ret == OHOS::AudioStandard::SUCCESS, TaiheAudioError::ThrowError(ret),
            "Unset one of preferred output device change callback failed!");
    }
    audioRoutingManagerImpl->preferredOutputDeviceCallbacks_.clear();
}

void AudioRoutingManagerImpl::OnMicBlockStatusChanged(callback_view<void(DeviceBlockStatusInfo const&)> callback)
{
    auto cacheCallback = TaiheParamUtils::TypeCallback(callback);
    RegisterMicrophoneBlockedCallback(cacheCallback, MICROPHONE_BLOCKED_CALLBACK_NAME, this);
}

void AudioRoutingManagerImpl::RegisterMicrophoneBlockedCallback(std::shared_ptr<uintptr_t> &callback,
    const std::string &cbName, AudioRoutingManagerImpl *audioRoutingManagerImpl)
{
    if ((audioRoutingManagerImpl == nullptr) || (audioRoutingManagerImpl->audioMngr_ == nullptr) ||
        (audioRoutingManagerImpl->audioRoutingMngr_ == nullptr)) {
        AUDIO_ERR_LOG("AudioRoutingManagerImpl::Failed to retrieve stream mgr taihe instance.");
        return;
    }
    std::lock_guard<std::mutex> lock(audioRoutingManagerImpl->mutex_);
    if (!audioRoutingManagerImpl->microphoneBlockedCallbackTaihe_) {
        audioRoutingManagerImpl->microphoneBlockedCallbackTaihe_ = std::make_shared<TaiheAudioManagerCallback>();
    }

    int32_t ret = audioRoutingManagerImpl->audioMngr_->SetMicrophoneBlockedCallback(
        audioRoutingManagerImpl->microphoneBlockedCallbackTaihe_);
    CHECK_AND_RETURN_RET_LOG(ret == OHOS::AudioStandard::SUCCESS, TaiheAudioError::ThrowError(ret),
        "Registering micro phone blocked Callback Failed %{public}d", ret);
    std::shared_ptr<TaiheAudioManagerCallback> cb =
        std::static_pointer_cast<TaiheAudioManagerCallback>(audioRoutingManagerImpl->microphoneBlockedCallbackTaihe_);
    CHECK_AND_RETURN_LOG(cb != nullptr, "cb is nullptr");
    cb->SaveMicrophoneBlockedCallbackReference(callback);
}

void AudioRoutingManagerImpl::OffMicBlockStatusChanged(
    optional_view<callback<void(DeviceBlockStatusInfo const&)>> callback)
{
    std::shared_ptr<uintptr_t> cacheCallback = nullptr;
    if (callback.has_value()) {
        cacheCallback = TaiheParamUtils::TypeCallback(callback.value());
    }
    UnregisterMicrophoneBlockedCallback(cacheCallback, this);
}

void AudioRoutingManagerImpl::UnregisterMicrophoneBlockedCallback(std::shared_ptr<uintptr_t> &callback,
    AudioRoutingManagerImpl *audioRoutingManagerImpl)
{
    if ((audioRoutingManagerImpl == nullptr) || (audioRoutingManagerImpl->audioMngr_ == nullptr) ||
        (audioRoutingManagerImpl->audioRoutingMngr_ == nullptr)) {
        AUDIO_ERR_LOG("AudioRoutingManagerImpl::Failed to retrieve stream mgr taihe instance.");
        return;
    }
    std::lock_guard<std::mutex> lock(audioRoutingManagerImpl->mutex_);
    if (audioRoutingManagerImpl->microphoneBlockedCallbackTaihe_ != nullptr) {
        std::shared_ptr<TaiheAudioManagerCallback> cb =
            std::static_pointer_cast<TaiheAudioManagerCallback>(
                audioRoutingManagerImpl->microphoneBlockedCallbackTaihe_);
        CHECK_AND_RETURN_LOG(cb != nullptr, "cb is nullptr");
        if (callback == nullptr || cb->GetMicrophoneBlockedCbListSize() == 0) {
            int32_t ret = audioRoutingManagerImpl->audioMngr_->UnsetMicrophoneBlockedCallback(
                audioRoutingManagerImpl->microphoneBlockedCallbackTaihe_);
            CHECK_AND_RETURN_LOG(ret == OHOS::AudioStandard::SUCCESS, "UnsetMicrophoneBlockedCallback Failed");
            audioRoutingManagerImpl->microphoneBlockedCallbackTaihe_.reset();
            audioRoutingManagerImpl->microphoneBlockedCallbackTaihe_ = nullptr;
            cb->RemoveAllMicrophoneBlockedCallback();
            return;
        }
        cb->RemoveMicrophoneBlockedCallbackReference(callback);
    } else {
        AUDIO_ERR_LOG("microphoneBlockedCallbackTaihe_ is null");
    }
}

void AudioRoutingManagerImpl::OffDeviceChange(optional_view<callback<void(DeviceChangeAction const&)>> callback)
{
    std::shared_ptr<uintptr_t> cacheCallback = nullptr;
    if (callback.has_value()) {
        cacheCallback = TaiheParamUtils::TypeCallback(callback.value());
    }
    UnregisterDeviceChangeCallback(cacheCallback, this);
}

void AudioRoutingManagerImpl::UnregisterDeviceChangeCallback(std::shared_ptr<uintptr_t> &callback,
    AudioRoutingManagerImpl *audioRoutingManagerImpl)
{
    if ((audioRoutingManagerImpl == nullptr) || (audioRoutingManagerImpl->audioMngr_ == nullptr) ||
        (audioRoutingManagerImpl->audioRoutingMngr_ == nullptr)) {
        AUDIO_ERR_LOG("AudioRoutingManagerImpl::Failed to retrieve stream mgr taihe instance.");
        return;
    }
    std::lock_guard<std::mutex> lock(audioRoutingManagerImpl->mutex_);
    if (audioRoutingManagerImpl->deviceChangeCallbackTaihe_ != nullptr) {
        std::shared_ptr<TaiheAudioManagerCallback> cb =
            std::static_pointer_cast<TaiheAudioManagerCallback>(audioRoutingManagerImpl->deviceChangeCallbackTaihe_);
        CHECK_AND_RETURN_LOG(cb != nullptr, "cb is nullptr");
        if (callback != nullptr) {
            cb->RemoveRoutingManagerDeviceChangeCbRef(callback);
        }
        if (callback == nullptr || cb->GetRoutingManagerDeviceChangeCbListSize() == 0) {
            int32_t ret = audioRoutingManagerImpl->audioMngr_->UnsetDeviceChangeCallback(
                OHOS::AudioStandard::ALL_L_D_DEVICES_FLAG, audioRoutingManagerImpl->deviceChangeCallbackTaihe_);
            CHECK_AND_RETURN_LOG(ret == OHOS::AudioStandard::SUCCESS, "UnsetDeviceChangeCallback Failed");
            audioRoutingManagerImpl->deviceChangeCallbackTaihe_.reset();
            audioRoutingManagerImpl->deviceChangeCallbackTaihe_ = nullptr;

            cb->RemoveAllRoutingManagerDeviceChangeCb();
        }
    } else {
        AUDIO_ERR_LOG("UnregisterDeviceChangeCallback: deviceChangeCallbackTaihe_ is null");
    }
}

void AudioRoutingManagerImpl::OffAvailableDeviceChange(
    optional_view<callback<void(DeviceChangeAction const&)>> callback)
{
    std::shared_ptr<uintptr_t> cacheCallback = nullptr;
    if (callback.has_value()) {
        cacheCallback = TaiheParamUtils::TypeCallback(callback.value());
    }
    UnregisterAvailableDeviceChangeCallback(cacheCallback, this);
}

void AudioRoutingManagerImpl::UnregisterAvailableDeviceChangeCallback(std::shared_ptr<uintptr_t> &callback,
    AudioRoutingManagerImpl *audioRoutingManagerImpl)
{
    if ((audioRoutingManagerImpl == nullptr) || (audioRoutingManagerImpl->audioMngr_ == nullptr) ||
        (audioRoutingManagerImpl->audioRoutingMngr_ == nullptr)) {
        AUDIO_ERR_LOG("AudioRoutingManagerImpl::Failed to retrieve stream mgr taihe instance.");
        return;
    }
    std::lock_guard<std::mutex> lock(audioRoutingManagerImpl->mutex_);
    if (audioRoutingManagerImpl->availableDeviceChangeCallbackTaihe_ != nullptr) {
        std::shared_ptr<TaiheAudioRountingAvailableDeviceChangeCallback> cb =
            std::static_pointer_cast<TaiheAudioRountingAvailableDeviceChangeCallback>(
            audioRoutingManagerImpl->availableDeviceChangeCallbackTaihe_);
        CHECK_AND_RETURN_LOG(cb != nullptr, "cb is nullptr");
        if (callback == nullptr || cb->GetRoutingAvailbleDeviceChangeCbListSize() == 0) {
            int32_t ret = audioRoutingManagerImpl->audioMngr_->UnsetAvailableDeviceChangeCallback(
                OHOS::AudioStandard::D_ALL_DEVICES);
            CHECK_AND_RETURN_LOG(ret == OHOS::AudioStandard::SUCCESS, "UnsetAvailableDeviceChangeCallback Failed");

            audioRoutingManagerImpl->availableDeviceChangeCallbackTaihe_.reset();
            audioRoutingManagerImpl->availableDeviceChangeCallbackTaihe_ = nullptr;
            cb->RemoveAllRoutinAvailbleDeviceChangeCb();
            return;
        }
        cb->RemoveRoutingAvailbleDeviceChangeCbRef(callback);
    } else {
        AUDIO_ERR_LOG("UnregisterAvailableDeviceChangeCallback: availableDeviceChangeCallbackTaihe_ is null");
    }
}

void AudioRoutingManagerImpl::OnDeviceChange(DeviceFlag deviceFlag,
    callback_view<void(DeviceChangeAction const&)> callback)
{
    auto cacheCallback = TaiheParamUtils::TypeCallback(callback);
    RegisterDeviceChangeCallback(deviceFlag, cacheCallback, DEVICE_CHANGE_CALLBACK_NAME, this);
}

void AudioRoutingManagerImpl::OnAvailableDeviceChange(DeviceUsage deviceUsage,
    callback_view<void(DeviceChangeAction const&)> callback)
{
    auto cacheCallback = TaiheParamUtils::TypeCallback(callback);
    RegisterAvaiableDeviceChangeCallback(deviceUsage, cacheCallback, AVAILABLE_DEVICE_CHANGE_CALLBACK_NAME, this);
}

void AudioRoutingManagerImpl::RegisterDeviceChangeCallback(DeviceFlag deviceFlag, std::shared_ptr<uintptr_t> &callback,
    const std::string &cbName, AudioRoutingManagerImpl *taiheRoutingMgr)
{
    if ((taiheRoutingMgr == nullptr) || (taiheRoutingMgr->audioMngr_ == nullptr) ||
        (taiheRoutingMgr->audioRoutingMngr_ == nullptr)) {
        AUDIO_ERR_LOG("AudioRoutingManagerImpl::Failed to retrieve stream mgr taihe instance.");
        return;
    }
    std::lock_guard<std::mutex> lock(taiheRoutingMgr->mutex_);
    int32_t flag = deviceFlag.get_value();
    if (!TaiheAudioEnum::IsLegalInputArgumentDeviceFlag(flag)) {
        TaiheAudioError::ThrowError(TAIHE_ERR_INVALID_PARAM,
            "parameter verification failed: The param of deviceFlag must be enum DeviceFlag");
    }
    OHOS::AudioStandard::DeviceFlag audioDeviceFlag = OHOS::AudioStandard::DeviceFlag(flag);
    if (!taiheRoutingMgr->deviceChangeCallbackTaihe_) {
        taiheRoutingMgr->deviceChangeCallbackTaihe_ = std::make_shared<TaiheAudioManagerCallback>();
    }
    CHECK_AND_RETURN_LOG(taiheRoutingMgr->deviceChangeCallbackTaihe_,
        "RegisterDeviceChangeCallback: Memory Allocation Failed !");

    int32_t ret = taiheRoutingMgr->audioMngr_->SetDeviceChangeCallback(audioDeviceFlag,
        taiheRoutingMgr->deviceChangeCallbackTaihe_);
    CHECK_AND_RETURN_RET_LOG(ret == OHOS::AudioStandard::SUCCESS, TaiheAudioError::ThrowError(ret),
        "RegisterDeviceChangeCallback: Registering Device Change Callback Failed %{public}d", ret);

    std::shared_ptr<TaiheAudioManagerCallback> cb =
        std::static_pointer_cast<TaiheAudioManagerCallback>(taiheRoutingMgr->deviceChangeCallbackTaihe_);
    CHECK_AND_RETURN_LOG(cb != nullptr, "cb is nullptr");
    cb->SaveRoutingManagerDeviceChangeCbRef(audioDeviceFlag, callback);
}

void AudioRoutingManagerImpl::RegisterAvaiableDeviceChangeCallback(DeviceUsage deviceUsage,
    std::shared_ptr<uintptr_t> &callback, const std::string &cbName, AudioRoutingManagerImpl *taiheRoutingMgr)
{
    if ((taiheRoutingMgr == nullptr) || (taiheRoutingMgr->audioMngr_ == nullptr) ||
        (taiheRoutingMgr->audioRoutingMngr_ == nullptr)) {
        AUDIO_ERR_LOG("AudioRoutingManagerImpl::Failed to retrieve stream mgr taihe instance.");
        return;
    }
    std::lock_guard<std::mutex> lock(taiheRoutingMgr->mutex_);
    int32_t flag = deviceUsage.get_value();
    AUDIO_INFO_LOG("RegisterDeviceChangeCallback:On deviceFlag: %{public}d", flag);
    if (!TaiheAudioEnum::IsLegalDeviceUsage(flag)) {
        TaiheAudioError::ThrowError(TAIHE_ERR_INVALID_PARAM,
            "parameter verification failed: The param of deviceUsage must be enum DeviceUsage");
    }
    OHOS::AudioStandard::AudioDeviceUsage usage = static_cast<OHOS::AudioStandard::AudioDeviceUsage>(flag);
    if (!taiheRoutingMgr->availableDeviceChangeCallbackTaihe_) {
        taiheRoutingMgr->availableDeviceChangeCallbackTaihe_ =
            std::make_shared<TaiheAudioRountingAvailableDeviceChangeCallback>();
    }
    CHECK_AND_RETURN_LOG(taiheRoutingMgr->availableDeviceChangeCallbackTaihe_ != nullptr,
        "RegisterDeviceChangeCallback: Memory Allocation Failed !");

    int32_t ret = taiheRoutingMgr->audioMngr_->SetAvailableDeviceChangeCallback(usage,
        taiheRoutingMgr->availableDeviceChangeCallbackTaihe_);
    CHECK_AND_RETURN_RET_LOG(ret == OHOS::AudioStandard::SUCCESS, TaiheAudioError::ThrowError(ret),
        "RegisterDeviceChangeCallback: Registering Device Change Callback Failed %{public}d", ret);

    std::shared_ptr<TaiheAudioRountingAvailableDeviceChangeCallback> cb =
        std::static_pointer_cast<TaiheAudioRountingAvailableDeviceChangeCallback>(
        taiheRoutingMgr->availableDeviceChangeCallbackTaihe_);
    CHECK_AND_RETURN_LOG(cb != nullptr, "cb is nullptr");
    cb->SaveRoutingAvailbleDeviceChangeCbRef(usage, callback);
}

void AudioRoutingManagerImpl::OnPreferOutputDeviceChangeForRendererInfo(AudioRendererInfo const &rendererInfo,
    callback_view<void(array_view<AudioDeviceDescriptor>)> callback)
{
    auto cacheCallback = TaiheParamUtils::TypeCallback(callback);
    RegisterPreferredOutputDeviceChangeCallback(rendererInfo, cacheCallback, PREFERRED_OUTPUT_DEVICE_CALLBACK_NAME,
        this);
}

void AudioRoutingManagerImpl::RegisterPreferredOutputDeviceChangeCallback(AudioRendererInfo const &rendererInfo,
    std::shared_ptr<uintptr_t> &callback, const std::string &cbName, AudioRoutingManagerImpl *audioRoutingManagerImpl)
{
    if ((audioRoutingManagerImpl == nullptr) || (audioRoutingManagerImpl->audioMngr_ == nullptr) ||
        (audioRoutingManagerImpl->audioRoutingMngr_ == nullptr)) {
        AUDIO_ERR_LOG("AudioRoutingManagerImpl::Failed to retrieve stream mgr taihe instance.");
        return;
    }
    CHECK_AND_RETURN_LOG(GetTaihePrefOutputDeviceChangeCb(callback, audioRoutingManagerImpl) == nullptr,
        "Do not allow duplicate registration of the same callback");

    OHOS::AudioStandard::AudioRendererInfo renderInfo;
    TaiheParamUtils::GetRendererInfo(renderInfo, rendererInfo);

    CHECK_AND_RETURN_RET_LOG(renderInfo.streamUsage != OHOS::AudioStandard::StreamUsage::STREAM_USAGE_INVALID,
        TaiheAudioError::ThrowError(TAIHE_ERR_INVALID_PARAM,
        "Parameter verification failed. Your usage in AudioRendererInfo is invalid."), "invalid streamUsage");

    std::shared_ptr<TaiheAudioPreferredOutputDeviceChangeCallback> cb =
        std::make_shared<TaiheAudioPreferredOutputDeviceChangeCallback>();
    CHECK_AND_RETURN_LOG(cb != nullptr, "Memory allocation failed!!");

    cb->SaveCallbackReference(callback);

    int32_t ret = audioRoutingManagerImpl->audioRoutingMngr_->SetPreferredOutputDeviceChangeCallback(
        renderInfo, cb);
    CHECK_AND_RETURN_RET_LOG(ret == OHOS::AudioStandard::SUCCESS, TaiheAudioError::ThrowError(ret),
        "Registering Preferred Output Device Change Callback Failed %{public}d", ret);

    AddPreferredOutputDeviceChangeCallback(audioRoutingManagerImpl, cb);
}

void AudioRoutingManagerImpl::AddPreferredOutputDeviceChangeCallback(AudioRoutingManagerImpl *audioRoutingManagerImpl,
    std::shared_ptr<TaiheAudioPreferredOutputDeviceChangeCallback> cb)
{
    CHECK_AND_RETURN_LOG(audioRoutingManagerImpl != nullptr, "audioRoutingManagerImpl is nullptr");
    std::lock_guard<std::mutex> lock(audioRoutingManagerImpl->preferredOutputDeviceMutex_);
    audioRoutingManagerImpl->preferredOutputDeviceCallbacks_.push_back(cb);
}

std::shared_ptr<TaiheAudioPreferredOutputDeviceChangeCallback>AudioRoutingManagerImpl::GetTaihePrefOutputDeviceChangeCb(
    std::shared_ptr<uintptr_t> &callback, AudioRoutingManagerImpl *audioRoutingManagerImpl)
{
    CHECK_AND_RETURN_RET_LOG(audioRoutingManagerImpl != nullptr, nullptr, "audioRoutingManagerImpl is nullptr");
    std::lock_guard<std::mutex> lock(audioRoutingManagerImpl->preferredOutputDeviceMutex_);
    std::shared_ptr<TaiheAudioPreferredOutputDeviceChangeCallback> cb = nullptr;
    for (auto &iter : audioRoutingManagerImpl->preferredOutputDeviceCallbacks_) {
        if (iter == nullptr) {
            AUDIO_ERR_LOG("iter is null");
            continue;
        }
        if (iter->ContainSameJsCallback(callback)) {
            cb = iter;
        }
    }
    return cb;
}
} // namespace ANI::Audio