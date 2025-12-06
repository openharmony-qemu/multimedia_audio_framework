/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#define LOG_TAG "OHAudioSessionManager"
#endif

#include "OHAudioSessionManager.h"
#include "audio_errors.h"
#include <ostream>
#include <iostream>

using OHOS::AudioStandard::OHAudioSessionManager;
using OHOS::AudioStandard::AudioSessionManager;
using OHOS::AudioStandard::OHAudioDeviceDescriptor;
using OHOS::AudioStandard::AudioDeviceUsage;
using namespace std;

static OHOS::AudioStandard::OHAudioSessionManager *convertManager(OH_AudioSessionManager* manager)
{
    return (OHAudioSessionManager*) manager;
}

static OHOS::AudioStandard::OHAudioDeviceDescriptor *convertDeviceDescriptor(
    OH_AudioDeviceDescriptor* deviceDescriptor)
{
    return (OHOS::AudioStandard::OHAudioDeviceDescriptor*) deviceDescriptor;
}

const std::set<OH_AudioDevice_Usage> VALID_OH_AUDIO_DEVICE_UASGES = {
    AUDIO_DEVICE_USAGE_MEDIA_OUTPUT,
    AUDIO_DEVICE_USAGE_MEDIA_INPUT,
    AUDIO_DEVICE_USAGE_MEDIA_ALL,
    AUDIO_DEVICE_USAGE_CALL_OUTPUT,
    AUDIO_DEVICE_USAGE_CALL_INPUT,
    AUDIO_DEVICE_USAGE_CALL_ALL
};

OH_AudioCommon_Result OH_AudioManager_GetAudioSessionManager(OH_AudioSessionManager **audioSessionManager)
{
    OHAudioSessionManager* ohAudioSessionManager = OHAudioSessionManager::GetInstance();
    if (audioSessionManager == nullptr) {
        AUDIO_ERR_LOG("audioSessionManager is nullptr");
        return AUDIOCOMMON_RESULT_SUCCESS;
    }
    *audioSessionManager = reinterpret_cast<OH_AudioSessionManager*>(ohAudioSessionManager);
    return AUDIOCOMMON_RESULT_SUCCESS;
}

OH_AudioCommon_Result OH_AudioSessionManager_RegisterSessionDeactivatedCallback(
    OH_AudioSessionManager *audioSessionManager, OH_AudioSession_DeactivatedCallback callback)
{
    OHAudioSessionManager* ohAudioSessionManager = convertManager(audioSessionManager);
    CHECK_AND_RETURN_RET_LOG(ohAudioSessionManager != nullptr,
        AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM, "ohAudioSessionManager is nullptr");
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM, "callback is nullptr");
    return ohAudioSessionManager->SetAudioSessionCallback(callback);
}

OH_AudioCommon_Result OH_AudioSessionManager_UnregisterSessionDeactivatedCallback(
    OH_AudioSessionManager *audioSessionManager, OH_AudioSession_DeactivatedCallback callback)
{
    OHAudioSessionManager* ohAudioSessionManager = convertManager(audioSessionManager);
    CHECK_AND_RETURN_RET_LOG(ohAudioSessionManager != nullptr,
        AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM, "ohAudioSessionManager is nullptr");
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM, "callback is nullptr");
    return ohAudioSessionManager->UnsetAudioSessionCallback(callback);
}

OH_AudioCommon_Result OH_AudioSessionManager_ActivateAudioSession(
    OH_AudioSessionManager *audioSessionManager, const OH_AudioSession_Strategy *strategy)
{
    OHAudioSessionManager* ohAudioSessionManager = convertManager(audioSessionManager);
    CHECK_AND_RETURN_RET_LOG(ohAudioSessionManager != nullptr,
        AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM, "ohAudioSessionManager is nullptr");
    CHECK_AND_RETURN_RET_LOG(strategy != nullptr,
        AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM, "strategy is nullptr");
    OHOS::AudioStandard::AudioSessionStrategy audioStrategy;
    audioStrategy.concurrencyMode =
        static_cast<OHOS::AudioStandard::AudioConcurrencyMode>(strategy->concurrencyMode);
    return ohAudioSessionManager->ActivateAudioSession(audioStrategy);
}

OH_AudioCommon_Result OH_AudioSessionManager_DeactivateAudioSession(
    OH_AudioSessionManager *audioSessionManager)
{
    OHAudioSessionManager* ohAudioSessionManager = convertManager(audioSessionManager);
    CHECK_AND_RETURN_RET_LOG(ohAudioSessionManager != nullptr,
        AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM, "ohAudioSessionManager is nullptr");
    return ohAudioSessionManager->DeactivateAudioSession();
}

bool OH_AudioSessionManager_IsAudioSessionActivated(
    OH_AudioSessionManager *audioSessionManager)
{
    OHAudioSessionManager* ohAudioSessionManager = convertManager(audioSessionManager);
    CHECK_AND_RETURN_RET_LOG(ohAudioSessionManager != nullptr, false, "ohAudioSessionManager is nullptr");
    return ohAudioSessionManager->IsAudioSessionActivated();
}

OH_AudioCommon_Result OH_AudioSessionManager_GetAvailableDevices(OH_AudioSessionManager *audioSessionManager,
    OH_AudioDevice_Usage deviceUsage, OH_AudioDeviceDescriptorArray **audioDeviceDescriptorArray)
{
    if (audioSessionManager == nullptr || !VALID_OH_AUDIO_DEVICE_UASGES.count(deviceUsage) ||
        audioDeviceDescriptorArray == nullptr) {
        AUDIO_ERR_LOG("Invalid params!");
        return AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM;
    }
    OHAudioSessionManager *ohAudioSessionManager = convertManager(audioSessionManager);
    CHECK_AND_RETURN_RET_LOG(ohAudioSessionManager != nullptr,
        AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM, "ohAudioSessionManager is nullptr");
    AudioDeviceUsage usage = static_cast<AudioDeviceUsage>(deviceUsage);

    *audioDeviceDescriptorArray = ohAudioSessionManager->GetAvailableDevices(usage);
    CHECK_AND_RETURN_RET_LOG(*audioDeviceDescriptorArray != nullptr,
        AUDIOCOMMON_RESULT_ERROR_NO_MEMORY, "*audioDeviceDescriptorArray is nullptr");

    return AUDIOCOMMON_RESULT_SUCCESS;
}

