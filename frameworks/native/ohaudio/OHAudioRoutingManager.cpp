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

#include "OHAudioRoutingManager.h"

#include <set>

#include "audio_errors.h"
#include "audio_routing_manager.h"

namespace {
const size_t MAX_VALID_SIZE = 128; // MAX AudioDevice size.
const std::set<OH_AudioDevice_Usage> VALID_OH_AUDIO_DEVICE_UASGES = {
    AUDIO_DEVICE_USAGE_MEDIA_OUTPUT,
    AUDIO_DEVICE_USAGE_MEDIA_INPUT,
    AUDIO_DEVICE_USAGE_MEDIA_ALL,
    AUDIO_DEVICE_USAGE_CALL_OUTPUT,
    AUDIO_DEVICE_USAGE_CALL_INPUT,
    AUDIO_DEVICE_USAGE_CALL_ALL
};
const std::set<OH_AudioStream_Usage> VALID_OH_STREAM_USAGES = {
    AUDIOSTREAM_USAGE_UNKNOWN,
    AUDIOSTREAM_USAGE_MUSIC,
    AUDIOSTREAM_USAGE_VOICE_COMMUNICATION,
    AUDIOSTREAM_USAGE_VOICE_ASSISTANT,
    AUDIOSTREAM_USAGE_ALARM,
    AUDIOSTREAM_USAGE_VOICE_MESSAGE,
    AUDIOSTREAM_USAGE_RINGTONE,
    AUDIOSTREAM_USAGE_NOTIFICATION,
    AUDIOSTREAM_USAGE_ACCESSIBILITY,
    AUDIOSTREAM_USAGE_MOVIE,
    AUDIOSTREAM_USAGE_GAME,
    AUDIOSTREAM_USAGE_AUDIOBOOK,
    AUDIOSTREAM_USAGE_NAVIGATION,
    AUDIOSTREAM_USAGE_VIDEO_COMMUNICATION
};
const std::set<OH_AudioStream_SourceType> VALID_OH_SOURCE_TYPES = {
    AUDIOSTREAM_SOURCE_TYPE_MIC,
    AUDIOSTREAM_SOURCE_TYPE_VOICE_RECOGNITION,
    AUDIOSTREAM_SOURCE_TYPE_PLAYBACK_CAPTURE,
    AUDIOSTREAM_SOURCE_TYPE_VOICE_COMMUNICATION,
    AUDIOSTREAM_SOURCE_TYPE_VOICE_MESSAGE
};
}

using OHOS::AudioStandard::OHAudioRoutingManager;
using OHOS::AudioStandard::OHAudioDeviceDescriptor;
using OHOS::AudioStandard::AudioRoutingManager;
using OHOS::AudioStandard::DeviceFlag;
using OHOS::AudioStandard::AudioDeviceUsage;
using OHOS::AudioStandard::StreamUsage;
using OHOS::AudioStandard::SourceType;

static OHOS::AudioStandard::OHAudioRoutingManager *convertManager(OH_AudioRoutingManager* manager)
{
    return (OHAudioRoutingManager*) manager;
}

OH_AudioCommon_Result OH_AudioManager_GetAudioRoutingManager(OH_AudioRoutingManager **audioRoutingManager)
{
    OHAudioRoutingManager* ohAudioRoutingManager = OHAudioRoutingManager::GetInstance();
    *audioRoutingManager = (OH_AudioRoutingManager*)ohAudioRoutingManager;
    return AUDIOCOMMON_RESULT_SUCCESS;
}

