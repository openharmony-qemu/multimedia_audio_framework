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
#ifdef SUPPORT_OLD_ENGINE
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
    CHECK_AND_RETURN_RET_LOG(bufferAttr != nullptr, ERROR, "bufferAttr is nullptr");
    AudioEffectChainManager *audioEffectChainManager = AudioEffectChainManager::GetInstance();
    CHECK_AND_RETURN_RET_LOG(audioEffectChainManager != nullptr, ERR_INVALID_HANDLE, "null audioEffectChainManager");
    std::string sceneTypeString = "";
    if (sceneType) {
        sceneTypeString = sceneType;
    }
    auto eBufferAttr = std::make_unique<EffectBufferAttr>(bufferAttr->bufIn, bufferAttr->bufOut, bufferAttr->numChanIn,
        bufferAttr->frameLen, bufferAttr->numChanOut, bufferAttr->outChanLayout);
    if (audioEffectChainManager->ApplyAudioEffectChain(sceneTypeString, eBufferAttr) != SUCCESS) {
        return ERROR;
    }
    bufferAttr->numChanOut = static_cast<int32_t>(eBufferAttr->outChannels);
    bufferAttr->outChanLayout = eBufferAttr->outChannelLayout;
    return SUCCESS;
}

bool EffectChainManagerExist(const char *sceneType, const char *effectMode)
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
    return audioEffectChainManager->ExistAudioEffectChain(sceneTypeString, effectModeString);
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
    if (audioEffectChainManager->UpdateSceneTypeList(sceneTypeString, ADD_SCENE_TYPE) != SUCCESS) {
        return ERROR;
    }
    if (audioEffectChainManager->GetOffloadEnabled()) {
        return SUCCESS;
    }
    if (audioEffectChainManager->CreateAudioEffectChainDynamic(sceneTypeString) != SUCCESS) {
        AUDIO_ERR_LOG("create effect chain fail");
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
    if (audioEffectChainManager->UpdateSceneTypeList(sceneTypeString, REMOVE_SCENE_TYPE) != SUCCESS) {
        return ERROR;
    }
    if (audioEffectChainManager->GetOffloadEnabled()) {
        return SUCCESS;
    }
    if (audioEffectChainManager->ReleaseAudioEffectChainDynamic(sceneTypeString) != SUCCESS) {
        AUDIO_ERR_LOG("release effect chain fail");
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

int32_t EffectChainManagerVolumeUpdate(const char *sessionID)
{
    AudioEffectChainManager *audioEffectChainManager = AudioEffectChainManager::GetInstance();
    CHECK_AND_RETURN_RET_LOG(audioEffectChainManager != nullptr, ERR_INVALID_HANDLE, "null audioEffectChainManager");
    if (audioEffectChainManager->EffectVolumeUpdate() != SUCCESS) {
        return ERROR;
    }
    return SUCCESS;
}

uint32_t ConvertChLayoutToPaChMap(const uint64_t channelLayout, pa_channel_map *paMap)
{
    CHECK_AND_RETURN_RET_LOG(paMap != nullptr, 0, "null paMap");
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
    int32_t streamUsage = 0;
    int32_t systemVolumeType = 0;
    std::string sceneTypeString = "";
    std::string sessionIDString = "";
    std::string sceneModeString = "";

    if (sceneType && pack.channelLayout && sessionID && pack.sceneMode && pack.streamUsage && pack.systemVolumeType) {
        sceneTypeString = sceneType;
        channelLayoutNum = std::strtoull(pack.channelLayout, nullptr, BASE_TEN);
        sessionIDString = sessionID;
        sceneModeString = pack.sceneMode;
        streamUsage = static_cast<int32_t>(std::strtol(pack.streamUsage, nullptr, BASE_TEN));
        systemVolumeType = static_cast<int32_t>(std::strtol(pack.systemVolumeType, nullptr, BASE_TEN));
    } else {
        AUDIO_ERR_LOG("map input parameters missing.");
        return ERROR;
    }

    SessionEffectInfo info;
    info.sceneMode = sceneModeString;
    info.sceneType = sceneTypeString;
    info.channels = pack.channels;
    info.channelLayout = channelLayoutNum;
    info.streamUsage = streamUsage;
    info.systemVolumeType = systemVolumeType;
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
    audioEffectChainManager->InitEffectBuffer(sessionIDString);
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
    return audioEffectChainManager->QueryEffectChannelInfo(sceneTypeString, chans, chLayout);
}

void EffectChainManagerEffectUpdate()
{
    AudioEffectChainManager *audioEffectChainManager = AudioEffectChainManager::GetInstance();
    CHECK_AND_RETURN_LOG(audioEffectChainManager != nullptr, "null audioEffectChainManager");
    audioEffectChainManager->UpdateDefaultAudioEffect();
}

void EffectChainManagerStreamUsageUpdate()
{
    AudioEffectChainManager *audioEffectChainManager = AudioEffectChainManager::GetInstance();
    CHECK_AND_RETURN_LOG(audioEffectChainManager != nullptr, "null audioEffectChainManager");
    audioEffectChainManager->UpdateStreamUsage();
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

uint32_t EffectChainManagerGetSceneCount(const char *sceneType)
{
    if (sceneType == nullptr) {
        return 0;
    }
    AudioEffectChainManager *audioEffectChainManager = AudioEffectChainManager::GetInstance();
    CHECK_AND_RETURN_RET_LOG(audioEffectChainManager != nullptr, false, "null audioEffectChainManager");
    std::string sceneTypeString = sceneType;
    return audioEffectChainManager->GetSceneTypeToChainCount(sceneType);
}
#endif // SUPPORT_OLD_ENGINE