OH_AudioCommon_Result OH_AudioSessionManager_SelectMediaInputDevice(
    OH_AudioSessionManager *audioSessionManager, OH_AudioDeviceDescriptor *deviceDescriptor)
{
    CHECK_AND_RETURN_RET_LOG(audioSessionManager != nullptr,
        AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM, "audioSessionManager is nullptr");
    OHAudioSessionManager *ohAudioSessionManager = convertManager(audioSessionManager);
    CHECK_AND_RETURN_RET_LOG(ohAudioSessionManager != nullptr,
        AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM, "ohAudioSessionManager is nullptr");

    if (deviceDescriptor == nullptr) {
        return ohAudioSessionManager->ClearSelectedMediaInputDevice();
    }

    OHAudioDeviceDescriptor* ohDeviceDescriptor = convertDeviceDescriptor(deviceDescriptor);
    CHECK_AND_RETURN_RET_LOG(ohDeviceDescriptor != nullptr,
        AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM, "ohDeviceDescriptor is nullptr");
    auto desc = ohDeviceDescriptor->GetAudioDeviceDescriptor();
    CHECK_AND_RETURN_RET_LOG(desc != nullptr,
        AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM, "audioDeviceDescriptor is nullptr");
    return ohAudioSessionManager->SelectMediaInputDevice(desc);
}

OH_AudioCommon_Result OH_AudioSessionManager_GetSelectedMediaInputDevice(
    OH_AudioSessionManager *audioSessionManager, OH_AudioDeviceDescriptor **audioDeviceDescriptor)
{
    if (audioSessionManager == nullptr || audioDeviceDescriptor == nullptr) {
        AUDIO_ERR_LOG("Invalid params!");
        return AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM;
    }
    OHAudioSessionManager *ohAudioSessionManager = convertManager(audioSessionManager);
    CHECK_AND_RETURN_RET_LOG(ohAudioSessionManager != nullptr,
        AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM, "ohAudioSessionManager is nullptr");
    *audioDeviceDescriptor = ohAudioSessionManager->GetSelectedMediaInputDevice();
    CHECK_AND_RETURN_RET_LOG(*audioDeviceDescriptor != nullptr,
        AUDIOCOMMON_RESULT_ERROR_NO_MEMORY, "*audioDeviceDescriptor is nullptr");

    return AUDIOCOMMON_RESULT_SUCCESS;
}

OH_AudioCommon_Result OH_AudioSessionManager_SetBluetoothAndNearlinkPreferredRecordCategory(
    OH_AudioSessionManager *audioSessionManager,
    OH_AudioSession_BluetoothAndNearlinkPreferredRecordCategory category)
{
    if (audioSessionManager == nullptr) {
        AUDIO_ERR_LOG("Invalid params!");
        return AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM;
    }
    OHAudioSessionManager *ohAudioSessionManager = convertManager(audioSessionManager);
    CHECK_AND_RETURN_RET_LOG(ohAudioSessionManager != nullptr,
        AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM, "ohAudioSessionManager is nullptr");
    
    OHOS::AudioStandard::BluetoothAndNearlinkPreferredRecordCategory preferCategory =
        static_cast<OHOS::AudioStandard::BluetoothAndNearlinkPreferredRecordCategory>(category);
    return ohAudioSessionManager->PreferBluetoothAndNearlinkRecord(preferCategory);
}

OH_AudioCommon_Result OH_AudioSessionManager_GetBluetoothAndNearlinkPreferredRecordCategory(
    OH_AudioSessionManager *audioSessionManager,
    OH_AudioSession_BluetoothAndNearlinkPreferredRecordCategory *category)
{
    if (audioSessionManager == nullptr) {
        AUDIO_ERR_LOG("Invalid params!");
        return AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM;
    }
    OHAudioSessionManager *ohAudioSessionManager = convertManager(audioSessionManager);
    CHECK_AND_RETURN_RET_LOG(ohAudioSessionManager != nullptr,
        AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM, "ohAudioSessionManager is nullptr");
    auto preferCategory = ohAudioSessionManager->GetPreferredBluetoothAndNearlinkRecord();
    *category = static_cast<OH_AudioSession_BluetoothAndNearlinkPreferredRecordCategory>(preferCategory);

    return AUDIOCOMMON_RESULT_SUCCESS;
}

OH_AudioCommon_Result OH_AudioSessionManager_SetScene(
    OH_AudioSessionManager *audioSessionManager, OH_AudioSession_Scene scene)
{
    OHAudioSessionManager *ohAudioSessionManager = convertManager(audioSessionManager);
    CHECK_AND_RETURN_RET_LOG(ohAudioSessionManager != nullptr,
        AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM, "ohAudioSessionManager is nullptr");
    CHECK_AND_RETURN_RET_LOG(((scene == AUDIO_SESSION_SCENE_MEDIA) ||
        (scene == AUDIO_SESSION_SCENE_GAME) ||
        (scene == AUDIO_SESSION_SCENE_VOICE_COMMUNICATION)),
        AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM, "scene is invalid");

    OHOS::AudioStandard::AudioSessionScene sessionScene =
        static_cast<OHOS::AudioStandard::AudioSessionScene>(scene);
    return ohAudioSessionManager->SetAudioSessionScene(sessionScene);
}

OH_AudioCommon_Result OH_AudioSessionManager_RegisterStateChangeCallback(
    OH_AudioSessionManager *audioSessionManager, OH_AudioSession_StateChangedCallback callback)
{
    OHAudioSessionManager *ohAudioSessionManager = convertManager(audioSessionManager);
    CHECK_AND_RETURN_RET_LOG(ohAudioSessionManager != nullptr,
        AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM, "ohAudioSessionManager is nullptr");
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM, "callback is nullptr");
    return ohAudioSessionManager->SetAudioSessionStateChangeCallback(callback);
}