OH_AudioCommon_Result OH_AudioRoutingManager_GetDevices(OH_AudioRoutingManager *audioRoutingManager,
    OH_AudioDevice_Flag deviceFlag, OH_AudioDeviceDescriptorArray **audioDeviceDescriptorArray)
{
    OHAudioRoutingManager* ohAudioRoutingManager = convertManager(audioRoutingManager);
    CHECK_AND_RETURN_RET_LOG(ohAudioRoutingManager != nullptr,
        AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM, "ohAudioRoutingManager is nullptr");
    CHECK_AND_RETURN_RET_LOG((
        deviceFlag == AUDIO_DEVICE_FLAG_NONE ||
        deviceFlag == AUDIO_DEVICE_FLAG_OUTPUT ||
        deviceFlag == AUDIO_DEVICE_FLAG_INPUT ||
        deviceFlag == AUDIO_DEVICE_FLAG_ALL),
        AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM, "deviceFlag is invalid");
    CHECK_AND_RETURN_RET_LOG(audioDeviceDescriptorArray != nullptr,
        AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM, "audioDeviceDescriptorArray is nullptr");
    DeviceFlag flag = static_cast<DeviceFlag>(deviceFlag);
    *audioDeviceDescriptorArray = ohAudioRoutingManager->GetDevices(flag);
    CHECK_AND_RETURN_RET_LOG(*audioDeviceDescriptorArray != nullptr,
        AUDIOCOMMON_RESULT_ERROR_NO_MEMORY, "*audioDeviceDescriptorArray is nullptr");
    return AUDIOCOMMON_RESULT_SUCCESS;
}

OH_AudioCommon_Result OH_AudioRoutingManager_GetAvailableDevices(OH_AudioRoutingManager *audioRoutingManager,
    OH_AudioDevice_Usage deviceUsage, OH_AudioDeviceDescriptorArray **audioDeviceDescriptorArray)
{
    if (audioRoutingManager == nullptr || !VALID_OH_AUDIO_DEVICE_UASGES.count(deviceUsage) ||
        audioDeviceDescriptorArray == nullptr) {
        AUDIO_ERR_LOG("Invalid params!");
        return AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM;
    }
    OHAudioRoutingManager* ohAudioRoutingManager = convertManager(audioRoutingManager);
    CHECK_AND_RETURN_RET_LOG(ohAudioRoutingManager != nullptr,
        AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM, "audioRoutingManager is nullptr");

    AudioDeviceUsage usage = static_cast<AudioDeviceUsage>(deviceUsage);
    *audioDeviceDescriptorArray = ohAudioRoutingManager->GetAvailableDevices(usage);
    CHECK_AND_RETURN_RET_LOG(*audioDeviceDescriptorArray != nullptr,
        AUDIOCOMMON_RESULT_ERROR_NO_MEMORY, "*audioDeviceDescriptorArray is nullptr");

    return AUDIOCOMMON_RESULT_SUCCESS;
}

OH_AudioCommon_Result OH_AudioRoutingManager_GetPreferredOutputDevice(OH_AudioRoutingManager *audioRoutingManager,
    OH_AudioStream_Usage streamUsage, OH_AudioDeviceDescriptorArray **audioDeviceDescriptorArray)
{
    if (audioRoutingManager == nullptr || !VALID_OH_STREAM_USAGES.count(streamUsage) ||
        audioDeviceDescriptorArray == nullptr) {
        AUDIO_ERR_LOG("Invalid params!");
        return AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM;
    }
    OHAudioRoutingManager* ohAudioRoutingManager = convertManager(audioRoutingManager);
    CHECK_AND_RETURN_RET_LOG(ohAudioRoutingManager != nullptr,
        AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM, "audioRoutingManager is nullptr");

    StreamUsage usage = static_cast<StreamUsage>(streamUsage);
    *audioDeviceDescriptorArray = ohAudioRoutingManager->GetPreferredOutputDevice(usage);
    CHECK_AND_RETURN_RET_LOG(*audioDeviceDescriptorArray != nullptr,
        AUDIOCOMMON_RESULT_ERROR_NO_MEMORY, "*audioDeviceDescriptorArray is nullptr");

    return AUDIOCOMMON_RESULT_SUCCESS;
}

