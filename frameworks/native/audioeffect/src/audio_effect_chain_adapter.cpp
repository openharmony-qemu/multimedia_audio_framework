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
#define LOG_TAG "AudioEffectChainAdapter"
#endif

#include "audio_effect_chain_adapter.h"
#include "audio_effect_chain_manager.h"
#include "audio_effect.h"
#include "audio_errors.h"
#include "audio_effect_log.h"
#include "securec.h"

using namespace OHOS::AudioStandard;

static std::map<AudioChannelSet, pa_channel_position> chSetToPaPositionMap = {
    {FRONT_LEFT, PA_CHANNEL_POSITION_FRONT_LEFT},
    {FRONT_RIGHT, PA_CHANNEL_POSITION_FRONT_RIGHT},
    {FRONT_CENTER, PA_CHANNEL_POSITION_FRONT_CENTER},
    {LOW_FREQUENCY, PA_CHANNEL_POSITION_LFE},
    {SIDE_LEFT, PA_CHANNEL_POSITION_SIDE_LEFT},
    {SIDE_RIGHT, PA_CHANNEL_POSITION_SIDE_RIGHT},
    {BACK_LEFT, PA_CHANNEL_POSITION_REAR_LEFT},
    {BACK_RIGHT, PA_CHANNEL_POSITION_REAR_RIGHT},
    {FRONT_LEFT_OF_CENTER, PA_CHANNEL_POSITION_FRONT_LEFT_OF_CENTER},
    {FRONT_RIGHT_OF_CENTER, PA_CHANNEL_POSITION_FRONT_RIGHT_OF_CENTER},
    {BACK_CENTER, PA_CHANNEL_POSITION_REAR_CENTER},
    {TOP_CENTER, PA_CHANNEL_POSITION_TOP_CENTER},
    {TOP_FRONT_LEFT, PA_CHANNEL_POSITION_TOP_FRONT_LEFT},
    {TOP_FRONT_CENTER, PA_CHANNEL_POSITION_TOP_FRONT_CENTER},
    {TOP_FRONT_RIGHT, PA_CHANNEL_POSITION_TOP_FRONT_RIGHT},
    {TOP_BACK_LEFT, PA_CHANNEL_POSITION_TOP_REAR_LEFT},
    {TOP_BACK_CENTER, PA_CHANNEL_POSITION_TOP_REAR_CENTER},
    {TOP_BACK_RIGHT, PA_CHANNEL_POSITION_TOP_REAR_RIGHT},
    /** Channel layout positions below do not have precise mapped pulseaudio positions */
    {STEREO_LEFT, PA_CHANNEL_POSITION_FRONT_LEFT},
    {STEREO_RIGHT, PA_CHANNEL_POSITION_FRONT_RIGHT},
    {WIDE_LEFT, PA_CHANNEL_POSITION_FRONT_LEFT},
    {WIDE_RIGHT, PA_CHANNEL_POSITION_FRONT_RIGHT},
    {SURROUND_DIRECT_LEFT, PA_CHANNEL_POSITION_SIDE_LEFT},
    {SURROUND_DIRECT_RIGHT, PA_CHANNEL_POSITION_SIDE_LEFT},
    {BOTTOM_FRONT_CENTER, PA_CHANNEL_POSITION_FRONT_CENTER},
    {BOTTOM_FRONT_LEFT, PA_CHANNEL_POSITION_FRONT_LEFT},
    {BOTTOM_FRONT_RIGHT, PA_CHANNEL_POSITION_FRONT_RIGHT},
    {TOP_SIDE_LEFT, PA_CHANNEL_POSITION_TOP_REAR_LEFT},
    {TOP_SIDE_RIGHT, PA_CHANNEL_POSITION_TOP_REAR_RIGHT},
    {LOW_FREQUENCY_2, PA_CHANNEL_POSITION_LFE},
};

int32_t EffectChainManagerProcess(char *sceneType, BufferAttr *bufferAttr)
{
    AudioEffectChainManager *audioEffectChainManager = AudioEffectChainManager::GetInstance();
    CHECK_AND_RETURN_RET_LOG(audioEffectChainManager != nullptr, ERR_INVALID_HANDLE, "null audioEffectChainManager");
    std::string sceneTypeString = "";
    if (sceneType) {
        sceneTypeString = sceneType;
    }
    auto eBufferAttr = std::make_unique<EffectBufferAttr>(bufferAttr->bufIn, bufferAttr->bufOut, bufferAttr->numChanIn,
        bufferAttr->frameLen);
    if (audioEffectChainManager->ApplyAudioEffectChain(sceneTypeString, eBufferAttr) != SUCCESS) {
        return ERROR;
    }
    return SUCCESS;
}