OH_AudioCommon_Result OH_AudioSessionManager_UnregisterStateChangeCallback(
    OH_AudioSessionManager *audioSessionManager, OH_AudioSession_StateChangedCallback callback)
{
    OHAudioSessionManager *ohAudioSessionManager = convertManager(audioSessionManager);
    CHECK_AND_RETURN_RET_LOG(ohAudioSessionManager != nullptr,
        AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM, "ohAudioSessionManager is nullptr");
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM, "callback is nullptr");
    return ohAudioSessionManager->UnsetAudioSessionStateChangeCallback(callback);
}

OH_AudioCommon_Result OH_AudioSessionManager_SetDefaultOutputDevice(
    OH_AudioSessionManager *audioSessionManager, OH_AudioDevice_Type deviceType)
{
    OHAudioSessionManager *ohAudioSessionManager = convertManager(audioSessionManager);
    CHECK_AND_RETURN_RET_LOG(ohAudioSessionManager != nullptr,
        AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM, "ohAudioSessionManager is nullptr");
    CHECK_AND_RETURN_RET_LOG(((deviceType == AUDIO_DEVICE_TYPE_EARPIECE) ||
        (deviceType == AUDIO_DEVICE_TYPE_SPEAKER) ||
        (deviceType == AUDIO_DEVICE_TYPE_DEFAULT)),
        AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM, "deviceType is invalid");

    OHOS::AudioStandard::DeviceType type = static_cast<OHOS::AudioStandard::DeviceType>(deviceType);
    return ohAudioSessionManager->SetDefaultOutputDevice(type);
}

OH_AudioCommon_Result OH_AudioSessionManager_GetDefaultOutputDevice(
    OH_AudioSessionManager *audioSessionManager, OH_AudioDevice_Type *deviceType)
{
    OHAudioSessionManager *ohAudioSessionManager = convertManager(audioSessionManager);
    CHECK_AND_RETURN_RET_LOG(ohAudioSessionManager != nullptr,
        AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM, "ohAudioSessionManager is nullptr");
    CHECK_AND_RETURN_RET_LOG(deviceType != nullptr,
        AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM, "deviceType is nullptr");

    OHOS::AudioStandard::DeviceType type = OHOS::AudioStandard::DEVICE_TYPE_INVALID;
    OH_AudioCommon_Result ret = ohAudioSessionManager->GetDefaultOutputDevice(type);
    *deviceType = static_cast<OH_AudioDevice_Type>(type);
    return ret;
}

OH_AudioCommon_Result OH_AudioSessionManager_ReleaseDevices(
    OH_AudioSessionManager *audioSessionManager,
    OH_AudioDeviceDescriptorArray *audioDeviceDescriptorArray)
{
    OHAudioSessionManager *ohAudioSessionManager = convertManager(audioSessionManager);
    CHECK_AND_RETURN_RET_LOG(ohAudioSessionManager != nullptr,
        AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM, "ohAudioSessionManager is nullptr");
    CHECK_AND_RETURN_RET_LOG(audioDeviceDescriptorArray != nullptr,
        AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM, "audioDeviceDescriptorArray is nullptr");
    if ((audioDeviceDescriptorArray == nullptr) || (audioDeviceDescriptorArray->descriptors == nullptr)) {
        return AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM;
    }
    for (uint32_t index = 0; index < audioDeviceDescriptorArray->size; index++) {
        OHAudioDeviceDescriptor *ohAudioDeviceDescriptor =
            (OHAudioDeviceDescriptor*)audioDeviceDescriptorArray->descriptors[index];
        if (ohAudioDeviceDescriptor != nullptr) {
            delete ohAudioDeviceDescriptor;
        }
        audioDeviceDescriptorArray->descriptors[index] = nullptr;
    }
    free(audioDeviceDescriptorArray->descriptors);
    audioDeviceDescriptorArray->descriptors = nullptr;
    free(audioDeviceDescriptorArray);
    audioDeviceDescriptorArray = nullptr;
    return AUDIOCOMMON_RESULT_SUCCESS;
}

OH_AudioCommon_Result OH_AudioSessionManager_ReleaseDevice(
    OH_AudioSessionManager *audioSessionManager,
    OH_AudioDeviceDescriptor *audioDeviceDescriptor)
{
    OHAudioSessionManager *ohAudioSessionManager = convertManager(audioSessionManager);
    CHECK_AND_RETURN_RET_LOG(ohAudioSessionManager != nullptr,
        AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM, "ohAudioSessionManager is nullptr");
    CHECK_AND_RETURN_RET_LOG(audioDeviceDescriptor != nullptr,
        AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM, "audioDeviceDescriptor is nullptr");

    OHAudioDeviceDescriptor *ohAudioDeviceDescriptor =
            (OHAudioDeviceDescriptor*)audioDeviceDescriptor;
    if (ohAudioDeviceDescriptor != nullptr) {
        delete ohAudioDeviceDescriptor;
    }
    return AUDIOCOMMON_RESULT_SUCCESS;
}

OH_AudioCommon_Result OH_AudioSessionManager_RegisterCurrentOutputDeviceChangeCallback(
    OH_AudioSessionManager *audioSessionManager,
    OH_AudioSession_CurrentOutputDeviceChangedCallback callback)
{
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM, "callback is nullptr");

    OHAudioSessionManager* ohAudioSessionManager = convertManager(audioSessionManager);
    CHECK_AND_RETURN_RET_LOG(ohAudioSessionManager != nullptr,
        AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM, "ohAudioSessionManager is nullptr");
    return ohAudioSessionManager->SetAudioSessionCurrentDeviceChangeCallback(callback);
}

OH_AudioCommon_Result OH_AudioSessionManager_UnregisterCurrentOutputDeviceChangeCallback(
    OH_AudioSessionManager *audioSessionManager,
    OH_AudioSession_CurrentOutputDeviceChangedCallback callback)
{
    OHAudioSessionManager* ohAudioSessionManager = convertManager(audioSessionManager);
    CHECK_AND_RETURN_RET_LOG(ohAudioSessionManager != nullptr,
        AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM, "ohAudioSessionManager is nullptr");
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM, "callback is nullptr");
    return ohAudioSessionManager->UnsetAudioSessionCurrentDeviceChangeCallback(callback);
}