OH_AudioCommon_Result OH_AudioRoutingManager_GetPreferredInputDevice(OH_AudioRoutingManager *audioRoutingManager,
    OH_AudioStream_SourceType sourceType, OH_AudioDeviceDescriptorArray **audioDeviceDescriptorArray)
{
    if (audioRoutingManager == nullptr || !VALID_OH_SOURCE_TYPES.count(sourceType) ||
        audioDeviceDescriptorArray == nullptr) {
        AUDIO_ERR_LOG("Invalid params!");
        return AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM;
    }
    OHAudioRoutingManager* ohAudioRoutingManager = convertManager(audioRoutingManager);
    CHECK_AND_RETURN_RET_LOG(ohAudioRoutingManager != nullptr,
        AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM, "audioRoutingManager is nullptr");

    SourceType type = static_cast<SourceType>(sourceType);
    *audioDeviceDescriptorArray = ohAudioRoutingManager->GetPreferredInputDevice(type);
    CHECK_AND_RETURN_RET_LOG(*audioDeviceDescriptorArray != nullptr,
        AUDIOCOMMON_RESULT_ERROR_NO_MEMORY, "*audioDeviceDescriptorArray is nullptr");

    return AUDIOCOMMON_RESULT_SUCCESS;
}

OH_AudioCommon_Result OH_AudioRoutingManager_RegisterDeviceChangeCallback(
    OH_AudioRoutingManager *audioRoutingManager, OH_AudioDevice_Flag deviceFlag,
    OH_AudioRoutingManager_OnDeviceChangedCallback callback)
{
    OHAudioRoutingManager* ohAudioRoutingManager = convertManager(audioRoutingManager);
    CHECK_AND_RETURN_RET_LOG(ohAudioRoutingManager != nullptr,
        AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM, "audioRoutingManager is nullptr");
    CHECK_AND_RETURN_RET_LOG((
        deviceFlag == AUDIO_DEVICE_FLAG_NONE ||
        deviceFlag == AUDIO_DEVICE_FLAG_OUTPUT ||
        deviceFlag == AUDIO_DEVICE_FLAG_INPUT ||
        deviceFlag == AUDIO_DEVICE_FLAG_ALL),
        AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM, "deviceFlag is invalid");
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM, "callback is nullptr");
    DeviceFlag flag = static_cast<DeviceFlag>(deviceFlag);
    ohAudioRoutingManager->SetDeviceChangeCallback(flag, callback);
    return AUDIOCOMMON_RESULT_SUCCESS;
}

OH_AudioCommon_Result OH_AudioRoutingManager_UnregisterDeviceChangeCallback(
    OH_AudioRoutingManager *audioRoutingManager,
    OH_AudioRoutingManager_OnDeviceChangedCallback callback)
{
    OHAudioRoutingManager* ohAudioRoutingManager = convertManager(audioRoutingManager);
    CHECK_AND_RETURN_RET_LOG(ohAudioRoutingManager != nullptr,
        AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM, "audioRoutingManager is nullptr");
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM, "callback is nullptr");
    DeviceFlag flag = static_cast<DeviceFlag>(AUDIO_DEVICE_FLAG_ALL);
    ohAudioRoutingManager->UnsetDeviceChangeCallback(flag, callback);
    return AUDIOCOMMON_RESULT_SUCCESS;
}