bool EffectChainManagerExist(const char *sceneType, const char *effectMode, const char *spatializationEnabled)
{
    AudioEffectChainManager *audioEffectChainManager = AudioEffectChainManager::GetInstance();
    CHECK_AND_RETURN_RET_LOG(audioEffectChainManager != nullptr, false, "null audioEffectChainManager");
    std::string sceneTypeString = "";
    if (sceneType) {
        sceneTypeString = sceneType;
    }
    std::string effectModeString = "";
    if (effectMode) {
        effectModeString = effectMode;
    }
    std::string spatializationEnabledString = "";
    if (spatializationEnabled) {
        spatializationEnabledString = spatializationEnabled;
    }
    return audioEffectChainManager->ExistAudioEffectChain(sceneTypeString, effectModeString,
        spatializationEnabledString);
}

int32_t EffectChainManagerCreateCb(const char *sceneType, const char *sessionID)
{
    AudioEffectChainManager *audioEffectChainManager = AudioEffectChainManager::GetInstance();
    CHECK_AND_RETURN_RET_LOG(audioEffectChainManager != nullptr, ERR_INVALID_HANDLE, "null audioEffectChainManager");
    std::string sceneTypeString = "";
    std::string sessionIDString = "";
    if (sceneType) {
        sceneTypeString = sceneType;
    }
    if (sessionID) {
        sessionIDString = sessionID;
    }
    if (!audioEffectChainManager->CheckAndAddSessionID(sessionIDString)) {
        return SUCCESS;
    }
    bool curSpatializationEnabled = audioEffectChainManager->GetCurSpatializationEnabled();
    std::string curDeviceType = audioEffectChainManager->GetDeviceTypeName();
    if (audioEffectChainManager->GetOffloadEnabled() ||
        ((curDeviceType == "DEVICE_TYPE_BLUETOOTH_A2DP") && !curSpatializationEnabled)) {
        audioEffectChainManager->RegisterEffectChainCountBackupMap(sceneTypeString, "Register");
        AUDIO_DEBUG_LOG("registerEffectChainCountBackupMap");
        return SUCCESS;
    }
    if (audioEffectChainManager->CreateAudioEffectChainDynamic(sceneTypeString) != SUCCESS) {
        AUDIO_DEBUG_LOG("createAudioEffectChainDynamic");
        return ERROR;
    }
    AUDIO_INFO_LOG("Create Audio Effect Chain Success, sessionID is %{public}s, sceneType is %{public}s",
        sessionIDString.c_str(), sceneTypeString.c_str());
    return SUCCESS;
}

int32_t EffectChainManagerReleaseCb(const char *sceneType, const char *sessionID)
{
    AudioEffectChainManager *audioEffectChainManager = AudioEffectChainManager::GetInstance();
    CHECK_AND_RETURN_RET_LOG(audioEffectChainManager != nullptr, ERR_INVALID_HANDLE, "null audioEffectChainManager");
    std::string sceneTypeString = "";
    std::string sessionIDString = "";
    if (sceneType) {
        sceneTypeString = sceneType;
    }
    if (sessionID) {
        sessionIDString = sessionID;
    }
    if (!audioEffectChainManager->CheckAndRemoveSessionID(sessionIDString)) {
        return SUCCESS;
    }
    bool curSpatializationEnabled = audioEffectChainManager->GetCurSpatializationEnabled();
    std::string curDeviceType = audioEffectChainManager->GetDeviceTypeName();
    if (audioEffectChainManager->GetOffloadEnabled() ||
        ((curDeviceType == "DEVICE_TYPE_BLUETOOTH_A2DP") && !curSpatializationEnabled)) {
        audioEffectChainManager->RegisterEffectChainCountBackupMap(sceneTypeString, "Deregister");
        AUDIO_DEBUG_LOG("deRegisterEffectChainCountBackupMap");
        return SUCCESS;
    }
    if (audioEffectChainManager->ReleaseAudioEffectChainDynamic(sceneTypeString) != SUCCESS) {
        AUDIO_DEBUG_LOG("releaseAudioEffectChainDynamic");
        return ERROR;
    }
    AUDIO_INFO_LOG("Release Audio Effect Chain Success, sessionID is %{public}s, sceneType is %{public}s",
        sessionIDString.c_str(), sceneTypeString.c_str());
    return SUCCESS;
}