OH_AudioCommon_Result OH_AudioSessionManager_RegisterAvailableDevicesChangeCallback(
    OH_AudioSessionManager *audioSessionManager, OH_AudioDevice_Usage deviceUsage,
    OH_AudioSession_AvailableDeviceChangedCallback callback)
{
    CHECK_AND_RETURN_RET_LOG(audioSessionManager != nullptr,
        AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM, "audioSessionManager is nullptr");
    CHECK_AND_RETURN_RET_LOG(VALID_OH_AUDIO_DEVICE_UASGES.count(deviceUsage),
        AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM, "invalid deviceUsage");
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM, "callback is nullptr");
    OHAudioSessionManager* ohAudioSessionManager = convertManager(audioSessionManager);
    CHECK_AND_RETURN_RET_LOG(ohAudioSessionManager != nullptr,
        AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM, "ohAudioSessionManager is nullptr");
    AudioDeviceUsage usage = static_cast<AudioDeviceUsage>(deviceUsage);
    return ohAudioSessionManager->SetAvailableDeviceChangeCallback(usage, callback);
}

OH_AudioCommon_Result OH_AudioSessionManager_UnregisterAvailableDevicesChangeCallback(
    OH_AudioSessionManager *audioSessionManager,
    OH_AudioSession_AvailableDeviceChangedCallback callback)
{
    CHECK_AND_RETURN_RET_LOG(audioSessionManager != nullptr,
        AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM, "audioSessionManager is nullptr");
    OHAudioSessionManager* ohAudioSessionManager = convertManager(audioSessionManager);
    CHECK_AND_RETURN_RET_LOG(ohAudioSessionManager != nullptr,
        AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM, "ohAudioSessionManager is nullptr");
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM, "callback is nullptr");
    return ohAudioSessionManager->UnsetAvailableDeviceChangeCallback(callback);
}

OH_AudioCommon_Result OH_AudioSessionManager_RegisterCurrentInputDeviceChangeCallback(
    OH_AudioSessionManager *audioSessionManager,
    OH_AudioSession_CurrentInputDeviceChangedCallback callback)
{
    CHECK_AND_RETURN_RET_LOG(audioSessionManager != nullptr,
        AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM, "audioSessionManager is nullptr");
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM, "callback is nullptr");
    OHAudioSessionManager* ohAudioSessionManager = convertManager(audioSessionManager);
    CHECK_AND_RETURN_RET_LOG(ohAudioSessionManager != nullptr,
        AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM, "ohAudioSessionManager is nullptr");

    return ohAudioSessionManager->SetAudioSessionCurrentInputDeviceChangeCallback(callback);
}

OH_AudioCommon_Result OH_AudioSessionManager_UnregisterCurrentInputDeviceChangeCallback(
    OH_AudioSessionManager *audioSessionManager,
    OH_AudioSession_CurrentInputDeviceChangedCallback callback)
{
    CHECK_AND_RETURN_RET_LOG(audioSessionManager != nullptr,
        AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM, "audioSessionManager is nullptr");
    OHAudioSessionManager* ohAudioSessionManager = convertManager(audioSessionManager);
    CHECK_AND_RETURN_RET_LOG(ohAudioSessionManager != nullptr,
        AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM, "ohAudioSessionManager is nullptr");
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM, "callback is nullptr");
    return ohAudioSessionManager->UnsetAudioSessionCurrentInputDeviceChangeCallback(callback);
}