OH_AudioCommon_Result OH_AudioRoutingManager_ReleaseDevices(
    OH_AudioRoutingManager *audioRoutingManager,
    OH_AudioDeviceDescriptorArray *audioDeviceDescriptorArray)
{
    OHAudioRoutingManager* ohAudioRoutingManager = convertManager(audioRoutingManager);
    CHECK_AND_RETURN_RET_LOG(ohAudioRoutingManager != nullptr,
        AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM, "audioRoutingManager is nullptr");
    CHECK_AND_RETURN_RET_LOG(audioDeviceDescriptorArray != nullptr,
        AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM, "audioDeviceDescriptorArray is nullptr");
    if (audioDeviceDescriptorArray == nullptr) {
        return AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM;
    }
    for (uint32_t index = 0; index < audioDeviceDescriptorArray->size; index++) {
        OHAudioDeviceDescriptor* ohAudioDeviceDescriptor =
            (OHAudioDeviceDescriptor*)audioDeviceDescriptorArray->descriptors[index];
        delete ohAudioDeviceDescriptor;
        audioDeviceDescriptorArray->descriptors[index] = nullptr;
    }
    free(audioDeviceDescriptorArray->descriptors);
    audioDeviceDescriptorArray->descriptors = nullptr;
    free(audioDeviceDescriptorArray);
    audioDeviceDescriptorArray = nullptr;
    return AUDIOCOMMON_RESULT_SUCCESS;
}