int32_t EffectChainManagerMultichannelUpdate(const char *sceneType)
{
    AudioEffectChainManager *audioEffectChainManager = AudioEffectChainManager::GetInstance();
    CHECK_AND_RETURN_RET_LOG(audioEffectChainManager != nullptr, ERR_INVALID_HANDLE, "null audioEffectChainManager");
    std::string sceneTypeString = "";
    if (sceneType != nullptr && strlen(sceneType)) {
        sceneTypeString = sceneType;
    } else {
        AUDIO_ERR_LOG("Scenetype is null.");
        return ERROR;
    }
    if (audioEffectChainManager->UpdateMultichannelConfig(sceneTypeString) != SUCCESS) {
        return ERROR;
    }
    return SUCCESS;
}

int32_t EffectChainManagerVolumeUpdate(const char *sessionID, const uint32_t volume)
{
    AudioEffectChainManager *audioEffectChainManager = AudioEffectChainManager::GetInstance();
    CHECK_AND_RETURN_RET_LOG(audioEffectChainManager != nullptr, ERR_INVALID_HANDLE, "null audioEffectChainManager");
    std::string sessionIDString = "";
    sessionIDString = sessionID;
    if (audioEffectChainManager->EffectVolumeUpdate(sessionIDString, volume) != SUCCESS) {
        return ERROR;
    }
    return SUCCESS;
}

uint32_t ConvertChLayoutToPaChMap(const uint64_t channelLayout, pa_channel_map *paMap)
{
    if (channelLayout == CH_LAYOUT_MONO) {
        pa_channel_map_init_mono(paMap);
        return AudioChannel::MONO;
    }
    uint32_t channelNum = 0;
    uint64_t mode = (channelLayout & CH_MODE_MASK) >> CH_MODE_OFFSET;
    switch (mode) {
        case 0: {
            for (auto bit = chSetToPaPositionMap.begin(); bit != chSetToPaPositionMap.end(); ++bit) {
                if ((channelLayout & (bit->first)) != 0) {
                    paMap->map[channelNum++] = bit->second;
                }
            }
            break;
        }
        case 1: {
            uint64_t order = (channelLayout & CH_HOA_ORDNUM_MASK) >> CH_HOA_ORDNUM_OFFSET;
            channelNum = (order + 1) * (order + 1);
            for (uint32_t i = 0; i < channelNum; ++i) {
                paMap->map[i] = chSetToPaPositionMap[FRONT_LEFT];
            }
            break;
        }
        default:
            channelNum = 0;
            break;
    }
    return channelNum;
}

int32_t EffectChainManagerSetHdiParam(const char *sceneType, const char *effectMode, bool enabled)
{
    AudioEffectChainManager *audioEffectChainManager = AudioEffectChainManager::GetInstance();
    CHECK_AND_RETURN_RET_LOG(audioEffectChainManager != nullptr, ERR_INVALID_HANDLE, "null audioEffectChainManager");
    std::string sceneTypeString = "";
    if (sceneType) {
        sceneTypeString = sceneType;
    }
    std::string effectModeString = "";
    if (effectMode) {
        effectModeString = effectMode;
    }
    return audioEffectChainManager->SetHdiParam(sceneTypeString, effectModeString, enabled);
}

int32_t EffectChainManagerInitCb(const char *sceneType)
{
    AudioEffectChainManager *audioEffectChainManager = AudioEffectChainManager::GetInstance();
    CHECK_AND_RETURN_RET_LOG(audioEffectChainManager != nullptr, ERR_INVALID_HANDLE, "null audioEffectChainManager");
    std::string sceneTypeString = "";
    if (sceneType) {
        sceneTypeString = sceneType;
    }
    if (audioEffectChainManager->InitAudioEffectChainDynamic(sceneTypeString) != SUCCESS) {
        return ERROR;
    }
    return SUCCESS;
}

bool EffectChainManagerCheckEffectOffload()
{
    AudioEffectChainManager *audioEffectChainManager = AudioEffectChainManager::GetInstance();
    CHECK_AND_RETURN_RET_LOG(audioEffectChainManager != nullptr, false, "null audioEffectChainManager");
    return audioEffectChainManager->GetOffloadEnabled();
}

