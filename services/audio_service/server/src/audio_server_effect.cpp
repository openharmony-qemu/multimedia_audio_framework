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

namespace OHOS {
namespace AudioStandard {
using namespace std;

void AudioServer::RecognizeAudioEffectType(const std::string &mainkey, const std::string &subkey,
    const std::string &extraSceneType)
{
    AudioEffectChainManager *audioEffectChainManager = AudioEffectChainManager::GetInstance();
    if (audioEffectChainManager == nullptr) {
        AUDIO_ERR_LOG("audioEffectChainManager is nullptr");
        return;
    }
    audioEffectChainManager->UpdateExtraSceneType(mainkey, subkey, extraSceneType);
}

bool AudioServer::CreateEffectChainManager(std::vector<EffectChain> &effectChains,
    const EffectChainManagerParam &effectParam, const EffectChainManagerParam &enhanceParam)
{
    if (!PermissionUtil::VerifyIsAudio()) {
        AUDIO_ERR_LOG("not audio calling!");
        return false;
    }
    AudioEffectChainManager *audioEffectChainManager = AudioEffectChainManager::GetInstance();
    audioEffectChainManager->InitAudioEffectChainManager(effectChains, effectParam,
        audioEffectServer_->GetEffectEntries());
    AudioEnhanceChainManager *audioEnhanceChainManager = AudioEnhanceChainManager::GetInstance();
    audioEnhanceChainManager->InitAudioEnhanceChainManager(effectChains, enhanceParam,
        audioEffectServer_->GetEffectEntries());
    return true;
}

void AudioServer::SetOutputDeviceSink(int32_t deviceType, std::string &sinkName)
{
    Trace trace("AudioServer::SetOutputDeviceSink:" + std::to_string(deviceType) + " sink:" + sinkName);
    if (!PermissionUtil::VerifyIsAudio()) {
        AUDIO_ERR_LOG("not audio calling!");
        return;
    }
    AudioEffectChainManager *audioEffectChainManager = AudioEffectChainManager::GetInstance();
    audioEffectChainManager->SetOutputDeviceSink(deviceType, sinkName);
    return;
}

int32_t AudioServer::UpdateSpatializationState(AudioSpatializationState spatializationState)
{
    int32_t callingUid = IPCSkeleton::GetCallingUid();
    CHECK_AND_RETURN_RET_LOG(PermissionUtil::VerifyIsAudio(), ERR_NOT_SUPPORTED, "refused for %{public}d", callingUid);
    AudioEffectChainManager *audioEffectChainManager = AudioEffectChainManager::GetInstance();
    if (audioEffectChainManager == nullptr) {
        AUDIO_ERR_LOG("audioEffectChainManager is nullptr");
        return ERROR;
    }
    return audioEffectChainManager->UpdateSpatializationState(spatializationState);
}

int32_t AudioServer::UpdateSpatialDeviceType(AudioSpatialDeviceType spatialDeviceType)
{
    int32_t callingUid = IPCSkeleton::GetCallingUid();
    CHECK_AND_RETURN_RET_LOG(PermissionUtil::VerifyIsAudio(), ERR_NOT_SUPPORTED, "refused for %{public}d", callingUid);

    AudioEffectChainManager *audioEffectChainManager = AudioEffectChainManager::GetInstance();
    CHECK_AND_RETURN_RET_LOG(audioEffectChainManager != nullptr, ERROR, "audioEffectChainManager is nullptr");

    return audioEffectChainManager->UpdateSpatialDeviceType(spatialDeviceType);
}

int32_t AudioServer::SetSystemVolumeToEffect(const AudioStreamType streamType, float volume)
{
    std::string sceneType;
    switch (streamType) {
        case STREAM_RING:
        case STREAM_ALARM:
            sceneType = "SCENE_RING";
            break;
        case STREAM_VOICE_ASSISTANT:
            sceneType = "SCENE_SPEECH";
            break;
        case STREAM_MUSIC:
            sceneType = "SCENE_MUSIC";
            break;
        case STREAM_ACCESSIBILITY:
            sceneType = "SCENE_OTHERS";
            break;
        default:
            return SUCCESS;
    }

    AudioEffectChainManager *audioEffectChainManager = AudioEffectChainManager::GetInstance();
    CHECK_AND_RETURN_RET_LOG(audioEffectChainManager != nullptr, ERROR, "audioEffectChainManager is nullptr");
    AUDIO_INFO_LOG("streamType : %{public}d , systemVolume : %{public}f", streamType, volume);
    audioEffectChainManager->SetSceneTypeSystemVolume(sceneType, volume);

    std::shared_ptr<AudioEffectVolume> audioEffectVolume = AudioEffectVolume::GetInstance();
    CHECK_AND_RETURN_RET_LOG(audioEffectVolume != nullptr, ERROR, "null audioEffectVolume");
    audioEffectChainManager->EffectVolumeUpdate(audioEffectVolume);

    return SUCCESS;
}

int32_t AudioServer::SetSpatializationSceneType(AudioSpatializationSceneType spatializationSceneType)
{
    int32_t callingUid = IPCSkeleton::GetCallingUid();
    CHECK_AND_RETURN_RET_LOG(PermissionUtil::VerifyIsAudio(), ERR_NOT_SUPPORTED, "refused for %{public}d", callingUid);

    AudioEffectChainManager *audioEffectChainManager = AudioEffectChainManager::GetInstance();
    CHECK_AND_RETURN_RET_LOG(audioEffectChainManager != nullptr, ERROR, "audioEffectChainManager is nullptr");
    return audioEffectChainManager->SetSpatializationSceneType(spatializationSceneType);
}

uint32_t AudioServer::GetEffectLatency(const std::string &sessionId)
{
    AudioEffectChainManager *audioEffectChainManager = AudioEffectChainManager::GetInstance();
    CHECK_AND_RETURN_RET_LOG(audioEffectChainManager != nullptr, ERROR, "audioEffectChainManager is nullptr");
    return audioEffectChainManager->GetLatency(sessionId);
}

bool AudioServer::GetEffectOffloadEnabled()
{
    int32_t callingUid = IPCSkeleton::GetCallingUid();
    CHECK_AND_RETURN_RET_LOG(PermissionUtil::VerifyIsAudio(), ERR_NOT_SUPPORTED, "refused for %{public}d", callingUid);

    AudioEffectChainManager *audioEffectChainManager = AudioEffectChainManager::GetInstance();
    CHECK_AND_RETURN_RET_LOG(audioEffectChainManager != nullptr, ERROR, "audioEffectChainManager is nullptr");
    return audioEffectChainManager->GetOffloadEnabled();
}

void AudioServer::LoadHdiEffectModel()
{
    int32_t callingUid = IPCSkeleton::GetCallingUid();
    CHECK_AND_RETURN_LOG(PermissionUtil::VerifyIsAudio(), "load hdi effect model refused for %{public}d", callingUid);

    AudioEffectChainManager *audioEffectChainManager = AudioEffectChainManager::GetInstance();
    CHECK_AND_RETURN_LOG(audioEffectChainManager != nullptr, "audioEffectChainManager is nullptr");
    audioEffectChainManager->InitHdiState();
}

int32_t AudioServer::SetAudioEffectProperty(const AudioEffectPropertyArray &propertyArray)
{
    int32_t callingUid = IPCSkeleton::GetCallingUid();
    CHECK_AND_RETURN_RET_LOG(PermissionUtil::VerifyIsAudio(), ERR_PERMISSION_DENIED,
        "SetA udio Effect Property refused for %{public}d", callingUid);
    AudioEffectChainManager *audioEffectChainManager = AudioEffectChainManager::GetInstance();
    CHECK_AND_RETURN_RET_LOG(audioEffectChainManager != nullptr, ERROR, "audioEffectChainManager is nullptr");
    return audioEffectChainManager->SetAudioEffectProperty(propertyArray);
}

int32_t AudioServer::GetAudioEffectProperty(AudioEffectPropertyArray &propertyArray)
{
    int32_t callingUid = IPCSkeleton::GetCallingUid();
    CHECK_AND_RETURN_RET_LOG(PermissionUtil::VerifyIsAudio(), ERR_PERMISSION_DENIED,
        "Get Audio Effect Property refused for %{public}d", callingUid);
    AudioEffectChainManager *audioEffectChainManager = AudioEffectChainManager::GetInstance();
    CHECK_AND_RETURN_RET_LOG(audioEffectChainManager != nullptr, ERROR, "audioEffectChainManager is nullptr");
    return audioEffectChainManager->GetAudioEffectProperty(propertyArray);
}

int32_t AudioServer::SetAudioEnhanceProperty(const AudioEnhancePropertyArray &propertyArray,
    DeviceType deviceType)
{
    int32_t callingUid = IPCSkeleton::GetCallingUid();
    CHECK_AND_RETURN_RET_LOG(PermissionUtil::VerifyIsAudio(), ERR_PERMISSION_DENIED,
        "Set Audio Enhance Property refused for %{public}d", callingUid);
    AudioEnhanceChainManager *audioEnhanceChainManager = AudioEnhanceChainManager::GetInstance();
    CHECK_AND_RETURN_RET_LOG(audioEnhanceChainManager != nullptr, ERROR, "audioEnhanceChainManager is nullptr");
    return audioEnhanceChainManager->SetAudioEnhanceProperty(propertyArray, deviceType);
}

int32_t AudioServer::GetAudioEnhanceProperty(AudioEnhancePropertyArray &propertyArray,
    DeviceType deviceType)
{
    int32_t callingUid = IPCSkeleton::GetCallingUid();
    CHECK_AND_RETURN_RET_LOG(PermissionUtil::VerifyIsAudio(), ERR_PERMISSION_DENIED,
        "Get Audio Enhance Property refused for %{public}d", callingUid);
    AudioEnhanceChainManager *audioEnhanceChainManager = AudioEnhanceChainManager::GetInstance();
    CHECK_AND_RETURN_RET_LOG(audioEnhanceChainManager != nullptr, ERROR, "audioEnhanceChainManager is nullptr");
    return audioEnhanceChainManager->GetAudioEnhanceProperty(propertyArray, deviceType);
}

void AudioServer::UpdateEffectBtOffloadSupported(const bool &isSupported)
{
    int32_t callingUid = IPCSkeleton::GetCallingUid();
    CHECK_AND_RETURN_LOG(PermissionUtil::VerifyIsAudio(), "refused for %{public}d", callingUid);

    AudioEffectChainManager *audioEffectChainManager = AudioEffectChainManager::GetInstance();
    CHECK_AND_RETURN_LOG(audioEffectChainManager != nullptr, "audioEffectChainManager is nullptr");
    audioEffectChainManager->UpdateEffectBtOffloadSupported(isSupported);
}

void AudioServer::SetRotationToEffect(const uint32_t rotate)
{
    int32_t callingUid = IPCSkeleton::GetCallingUid();
    CHECK_AND_RETURN_LOG(PermissionUtil::VerifyIsAudio(), "set rotation to effect refused for %{public}d", callingUid);

    AudioEffectChainManager *audioEffectChainManager = AudioEffectChainManager::GetInstance();
    CHECK_AND_RETURN_LOG(audioEffectChainManager != nullptr, "audioEffectChainManager is nullptr");
    audioEffectChainManager->EffectRotationUpdate(rotate);
}

int32_t AudioServer::SetVolumeInfoForEnhanceChain(const AudioStreamType &streamType)
{
    AudioEnhanceChainManager *audioEnhanceChainManager = AudioEnhanceChainManager::GetInstance();
    CHECK_AND_RETURN_RET_LOG(audioEnhanceChainManager != nullptr, ERROR, "audioEnhanceChainManager is nullptr");
    AudioVolumeType volumeType = VolumeUtils::GetVolumeTypeFromStreamType(streamType);
    DeviceType deviceType = PolicyHandler::GetInstance().GetActiveOutPutDevice();
    Volume vol = {false, 0.0f, 0};
    PolicyHandler::GetInstance().GetSharedVolume(volumeType, deviceType, vol);
    float systemVol = vol.isMute ? 0.0f : vol.volumeFloat;
    if (PolicyHandler::GetInstance().IsAbsVolumeSupported() &&
        PolicyHandler::GetInstance().GetActiveOutPutDevice() == DEVICE_TYPE_BLUETOOTH_A2DP) {
        systemVol = 1.0f; // 1.0f for a2dp abs volume
    }
    return audioEnhanceChainManager->SetVolumeInfo(volumeType, systemVol);
}

int32_t AudioServer::SetMicrophoneMuteForEnhanceChain(const bool &isMute)
{
    AudioEnhanceChainManager *audioEnhanceChainManager = AudioEnhanceChainManager::GetInstance();
    CHECK_AND_RETURN_RET_LOG(audioEnhanceChainManager != nullptr, ERROR, "audioEnhanceChainManager is nullptr");
    return audioEnhanceChainManager->SetMicrophoneMuteInfo(isMute);
}

bool AudioServer::LoadAudioEffectLibraries(const std::vector<Library> libraries, const std::vector<Effect> effects,
    std::vector<Effect>& successEffectList)
{
    int32_t callingUid = IPCSkeleton::GetCallingUid();
    CHECK_AND_RETURN_RET_LOG(PermissionUtil::VerifyIsAudio(), false, "LoadAudioEffectLibraries refused for %{public}d",
        callingUid);
    bool loadSuccess = audioEffectServer_->LoadAudioEffects(libraries, effects, successEffectList);
    if (!loadSuccess) {
        AUDIO_WARNING_LOG("Load audio effect failed, please check log");
    }
    return loadSuccess;
}
} // namespace AudioStandard
} // namespace OHOS
