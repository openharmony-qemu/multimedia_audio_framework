/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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
#define LOG_TAG "AudioServer"
#endif

#include "audio_server.h"

#include "audio_effect_chain_manager.h"
#include "audio_enhance_chain_manager.h"
#include "common/hdi_adapter_info.h"
#include "manager/hdi_adapter_manager.h"
#include "i_hpae_manager.h"
#include "audio_utils.h"

namespace OHOS {
namespace AudioStandard {
using namespace std;

void AudioServer::RecognizeAudioEffectType(const std::string &mainkey, const std::string &subkey,
    const std::string &extraSceneType)
{
    int32_t engineFlag = GetEngineFlag();
    if (engineFlag == 1) {
        HPAE::IHpaeManager::GetHpaeManager().UpdateParamExtra(mainkey, subkey, extraSceneType);
    } else {
        AudioEffectChainManager *audioEffectChainManager = AudioEffectChainManager::GetInstance();
        if (audioEffectChainManager == nullptr) {
            AUDIO_ERR_LOG("audioEffectChainManager is nullptr!");
            return;
        }
        audioEffectChainManager->UpdateParamExtra(mainkey, subkey, extraSceneType);
        
        AudioEnhanceChainManager *audioEnhanceChainManager = AudioEnhanceChainManager::GetInstance();
        CHECK_AND_RETURN_LOG(audioEnhanceChainManager != nullptr, "audioEnhanceChainManager is nullptr!");
        return audioEnhanceChainManager->UpdateExtraSceneType(mainkey, subkey, extraSceneType);
    }
}

// LCOV_EXCL_START
int32_t AudioServer::CreateEffectChainManager(const std::vector<EffectChain> &effectChains,
    const EffectChainManagerParam &effectParam, const EffectChainManagerParam &enhanceParam)
{
    CHECK_AND_RETURN_RET_LOG(effectChains.size() >= 0 && effectChains.size() <= AUDIO_EFFECT_CHAIN_COUNT_UPPER_LIMIT,
        AUDIO_ERR, "Create audio effect chains failed, invalid countChains!");
    CHECK_AND_RETURN_RET_LOG(PermissionUtil::VerifyIsAudio(), ERR_PERMISSION_DENIED, "not audio calling!");
    int32_t engineFlag = GetEngineFlag();
    if (engineFlag == 1) {
        HPAE::IHpaeManager::GetHpaeManager().InitAudioEffectChainManager(effectChains, effectParam,
            audioEffectServer_->GetEffectEntries());
        HPAE::IHpaeManager::GetHpaeManager().InitAudioEnhanceChainManager(effectChains, enhanceParam,
            audioEffectServer_->GetEffectEntries());
        AUDIO_INFO_LOG("AudioEffectChainManager Init");
    } else {
        AudioEffectChainManager *audioEffectChainManager = AudioEffectChainManager::GetInstance();
        audioEffectChainManager->InitAudioEffectChainManager(effectChains, effectParam,
            audioEffectServer_->GetEffectEntries());
        AudioEnhanceChainManager *audioEnhanceChainManager = AudioEnhanceChainManager::GetInstance();
        audioEnhanceChainManager->InitAudioEnhanceChainManager(effectChains, enhanceParam,
            audioEffectServer_->GetEffectEntries());
    }
    return SUCCESS;
}

int32_t AudioServer::SetOutputDeviceSink(int32_t deviceType, const std::string &sinkName)
{
    CHECK_AND_RETURN_RET_LOG(deviceType >= DEVICE_TYPE_NONE && deviceType <= DEVICE_TYPE_MAX, AUDIO_ERR,
        "Set output device sink failed, please check log");
    Trace trace("AudioServer::SetOutputDeviceSink:" + std::to_string(deviceType) + " sink:" + sinkName);
    if (!PermissionUtil::VerifyIsAudio()) {
        AUDIO_ERR_LOG("not audio calling!");
        return SUCCESS;
    }

    int32_t engineFlag = GetEngineFlag();
    if (engineFlag == 1) {
        HPAE::IHpaeManager::GetHpaeManager().SetOutputDeviceSink(deviceType, sinkName);
    } else {
        AudioEffectChainManager *audioEffectChainManager = AudioEffectChainManager::GetInstance();
        audioEffectChainManager->SetOutputDeviceSink(deviceType, sinkName);
    }
    return SUCCESS;
}

int32_t AudioServer::UpdateSpatializationState(const AudioSpatializationState& spatializationState)
{
    int32_t callingUid = IPCSkeleton::GetCallingUid();
    CHECK_AND_RETURN_RET_LOG(PermissionUtil::VerifyIsAudio(), ERR_NOT_SUPPORTED, "refused for %{public}d", callingUid);

    int32_t engineFlag = GetEngineFlag();
    if (engineFlag == 1) {
        return HPAE::IHpaeManager::GetHpaeManager().UpdateSpatializationState(spatializationState);
    } else {
        AudioEffectChainManager *audioEffectChainManager = AudioEffectChainManager::GetInstance();
        if (audioEffectChainManager == nullptr) {
            AUDIO_ERR_LOG("audioEffectChainManager is nullptr!");
            return ERROR;
        }
        return audioEffectChainManager->UpdateSpatializationState(spatializationState);
    }
}

int32_t AudioServer::UpdateSpatialDeviceType(int32_t spatialDeviceType)
{
    int32_t callingUid = IPCSkeleton::GetCallingUid();
    CHECK_AND_RETURN_RET_LOG(PermissionUtil::VerifyIsAudio(), ERR_NOT_SUPPORTED, "refused for %{public}d", callingUid);

    int32_t engineFlag = GetEngineFlag();
    if (engineFlag == 1) {
        return HPAE::IHpaeManager::GetHpaeManager().UpdateSpatialDeviceType(
            static_cast<AudioSpatialDeviceType>(spatialDeviceType));
    } else {
        AudioEffectChainManager *audioEffectChainManager = AudioEffectChainManager::GetInstance();
        CHECK_AND_RETURN_RET_LOG(audioEffectChainManager != nullptr, ERROR, "audioEffectChainManager is nullptr!");

        return audioEffectChainManager->UpdateSpatialDeviceType(
            static_cast<AudioSpatialDeviceType>(spatialDeviceType));
    }
}
// LCOV_EXCL_STOP

int32_t AudioServer::SetSystemVolumeToEffect(const AudioStreamType streamType, float volume)
{
    AudioVolumeType systemVolumeType = VolumeUtils::GetVolumeTypeFromStreamType(streamType);

    int32_t engineFlag = GetEngineFlag();
    if (engineFlag == 1) {
        HPAE::IHpaeManager::GetHpaeManager().SetEffectSystemVolume(systemVolumeType, volume);
    } else {
        AudioEffectChainManager *audioEffectChainManager = AudioEffectChainManager::GetInstance();
        CHECK_AND_RETURN_RET_LOG(audioEffectChainManager != nullptr, ERROR, "audioEffectChainManager is nullptr");
        AUDIO_INFO_LOG("streamType: %{public}d, systemVolume: %{public}f", streamType, volume);
        audioEffectChainManager->SetEffectSystemVolume(systemVolumeType, volume);

        audioEffectChainManager->EffectVolumeUpdate();
    }
    return SUCCESS;
}

// LCOV_EXCL_START
int32_t AudioServer::SetSpatializationSceneType(int32_t spatializationSceneType)
{
    int32_t callingUid = IPCSkeleton::GetCallingUid();
    CHECK_AND_RETURN_RET_LOG(PermissionUtil::VerifyIsAudio(), ERR_NOT_SUPPORTED, "refused for %{public}d", callingUid);

    int32_t engineFlag = GetEngineFlag();
    if (engineFlag == 1) {
        return HPAE::IHpaeManager::GetHpaeManager().SetSpatializationSceneType(
            static_cast<AudioSpatializationSceneType>(spatializationSceneType));
    } else {
        AudioEffectChainManager *audioEffectChainManager = AudioEffectChainManager::GetInstance();
        CHECK_AND_RETURN_RET_LOG(audioEffectChainManager != nullptr, ERROR, "audioEffectChainManager is nullptr");
        return audioEffectChainManager->SetSpatializationSceneType(
            static_cast<AudioSpatializationSceneType>(spatializationSceneType));
    }
}
// LCOV_EXCL_STOP

int32_t AudioServer::GetEffectLatency(const std::string &sessionId, uint32_t& latency)
{
    AudioEffectChainManager *audioEffectChainManager = AudioEffectChainManager::GetInstance();
    CHECK_AND_RETURN_RET_LOG(audioEffectChainManager != nullptr, ERROR, "audioEffectChainManager is nullptr");
    latency = audioEffectChainManager->GetLatency(sessionId);
    return SUCCESS;
}

// LCOV_EXCL_START
int32_t AudioServer::GetEffectOffloadEnabled(bool& isEffectOffloadEnabled)
{
    int32_t callingUid = IPCSkeleton::GetCallingUid();
    CHECK_AND_RETURN_RET_LOG(PermissionUtil::VerifyIsAudio(), ERR_NOT_SUPPORTED, "refused for %{public}d", callingUid);

    AudioEffectChainManager *audioEffectChainManager = AudioEffectChainManager::GetInstance();
    CHECK_AND_RETURN_RET_LOG(audioEffectChainManager != nullptr, ERROR, "audioEffectChainManager is nullptr");
    isEffectOffloadEnabled = audioEffectChainManager->GetOffloadEnabled();
    return SUCCESS;
}

int32_t AudioServer::LoadHdiEffectModel()
{
    int32_t callingUid = IPCSkeleton::GetCallingUid();
    CHECK_AND_RETURN_RET_LOG(PermissionUtil::VerifyIsAudio(), ERR_PERMISSION_DENIED,
        "load hdi effect model refused for %{public}d", callingUid);

    int32_t engineFlag = GetEngineFlag();
    if (engineFlag == 1) {
        HPAE::IHpaeManager::GetHpaeManager().InitHdiState();
    } else {
        AudioEffectChainManager *audioEffectChainManager = AudioEffectChainManager::GetInstance();
        CHECK_AND_RETURN_RET_LOG(audioEffectChainManager != nullptr, ERROR, "audioEffectChainManager is nullptr");
        audioEffectChainManager->InitHdiState();
    }
    return SUCCESS;
}

int32_t AudioServer::SetAudioEffectProperty(const AudioEffectPropertyArrayV3 &propertyArray,
    int32_t deviceType)
{
    size_t size = propertyArray.property.size();
    CHECK_AND_RETURN_RET_LOG(size > 0 && size <= AUDIO_EFFECT_COUNT_UPPER_LIMIT,
        ERROR_INVALID_PARAM, "audio enhance property array size invalid");
    int32_t callingUid = IPCSkeleton::GetCallingUid();
    CHECK_AND_RETURN_RET_LOG(PermissionUtil::VerifyIsAudio(), ERR_PERMISSION_DENIED,
                             "SetA udio Effect Property refused for %{public}d", callingUid);
    AudioEffectPropertyArrayV3 effectPropertyArray = {};
    AudioEffectPropertyArrayV3 enhancePropertyArray = {};
    for (auto &item : propertyArray.property) {
        if (item.flag == CAPTURE_EFFECT_FLAG) {
            enhancePropertyArray.property.push_back(item);
        } else {
            effectPropertyArray.property.push_back(item);
        }
    }
    if (enhancePropertyArray.property.size() > 0) {
        CHECK_AND_RETURN_RET_LOG(SetAudioEnhanceChainProperty(
            enhancePropertyArray, static_cast<DeviceType>(deviceType)) == AUDIO_OK,
            ERR_OPERATION_FAILED, "set audio enhancce property failed");
    }
    if (effectPropertyArray.property.size() > 0) {
        CHECK_AND_RETURN_RET_LOG(SetAudioEffectChainProperty(effectPropertyArray) == AUDIO_OK,
            ERR_OPERATION_FAILED, "set audio effect property failed");
    }
    return AUDIO_OK;
}

int32_t AudioServer::GetAudioEffectProperty(AudioEffectPropertyArrayV3 &propertyArray, int32_t deviceType)
{
    int32_t callingUid = IPCSkeleton::GetCallingUid();
    CHECK_AND_RETURN_RET_LOG(PermissionUtil::VerifyIsAudio(), ERR_PERMISSION_DENIED,
        "get audio effect property refused for %{public}d", callingUid);
    AudioEffectPropertyArrayV3 effectPropertyArray = {};
    (void)GetAudioEffectPropertyArray(effectPropertyArray);
    propertyArray.property.insert(propertyArray.property.end(),
        effectPropertyArray.property.begin(), effectPropertyArray.property.end());

    AudioEffectPropertyArrayV3 enhancePropertyArray = {};
    (void)GetAudioEnhancePropertyArray(enhancePropertyArray, static_cast<DeviceType>(deviceType));
    propertyArray.property.insert(propertyArray.property.end(),
        enhancePropertyArray.property.begin(), enhancePropertyArray.property.end());

    size_t size = propertyArray.property.size();
    CHECK_AND_RETURN_RET_LOG(size <= AUDIO_EFFECT_COUNT_UPPER_LIMIT,
        ERROR_INVALID_PARAM, "audio enhance property array size invalid");
    return AUDIO_OK;
}

int32_t AudioServer::SetAudioEffectProperty(const AudioEffectPropertyArray &propertyArray)
{
    size_t size = propertyArray.property.size();
    CHECK_AND_RETURN_RET_LOG(size > 0 && size <= AUDIO_EFFECT_COUNT_UPPER_LIMIT,
        ERROR_INVALID_PARAM, "audio enhance property array size invalid");
    int32_t callingUid = IPCSkeleton::GetCallingUid();
    CHECK_AND_RETURN_RET_LOG(PermissionUtil::VerifyIsAudio(), ERR_PERMISSION_DENIED,
        "SetA udio Effect Property refused for %{public}d", callingUid);

    int32_t engineFlag = GetEngineFlag();
    if (engineFlag == 1) {
        return HPAE::IHpaeManager::GetHpaeManager().SetAudioEffectProperty(propertyArray);
    } else {
        AudioEffectChainManager *audioEffectChainManager = AudioEffectChainManager::GetInstance();
        CHECK_AND_RETURN_RET_LOG(audioEffectChainManager != nullptr, ERROR, "audioEffectChainManager is nullptr");
        return audioEffectChainManager->SetAudioEffectProperty(propertyArray);
    }
}

int32_t AudioServer::GetAudioEffectProperty(AudioEffectPropertyArray &propertyArray)
{
    int32_t callingUid = IPCSkeleton::GetCallingUid();
    CHECK_AND_RETURN_RET_LOG(PermissionUtil::VerifyIsAudio(), ERR_PERMISSION_DENIED,
        "Get Audio Effect Property refused for %{public}d", callingUid);
    AudioEffectChainManager *audioEffectChainManager = AudioEffectChainManager::GetInstance();
    CHECK_AND_RETURN_RET_LOG(audioEffectChainManager != nullptr, ERROR, "audioEffectChainManager is nullptr");
    int32_t ret = audioEffectChainManager->GetAudioEffectProperty(propertyArray);
    size_t size = propertyArray.property.size();
    CHECK_AND_RETURN_RET_LOG(size <= AUDIO_EFFECT_COUNT_UPPER_LIMIT,
        ERROR_INVALID_PARAM, "audio enhance property array size invalid");
    return ret;
}

int32_t AudioServer::SetAudioEnhanceProperty(const AudioEnhancePropertyArray &propertyArray,
    int32_t deviceType)
{
    size_t size = propertyArray.property.size();
    CHECK_AND_RETURN_RET_LOG(size > 0 && size <= AUDIO_EFFECT_COUNT_UPPER_LIMIT,
        ERROR_INVALID_PARAM, "Audio enhance property array size invalid");
    int32_t callingUid = IPCSkeleton::GetCallingUid();
    CHECK_AND_RETURN_RET_LOG(PermissionUtil::VerifyIsAudio(), ERR_PERMISSION_DENIED,
        "Set Audio Enhance Property refused for %{public}d", callingUid);
    int32_t engineFlag = GetEngineFlag();
    if (engineFlag == 1) {
        return HPAE::IHpaeManager::GetHpaeManager().SetAudioEnhanceProperty(
            propertyArray, static_cast<DeviceType>(deviceType));
    } else {
        AudioEnhanceChainManager *audioEnhanceChainManager = AudioEnhanceChainManager::GetInstance();
        CHECK_AND_RETURN_RET_LOG(audioEnhanceChainManager != nullptr, ERROR, "audioEnhanceChainManager is nullptr");
        return audioEnhanceChainManager->SetAudioEnhanceProperty(propertyArray, static_cast<DeviceType>(deviceType));
    }
}

int32_t AudioServer::GetAudioEnhanceProperty(AudioEnhancePropertyArray &propertyArray,
    int32_t deviceType)
{
    int32_t callingUid = IPCSkeleton::GetCallingUid();
    CHECK_AND_RETURN_RET_LOG(PermissionUtil::VerifyIsAudio(), ERR_PERMISSION_DENIED,
        "Get Audio Enhance Property refused for %{public}d", callingUid);
    AudioEnhanceChainManager *audioEnhanceChainManager = AudioEnhanceChainManager::GetInstance();
    CHECK_AND_RETURN_RET_LOG(audioEnhanceChainManager != nullptr, ERROR, "audioEnhanceChainManager is nullptr");
    int32_t ret = audioEnhanceChainManager->GetAudioEnhanceProperty(propertyArray, static_cast<DeviceType>(deviceType));
    size_t size = propertyArray.property.size();
    CHECK_AND_RETURN_RET_LOG(size <= AUDIO_EFFECT_COUNT_UPPER_LIMIT,
        ERROR_INVALID_PARAM, "Audio enhance property array size invalid");
    return ret;
}
// LCOV_EXCL_STOP

int32_t AudioServer::SetAudioEffectChainProperty(const AudioEffectPropertyArrayV3 &propertyArray)
{
    int32_t engineFlag = GetEngineFlag();
    if (engineFlag == 1) {
        return HPAE::IHpaeManager::GetHpaeManager().SetAudioEffectProperty(propertyArray);
        return SUCCESS;
    } else {
        AudioEffectChainManager *audioEffectChainManager = AudioEffectChainManager::GetInstance();
        CHECK_AND_RETURN_RET_LOG(audioEffectChainManager != nullptr, ERROR, "audioEffectChainManager is nullptr");
        return audioEffectChainManager->SetAudioEffectProperty(propertyArray);
    }
}

int32_t AudioServer::SetAudioEnhanceChainProperty(const AudioEffectPropertyArrayV3 &propertyArray,
    const DeviceType& deviceType)
{
    int32_t engineFlag = GetEngineFlag();
    if (engineFlag == 1) {
        return HPAE::IHpaeManager::GetHpaeManager().SetAudioEnhanceProperty(propertyArray, deviceType);
    } else {
        AudioEnhanceChainManager *audioEnhanceChainManager = AudioEnhanceChainManager::GetInstance();
        CHECK_AND_RETURN_RET_LOG(audioEnhanceChainManager != nullptr, ERROR, "audioEnhanceChainManager is nullptr");
        return audioEnhanceChainManager->SetAudioEnhanceProperty(propertyArray, deviceType);
    }
}

int32_t AudioServer::GetAudioEffectPropertyArray(AudioEffectPropertyArrayV3 &propertyArray)
{
    AudioEffectChainManager *audioEffectChainManager = AudioEffectChainManager::GetInstance();
    CHECK_AND_RETURN_RET_LOG(audioEffectChainManager != nullptr, ERROR, "audioEffectChainManager is nullptr");
    return audioEffectChainManager->GetAudioEffectProperty(propertyArray);
}

int32_t AudioServer::GetAudioEnhancePropertyArray(AudioEffectPropertyArrayV3 &propertyArray,
    const DeviceType& deviceType)
{
    AudioEnhanceChainManager *audioEnhanceChainManager = AudioEnhanceChainManager::GetInstance();
    CHECK_AND_RETURN_RET_LOG(audioEnhanceChainManager != nullptr, ERROR, "audioEnhanceChainManager is nullptr");
    return audioEnhanceChainManager->GetAudioEnhanceProperty(propertyArray, deviceType);
}

// LCOV_EXCL_START
int32_t AudioServer::UpdateEffectBtOffloadSupported(bool isSupported)
{
    int32_t callingUid = IPCSkeleton::GetCallingUid();
    CHECK_AND_RETURN_RET_LOG(PermissionUtil::VerifyIsAudio(), ERR_PERMISSION_DENIED,
        "refused for %{public}d", callingUid);

    int32_t engineFlag = GetEngineFlag();
    if (engineFlag == 1) {
        HPAE::IHpaeManager::GetHpaeManager().UpdateEffectBtOffloadSupported(isSupported);
    } else {
        AudioEffectChainManager *audioEffectChainManager = AudioEffectChainManager::GetInstance();
        CHECK_AND_RETURN_RET_LOG(audioEffectChainManager != nullptr, ERROR, "audioEffectChainManager is nullptr");
        audioEffectChainManager->UpdateEffectBtOffloadSupported(isSupported);
    }
    return SUCCESS;
}

int32_t AudioServer::SetRotationToEffect(const uint32_t rotate)
{
    int32_t callingUid = IPCSkeleton::GetCallingUid();
    CHECK_AND_RETURN_RET_LOG(PermissionUtil::VerifyIsAudio(), ERR_PERMISSION_DENIED,
        "set rotation to effect refused for %{public}d", callingUid);

    int32_t engineFlag = GetEngineFlag();
    if (engineFlag == 1) {
        HPAE::IHpaeManager::GetHpaeManager().EffectRotationUpdate(rotate);
    } else {
        AudioEffectChainManager *audioEffectChainManager = AudioEffectChainManager::GetInstance();
        CHECK_AND_RETURN_RET_LOG(audioEffectChainManager != nullptr, ERROR, "audioEffectChainManager is nullptr");
        audioEffectChainManager->EffectRotationUpdate(rotate);
    }

    std::string value = "rotation=" + std::to_string(rotate);
    HdiAdapterManager &manager = HdiAdapterManager::GetInstance();
    std::shared_ptr<IDeviceManager> deviceManager = manager.GetDeviceManager(HDI_DEVICE_MANAGER_TYPE_LOCAL);
    CHECK_AND_RETURN_RET_LOG(deviceManager != nullptr, ERROR, "local device manager is nullptr");
    deviceManager->SetAudioParameter("primary", AudioParamKey::NONE, "", value);
    return SUCCESS;
}
// LCOV_EXCL_STOP

int32_t AudioServer::SetVolumeInfoForEnhanceChain(const AudioStreamType &streamType)
{
    AudioVolumeType volumeType = VolumeUtils::GetVolumeTypeFromStreamType(streamType);
    DeviceType deviceType = PolicyHandler::GetInstance().GetActiveOutPutDevice();
    Volume vol = {false, 0.0f, 0};
    PolicyHandler::GetInstance().GetSharedVolume(volumeType, deviceType, vol);
    float systemVol = vol.isMute ? 0.0f : vol.volumeFloat;
    if (PolicyHandler::GetInstance().IsAbsVolumeSupported() &&
        PolicyHandler::GetInstance().GetActiveOutPutDevice() == DEVICE_TYPE_BLUETOOTH_A2DP) {
        systemVol = 1.0f; // 1.0f for a2dp abs volume
    }

    int32_t engineFlag = GetEngineFlag();
    if (engineFlag == 1) {
        return HPAE::IHpaeManager::GetHpaeManager().SetVolumeInfo(volumeType, systemVol);
    } else {
        AudioEnhanceChainManager *audioEnhanceChainManager = AudioEnhanceChainManager::GetInstance();
        CHECK_AND_RETURN_RET_LOG(audioEnhanceChainManager != nullptr, ERROR, "audioEnhanceChainManager is nullptr");
        return audioEnhanceChainManager->SetVolumeInfo(volumeType, systemVol);
    }
}

int32_t AudioServer::SetMicrophoneMuteForEnhanceChain(const bool &isMute)
{
    int32_t engineFlag = GetEngineFlag();
    if (engineFlag == 1) {
        return HPAE::IHpaeManager::GetHpaeManager().SetMicrophoneMuteInfo(isMute);
    } else {
        AudioEnhanceChainManager *audioEnhanceChainManager = AudioEnhanceChainManager::GetInstance();
        CHECK_AND_RETURN_RET_LOG(audioEnhanceChainManager != nullptr, ERROR, "audioEnhanceChainManager is nullptr");
        return audioEnhanceChainManager->SetMicrophoneMuteInfo(isMute);
    }
}

// LCOV_EXCL_START
int32_t AudioServer::LoadAudioEffectLibraries(const std::vector<Library> &libraries,
    const std::vector<Effect> &effects, std::vector<Effect> &successEffectList, bool &hasEffectsLoaded)
{
    CHECK_AND_RETURN_RET_LOG((libraries.size() >= 0) && (libraries.size() <= AUDIO_EFFECT_COUNT_UPPER_LIMIT) &&
        (effects.size() >= 0) && (effects.size() <= AUDIO_EFFECT_COUNT_UPPER_LIMIT), AUDIO_ERR,
        "LOAD_AUDIO_EFFECT_LIBRARIES read data failed");
    CHECK_AND_RETURN_RET(libraries.size() > 0, SUCCESS);
    int32_t callingUid = IPCSkeleton::GetCallingUid();
    CHECK_AND_RETURN_RET_LOG(PermissionUtil::VerifyIsAudio(), ERR_PERMISSION_DENIED,
        "LoadAudioEffectLibraries refused for %{public}d", callingUid);
    CHECK_AND_RETURN_RET_LOG(audioEffectServer_ != nullptr, ERROR, "audioEffectServer_ is nullptr");
    hasEffectsLoaded = audioEffectServer_->LoadAudioEffects(libraries, effects, successEffectList);
    if (!hasEffectsLoaded) {
        AUDIO_WARNING_LOG("Load audio effect failed, please check log");
        successEffectList.clear();
        return ERR_INVALID_OPERATION;
    }
    return SUCCESS;
}

int32_t AudioServer::NotifyAccountsChanged()
{
    int32_t callingUid = IPCSkeleton::GetCallingUid();
    CHECK_AND_RETURN_RET_LOG(PermissionUtil::VerifyIsAudio(), ERR_PERMISSION_DENIED,
        "refused for %{public}d", callingUid);

    int32_t engineFlag = GetEngineFlag();
    if (engineFlag == 1) {
        HPAE::IHpaeManager::GetHpaeManager().NotifyAccountsChanged();
    } else {
        AudioEffectChainManager *audioEffectChainManager = AudioEffectChainManager::GetInstance();
        CHECK_AND_RETURN_RET_LOG(audioEffectChainManager != nullptr, ERROR, "audioEffectChainManager is nullptr");
        audioEffectChainManager->LoadEffectProperties();
    }
    return SUCCESS;
}

int32_t AudioServer::NotifySettingsDataReady()
{
    int32_t callingUid = IPCSkeleton::GetCallingUid();
    CHECK_AND_RETURN_RET_LOG(PermissionUtil::VerifyIsAudio(), ERR_PERMISSION_DENIED,
        "refused for %{public}d", callingUid);
    int32_t engineFlag = GetEngineFlag();
    if (engineFlag == 1) {
        HPAE::IHpaeManager::GetHpaeManager().NotifySettingsDataReady();
    } else {
        AudioEffectChainManager *audioEffectChainManager = AudioEffectChainManager::GetInstance();
        CHECK_AND_RETURN_RET_LOG(audioEffectChainManager != nullptr, ERROR, "audioEffectChainManager is nullptr");
        audioEffectChainManager->LoadEffectProperties();
    }
    return SUCCESS;
}

int32_t AudioServer::IsAcousticEchoCancelerSupported(int32_t sourceType,  bool& isSupported)
{
    int32_t callingUid = IPCSkeleton::GetCallingUid();
    CHECK_AND_RETURN_RET_LOG(PermissionUtil::VerifyIsAudio(), ERR_PERMISSION_DENIED,
        "IsAcousticEchoCancelerSupported refused for %{public}d", callingUid);
    int32_t engineFlag = GetEngineFlag();
    if (engineFlag == 1) {
        isSupported = HPAE::IHpaeManager::GetHpaeManager().IsAcousticEchoCancelerSupported(
            static_cast<SourceType>(sourceType));
        return SUCCESS;
    }
    AUDIO_INFO_LOG("IsAcousticEchoCancelerSupported not support");
    return SUCCESS;
}

int32_t AudioServer::SetKaraokeParameters(const std::string &parameters, bool &ret)
{
    int32_t callingUid = IPCSkeleton::GetCallingUid();
    CHECK_AND_RETURN_RET_LOG(PermissionUtil::VerifyIsAudio(), ERR_PERMISSION_DENIED,
        "SetKaraokeParameters refused for %{public}d", callingUid);
    HdiAdapterManager &manager = HdiAdapterManager::GetInstance();
    std::shared_ptr<IDeviceManager> deviceManager = manager.GetDeviceManager(HDI_DEVICE_MANAGER_TYPE_LOCAL);
    CHECK_AND_RETURN_RET_LOG(deviceManager != nullptr, ERROR, "local device manager is nullptr");
    deviceManager->SetAudioParameter("primary", AudioParamKey::NONE, "", parameters);
    ret = true;
    return SUCCESS;
}

int32_t AudioServer::IsAudioLoopbackSupported(int32_t mode, bool &isSupported)
{
    isSupported = false;
    int32_t callingUid = IPCSkeleton::GetCallingUid();
    CHECK_AND_RETURN_RET_LOG(PermissionUtil::VerifyIsAudio(), ERR_PERMISSION_DENIED,
        "IsAudioLoopbackSupported refused for %{public}d", callingUid);
#ifdef SUPPORT_LOW_LATENCY
    if (mode == AudioLoopbackMode::LOOPBACK_HARDWARE) {
        HdiAdapterManager &manager = HdiAdapterManager::GetInstance();
        std::shared_ptr<IDeviceManager> deviceManager = manager.GetDeviceManager(HDI_DEVICE_MANAGER_TYPE_LOCAL);
        CHECK_AND_RETURN_RET_LOG(deviceManager != nullptr, ERROR, "local device manager is nullptr");
        std::string ret = deviceManager->GetAudioParameter("primary", AudioParamKey::PARAM_KEY_STATE,
            "is_audioloop_support");
        AUDIO_INFO_LOG("IsAudioLoopbackSupported ret: %{public}s", ret.c_str());
        isSupported = ret == "true";
        return SUCCESS;
    }
#endif
    AUDIO_ERR_LOG("IsAudioLoopbackSupported not support");
    return SUCCESS;
}
// LCOV_EXCL_STOP
} // namespace AudioStandard
} // namespace OHOS