namespace OHOS {
namespace AudioStandard {

void DestroyAudioDeviceDescriptor(OH_AudioDeviceDescriptorArray *array)
{
    if (array) {
        for (uint32_t index = 0; index < array->size; index++) {
            OHAudioDeviceDescriptor* ohAudioDeviceDescriptor = (OHAudioDeviceDescriptor*)array->descriptors[index];
            delete ohAudioDeviceDescriptor;
            array->descriptors[index] = nullptr;
        }
        free(array->descriptors);
        free(array);
    }
}

OHAudioRoutingManager::OHAudioRoutingManager()
{
    AUDIO_INFO_LOG("OHAudioRoutingManager created!");
}

OHAudioRoutingManager::~OHAudioRoutingManager()
{
    AUDIO_INFO_LOG("OHAudioRoutingManager destroyed!");
}

OH_AudioDeviceDescriptorArray *OHAudioRoutingManager::ConvertDesc(std::vector<sptr<AudioDeviceDescriptor>> &desc)
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

OH_AudioDeviceDescriptorArray* OHAudioRoutingManager::GetDevices(DeviceFlag deviceFlag)
{
    CHECK_AND_RETURN_RET_LOG(audioSystemManager_ != nullptr,
        nullptr, "failed, audioSystemManager is null");
    std::vector<sptr<AudioDeviceDescriptor>> audioDeviceDescriptors = audioSystemManager_->GetDevices(deviceFlag);
    uint32_t size = audioDeviceDescriptors.size();
    if (size <= 0) {
        AUDIO_ERR_LOG("audioDeviceDescriptors is null");
        return nullptr;
    }
    return ConvertDesc(audioDeviceDescriptors);
}

OH_AudioDeviceDescriptorArray *OHAudioRoutingManager::GetAvailableDevices(AudioDeviceUsage deviceUsage)
{
    std::vector<std::unique_ptr<AudioDeviceDescriptor>> tempDesc =
        AudioRoutingManager::GetInstance()->GetAvailableDevices(deviceUsage);
    if (tempDesc.size() == 0) {
        AUDIO_ERR_LOG("get no device");
        return nullptr;
    }
    std::vector<sptr<AudioDeviceDescriptor>> altaDesc = {};
    for (const auto &availableDesc : tempDesc) {
        sptr<AudioDeviceDescriptor> dec = new(std::nothrow) AudioDeviceDescriptor(*availableDesc);
        altaDesc.push_back(dec);
    }
    return ConvertDesc(altaDesc);
}

OH_AudioDeviceDescriptorArray *OHAudioRoutingManager::GetPreferredOutputDevice(StreamUsage streamUsage)
{
    AudioRendererInfo rendererInfo = {};
    rendererInfo.streamUsage = streamUsage;
    std::vector<sptr<AudioDeviceDescriptor>> desc = {};

    int32_t ret = AudioRoutingManager::GetInstance()->GetPreferredOutputDeviceForRendererInfo(rendererInfo, desc);
    if (ret != SUCCESS) {
        AUDIO_ERR_LOG("call failed!");
        return nullptr;
    }
    return ConvertDesc(desc);
}

OH_AudioDeviceDescriptorArray *OHAudioRoutingManager::GetPreferredInputDevice(SourceType sourceType)
{
    AudioCapturerInfo capturerInfo = {};
    capturerInfo.sourceType = sourceType;
    std::vector<sptr<AudioDeviceDescriptor>> desc = {};

    int32_t ret = AudioRoutingManager::GetInstance()->GetPreferredInputDeviceForCapturerInfo(capturerInfo, desc);
    if (ret != SUCCESS) {
        AUDIO_ERR_LOG("call failed!");
        return nullptr;
    }
    return ConvertDesc(desc);
}

OH_AudioCommon_Result OHAudioRoutingManager::SetDeviceChangeCallback(const DeviceFlag deviceFlag,
    OH_AudioRoutingManager_OnDeviceChangedCallback callback)
{
    CHECK_AND_RETURN_RET_LOG(audioSystemManager_ != nullptr,
        AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM, "failed, audioSystemManager is null");
    std::shared_ptr<OHAudioDeviceChangedCallback> ohAudioOnDeviceChangedCallback =
        std::make_shared<OHAudioDeviceChangedCallback>(callback);
    if (ohAudioOnDeviceChangedCallback) {
        audioSystemManager_->SetDeviceChangeCallback(deviceFlag, ohAudioOnDeviceChangedCallback);
        ohAudioOnDeviceChangedCallbackArray_.push_back(ohAudioOnDeviceChangedCallback);
        return AUDIOCOMMON_RESULT_SUCCESS;
    }
    return AUDIOCOMMON_RESULT_ERROR_NO_MEMORY;
}

OH_AudioCommon_Result OHAudioRoutingManager::UnsetDeviceChangeCallback(DeviceFlag deviceFlag,
    OH_AudioRoutingManager_OnDeviceChangedCallback ohOnDeviceChangedcallback)
{
    CHECK_AND_RETURN_RET_LOG(audioSystemManager_ != nullptr,
        AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM, "failed, audioSystemManager is null");
    auto iter = std::find_if(ohAudioOnDeviceChangedCallbackArray_.begin(), ohAudioOnDeviceChangedCallbackArray_.end(),
        [&](const std::shared_ptr<OHAudioDeviceChangedCallback> &item) {
        return item->GetCallback() == ohOnDeviceChangedcallback;
    });
    if (iter == ohAudioOnDeviceChangedCallbackArray_.end()) {
        return AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM;
    }
    audioSystemManager_->UnsetDeviceChangeCallback(deviceFlag);
    ohAudioOnDeviceChangedCallbackArray_.erase(iter);
    return AUDIOCOMMON_RESULT_SUCCESS;
}

void OHAudioDeviceChangedCallback::OnDeviceChange(const DeviceChangeAction &deviceChangeAction)
{
    CHECK_AND_RETURN_LOG(callback_ != nullptr, "failed, pointer to the fuction is nullptr");
    OH_AudioDevice_ChangeType type = static_cast<OH_AudioDevice_ChangeType>(deviceChangeAction.type);
    uint32_t size = deviceChangeAction.deviceDescriptors.size();
    if (size <= 0) {
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
        for (auto deviceDescriptor : deviceChangeAction.deviceDescriptors) {
            audioDeviceDescriptorArray->descriptors[index] =
                (OH_AudioDeviceDescriptor *)(new OHAudioDeviceDescriptor(deviceDescriptor));
            if (audioDeviceDescriptorArray->descriptors[index] == nullptr) {
                DestroyAudioDeviceDescriptor(audioDeviceDescriptorArray);
                return;
            }
            index++;
        }
    }
    callback_(type, audioDeviceDescriptorArray);
}
}  // namespace AudioStandard
}  // namespace OHOS