int32_t EffectChainManagerAddSessionInfo(const char *sceneType, const char *sessionID, SessionInfoPack pack)
{
    AudioEffectChainManager *audioEffectChainManager = AudioEffectChainManager::GetInstance();
    CHECK_AND_RETURN_RET_LOG(audioEffectChainManager != nullptr, ERR_INVALID_HANDLE, "null audioEffectChainManager");

    uint64_t channelLayoutNum = 0;
    std::string sceneTypeString = "";
    std::string sessionIDString = "";
    std::string sceneModeString = "";
    std::string spatializationEnabledString = "";
    if (sceneType && pack.channelLayout && sessionID && pack.sceneMode && pack.spatializationEnabled) {
        sceneTypeString = sceneType;
        channelLayoutNum = std::strtoull(pack.channelLayout, nullptr, BASE_TEN);
        sessionIDString = sessionID;
        sceneModeString = pack.sceneMode;
        spatializationEnabledString = pack.spatializationEnabled;
    } else {
        AUDIO_ERR_LOG("map input parameters missing.");
        return ERROR;
    }

    SessionEffectInfo info;
    info.sceneMode = sceneModeString;
    info.sceneType = sceneTypeString;
    info.channels = pack.channels;
    info.channelLayout = channelLayoutNum;
    info.spatializationEnabled = spatializationEnabledString;
    info.volume = pack.volume;
    return audioEffectChainManager->SessionInfoMapAdd(sessionIDString, info);
}

int32_t EffectChainManagerDeleteSessionInfo(const char *sceneType, const char *sessionID)
{
    AudioEffectChainManager *audioEffectChainManager = AudioEffectChainManager::GetInstance();
    CHECK_AND_RETURN_RET_LOG(audioEffectChainManager != nullptr, ERR_INVALID_HANDLE, "null audioEffectChainManager");

    std::string sceneTypeString = "";
    std::string sessionIDString = "";
    if (sceneType && sessionID) {
        sceneTypeString = sceneType;
        sessionIDString = sessionID;
    } else {
        AUDIO_ERR_LOG("map unlink parameters missing.");
        return ERROR;
    }

    return audioEffectChainManager->SessionInfoMapDelete(sceneTypeString, sessionIDString);
}

int32_t EffectChainManagerReturnEffectChannelInfo(const char *sceneType, uint32_t *channels, uint64_t *channelLayout)
{
    if (sceneType == nullptr || channels == nullptr || channelLayout == nullptr) {
        return ERROR;
    }
    std::string sceneTypeString = sceneType;
    AudioEffectChainManager *audioEffectChainManager = AudioEffectChainManager::GetInstance();
    uint32_t &chans = *channels;
    uint64_t &chLayout = *channelLayout;
    return audioEffectChainManager->ReturnEffectChannelInfo(sceneTypeString, chans, chLayout);
}

int32_t EffectChainManagerReturnMultiChannelInfo(uint32_t *channels, uint64_t *channelLayout)
{
    if (channels == nullptr || channelLayout == nullptr) {
        return ERROR;
    }
    AudioEffectChainManager *audioEffectChainManager = AudioEffectChainManager::GetInstance();
    return audioEffectChainManager->ReturnMultiChannelInfo(channels, channelLayout);
}

bool EffectChainManagerGetSpatializationEnabled()
{
    AudioEffectChainManager *audioEffectChainManager = AudioEffectChainManager::GetInstance();
    CHECK_AND_RETURN_RET_LOG(audioEffectChainManager != nullptr, false, "null audioEffectChainManager");
    return audioEffectChainManager->GetCurSpatializationEnabled();
}

void EffectChainManagerFlush(void)
{
    AudioEffectChainManager *audioEffectChainManager = AudioEffectChainManager::GetInstance();
    CHECK_AND_RETURN_LOG(audioEffectChainManager != nullptr, "null audioEffectChainManager");
    return audioEffectChainManager->ResetEffectBuffer();
}

void EffectChainManagerEffectUpdate()
{
    AudioEffectChainManager *audioEffectChainManager = AudioEffectChainManager::GetInstance();
    CHECK_AND_RETURN_LOG(audioEffectChainManager != nullptr, "null audioEffectChainManager");
    audioEffectChainManager->UpdateRealAudioEffect();
}

bool EffectChainManagerSceneCheck(const char *sinkSceneType, const char *sceneType)
{
    if (sceneType == nullptr || sinkSceneType == nullptr) {
        return false;
    }
    AudioEffectChainManager *audioEffectChainManager = AudioEffectChainManager::GetInstance();
    CHECK_AND_RETURN_RET_LOG(audioEffectChainManager != nullptr, false, "null audioEffectChainManager");
    std::string sceneTypeString = sceneType;
    std::string sinkSceneTypeString = sinkSceneType;
    return audioEffectChainManager->CheckSceneTypeMatch(sinkSceneTypeString, sceneTypeString);
}