namespace OHOS {
namespace AudioStandard {

namespace {
const size_t MAX_VALID_SIZE = 128; // MAX AudioDevice size.
}

static void DestroyAudioDeviceDescriptor(OH_AudioDeviceDescriptorArray *array)
{
    if (array) {
        for (uint32_t index = 0; index < array->size; index++) {
            OHAudioDeviceDescriptor* ohAudioDeviceDescriptor = (OHAudioDeviceDescriptor*)array->descriptors[index];
            delete ohAudioDeviceDescriptor;
            array->descriptors[index] = nullptr;
        }
        free(array->descriptors);
        array->descriptors = nullptr;
        free(array);
        array = nullptr;
    }
}

OH_AudioDeviceDescriptorArray *ConvertDesc(
    const std::vector<std::shared_ptr<AudioDeviceDescriptor>> &desc)
{
    size_t size = desc.size();
    if (size == 0 || size >= MAX_VALID_SIZE) {
        AUDIO_ERR_LOG("failed to convert device info, size is %{public}zu", size);
        return nullptr;
    }

    OH_AudioDeviceDescriptorArray *audioDeviceDescriptorArray =
        (OH_AudioDeviceDescriptorArray *)malloc(sizeof(OH_AudioDeviceDescriptorArray));

    if (audioDeviceDescriptorArray == nullptr) {
        AUDIO_ERR_LOG("failed to malloc.");
        return nullptr;
    }
    audioDeviceDescriptorArray->size = 0;
    audioDeviceDescriptorArray->descriptors =
        (OH_AudioDeviceDescriptor **)malloc(sizeof(OH_AudioDeviceDescriptor *) * size);
    if (audioDeviceDescriptorArray->descriptors == nullptr) {
        free(audioDeviceDescriptorArray);
        audioDeviceDescriptorArray = nullptr;
        AUDIO_ERR_LOG("failed to malloc descriptors.");
        return nullptr;
    }

    uint32_t index = 0;
    for (auto deviceDescriptor : desc) {
        audioDeviceDescriptorArray->descriptors[index] =
            (OH_AudioDeviceDescriptor *)(new OHAudioDeviceDescriptor(deviceDescriptor));
        if (audioDeviceDescriptorArray->descriptors[index] == nullptr) {
            DestroyAudioDeviceDescriptor(audioDeviceDescriptorArray);
            return nullptr;
        }
        index++;
        audioDeviceDescriptorArray->size = index;
    }
    return audioDeviceDescriptorArray;
}

OHAudioSessionManager::OHAudioSessionManager()
{
    AUDIO_INFO_LOG("OHAudioSessionManager created!");
}

OHAudioSessionManager::~OHAudioSessionManager()
{
    AUDIO_INFO_LOG("OHAudioSessionManager destroyed!");
}

OH_AudioCommon_Result OHAudioSessionManager::SetAudioSessionCallback(OH_AudioSession_DeactivatedCallback callback)
{
    CHECK_AND_RETURN_RET_LOG(audioSessionManager_ != nullptr,
        AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM, "failed, audioSessionManager_ is null");
    std::shared_ptr<OHAudioSessionCallback> ohAudioSessionCallback =
        std::make_shared<OHAudioSessionCallback>(callback);
    audioSessionManager_->SetAudioSessionCallback(ohAudioSessionCallback);
    return AUDIOCOMMON_RESULT_SUCCESS;
}

OH_AudioCommon_Result OHAudioSessionManager::UnsetAudioSessionCallback(OH_AudioSession_DeactivatedCallback callback)
{
    CHECK_AND_RETURN_RET_LOG(audioSessionManager_ != nullptr,
        AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM, "failed, audioSessionManager_ is null");
    std::shared_ptr<OHAudioSessionCallback> ohAudioSessionCallback =
        std::make_shared<OHAudioSessionCallback>(callback);
    audioSessionManager_->UnsetAudioSessionCallback(ohAudioSessionCallback);
    return AUDIOCOMMON_RESULT_SUCCESS;
}

OH_AudioCommon_Result OHAudioSessionManager::ActivateAudioSession(const AudioSessionStrategy &strategy)
{
    CHECK_AND_RETURN_RET_LOG(audioSessionManager_ != nullptr,
        AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM, "failed, audioSessionManager_ is null");
    int32_t ret = audioSessionManager_->ActivateAudioSession(strategy);
    if (ret == 0) {
        return AUDIOCOMMON_RESULT_SUCCESS;
    } else {
        return AUDIOCOMMON_RESULT_ERROR_ILLEGAL_STATE;
    }
}

OH_AudioCommon_Result OHAudioSessionManager::DeactivateAudioSession()
{
    CHECK_AND_RETURN_RET_LOG(audioSessionManager_ != nullptr,
        AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM, "failed, audioSessionManager_ is null");
    int32_t ret = audioSessionManager_->DeactivateAudioSession();
    if (ret == 0) {
        return AUDIOCOMMON_RESULT_SUCCESS;
    } else {
        return AUDIOCOMMON_RESULT_ERROR_ILLEGAL_STATE;
    }
}

OH_AudioDeviceDescriptorArray *OHAudioSessionManager::GetAvailableDevices(AudioDeviceUsage deviceUsage)
{
    CHECK_AND_RETURN_RET_LOG(audioSessionManager_ != nullptr,
        nullptr, "failed, audioSessionManager_ is null");
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> tempDesc =
        audioSessionManager_->GetAvailableDevices(deviceUsage);
    if (tempDesc.size() == 0) {
        AUDIO_ERR_LOG("get no device");
        return nullptr;
    }
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> altaDesc = {};
    for (const auto &availableDesc : tempDesc) {
        std::shared_ptr<AudioDeviceDescriptor> dec = std::make_shared<AudioDeviceDescriptor>(*availableDesc);
        altaDesc.push_back(dec);
    }
    return ConvertDesc(altaDesc);
}

OH_AudioCommon_Result OHAudioSessionManager::SelectMediaInputDevice(
    const std::shared_ptr<AudioDeviceDescriptor> &deviceDescriptor)
{
    CHECK_AND_RETURN_RET_LOG(audioSessionManager_ != nullptr,
        AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM, "failed, audioSessionManager_ is null");
    int32_t result = audioSessionManager_->SelectInputDevice(deviceDescriptor);
    return result == 0 ? AUDIOCOMMON_RESULT_SUCCESS : AUDIOCOMMON_RESULT_ERROR_ILLEGAL_STATE;
}

OH_AudioDeviceDescriptor *OHAudioSessionManager::GetSelectedMediaInputDevice()
{
    CHECK_AND_RETURN_RET_LOG(audioSessionManager_ != nullptr,
        nullptr, "failed, audioSessionManager_ is null");
    std::shared_ptr<AudioDeviceDescriptor> deviceDescriptor = audioSessionManager_->GetSelectedInputDevice();
    return (OH_AudioDeviceDescriptor *)(new OHAudioDeviceDescriptor(deviceDescriptor));
}

OH_AudioCommon_Result OHAudioSessionManager::ClearSelectedMediaInputDevice()
{
    CHECK_AND_RETURN_RET_LOG(audioSessionManager_ != nullptr,
        AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM, "failed, audioSessionManager_ is null");
    int32_t result = audioSessionManager_->ClearSelectedInputDevice();
    return result == 0 ? AUDIOCOMMON_RESULT_SUCCESS : AUDIOCOMMON_RESULT_ERROR_ILLEGAL_STATE;
}

OH_AudioCommon_Result OHAudioSessionManager::PreferBluetoothAndNearlinkRecord(
    BluetoothAndNearlinkPreferredRecordCategory category)
{
    CHECK_AND_RETURN_RET_LOG(audioSessionManager_ != nullptr,
        AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM, "failed, audioSessionManager_ is null");
    int32_t result = audioSessionManager_->PreferBluetoothAndNearlinkRecord(category);
    return result == 0 ? AUDIOCOMMON_RESULT_SUCCESS : AUDIOCOMMON_RESULT_ERROR_ILLEGAL_STATE;
}

BluetoothAndNearlinkPreferredRecordCategory OHAudioSessionManager::GetPreferredBluetoothAndNearlinkRecord()
{
    CHECK_AND_RETURN_RET_LOG(audioSessionManager_ != nullptr,
        BluetoothAndNearlinkPreferredRecordCategory::PREFERRED_NONE,
        "failed, audioSessionManager_ is null");
    return audioSessionManager_->GetPreferBluetoothAndNearlinkRecord();
}

bool OHAudioSessionManager::IsAudioSessionActivated()
{
    CHECK_AND_RETURN_RET_LOG(audioSessionManager_ != nullptr, false, "failed, audioSessionManager_ is null");
    return audioSessionManager_->IsAudioSessionActivated();
}

OH_AudioCommon_Result OHAudioSessionManager::SetAudioSessionScene(AudioSessionScene sene)
{
    CHECK_AND_RETURN_RET_LOG(audioSessionManager_ != nullptr,
        AUDIOCOMMON_RESULT_ERROR_SYSTEM, "failed, audioSessionManager_ is null");

    int32_t ret = audioSessionManager_->SetAudioSessionScene(sene);
    if (ret == OHOS::AudioStandard::ERR_NOT_SUPPORTED) {
        AUDIO_ERR_LOG("session satet error, set scene failed.");
        return AUDIOCOMMON_RESULT_ERROR_ILLEGAL_STATE;
    } else if (ret != AUDIOCOMMON_RESULT_SUCCESS) {
        AUDIO_ERR_LOG("failed to SetAudioSessionScene.");
        return AUDIOCOMMON_RESULT_ERROR_SYSTEM;
    }

    return AUDIOCOMMON_RESULT_SUCCESS;
}

OH_AudioCommon_Result OHAudioSessionManager::SetAudioSessionStateChangeCallback(
    OH_AudioSession_StateChangedCallback callback)
{
    CHECK_AND_RETURN_RET_LOG(audioSessionManager_ != nullptr,
        AUDIOCOMMON_RESULT_ERROR_SYSTEM, "failed, audioSessionManager_ is null");

    if (callback == nullptr) {
        AUDIO_ERR_LOG("invalid callback");
        return AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM;
    }

    std::lock_guard<std::mutex> lock(sessionStateCbMutex_);
    if (sessionStateCallbacks_.count(callback) != 0) {
        AUDIO_INFO_LOG("callback already registed");
        return AUDIOCOMMON_RESULT_SUCCESS;
    }

    std::shared_ptr<OHAudioSessionStateCallback> ohAudioSessionStateCallback =
        std::make_shared<OHAudioSessionStateCallback>(callback);
    CHECK_AND_RETURN_RET_LOG(ohAudioSessionStateCallback != nullptr, AUDIOCOMMON_RESULT_ERROR_SYSTEM,
        "Failed to create AudioSessionState callback!");

    int32_t ret = audioSessionManager_->SetAudioSessionStateChangeCallback(ohAudioSessionStateCallback);
    if (ret != AUDIOCOMMON_RESULT_SUCCESS) {
        AUDIO_ERR_LOG("failed to SetAudioSessionStateChangeCallback.");
        return AUDIOCOMMON_RESULT_ERROR_SYSTEM;
    }
    sessionStateCallbacks_.emplace(callback, ohAudioSessionStateCallback);

    return AUDIOCOMMON_RESULT_SUCCESS;
}

OH_AudioCommon_Result OHAudioSessionManager::UnsetAudioSessionStateChangeCallback(
    OH_AudioSession_StateChangedCallback callback)
{
    CHECK_AND_RETURN_RET_LOG(audioSessionManager_ != nullptr,
        AUDIOCOMMON_RESULT_ERROR_SYSTEM, "failed, audioSessionManager_ is null");

    std::lock_guard<std::mutex> lock(sessionStateCbMutex_);
    if ((callback == nullptr) || (sessionStateCallbacks_.count(callback) == 0)) {
        AUDIO_ERR_LOG("invalid callback or callback not registered");
        return AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM;
    }

    int32_t ret = audioSessionManager_->UnsetAudioSessionStateChangeCallback(sessionStateCallbacks_[callback]);
    if (ret != AUDIOCOMMON_RESULT_SUCCESS) {
        AUDIO_ERR_LOG("failed to UnsetAudioSessionStateChangeCallback.");
        return AUDIOCOMMON_RESULT_ERROR_SYSTEM;
    }
    sessionStateCallbacks_.erase(callback);
    return AUDIOCOMMON_RESULT_SUCCESS;
}

OH_AudioCommon_Result OHAudioSessionManager::SetDefaultOutputDevice(DeviceType deviceType)
{
    CHECK_AND_RETURN_RET_LOG(audioSessionManager_ != nullptr,
        AUDIOCOMMON_RESULT_ERROR_SYSTEM, "failed, audioSessionManager_ is null");
    int32_t ret = audioSessionManager_->SetDefaultOutputDevice(deviceType);
    if (ret != AUDIOCOMMON_RESULT_SUCCESS) {
        AUDIO_ERR_LOG("system error when calling this function");
        return AUDIOCOMMON_RESULT_ERROR_SYSTEM;
    }
    return AUDIOCOMMON_RESULT_SUCCESS;
}

OH_AudioCommon_Result OHAudioSessionManager::GetDefaultOutputDevice(DeviceType &deviceType)
{
    CHECK_AND_RETURN_RET_LOG(audioSessionManager_ != nullptr,
        AUDIOCOMMON_RESULT_ERROR_SYSTEM, "failed, audioSessionManager_ is null");

    int32_t ret = audioSessionManager_->GetDefaultOutputDevice(deviceType);
    if (ret != AUDIOCOMMON_RESULT_SUCCESS) {
        AUDIO_ERR_LOG("failed to GetDefaultOutputDevice.");
        return AUDIOCOMMON_RESULT_ERROR_ILLEGAL_STATE;
    }
    return AUDIOCOMMON_RESULT_SUCCESS;
}

OH_AudioCommon_Result OHAudioSessionManager::SetAvailableDeviceChangeCallback(
    AudioDeviceUsage deviceUsage, OH_AudioSession_AvailableDeviceChangedCallback callback)
{
    CHECK_AND_RETURN_RET_LOG(audioMngr_ != nullptr,
        AUDIOCOMMON_RESULT_ERROR_SYSTEM, "failed, audioMngr_ is null");

    if (callback == nullptr) {
        AUDIO_ERR_LOG("invalid callback");
        return AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM;
    }

    std::lock_guard<std::mutex> lock(availableDeviceCbMutex_);
    if (availebleDeviceCallbacks_.count(callback) != 0) {
        AUDIO_INFO_LOG("callback already registed");
        return AUDIOCOMMON_RESULT_SUCCESS;
    }

    std::shared_ptr<OHAudioAvailableDeviceCallback> ohAudioAvailableDeviceCallback =
        std::make_shared<OHAudioAvailableDeviceCallback>(deviceUsage, callback);
    CHECK_AND_RETURN_RET_LOG(ohAudioAvailableDeviceCallback != nullptr, AUDIOCOMMON_RESULT_ERROR_SYSTEM,
        "Failed to create AudioAvailableDevice callback!");

    int32_t ret = audioMngr_->SetAvailableDeviceChangeCallback(deviceUsage, ohAudioAvailableDeviceCallback);
    if (ret != AUDIOCOMMON_RESULT_SUCCESS) {
        AUDIO_ERR_LOG("failed to SetAvailableDeviceChangeCallback.");
        return AUDIOCOMMON_RESULT_ERROR_SYSTEM;
    }
    availebleDeviceCallbacks_.emplace(callback, ohAudioAvailableDeviceCallback);

    return AUDIOCOMMON_RESULT_SUCCESS;
}

OH_AudioCommon_Result OHAudioSessionManager::UnsetAvailableDeviceChangeCallback(
    OH_AudioSession_AvailableDeviceChangedCallback callback)
{
    CHECK_AND_RETURN_RET_LOG(audioMngr_ != nullptr,
        AUDIOCOMMON_RESULT_ERROR_SYSTEM, "failed, audioMngr_ is null");

    std::lock_guard<std::mutex> lock(availableDeviceCbMutex_);
    if ((callback == nullptr) || (availebleDeviceCallbacks_.count(callback) == 0)) {
        AUDIO_ERR_LOG("invalid callback or callback not registered");
        return AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM;
    }

    int32_t ret = audioMngr_->UnsetAvailableDeviceChangeCallback(D_ALL_DEVICES);
    if (ret != AUDIOCOMMON_RESULT_SUCCESS) {
        AUDIO_ERR_LOG("failed to UnsetAvailableDeviceChangeCallback.");
        return AUDIOCOMMON_RESULT_ERROR_SYSTEM;
    }
    availebleDeviceCallbacks_.erase(callback);
    return AUDIOCOMMON_RESULT_SUCCESS;
}

OH_AudioCommon_Result OHAudioSessionManager::SetAudioSessionCurrentInputDeviceChangeCallback(
    OH_AudioSession_CurrentInputDeviceChangedCallback callback)
{
    CHECK_AND_RETURN_RET_LOG(audioSessionManager_ != nullptr,
        AUDIOCOMMON_RESULT_ERROR_SYSTEM, "failed, audioSessionManager_ is null");

    if (callback == nullptr) {
        AUDIO_ERR_LOG("invalid callback");
        return AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM;
    }

    std::lock_guard<std::mutex> lock(sessionInputDeviceCbMutex_);
    if (sessionInputDeviceCallbacks_.count(callback) != 0) {
        AUDIO_INFO_LOG("callback already registed");
        return AUDIOCOMMON_RESULT_SUCCESS;
    }

    std::shared_ptr<OHAudioSessionInputDeviceCallback> ohAudioSessionInputDeviceCallback =
        std::make_shared<OHAudioSessionInputDeviceCallback>(callback);
    CHECK_AND_RETURN_RET_LOG(ohAudioSessionInputDeviceCallback != nullptr, AUDIOCOMMON_RESULT_ERROR_SYSTEM,
        "Failed to create AudioSessionInputDevice callback!");

    int32_t ret =
        audioSessionManager_->SetAudioSessionCurrentInputDeviceChangeCallback(ohAudioSessionInputDeviceCallback);
    if (ret != AUDIOCOMMON_RESULT_SUCCESS) {
        AUDIO_ERR_LOG("failed to SetAudioSessionCurrentInputDeviceChangeCallback.");
        return AUDIOCOMMON_RESULT_ERROR_SYSTEM;
    }
    sessionInputDeviceCallbacks_.emplace(callback, ohAudioSessionInputDeviceCallback);

    return AUDIOCOMMON_RESULT_SUCCESS;
}

OH_AudioCommon_Result OHAudioSessionManager::UnsetAudioSessionCurrentInputDeviceChangeCallback(
    OH_AudioSession_CurrentInputDeviceChangedCallback callback)
{
    CHECK_AND_RETURN_RET_LOG(audioSessionManager_ != nullptr,
        AUDIOCOMMON_RESULT_ERROR_SYSTEM, "failed, audioSessionManager_ is null");

    std::lock_guard<std::mutex> lock(sessionInputDeviceCbMutex_);
    if ((callback == nullptr) || (sessionInputDeviceCallbacks_.count(callback) == 0)) {
        AUDIO_ERR_LOG("invalid callback or callback not registered");
        return AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM;
    }

    int32_t ret = audioSessionManager_->UnsetAudioSessionCurrentInputDeviceChangeCallback(
        sessionInputDeviceCallbacks_[callback]);
    if (ret != AUDIOCOMMON_RESULT_SUCCESS) {
        AUDIO_ERR_LOG("failed to UnsetAudioSessionCurrentInputDeviceChangeCallback.");
        return AUDIOCOMMON_RESULT_ERROR_SYSTEM;
    }
    sessionInputDeviceCallbacks_.erase(callback);
    return AUDIOCOMMON_RESULT_SUCCESS;
}

OH_AudioCommon_Result OHAudioSessionManager::SetAudioSessionCurrentDeviceChangeCallback(
    OH_AudioSession_CurrentOutputDeviceChangedCallback callback)
{
    CHECK_AND_RETURN_RET_LOG(audioSessionManager_ != nullptr,
        AUDIOCOMMON_RESULT_ERROR_SYSTEM, "failed, audioSessionManager_ is null");

    if (callback == nullptr) {
        AUDIO_ERR_LOG("invalid callback");
        return AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM;
    }

    std::lock_guard<std::mutex> lock(sessionDeviceCbMutex_);
    if (sessionDeviceCallbacks_.count(callback) != 0) {
        AUDIO_INFO_LOG("callback already registed");
        return AUDIOCOMMON_RESULT_SUCCESS;
    }

    std::shared_ptr<OHAudioSessionDeviceCallback> ohAudioSessionDeviceCallback =
        std::make_shared<OHAudioSessionDeviceCallback>(callback);
    CHECK_AND_RETURN_RET_LOG(ohAudioSessionDeviceCallback != nullptr, AUDIOCOMMON_RESULT_ERROR_SYSTEM,
        "Failed to create AudioSessionState callback!");

    int32_t ret = audioSessionManager_->SetAudioSessionCurrentDeviceChangeCallback(ohAudioSessionDeviceCallback);
    if (ret != AUDIOCOMMON_RESULT_SUCCESS) {
        AUDIO_ERR_LOG("failed to SetAudioSessionCurrentDeviceChangeCallback.");
        return AUDIOCOMMON_RESULT_ERROR_SYSTEM;
    }
    sessionDeviceCallbacks_.emplace(callback, ohAudioSessionDeviceCallback);

    return AUDIOCOMMON_RESULT_SUCCESS;
}

OH_AudioCommon_Result OHAudioSessionManager::UnsetAudioSessionCurrentDeviceChangeCallback(
    OH_AudioSession_CurrentOutputDeviceChangedCallback callback)
{
    CHECK_AND_RETURN_RET_LOG(audioSessionManager_ != nullptr,
        AUDIOCOMMON_RESULT_ERROR_SYSTEM, "failed, audioSessionManager_ is null");

    std::lock_guard<std::mutex> lock(sessionDeviceCbMutex_);
    if ((callback == nullptr) || (sessionDeviceCallbacks_.count(callback) == 0)) {
        AUDIO_ERR_LOG("invalid callback or callback not registered");
        return AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM;
    }

    int32_t ret = audioSessionManager_->UnsetAudioSessionCurrentDeviceChangeCallback(
        sessionDeviceCallbacks_[callback]);
    if (ret != AUDIOCOMMON_RESULT_SUCCESS) {
        AUDIO_ERR_LOG("failed to UnsetAudioSessionCurrentDeviceChangeCallback.");
        return AUDIOCOMMON_RESULT_ERROR_SYSTEM;
    }
    sessionDeviceCallbacks_.erase(callback);
    return AUDIOCOMMON_RESULT_SUCCESS;
}

void OHAudioSessionCallback::OnAudioSessionDeactive(const AudioSessionDeactiveEvent &deactiveEvent)
{
    OH_AudioSession_DeactivatedEvent event;
    event.reason = static_cast<OH_AudioSession_DeactivatedReason>(deactiveEvent.deactiveReason);
    callback_(event);
}

void OHAudioSessionStateCallback::OnAudioSessionStateChanged(const AudioSessionStateChangedEvent &stateChangedEvent)
{
    CHECK_AND_RETURN_LOG(callback_ != nullptr, "failed, pointer to the function is nullptr");
    OH_AudioSession_StateChangedEvent event;
    event.stateChangeHint = static_cast<OH_AudioSession_StateChangeHint>(stateChangedEvent.stateChangeHint);
    callback_(event);
}

void OHAudioSessionDeviceCallback::OnAudioSessionCurrentDeviceChanged(
    const CurrentOutputDeviceChangedEvent &deviceChangedEvent)
{
    CHECK_AND_RETURN_LOG(callback_ != nullptr, "failed, pointer to the function is nullptr");

    uint32_t size = deviceChangedEvent.devices.size();
    if ((size == 0) || (size > MAX_VALID_SIZE)) {
        AUDIO_ERR_LOG("audioDeviceDescriptors is null");
        return;
    }
    
    OH_AudioDeviceDescriptorArray *audioDeviceDescriptorArray =
        (OH_AudioDeviceDescriptorArray *)malloc(sizeof(OH_AudioDeviceDescriptorArray));
    if (audioDeviceDescriptorArray) {
        audioDeviceDescriptorArray->descriptors =
            (OH_AudioDeviceDescriptor**)malloc(sizeof(OH_AudioDeviceDescriptor*) * size);
        if (audioDeviceDescriptorArray->descriptors == nullptr) {
            free(audioDeviceDescriptorArray);
            audioDeviceDescriptorArray = nullptr;
            AUDIO_ERR_LOG("failed to malloc descriptors.");
            return;
        }
        audioDeviceDescriptorArray->size = size;
        uint32_t index = 0;
        for (auto deviceDescriptor : deviceChangedEvent.devices) {
            audioDeviceDescriptorArray->descriptors[index] =
                (OH_AudioDeviceDescriptor *)(new OHAudioDeviceDescriptor(deviceDescriptor));
            if (audioDeviceDescriptorArray->descriptors[index] == nullptr) {
                DestroyAudioDeviceDescriptor(audioDeviceDescriptorArray);
                return;
            }
            index++;
        }
    }

    callback_(audioDeviceDescriptorArray,
        static_cast<OH_AudioStream_DeviceChangeReason>(deviceChangedEvent.changeReason),
        static_cast<OH_AudioSession_OutputDeviceChangeRecommendedAction>(deviceChangedEvent.recommendedAction));
}

void OHAudioAvailableDeviceCallback::OnAvailableDeviceChange(const AudioDeviceUsage usage,
    const DeviceChangeAction &deviceChangeAction)
{
    CHECK_AND_RETURN_LOG(callback_ != nullptr, "failed, pointer to the function is nullptr");
    CHECK_AND_RETURN_LOG(usage == deviceUsage_, "other device usage's callback");

    uint32_t size = deviceChangeAction.deviceDescriptors.size();
    if ((size == 0) || (size > MAX_VALID_SIZE)) {
        AUDIO_ERR_LOG("audioDeviceDescriptors is null");
        return;
    }

    OH_AudioDeviceDescriptorArray *array = ConvertDesc(deviceChangeAction.deviceDescriptors);
    callback_(static_cast<OH_AudioDevice_ChangeType>(deviceChangeAction.type), array);
}

void OHAudioSessionInputDeviceCallback::OnAudioSessionCurrentInputDeviceChanged(
    const CurrentInputDeviceChangedEvent &deviceChangedEvent)
{
    CHECK_AND_RETURN_LOG(callback_ != nullptr, "failed, pointer to the function is nullptr");

    uint32_t size = deviceChangedEvent.devices.size();
    if ((size == 0) || (size > MAX_VALID_SIZE)) {
        AUDIO_ERR_LOG("audioDeviceDescriptors is null");
        return;
    }
    OH_AudioDeviceDescriptorArray *array = ConvertDesc(deviceChangedEvent.devices);
    callback_(array, static_cast<OH_AudioStream_DeviceChangeReason>(deviceChangedEvent.changeReason));
}
} // namespace AudioStandard
} // namespace OHOS