/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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
#define LOG_TAG "AudioEffectChainManager"
#endif

#include "audio_effect_chain_manager.h"
#include "audio_effect.h"
#include "audio_errors.h"
#include "audio_effect_log.h"
#include "audio_utils.h"
#include "securec.h"

#define DEVICE_FLAG

namespace OHOS {
namespace AudioStandard {
static int32_t CheckValidEffectLibEntry(const std::shared_ptr<AudioEffectLibEntry> &libEntry, const std::string &effect,
    const std::string &libName)
{
    CHECK_AND_RETURN_RET_LOG(libEntry != nullptr, ERROR, "Effect [%{public}s] in lib [%{public}s] is nullptr",
        effect.c_str(), libName.c_str());

    CHECK_AND_RETURN_RET_LOG(libEntry->audioEffectLibHandle, ERROR,
        "AudioEffectLibHandle of Effect [%{public}s] in lib [%{public}s] is nullptr", effect.c_str(), libName.c_str());

    CHECK_AND_RETURN_RET_LOG(libEntry->audioEffectLibHandle->createEffect, ERROR,
        "CreateEffect function of Effect [%{public}s] in lib [%{public}s] is nullptr", effect.c_str(), libName.c_str());

    CHECK_AND_RETURN_RET_LOG(libEntry->audioEffectLibHandle->releaseEffect, ERROR,
        "ReleaseEffect function of Effect [%{public}s] in lib [%{public}s] is nullptr", effect.c_str(),
        libName.c_str());
    return SUCCESS;
}

static int32_t FindEffectLib(const std::string &effect,
    const std::vector<std::shared_ptr<AudioEffectLibEntry>> &effectLibraryList,
    std::shared_ptr<AudioEffectLibEntry> &libEntry, std::string &libName)
{
    for (const std::shared_ptr<AudioEffectLibEntry> &lib : effectLibraryList) {
        if (lib->libraryName == effect) {
            libName = lib->libraryName;
            libEntry = lib;
            return SUCCESS;
        }
    }
    return ERROR;
}

static bool IsChannelLayoutSupported(const uint64_t channelLayout)
{
    return find(AUDIO_EFFECT_SUPPORTED_CHANNELLAYOUTS.begin(),
        AUDIO_EFFECT_SUPPORTED_CHANNELLAYOUTS.end(), channelLayout) != AUDIO_EFFECT_SUPPORTED_CHANNELLAYOUTS.end();
}

static void FindMaxSessionID(uint32_t &maxSessionID, std::string &sceneType,
    const std::string &scenePairType, std::set<std::string> &sessions)
{
    for (auto& sessionID : sessions) {
        uint32_t sessionIDInt = static_cast<uint32_t>(std::stoul(sessionID));
        if (sessionIDInt > maxSessionID) {
            maxSessionID = sessionIDInt;
            sceneType = scenePairType;
        }
    }
}

AudioEffectChainManager::AudioEffectChainManager()
{
    EffectToLibraryEntryMap_.clear();
    EffectToLibraryNameMap_.clear();
    EffectChainToEffectsMap_.clear();
    SceneTypeAndModeToEffectChainNameMap_.clear();
    SceneTypeToEffectChainMap_.clear();
    SceneTypeToEffectChainCountMap_.clear();
    SessionIDSet_.clear();
    SceneTypeToSessionIDMap_.clear();
    SessionIDToEffectInfoMap_.clear();
    SceneTypeToEffectChainCountBackupMap_.clear();
    deviceType_ = DEVICE_TYPE_SPEAKER;
    deviceSink_ = DEFAULT_DEVICE_SINK;
    spatialDeviceType_ = EARPHONE_TYPE_OTHERS;
    isInitialized_ = false;

#ifdef SENSOR_ENABLE
    headTracker_ = std::make_shared<HeadTracker>();
#endif

#ifdef WINDOW_MANAGER_ENABLE
    audioRotationListener_ = new AudioRotationListener();
#endif

    audioEffectHdiParam_ = std::make_shared<AudioEffectHdiParam>();
    memset_s(static_cast<void *>(effectHdiInput_), sizeof(effectHdiInput_), 0, sizeof(effectHdiInput_));
    GetSysPara("const.build.product", deviceClass_);
    int32_t flag = 0;
    GetSysPara("persist.multimedia.audioflag.debugarmflag", flag);
    debugArmFlag_ = flag == 1 ? true : false;
}

AudioEffectChainManager::~AudioEffectChainManager()
{
    AUDIO_INFO_LOG("~AudioEffectChainManager()");
}

AudioEffectChainManager *AudioEffectChainManager::GetInstance()
{
    static AudioEffectChainManager audioEffectChainManager;
    return &audioEffectChainManager;
}

int32_t AudioEffectChainManager::UpdateDeviceInfo(int32_t device, const std::string &sinkName)
{
    if (!isInitialized_) {
        deviceType_ = (DeviceType)device;
        deviceSink_ = sinkName;
        AUDIO_INFO_LOG("has not beed initialized");
        return ERROR;
    }

    if (deviceSink_ == sinkName) {
        AUDIO_INFO_LOG("Same DeviceSinkName");
    }
    deviceSink_ = sinkName;

    if (deviceType_ == (DeviceType)device) {
        AUDIO_INFO_LOG("DeviceType do not need to be Updated");
        return ERROR;
    }
    // Delete effectChain in AP and store in backup map
    AUDIO_INFO_LOG("delete all chains when device type change");
    DeleteAllChains();
    deviceType_ = (DeviceType)device;

    return SUCCESS;
}

void AudioEffectChainManager::SetSpkOffloadState()
{
    int32_t ret;
    if (deviceType_ == DEVICE_TYPE_SPEAKER) {
        if (!spkOffloadEnabled_) {
            effectHdiInput_[0] = HDI_INIT;
            ret = audioEffectHdiParam_->UpdateHdiState(effectHdiInput_, DEVICE_TYPE_SPEAKER);
            if (ret != SUCCESS || !CheckIfSpkDsp()) {
                AUDIO_WARNING_LOG("set hdi init failed, backup speaker entered");
                spkOffloadEnabled_ = false;
                RecoverAllChains();
            } else {
                AUDIO_INFO_LOG("set hdi init succeeded, normal speaker entered");
                spkOffloadEnabled_ = true;
            }
        }
    } else {
        if (spkOffloadEnabled_) {
            effectHdiInput_[0] = HDI_DESTROY;
            ret = audioEffectHdiParam_->UpdateHdiState(effectHdiInput_, DEVICE_TYPE_SPEAKER);
            if (ret != SUCCESS) {
                AUDIO_WARNING_LOG("set hdi destroy failed, backup speaker entered");
            }
            spkOffloadEnabled_ = false;
        }

        if (deviceType_ != DEVICE_TYPE_BLUETOOTH_A2DP) {
            AUDIO_INFO_LOG("recover all chains if device type not bt.");
            RecoverAllChains();
        }
    }
}

void AudioEffectChainManager::SetOutputDeviceSink(int32_t device, const std::string &sinkName)
{
    std::lock_guard<std::recursive_mutex> lock(dynamicMutex_);
    if (UpdateDeviceInfo(device, sinkName) != SUCCESS) {
        return;
    }
    // recover effectChain in speaker mode
    SetSpkOffloadState();
    return;
}

std::string AudioEffectChainManager::GetDeviceTypeName()
{
    std::string name = "";
    auto device = SUPPORTED_DEVICE_TYPE.find(deviceType_);
    if (device != SUPPORTED_DEVICE_TYPE.end()) {
        name = device->second;
    }
    return name;
}

bool AudioEffectChainManager::GetOffloadEnabled()
{
    if (deviceType_ == DEVICE_TYPE_SPEAKER) {
        return spkOffloadEnabled_;
    } else {
        return btOffloadEnabled_;
    }
}

void AudioEffectChainManager::InitHdiState()
{
    if (audioEffectHdiParam_ == nullptr) {
        AUDIO_INFO_LOG("audioEffectHdiParam_ is nullptr.");
        return;
    }
    audioEffectHdiParam_->InitHdi();
    effectHdiInput_[0] = HDI_BLUETOOTH_MODE;
    effectHdiInput_[1] = 1;
    AUDIO_INFO_LOG("set hdi bluetooth mode: %{public}d", effectHdiInput_[1]);
    int32_t ret = audioEffectHdiParam_->UpdateHdiState(effectHdiInput_, DEVICE_TYPE_BLUETOOTH_A2DP);
    if (ret != SUCCESS) {
        AUDIO_WARNING_LOG("set hdi bluetooth mode failed");
    }
    effectHdiInput_[0] = HDI_INIT;
    ret = audioEffectHdiParam_->UpdateHdiState(effectHdiInput_, DEVICE_TYPE_SPEAKER);
    if (ret != SUCCESS) {
        AUDIO_WARNING_LOG("set hdi init failed, backup speaker entered");
        spkOffloadEnabled_ = false;
    } else {
        AUDIO_INFO_LOG("set hdi init succeeded, normal speaker entered");
        spkOffloadEnabled_ = true;
    }
}

// Boot initialize
void AudioEffectChainManager::InitAudioEffectChainManager(std::vector<EffectChain> &effectChains,
    std::unordered_map<std::string, std::string> &map,
    std::vector<std::shared_ptr<AudioEffectLibEntry>> &effectLibraryList)
{
    std::set<std::string> effectSet;
    for (EffectChain efc: effectChains) {
        for (std::string effect: efc.apply) {
            effectSet.insert(effect);
        }
    }

    // Construct EffectToLibraryEntryMap that stores libEntry for each effect name
    std::shared_ptr<AudioEffectLibEntry> libEntry = nullptr;
    std::string libName;
    for (std::string effect: effectSet) {
        int32_t ret = FindEffectLib(effect, effectLibraryList, libEntry, libName);
        CHECK_AND_CONTINUE_LOG(ret != ERROR, "Couldn't find libEntry of effect %{public}s", effect.c_str());
        ret = CheckValidEffectLibEntry(libEntry, effect, libName);
        CHECK_AND_CONTINUE_LOG(ret != ERROR, "Invalid libEntry of effect %{public}s", effect.c_str());
        EffectToLibraryEntryMap_[effect] = libEntry;
        EffectToLibraryNameMap_[effect] = libName;
    }
    // Construct EffectChainToEffectsMap that stores all effect names of each effect chain
    for (EffectChain efc: effectChains) {
        std::string key = efc.name;
        std::vector <std::string> effects;
        for (std::string effectName: efc.apply) {
            effects.emplace_back(effectName);
        }
        EffectChainToEffectsMap_[key] = effects;
    }

    // Constrcut SceneTypeAndModeToEffectChainNameMap that stores effectMode associated with the effectChainName
    for (auto item = map.begin(); item != map.end(); ++item) {
        SceneTypeAndModeToEffectChainNameMap_[item->first] = item->second;
    }

    AUDIO_INFO_LOG("EffectToLibraryEntryMap size %{public}zu", EffectToLibraryEntryMap_.size());
    AUDIO_DEBUG_LOG("EffectChainToEffectsMap size %{public}zu", EffectChainToEffectsMap_.size());
    AUDIO_DEBUG_LOG("SceneTypeAndModeToEffectChainNameMap size %{public}zu",
        SceneTypeAndModeToEffectChainNameMap_.size());
    InitHdiState();
#ifdef WINDOW_MANAGER_ENABLE
    AUDIO_DEBUG_LOG("Call RegisterDisplayListener.");
#endif
    isInitialized_ = true;
}

bool AudioEffectChainManager::CheckAndAddSessionID(const std::string &sessionID)
{
    std::lock_guard<std::recursive_mutex> lock(dynamicMutex_);
    if (SessionIDSet_.count(sessionID)) {
        return false;
    }
    SessionIDSet_.insert(sessionID);
    return true;
}

int32_t AudioEffectChainManager::CreateAudioEffectChainDynamic(const std::string &sceneType)
{
    std::lock_guard<std::recursive_mutex> lock(dynamicMutex_);
    CHECK_AND_RETURN_RET_LOG(isInitialized_, ERROR, "has not been initialized");
    CHECK_AND_RETURN_RET_LOG(sceneType != "", ERROR, "null sceneType");

    std::shared_ptr<AudioEffectChain> audioEffectChain = nullptr;
    std::string sceneTypeAndDeviceKey = sceneType + "_&_" + GetDeviceTypeName();
    std::string commonSceneTypeAndDeviceKey = COMMON_SCENE_TYPE + "_&_" + GetDeviceTypeName();

    if ((sceneType == COMMON_SCENE_TYPE) && isCommonEffectChainExisted_) {
        if (SceneTypeToEffectChainMap_.count(sceneTypeAndDeviceKey) &&
            SceneTypeToEffectChainCountMap_.count(sceneTypeAndDeviceKey) &&
            SceneTypeToEffectChainCountMap_[sceneTypeAndDeviceKey] >= 1) {
            ChangeEffectChainCountMapForCreate(sceneType);
            return SUCCESS;
        }
        AUDIO_DEBUG_LOG("Current type is common type, and no need to check exist.");
    } else if (SceneTypeToEffectChainMap_.count(sceneTypeAndDeviceKey)) {
        if (!SceneTypeToEffectChainCountMap_.count(sceneTypeAndDeviceKey) ||
            SceneTypeToEffectChainCountMap_[sceneTypeAndDeviceKey] < 1) {
            EraseEffectChainSetAndMapForCreate(sceneType);
            return ERROR;
        }
        ChangeEffectChainCountMapForCreate(sceneType);
        return SUCCESS;
    }
    audioEffectChain = CreateAudioEffectChain(sceneType);

    SceneTypeToEffectChainMap_[sceneTypeAndDeviceKey] = audioEffectChain;
    SceneTypeToEffectChainCountMap_[sceneTypeAndDeviceKey] = 1;
    if (!AUDIO_SUPPORTED_SCENE_MODES.count(EFFECT_DEFAULT)) {
        return ERROR;
    }
    std::string effectMode = AUDIO_SUPPORTED_SCENE_MODES.find(EFFECT_DEFAULT)->second;
    if (!SceneTypeToSpecialEffectSet_.count(sceneType) && commonEffectChainCount_ > 1) {
        return SUCCESS;
    }
    std::string createSceneType = SceneTypeToSpecialEffectSet_.count(sceneType) ? sceneType : COMMON_SCENE_TYPE;
    if (SetAudioEffectChainDynamic(createSceneType, effectMode) != SUCCESS) {
        return ERROR;
    }
    return SUCCESS;
}

int32_t AudioEffectChainManager::SetAudioEffectChainDynamic(const std::string &sceneType, const std::string &effectMode)
{
    std::string sceneTypeAndDeviceKey = sceneType + "_&_" + GetDeviceTypeName();
    CHECK_AND_RETURN_RET_LOG(SceneTypeToEffectChainMap_.count(sceneTypeAndDeviceKey), ERROR,
        "SceneType [%{public}s] does not exist, failed to set", sceneType.c_str());

    std::shared_ptr<AudioEffectChain> audioEffectChain = SceneTypeToEffectChainMap_[sceneTypeAndDeviceKey];

    std::string effectChain;
    std::string effectChainKey = sceneType + "_&_" + effectMode + "_&_" + GetDeviceTypeName();
    std::string effectNone = AUDIO_SUPPORTED_SCENE_MODES.find(EFFECT_NONE)->second;
    if (!SceneTypeAndModeToEffectChainNameMap_.count(effectChainKey)) {
        AUDIO_ERR_LOG("EffectChain key [%{public}s] does not exist, auto set to %{public}s",
            effectChainKey.c_str(), effectNone.c_str());
        effectChain = effectNone;
    } else {
        effectChain = SceneTypeAndModeToEffectChainNameMap_[effectChainKey];
    }

    if (effectChain != effectNone && !EffectChainToEffectsMap_.count(effectChain)) {
        AUDIO_ERR_LOG("EffectChain name [%{public}s] does not exist, auto set to %{public}s",
            effectChain.c_str(), effectNone.c_str());
        effectChain = effectNone;
    }

    audioEffectChain->SetEffectMode(effectMode);
    audioEffectChain->SetExtraSceneType(extraSceneType_);
    audioEffectChain->SetSpatialDeviceType(spatialDeviceType_);
    for (std::string effect: EffectChainToEffectsMap_[effectChain]) {
        AudioEffectHandle handle = nullptr;
        AudioEffectDescriptor descriptor;
        descriptor.libraryName = EffectToLibraryNameMap_[effect];
        descriptor.effectName = effect;
        int32_t ret = EffectToLibraryEntryMap_[effect]->audioEffectLibHandle->createEffect(descriptor, &handle);
        CHECK_AND_CONTINUE_LOG(ret == 0, "EffectToLibraryEntryMap[%{public}s] createEffect fail", effect.c_str());
        AUDIO_DEBUG_LOG("createEffect, EffectToLibraryEntryMap [%{public}s], effectChainKey [%{public}s]",
            effect.c_str(), effectChainKey.c_str());
        AudioEffectScene currSceneType;
        if (!spatializationEnabled_ || (GetDeviceTypeName() != "DEVICE_TYPE_BLUETOOTH_A2DP")) {
            currSceneType = static_cast<AudioEffectScene>(GetKeyFromValue(AUDIO_SUPPORTED_SCENE_TYPES, sceneType));
        } else {
            currSceneType = GetSceneTypeFromSpatializationSceneType(static_cast<AudioEffectScene>(
                GetKeyFromValue(AUDIO_SUPPORTED_SCENE_TYPES, sceneType)));
        }
        audioEffectChain->AddEffectHandle(handle, EffectToLibraryEntryMap_[effect]->audioEffectLibHandle,
            currSceneType);
    }
    audioEffectChain->ResetIoBufferConfig();

    if (audioEffectChain->IsEmptyEffectHandles()) {
        AUDIO_ERR_LOG("Effectchain is empty, copy bufIn to bufOut like EFFECT_NONE mode");
    }

    AUDIO_INFO_LOG("The delay of SceneType %{public}s is %{public}u", sceneType.c_str(),
        audioEffectChain->GetLatency());
    return SUCCESS;
}

bool AudioEffectChainManager::CheckAndRemoveSessionID(const std::string &sessionID)
{
    std::lock_guard<std::recursive_mutex> lock(dynamicMutex_);
    if (!SessionIDSet_.count(sessionID)) {
        return false;
    }
    SessionIDSet_.erase(sessionID);
    return true;
}

int32_t AudioEffectChainManager::ReleaseAudioEffectChainDynamic(const std::string &sceneType)
{
    std::lock_guard<std::recursive_mutex> lock(dynamicMutex_);
    CHECK_AND_RETURN_RET_LOG(isInitialized_, ERROR, "has not been initialized");
    CHECK_AND_RETURN_RET_LOG(sceneType != "", ERROR, "null sceneType");

    std::string sceneTypeAndDeviceKey = sceneType + "_&_" + GetDeviceTypeName();
    std::string commonSceneTypeAndDeviceKey = COMMON_SCENE_TYPE + "_&_" + GetDeviceTypeName();
    if (!SceneTypeToEffectChainMap_.count(sceneTypeAndDeviceKey)) {
        SceneTypeToEffectChainCountMap_.erase(sceneTypeAndDeviceKey);
        return SUCCESS;
    } else if (SceneTypeToEffectChainCountMap_.count(sceneTypeAndDeviceKey) &&
        SceneTypeToEffectChainCountMap_[sceneTypeAndDeviceKey] > 1) {
        SceneTypeToEffectChainCountMap_[sceneTypeAndDeviceKey]--;
        if (SceneTypeToEffectChainMap_[sceneTypeAndDeviceKey] ==
            SceneTypeToEffectChainMap_[commonSceneTypeAndDeviceKey]) {
            commonEffectChainCount_--;
        }
        return SUCCESS;
    }
    
    SceneTypeToSpecialEffectSet_.erase(sceneType);
    CheckAndReleaseCommonEffectChain(sceneType);
    SceneTypeToEffectChainCountMap_.erase(sceneTypeAndDeviceKey);
    if (sceneType != COMMON_SCENE_TYPE) {
        SceneTypeToEffectChainMap_.erase(sceneTypeAndDeviceKey);
    }

    if (debugArmFlag_ && !spkOffloadEnabled_ && CheckIfSpkDsp()) {
        effectHdiInput_[0] = HDI_INIT;
        int32_t ret = audioEffectHdiParam_->UpdateHdiState(effectHdiInput_, DEVICE_TYPE_SPEAKER);
        if (ret == SUCCESS) {
            AUDIO_INFO_LOG("set hdi init succeeded, normal speaker entered");
            spkOffloadEnabled_ = true;
            DeleteAllChains();
        }
    }
    if (!SceneTypeToEffectChainMap_.count(commonSceneTypeAndDeviceKey)) {
        isCommonEffectChainExisted_ = false;
    }
    AUDIO_DEBUG_LOG("releaseEffect, sceneTypeAndDeviceKey [%{public}s]", sceneTypeAndDeviceKey.c_str());
    return SUCCESS;
}

bool AudioEffectChainManager::ExistAudioEffectChain(const std::string &sceneType, const std::string &effectMode,
    const std::string &spatializationEnabled)
{
    std::lock_guard<std::recursive_mutex> lock(dynamicMutex_);
    if (!isInitialized_) {
        if (initializedLogFlag_) {
            AUDIO_ERR_LOG("audioEffectChainManager has not been initialized");
            initializedLogFlag_ = false;
        }
        return false;
    }
    initializedLogFlag_ = true;
    CHECK_AND_RETURN_RET(sceneType != "", false);
    CHECK_AND_RETURN_RET_LOG(GetDeviceTypeName() != "", false, "null deviceType");

#ifndef DEVICE_FLAG
    if (deviceType_ != DEVICE_TYPE_SPEAKER) {
        return false;
    }
#endif
    if ((deviceType_ == DEVICE_TYPE_SPEAKER) && (spkOffloadEnabled_)) {
        return false;
    }

    if ((deviceType_ == DEVICE_TYPE_BLUETOOTH_A2DP) &&
        (btOffloadEnabled_ || (!spatializationEnabled_ || (spatializationEnabled == "0")))) {
        return false;
    }

    std::string effectChainKey;
    if (sceneType == "SCENE_RING" || sceneType == "SCENE_OTHERS") {
        effectChainKey = sceneType + "_&_" + effectMode + "_&_" + GetDeviceTypeName();
    } else {
        std::string effectModeTrue = AUDIO_SUPPORTED_SCENE_MODES.find(EFFECT_DEFAULT)->second;
        effectChainKey = sceneType + "_&_" + effectModeTrue + "_&_" + GetDeviceTypeName();
    }

    if (!SceneTypeAndModeToEffectChainNameMap_.count(effectChainKey)) {
        return false;
    }
    std::string sceneTypeAndDeviceKey = sceneType + "_&_" + GetDeviceTypeName();
    // if the effectChain exist, see if it is empty
    if (!SceneTypeToEffectChainMap_.count(sceneTypeAndDeviceKey) ||
        SceneTypeToEffectChainMap_[sceneTypeAndDeviceKey] == nullptr) {
        return false;
    }
    auto audioEffectChain = SceneTypeToEffectChainMap_[sceneTypeAndDeviceKey];
    return !audioEffectChain->IsEmptyEffectHandles();
}

int32_t AudioEffectChainManager::ApplyAudioEffectChain(const std::string &sceneType,
    const std::unique_ptr<EffectBufferAttr> &bufferAttr)
{
    std::string sceneTypeAndDeviceKey = sceneType + "_&_" + GetDeviceTypeName();
    size_t totLen = static_cast<size_t>(bufferAttr->frameLen * bufferAttr->numChans * sizeof(float));
    std::lock_guard<std::recursive_mutex> lock(dynamicMutex_);
    auto it = SceneTypeToEffectChainMap_.find(sceneTypeAndDeviceKey);
#ifdef DEVICE_FLAG
    if (it == SceneTypeToEffectChainMap_.end() || it->second == nullptr) {
        CHECK_AND_RETURN_RET_LOG(memcpy_s(bufferAttr->bufOut, totLen, bufferAttr->bufIn, totLen) == 0, ERROR,
            "memcpy error when no effect applied");
        return ERROR;
    }
#else
    if (deviceType_ != DEVICE_TYPE_SPEAKER || it == SceneTypeToEffectChainMap_.end()
        || it->second == nullptr) {
        CHECK_AND_RETURN_RET_LOG(memcpy_s(bufferAttr->bufOut, totLen, bufferAttr->bufIn, totLen) == 0, ERROR,
            "memcpy error when no effect applied");
        return SUCCESS;
    }
#endif
    auto audioEffectChain = it->second;
    AudioEffectProcInfo procInfo = {headTrackingEnabled_, btOffloadEnabled_};
    audioEffectChain->ApplyEffectChain(bufferAttr->bufIn, bufferAttr->bufOut, bufferAttr->frameLen, procInfo);
    return SUCCESS;
}

void AudioEffectChainManager::Dump()
{
    AUDIO_INFO_LOG("Dump START");
    for (auto item : SceneTypeToEffectChainMap_) {
        std::shared_ptr<AudioEffectChain> audioEffectChain = item.second;
        audioEffectChain->Dump();
    }
}

int32_t AudioEffectChainManager::EffectDspVolumeUpdate(std::shared_ptr<AudioEffectVolume> audioEffectVolume)
{
    AUDIO_INFO_LOG("send volume to dsp.");
    CHECK_AND_RETURN_RET_LOG(audioEffectVolume != nullptr, ERROR, "null audioEffectVolume");
    uint32_t volumeMax = 0;
    for (auto it = SceneTypeToSessionIDMap_.begin(); it != SceneTypeToSessionIDMap_.end(); it++) {
        std::set<std::string> sessions = SceneTypeToSessionIDMap_[it->first];
        for (auto s = sessions.begin(); s != sessions.end(); s++) {
            SessionEffectInfo info = SessionIDToEffectInfoMap_[*s];
            volumeMax = info.volume > volumeMax ? info.volume : volumeMax;
        }
    }
    if (audioEffectVolume->GetDspVolume() != volumeMax) {
        audioEffectVolume->SetDspVolume(volumeMax);
        effectHdiInput_[0] = HDI_VOLUME;
        int32_t ret = memcpy_s(&effectHdiInput_[1], SEND_HDI_COMMAND_LEN, &volumeMax, sizeof(uint32_t));
        CHECK_AND_RETURN_RET_LOG(ret == 0, ERROR, "memcpy volume failed");
        AUDIO_INFO_LOG("set hdi volume: %{public}u", *(reinterpret_cast<uint32_t *>(&effectHdiInput_[1])));
        ret = audioEffectHdiParam_->UpdateHdiState(effectHdiInput_);
        CHECK_AND_RETURN_RET_LOG(ret == 0, ERROR, "set hdi volume failed");
    }
    return SUCCESS;
}

int32_t AudioEffectChainManager::EffectApVolumeUpdate(std::shared_ptr<AudioEffectVolume> audioEffectVolume)
{
    AUDIO_INFO_LOG("send volume to ap.");
    CHECK_AND_RETURN_RET_LOG(audioEffectVolume != nullptr, ERROR, "null audioEffectVolume");
    for (auto it = SceneTypeToSessionIDMap_.begin(); it != SceneTypeToSessionIDMap_.end(); it++) {
        uint32_t volumeMax = 0;
        std::set<std::string> sessions = it->second;
        for (auto s = sessions.begin(); s != sessions.end(); s++) {
            SessionEffectInfo info = SessionIDToEffectInfoMap_[*s];
            volumeMax = info.volume > volumeMax ? info.volume : volumeMax;
        }
        if (audioEffectVolume->GetApVolume(it->first) != volumeMax) {
            audioEffectVolume->SetApVolume(it->first, volumeMax);
            std::string sceneTypeAndDeviceKey = it->first + "_&_" + GetDeviceTypeName();
            if (!SceneTypeToEffectChainMap_.count(sceneTypeAndDeviceKey)) {
                return ERROR;
            }
            auto audioEffectChain = SceneTypeToEffectChainMap_[sceneTypeAndDeviceKey];
            if (audioEffectChain == nullptr) {
                return ERROR;
            }
            AudioEffectScene currSceneType;
            if (!spatializationEnabled_ || (GetDeviceTypeName() != "DEVICE_TYPE_BLUETOOTH_A2DP")) {
                currSceneType = static_cast<AudioEffectScene>(GetKeyFromValue(AUDIO_SUPPORTED_SCENE_TYPES, it->first));
            } else {
                currSceneType = GetSceneTypeFromSpatializationSceneType(static_cast<AudioEffectScene>(
                    GetKeyFromValue(AUDIO_SUPPORTED_SCENE_TYPES, it->first)));
            }
            audioEffectChain->SetEffectCurrSceneType(currSceneType);
            int32_t ret = audioEffectChain->UpdateEffectParam();
            CHECK_AND_RETURN_RET_LOG(ret == 0, ERROR, "set ap volume failed");
            AUDIO_INFO_LOG("The delay of SceneType %{public}s is %{public}u, volume changed to %{public}u",
                it->first.c_str(), audioEffectChain->GetLatency(), volumeMax);
        }
    }
    return SUCCESS;
}

int32_t AudioEffectChainManager::EffectVolumeUpdate(const std::string sessionIDString, const uint32_t volume)
{
    std::lock_guard<std::recursive_mutex> lock(dynamicMutex_);
    // update session info
    if (SessionIDToEffectInfoMap_.count(sessionIDString)) {
        SessionIDToEffectInfoMap_[sessionIDString].volume = volume;
    }
    std::shared_ptr<AudioEffectVolume> audioEffectVolume = AudioEffectVolume::GetInstance();
    int32_t ret;
    if (((deviceType_ == DEVICE_TYPE_SPEAKER) && (spkOffloadEnabled_)) ||
        ((deviceType_ == DEVICE_TYPE_BLUETOOTH_A2DP) && (btOffloadEnabled_))) {
        ret = EffectDspVolumeUpdate(audioEffectVolume);
    } else {
        ret = EffectApVolumeUpdate(audioEffectVolume);
    }
    return ret;
}

#ifdef WINDOW_MANAGER_ENABLE
int32_t AudioEffectChainManager::EffectDspRotationUpdate(std::shared_ptr<AudioEffectRotation> audioEffectRotation,
    const uint32_t rotationState)
{
    // send rotation to dsp
    AUDIO_DEBUG_LOG("rotationState change, new state: %{public}d, previous state: %{public}d",
        rotationState, audioEffectRotation->GetRotation());
    effectHdiInput_[0] = HDI_ROTATION;
    effectHdiInput_[1] = rotationState;
    AUDIO_INFO_LOG("set hdi rotation: %{public}d", effectHdiInput_[1]);
    int32_t ret = audioEffectHdiParam_->UpdateHdiState(effectHdiInput_);
    CHECK_AND_RETURN_RET_LOG(ret == 0, ERROR, "set hdi rotation failed");
    return SUCCESS;
}

int32_t AudioEffectChainManager::EffectApRotationUpdate(std::shared_ptr<AudioEffectRotation> audioEffectRotation,
    const uint32_t rotationState)
{
    // send rotation to ap
    AUDIO_DEBUG_LOG("rotationState change, new state: %{public}d, previous state: %{public}d",
        rotationState, audioEffectRotation->GetRotation());
    for (auto it = SceneTypeToSessionIDMap_.begin(); it != SceneTypeToSessionIDMap_.end(); it++) {
        std::string sceneTypeAndDeviceKey = it->first + "_&_" + GetDeviceTypeName();
        if (!SceneTypeToEffectChainMap_.count(sceneTypeAndDeviceKey)) {
            return ERROR;
        }
        auto audioEffectChain = SceneTypeToEffectChainMap_[sceneTypeAndDeviceKey];
        if (audioEffectChain == nullptr) {
            return ERROR;
        }
        AudioEffectScene currSceneType;
        if (!spatializationEnabled_ || (GetDeviceTypeName() != "DEVICE_TYPE_BLUETOOTH_A2DP")) {
            currSceneType = static_cast<AudioEffectScene>(GetKeyFromValue(AUDIO_SUPPORTED_SCENE_TYPES, it->first));
        } else {
            currSceneType = GetSceneTypeFromSpatializationSceneType(static_cast<AudioEffectScene>(
                GetKeyFromValue(AUDIO_SUPPORTED_SCENE_TYPES, it->first)));
        }
        audioEffectChain->SetEffectCurrSceneType(currSceneType);
        int32_t ret = audioEffectChain->UpdateEffectParam();
        CHECK_AND_RETURN_RET_LOG(ret == 0, ERROR, "set ap rotation failed");
        AUDIO_INFO_LOG("The delay of SceneType %{public}s is %{public}u, rotation changed to %{public}u",
            it->first.c_str(), audioEffectChain->GetLatency(), rotationState);
        }

    return SUCCESS;
}

int32_t AudioEffectChainManager::EffectRotationUpdate(const uint32_t rotationState)
{
    std::lock_guard<std::recursive_mutex> lock(dynamicMutex_);
    std::shared_ptr<AudioEffectRotation> audioEffectRotation = AudioEffectRotation::GetInstance();
    if (audioEffectRotation->GetRotation() != rotationState) {
        audioEffectRotation->SetRotation(rotationState);
        AUDIO_INFO_LOG("rotation update to %{public}u", rotationState);
        EffectDspRotationUpdate(audioEffectRotation, rotationState);
        EffectApRotationUpdate(audioEffectRotation, rotationState);
    }
    
    return SUCCESS;
}
#endif

int32_t AudioEffectChainManager::UpdateMultichannelConfig(const std::string &sceneType)
{
    std::lock_guard<std::recursive_mutex> lock(dynamicMutex_);
    std::string sceneTypeAndDeviceKey = sceneType + "_&_" + GetDeviceTypeName();
    if (!SceneTypeToEffectChainMap_.count(sceneTypeAndDeviceKey)) {
        return ERROR;
    }
    uint32_t inputChannels = DEFAULT_NUM_CHANNEL;
    uint64_t inputChannelLayout = DEFAULT_NUM_CHANNELLAYOUT;
    ReturnEffectChannelInfo(sceneType, inputChannels, inputChannelLayout);

    auto audioEffectChain = SceneTypeToEffectChainMap_[sceneTypeAndDeviceKey];
    if (audioEffectChain == nullptr) {
        return ERROR;
    }
    audioEffectChain->UpdateMultichannelIoBufferConfig(inputChannels, inputChannelLayout);
    return SUCCESS;
}

int32_t AudioEffectChainManager::InitAudioEffectChainDynamic(const std::string &sceneType)
{
    std::lock_guard<std::recursive_mutex> lock(dynamicMutex_);
    CHECK_AND_RETURN_RET_LOG(isInitialized_, ERROR, "has not been initialized");
    CHECK_AND_RETURN_RET_LOG(sceneType != "", ERROR, "null sceneType");

    std::shared_ptr<AudioEffectChain> audioEffectChain = nullptr;
    std::string sceneTypeAndDeviceKey = sceneType + "_&_" + GetDeviceTypeName();
    if (!SceneTypeToEffectChainMap_.count(sceneTypeAndDeviceKey)) {
        return SUCCESS;
    } else {
        audioEffectChain = SceneTypeToEffectChainMap_[sceneTypeAndDeviceKey];
    }
    if (audioEffectChain != nullptr) {
        audioEffectChain->InitEffectChain();
    }

    return SUCCESS;
}

int32_t AudioEffectChainManager::UpdateSpatializationState(AudioSpatializationState spatializationState)
{
    AUDIO_INFO_LOG("UpdateSpatializationState entered, current state: %{public}d and %{public}d, previous state: \
        %{public}d and %{public}d", spatializationState.spatializationEnabled, spatializationState.headTrackingEnabled,
        spatializationEnabled_.load(), headTrackingEnabled_);
    std::lock_guard<std::recursive_mutex> lock(dynamicMutex_);
    if (spatializationEnabled_ != spatializationState.spatializationEnabled) {
        UpdateSpatializationEnabled(spatializationState);
    }
    if (headTrackingEnabled_ != spatializationState.headTrackingEnabled) {
        headTrackingEnabled_ = spatializationState.headTrackingEnabled;
        UpdateSensorState();
    }
    return SUCCESS;
}

int32_t AudioEffectChainManager::UpdateSpatialDeviceType(AudioSpatialDeviceType spatialDeviceType)
{
    int32_t ret{ SUCCESS };
    spatialDeviceType_ = spatialDeviceType;

    effectHdiInput_[0] = HDI_UPDATE_SPATIAL_DEVICE_TYPE;
    effectHdiInput_[1] = spatialDeviceType_;
    AUDIO_INFO_LOG("set hdi spatialDeviceType: %{public}d", effectHdiInput_[1]);
    ret = audioEffectHdiParam_->UpdateHdiState(effectHdiInput_, DEVICE_TYPE_BLUETOOTH_A2DP);
    if (ret != SUCCESS) {
        AUDIO_WARNING_LOG("set hdi update spatial device type failed");
    }

    std::lock_guard<std::recursive_mutex> lock(dynamicMutex_);
    for (auto& sceneType2EffectChain : SceneTypeToEffectChainMap_) {
        auto audioEffectChain = sceneType2EffectChain.second;
        if (audioEffectChain != nullptr) {
            audioEffectChain->SetSpatialDeviceType(spatialDeviceType_);
            ret = audioEffectChain->UpdateEffectParam();
            CHECK_AND_CONTINUE_LOG(ret == SUCCESS, "UpdateEffectParam failed.");
        }
    }

    return SUCCESS;
}

int32_t AudioEffectChainManager::ReturnEffectChannelInfo(const std::string &sceneType, uint32_t &channels,
    uint64_t &channelLayout)
{
    std::lock_guard<std::recursive_mutex> lock(dynamicMutex_);
    std::string sceneTypeAndDeviceKey = sceneType + "_&_" + GetDeviceTypeName();
    if (!SceneTypeToEffectChainMap_.count(sceneTypeAndDeviceKey)) {
        return ERROR;
    }
    for (auto& scenePair : SceneTypeToSessionIDMap_) {
        std::string pairSceneTypeAndDeviceKey = scenePair.first + "_&_" + GetDeviceTypeName();
        if (SceneTypeToEffectChainMap_[sceneTypeAndDeviceKey] ==
            SceneTypeToEffectChainMap_[pairSceneTypeAndDeviceKey]) {
            std::set<std::string> sessions = scenePair.second;
            FindMaxEffectChannels(scenePair.first, sessions, channels, channelLayout);
        }
    }
    return SUCCESS;
}

int32_t AudioEffectChainManager::ReturnMultiChannelInfo(uint32_t *channels, uint64_t *channelLayout)
{
    std::lock_guard<std::recursive_mutex> lock(dynamicMutex_);
    for (auto it = SceneTypeToSessionIDMap_.begin(); it != SceneTypeToSessionIDMap_.end(); it++) {
        std::set<std::string> sessions = SceneTypeToSessionIDMap_[it->first];
        for (auto s = sessions.begin(); s != sessions.end(); ++s) {
            SessionEffectInfo info = SessionIDToEffectInfoMap_[*s];
            uint32_t tmpChannelCount = DEFAULT_MCH_NUM_CHANNEL;
            uint64_t tmpChannelLayout = DEFAULT_MCH_NUM_CHANNELLAYOUT;
            if (info.channels > DEFAULT_NUM_CHANNEL &&
                info.channels <= DSP_MAX_NUM_CHANNEL &&
                !ExistAudioEffectChain(it->first, info.sceneMode, info.spatializationEnabled) &&
                IsChannelLayoutSupported(info.channelLayout)) {
                tmpChannelLayout = info.channelLayout;
                tmpChannelCount = info.channels;
            }

            if (tmpChannelCount >= *channels) {
                *channels = tmpChannelCount;
                *channelLayout = tmpChannelLayout;
            }
        }
    }
    return SUCCESS;
}

int32_t AudioEffectChainManager::SessionInfoMapAdd(const std::string &sessionID, const SessionEffectInfo &info)
{
    std::lock_guard<std::recursive_mutex> lock(dynamicMutex_);
    CHECK_AND_RETURN_RET_LOG(sessionID != "", ERROR, "null sessionID");
    if (!SessionIDToEffectInfoMap_.count(sessionID)) {
        SceneTypeToSessionIDMap_[info.sceneType].insert(sessionID);
        SessionIDToEffectInfoMap_[sessionID] = info;
    } else if (SessionIDToEffectInfoMap_[sessionID].sceneMode != info.sceneMode ||
        SessionIDToEffectInfoMap_[sessionID].spatializationEnabled != info.spatializationEnabled ||
        SessionIDToEffectInfoMap_[sessionID].volume != info.volume) {
        SessionIDToEffectInfoMap_[sessionID] = info;
    } else {
        return ERROR;
    }
    return SUCCESS;
}

int32_t AudioEffectChainManager::SessionInfoMapDelete(const std::string &sceneType, const std::string &sessionID)
{
    std::lock_guard<std::recursive_mutex> lock(dynamicMutex_);
    if (!SceneTypeToSessionIDMap_.count(sceneType)) {
        return ERROR;
    }
    if (SceneTypeToSessionIDMap_[sceneType].erase(sessionID)) {
        if (SceneTypeToSessionIDMap_[sceneType].empty()) {
            SceneTypeToSessionIDMap_.erase(sceneType);
        }
    } else {
        return ERROR;
    }
    if (!SessionIDToEffectInfoMap_.erase(sessionID)) {
        return ERROR;
    }
    return SUCCESS;
}

int32_t AudioEffectChainManager::SetHdiParam(const std::string &sceneType, const std::string &effectMode, bool enabled)
{
    std::lock_guard<std::recursive_mutex> lock(dynamicMutex_);
    if (!isInitialized_) {
        if (initializedLogFlag_) {
            AUDIO_ERR_LOG("audioEffectChainManager has not been initialized");
            initializedLogFlag_ = false;
        }
        return ERROR;
    }
    CHECK_AND_RETURN_RET_LOG(sceneType != "", ERROR, "null sceneType");
    hdiSceneType_ = GetKeyFromValue(AUDIO_SUPPORTED_SCENE_TYPES, sceneType);
    memset_s(static_cast<void *>(effectHdiInput_), sizeof(effectHdiInput_), 0, sizeof(effectHdiInput_));
    effectHdiInput_[0] = HDI_BYPASS;
    effectHdiInput_[1] = enabled == true ? 0 : 1;
    AUDIO_INFO_LOG("set hdi bypass: %{public}d", effectHdiInput_[1]);
    int32_t ret = audioEffectHdiParam_->UpdateHdiState(effectHdiInput_, DEVICE_TYPE_BLUETOOTH_A2DP);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERROR, "set hdi bypass failed, ret is %{public}d", ret);

    effectHdiInput_[0] = HDI_ROOM_MODE;
    if (!spatializationEnabled_ || (GetDeviceTypeName() != "DEVICE_TYPE_BLUETOOTH_A2DP")) {
        effectHdiInput_[1] = hdiSceneType_;
    } else {
        effectHdiInput_[1] =
            static_cast<int32_t>(GetSceneTypeFromSpatializationSceneType(static_cast<AudioEffectScene>(hdiSceneType_)));
    }
    hdiEffectMode_ = GetKeyFromValue(AUDIO_SUPPORTED_SCENE_MODES, effectMode);
    effectHdiInput_[HDI_ROOM_MODE_INDEX_TWO] = hdiEffectMode_;
    AUDIO_INFO_LOG("set hdi room mode sceneType: %{public}d, effectMode: %{public}d", effectHdiInput_[1],
        effectHdiInput_[HDI_ROOM_MODE_INDEX_TWO]);
    ret = audioEffectHdiParam_->UpdateHdiState(effectHdiInput_, DEVICE_TYPE_BLUETOOTH_A2DP);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERROR, "set hdi room mode failed, ret is %{public}d", ret);
    return SUCCESS;
}

void AudioEffectChainManager::UpdateSensorState()
{
    effectHdiInput_[0] = HDI_HEAD_MODE;
    effectHdiInput_[1] = headTrackingEnabled_ == true ? 1 : 0;
    AUDIO_INFO_LOG("set hdi head mode: %{public}d", effectHdiInput_[1]);
    int32_t ret = audioEffectHdiParam_->UpdateHdiState(effectHdiInput_, DEVICE_TYPE_BLUETOOTH_A2DP);
    if (ret != SUCCESS) {
        AUDIO_WARNING_LOG("set hdi head mode failed");
    }

    if (headTrackingEnabled_) {
#ifdef SENSOR_ENABLE
        if (btOffloadEnabled_) {
            headTracker_->SensorInit();
            ret = headTracker_->SensorSetConfig(DSP_SPATIALIZER_ENGINE);
        } else {
            headTracker_->SensorInit();
            ret = headTracker_->SensorSetConfig(ARM_SPATIALIZER_ENGINE);
        }

        if (ret != SUCCESS) {
            AUDIO_ERR_LOG("SensorSetConfig error!");
        }

        if (headTracker_->SensorActive() != SUCCESS) {
            AUDIO_ERR_LOG("SensorActive failed");
        }
#endif
        return;
    }

    if (btOffloadEnabled_) {
        return;
    }

#ifdef SENSOR_ENABLE
    if (headTracker_->SensorDeactive() != SUCCESS) {
        AUDIO_ERR_LOG("SensorDeactive failed");
    }
    headTracker_->SensorUnsubscribe();
    HeadPostureData headPostureData = {1, 1.0, 0.0, 0.0, 0.0}; // ori head posturedata
    headTracker_->SetHeadPostureData(headPostureData);
    for (auto it = SceneTypeToEffectChainMap_.begin(); it != SceneTypeToEffectChainMap_.end(); ++it) {
        auto audioEffectChain = it->second;
        if (audioEffectChain == nullptr) {
            continue;
        }
        audioEffectChain->SetHeadTrackingDisabled();
    }
#endif
}

void AudioEffectChainManager::DeleteAllChains()
{
    for (auto it = SceneTypeToEffectChainCountMap_.begin(); it != SceneTypeToEffectChainCountMap_.end(); ++it) {
        AUDIO_DEBUG_LOG("sceneTypeAndDeviceKey %{public}s count:%{public}d", it->first.c_str(), it->second);
        SceneTypeToEffectChainCountBackupMap_.insert(std::make_pair(it->first, it->second));
    }
    
    for (auto it = SceneTypeToEffectChainCountBackupMap_.begin(); it != SceneTypeToEffectChainCountBackupMap_.end();
        ++it) {
        std::string sceneType = it->first.substr(0, static_cast<size_t>(it->first.find("_&_")));
        for (int32_t k = 0; k < it->second; ++k) {
            ReleaseAudioEffectChainDynamic(sceneType);
        }
    }
    return;
}

void AudioEffectChainManager::RecoverAllChains()
{
    for (auto it = SceneTypeToEffectChainCountBackupMap_.begin(); it != SceneTypeToEffectChainCountBackupMap_.end();
        ++it) {
        AUDIO_DEBUG_LOG("sceneTypeAndDeviceKey %{public}s count:%{public}d", it->first.c_str(), it->second);
        std::string sceneType = it->first.substr(0, static_cast<size_t>(it->first.find("_&_")));
        for (int32_t k = 0; k < it->second; ++k) {
            CreateAudioEffectChainDynamic(sceneType);
        }
        UpdateMultichannelConfig(sceneType);
    }
    SceneTypeToEffectChainCountBackupMap_.clear();
}

void AudioEffectChainManager::RegisterEffectChainCountBackupMap(const std::string &sceneType,
    const std::string &operation)
{
    std::lock_guard<std::recursive_mutex> lock(dynamicMutex_);
    std::string sceneTypeAndDeviceKey = sceneType + "_&_" + GetDeviceTypeName();
    if (operation == "Register") {
        if (!SceneTypeToEffectChainCountBackupMap_.count(sceneTypeAndDeviceKey)) {
            SceneTypeToEffectChainCountBackupMap_[sceneTypeAndDeviceKey] = 1;
            return;
        }
        SceneTypeToEffectChainCountBackupMap_[sceneTypeAndDeviceKey]++;
    } else if (operation == "Deregister") {
        if (SceneTypeToEffectChainCountBackupMap_.count(sceneTypeAndDeviceKey) == 1) {
            SceneTypeToEffectChainCountBackupMap_.erase(sceneTypeAndDeviceKey);
            return;
        }
        SceneTypeToEffectChainCountBackupMap_[sceneTypeAndDeviceKey]--;
    } else {
        AUDIO_ERR_LOG("Wrong operation to SceneTypeToEffectChainCountBackupMap.");
    }
    return;
}

uint32_t AudioEffectChainManager::GetLatency(const std::string &sessionId)
{
    if (((deviceType_ == DEVICE_TYPE_SPEAKER) && (spkOffloadEnabled_)) ||
        ((deviceType_ == DEVICE_TYPE_BLUETOOTH_A2DP) && (btOffloadEnabled_))) {
        AUDIO_DEBUG_LOG("offload enabled, return 0");
        return 0;
    }
    std::lock_guard<std::recursive_mutex> lock(dynamicMutex_);
    CHECK_AND_RETURN_RET_LOG(SessionIDToEffectInfoMap_.count(sessionId), 0, "no such sessionId in map");
    if (SessionIDToEffectInfoMap_[sessionId].sceneMode == "" ||
        SessionIDToEffectInfoMap_[sessionId].sceneMode == "None") {
        AUDIO_DEBUG_LOG("seceneMode is None, return 0");
        return 0;
    }
    if (SessionIDToEffectInfoMap_[sessionId].spatializationEnabled == "0" &&
        GetDeviceTypeName() == "DEVICE_TYPE_BLUETOOTH_A2DP") {
        return 0;
    }
    std::string sceneTypeAndDeviceKey = SessionIDToEffectInfoMap_[sessionId].sceneType + "_&_" + GetDeviceTypeName();
    CHECK_AND_RETURN_RET_LOG(SceneTypeToEffectChainMap_.count(sceneTypeAndDeviceKey) &&
        SceneTypeToEffectChainMap_[sceneTypeAndDeviceKey] != nullptr, 0, "no such sceneTypeAndDeviceKey in map");
    return SceneTypeToEffectChainMap_[sceneTypeAndDeviceKey]->GetLatency();
}

int32_t AudioEffectChainManager::SetSpatializationSceneType(AudioSpatializationSceneType spatializationSceneType)
{
    std::lock_guard<std::recursive_mutex> lock(dynamicMutex_);
    AUDIO_INFO_LOG("spatialization scene type is set to be %{public}d", spatializationSceneType);
    spatializationSceneType_ = spatializationSceneType;

    if (!spatializationEnabled_ || (GetDeviceTypeName() != "DEVICE_TYPE_BLUETOOTH_A2DP")) {
        return SUCCESS;
    }

    effectHdiInput_[0] = HDI_ROOM_MODE;
    AudioEffectScene sceneType = GetSceneTypeFromSpatializationSceneType(static_cast<AudioEffectScene>(hdiSceneType_));
    effectHdiInput_[1] = static_cast<int32_t>(sceneType);
    effectHdiInput_[HDI_ROOM_MODE_INDEX_TWO] = hdiEffectMode_;
    if (audioEffectHdiParam_->UpdateHdiState(effectHdiInput_) != SUCCESS) {
        AUDIO_WARNING_LOG("set hdi room mode failed");
    }
    AUDIO_DEBUG_LOG("set spatialization scene type to hdi: %{public}d", effectHdiInput_[1]);

    UpdateEffectChainParams(sceneType);

    return SUCCESS;
}

AudioEffectScene AudioEffectChainManager::GetSceneTypeFromSpatializationSceneType(AudioEffectScene sceneType)
{
    if (spatializationSceneType_ == SPATIALIZATION_SCENE_TYPE_DEFAULT) {
        return sceneType;
    } else if (spatializationSceneType_ == SPATIALIZATION_SCENE_TYPE_MUSIC) {
        return SCENE_MUSIC;
    } else if (spatializationSceneType_ == SPATIALIZATION_SCENE_TYPE_MOVIE) {
        return SCENE_MOVIE;
    } else if (spatializationSceneType_ == SPATIALIZATION_SCENE_TYPE_AUDIOBOOK) {
        return SCENE_SPEECH;
    } else {
        AUDIO_WARNING_LOG("wrong spatialization scene type: %{public}d", spatializationSceneType_);
    }
    return sceneType;
}

void AudioEffectChainManager::UpdateExtraSceneType(const std::string &extraSceneType)
{
    std::lock_guard<std::recursive_mutex> lock(dynamicMutex_);
    AUDIO_INFO_LOG("Update scene type: %{public}s to effect chain", extraSceneType.c_str());
    extraSceneType_ = extraSceneType;
    for (auto it = SceneTypeToEffectChainMap_.begin(); it != SceneTypeToEffectChainMap_.end(); ++it) {
        auto audioEffectChain = it->second;
        if (audioEffectChain == nullptr) {
            continue;
        }
        audioEffectChain->SetExtraSceneType(extraSceneType);
        if (audioEffectChain->UpdateEffectParam() != SUCCESS) {
            AUDIO_WARNING_LOG("Update scene type to effect chain failed");
            continue;
        }
    }
}

void AudioEffectChainManager::UpdateEffectChainParams(AudioEffectScene sceneType)
{
    AUDIO_INFO_LOG("Update param: %{public}d to effect chain", sceneType);
    for (auto it = SceneTypeToEffectChainMap_.begin(); it != SceneTypeToEffectChainMap_.end(); ++it) {
        auto audioEffectChain = it->second;
        if (audioEffectChain == nullptr) {
            continue;
        }
        audioEffectChain->SetEffectCurrSceneType(sceneType);
        if (audioEffectChain->UpdateEffectParam() != SUCCESS) {
            AUDIO_WARNING_LOG("Update param to effect chain failed");
            continue;
        }
    }
}

bool AudioEffectChainManager::GetCurSpatializationEnabled()
{
    return spatializationEnabled_;
}

void AudioEffectChainManager::ResetEffectBuffer()
{
    std::lock_guard<std::recursive_mutex> lock(dynamicMutex_);
    for (const auto &[sceneType, effectChain] : SceneTypeToEffectChainMap_) {
        if (effectChain == nullptr) continue;
        effectChain->InitEffectChain();
    }
}

void AudioEffectChainManager::ResetInfo()
{
    EffectToLibraryEntryMap_.clear();
    EffectToLibraryNameMap_.clear();
    EffectChainToEffectsMap_.clear();
    SceneTypeAndModeToEffectChainNameMap_.clear();
    SceneTypeToEffectChainMap_.clear();
    SceneTypeToEffectChainCountMap_.clear();
    SessionIDSet_.clear();
    SceneTypeToSessionIDMap_.clear();
    SessionIDToEffectInfoMap_.clear();
    SceneTypeToEffectChainCountBackupMap_.clear();
    SceneTypeToSpecialEffectSet_.clear();
    deviceType_ = DEVICE_TYPE_SPEAKER;
    deviceSink_ = DEFAULT_DEVICE_SINK;
    isInitialized_ = false;
    spatializationEnabled_ = false;
    headTrackingEnabled_ = false;
    btOffloadEnabled_ = false;
    spkOffloadEnabled_ = false;
    initializedLogFlag_ = true;
    spatializationSceneType_ = SPATIALIZATION_SCENE_TYPE_DEFAULT;
    hdiSceneType_ = 0;
    hdiEffectMode_ = 0;
    isCommonEffectChainExisted_ = false;
}

void AudioEffectChainManager::UpdateRealAudioEffect()
{
    std::lock_guard<std::recursive_mutex> lock(dynamicMutex_);
    uint32_t maxSessionID = 0;
    std::string sceneType = "";
    for (auto& scenePair : SceneTypeToSessionIDMap_) {
        if (!SceneTypeToSpecialEffectSet_.count(scenePair.first)) {
            std::set<std::string> &sessions = scenePair.second;
            FindMaxSessionID(maxSessionID, sceneType, scenePair.first, sessions);
        }
    }
    std::string key = sceneType + "_&_" + GetDeviceTypeName();
    if (!sceneType.empty() && SceneTypeToEffectChainMap_.count(key) && SceneTypeToEffectChainMap_[key] != nullptr) {
        std::shared_ptr<AudioEffectChain> audioEffectChain = SceneTypeToEffectChainMap_[key];
        AudioEffectScene currSceneType;
        UpdateCurrSceneType(currSceneType, sceneType);
        audioEffectChain->SetEffectCurrSceneType(currSceneType);
        audioEffectChain->UpdateEffectParam();
    }
}

bool AudioEffectChainManager::CheckSceneTypeMatch(const std::string &sinkSceneType, const std::string &sceneType)
{
    std::lock_guard<std::recursive_mutex> lock(dynamicMutex_);
    std::string sceneTypeAndDeviceKey = sceneType + "_&_" + GetDeviceTypeName();
    std::string sinkSceneTypeAndDeviceKey = sinkSceneType + "_&_" + GetDeviceTypeName();
    std::string commonSceneTypeAndDeviceKey = COMMON_SCENE_TYPE + "_&_" + GetDeviceTypeName();
    if (!SceneTypeToEffectChainMap_.count(sceneTypeAndDeviceKey) ||
        !SceneTypeToEffectChainMap_.count(sinkSceneTypeAndDeviceKey)) {
        return false;
    }
    if (sceneType == sinkSceneType && SceneTypeToSpecialEffectSet_.count(sinkSceneType)) {
        return true;
    } else {
        if (SceneTypeToEffectChainMap_[sceneTypeAndDeviceKey] ==
            SceneTypeToEffectChainMap_[sinkSceneTypeAndDeviceKey]) {
            return sceneTypeAndDeviceKey == commonSceneTypeAndDeviceKey;
        }
    }
    return false;
}

void AudioEffectChainManager::UpdateCurrSceneType(AudioEffectScene &currSceneType, std::string &sceneType)
{
    if (!spatializationEnabled_ || (GetDeviceTypeName() != "DEVICE_TYPE_BLUETOOTH_A2DP")) {
        currSceneType = static_cast<AudioEffectScene>(GetKeyFromValue(AUDIO_SUPPORTED_SCENE_TYPES, sceneType));
    } else {
        currSceneType = GetSceneTypeFromSpatializationSceneType(static_cast<AudioEffectScene>(
            GetKeyFromValue(AUDIO_SUPPORTED_SCENE_TYPES, sceneType)));
    }
}

void AudioEffectChainManager::ChangeEffectChainCountMapForCreate(const std::string &sceneType)
{
    std::string sceneTypeAndDeviceKey = sceneType + "_&_" + GetDeviceTypeName();
    std::string commonSceneTypeAndDeviceKey = COMMON_SCENE_TYPE + "_&_" + GetDeviceTypeName();
    SceneTypeToEffectChainCountMap_[sceneTypeAndDeviceKey]++;
    if (SceneTypeToEffectChainMap_[sceneTypeAndDeviceKey] ==
        SceneTypeToEffectChainMap_[commonSceneTypeAndDeviceKey]) {
        commonEffectChainCount_++;
    }
}

void AudioEffectChainManager::EraseEffectChainSetAndMapForCreate(const std::string &sceneType)
{
    std::string sceneTypeAndDeviceKey = sceneType + "_&_" + GetDeviceTypeName();
    SceneTypeToEffectChainCountMap_.erase(sceneTypeAndDeviceKey);
    SceneTypeToEffectChainMap_.erase(sceneTypeAndDeviceKey);
    SceneTypeToSpecialEffectSet_.erase(sceneType);
}

void AudioEffectChainManager::FindMaxEffectChannels(const std::string &sceneType,
    const std::set<std::string> &sessions, uint32_t &channels, uint64_t &channelLayout)
{
    for (auto s = sessions.begin(); s != sessions.end(); ++s) {
        SessionEffectInfo info = SessionIDToEffectInfoMap_[*s];
        uint32_t tmpChannelCount;
        uint64_t tmpChannelLayout;
        std::string deviceType = GetDeviceTypeName();
        if (((deviceType == "DEVICE_TYPE_BLUETOOTH_A2DP") || (deviceType == "DEVICE_TYPE_SPEAKER"))
            && ExistAudioEffectChain(sceneType, info.sceneMode, info.spatializationEnabled)
            && IsChannelLayoutSupported(info.channelLayout)) {
            tmpChannelLayout = info.channelLayout;
            tmpChannelCount = info.channels;
        } else {
            tmpChannelCount = DEFAULT_NUM_CHANNEL;
            tmpChannelLayout = DEFAULT_NUM_CHANNELLAYOUT;
        }

        if (tmpChannelCount >= channels) {
            channels = tmpChannelCount;
            channelLayout = tmpChannelLayout;
        }
    }
}

std::shared_ptr<AudioEffectChain> AudioEffectChainManager::CreateAudioEffectChain(const std::string &sceneType)
{
    std::shared_ptr<AudioEffectChain> audioEffectChain = nullptr;
    std::string sceneTypeAndDeviceKey = sceneType + "_&_" + GetDeviceTypeName();
    std::string commonSceneTypeAndDeviceKey = COMMON_SCENE_TYPE + "_&_" + GetDeviceTypeName();

    if ((DEFAULT_NUM_EFFECT_INSTANCES - SceneTypeToSpecialEffectSet_.size()) > 1 && sceneType != COMMON_SCENE_TYPE) {
        SceneTypeToSpecialEffectSet_.insert(sceneType);
#ifdef SENSOR_ENABLE
        audioEffectChain = std::make_shared<AudioEffectChain>(sceneType, headTracker_);
#else
        audioEffectChain = std::make_shared<AudioEffectChain>(sceneType);
#endif
    } else {
        if (!isCommonEffectChainExisted_) {
#ifdef SENSOR_ENABLE
            audioEffectChain = std::make_shared<AudioEffectChain>(COMMON_SCENE_TYPE, headTracker_);
#else
            audioEffectChain = std::make_shared<AudioEffectChain>(COMMON_SCENE_TYPE);
#endif
            if (sceneType != COMMON_SCENE_TYPE) {
                SceneTypeToEffectChainMap_[commonSceneTypeAndDeviceKey] = audioEffectChain;
            }
            commonEffectChainCount_ = 1;
            isCommonEffectChainExisted_ = true;
        } else {
            audioEffectChain = SceneTypeToEffectChainMap_[commonSceneTypeAndDeviceKey];
            commonEffectChainCount_++;
        }
    }
    return audioEffectChain;
}

void AudioEffectChainManager::CheckAndReleaseCommonEffectChain(const std::string &sceneType)
{
    std::string sceneTypeAndDeviceKey = sceneType + "_&_" + GetDeviceTypeName();
    std::string commonSceneTypeAndDeviceKey = COMMON_SCENE_TYPE + "_&_" + GetDeviceTypeName();
    if (!isCommonEffectChainExisted_) {
        return;
    }
    if (SceneTypeToEffectChainMap_[commonSceneTypeAndDeviceKey] == SceneTypeToEffectChainMap_[sceneTypeAndDeviceKey]) {
        if (commonEffectChainCount_ <= 1) {
            SceneTypeToEffectChainMap_.erase(commonSceneTypeAndDeviceKey);
            SceneTypeToEffectChainCountMap_.erase(commonSceneTypeAndDeviceKey);
            commonEffectChainCount_= 0;
            isCommonEffectChainExisted_ = false;
        } else {
            commonEffectChainCount_--;
        }
    }
}

#ifdef WINDOW_MANAGER_ENABLE
void AudioEffectChainManager::AudioRotationListener::OnCreate(Rosen::DisplayId displayId)
{
    AUDIO_DEBUG_LOG("Onchange displayId: %{public}d.", static_cast<int32_t>(displayId));
}

void AudioEffectChainManager::AudioRotationListener::OnDestroy(Rosen::DisplayId displayId)
{
    AUDIO_DEBUG_LOG("Onchange displayId: %{public}d.", static_cast<int32_t>(displayId));
}

void AudioEffectChainManager::AudioRotationListener::OnChange(Rosen::DisplayId displayId)
{
    int32_t newRotationState = 0;
    AudioEffectChainManager *audioEffectChainManager = AudioEffectChainManager::GetInstance();
    audioEffectChainManager->EffectRotationUpdate(static_cast<uint32_t>(newRotationState));
}
#endif

void AudioEffectChainManager::UpdateSpatializationEnabled(AudioSpatializationState spatializationState)
{
    std::lock_guard<std::recursive_mutex> lock(dynamicMutex_);
    spatializationEnabled_ = spatializationState.spatializationEnabled;

    memset_s(static_cast<void *>(effectHdiInput_), sizeof(effectHdiInput_), 0, sizeof(effectHdiInput_));
    if (spatializationEnabled_) {
        if ((deviceType_ == DEVICE_TYPE_BLUETOOTH_A2DP) && (!btOffloadSupported_)) {
            AUDIO_INFO_LOG("A2dp-hal, enter ARM processing");
            btOffloadEnabled_ = false;
            RecoverAllChains();
            return;
        }
        effectHdiInput_[0] = HDI_INIT;
        int32_t ret = audioEffectHdiParam_->UpdateHdiState(effectHdiInput_, DEVICE_TYPE_BLUETOOTH_A2DP);
        if (ret != SUCCESS) {
            AUDIO_ERR_LOG("set hdi init failed, enter route of escape in ARM");
            btOffloadEnabled_ = false;
            RecoverAllChains();
        } else {
            AUDIO_INFO_LOG("set hdi init succeeded, normal spatialization entered");
            btOffloadEnabled_ = true;
        }
    } else {
        effectHdiInput_[0] = HDI_DESTROY;
        AUDIO_INFO_LOG("set hdi destroy.");
        int32_t ret = audioEffectHdiParam_->UpdateHdiState(effectHdiInput_, DEVICE_TYPE_BLUETOOTH_A2DP);
        if (ret != SUCCESS) {
            AUDIO_ERR_LOG("set hdi destroy failed");
        }
        if (deviceType_ == DEVICE_TYPE_BLUETOOTH_A2DP) {
            AUDIO_INFO_LOG("delete all chains if device type is bt.");
            DeleteAllChains();
        }
        btOffloadEnabled_ = false;
    }
}
// for AISS temporarily
bool AudioEffectChainManager::CheckIfSpkDsp()
{
    if (deviceType_ != DEVICE_TYPE_SPEAKER) {
        return true;
    }
    if (debugArmFlag_) {
        for (auto &[key, count] : SceneTypeToEffectChainCountMap_) {
            std::string sceneType = key.substr(0, static_cast<size_t>(key.find("_&_")));
            if (sceneType == "SCENE_MOVIE" && count > 0) {
                return false;
            }
        }
    }
    return true;
}

void AudioEffectChainManager::UpdateEffectBtOffloadSupported(const bool &isSupported)
{
    std::lock_guard<std::recursive_mutex> lock(dynamicMutex_);
    if (isSupported == btOffloadSupported_) {
        return;
    }
    if (!isSupported) {
        btOffloadSupported_ = isSupported;
        AUDIO_INFO_LOG("btOffloadSupported_ off, device disconnect from %{public}d", deviceType_);
        return;
    }

    if (!spatializationEnabled_) {
        btOffloadSupported_ = isSupported;
        AUDIO_INFO_LOG("btOffloadSupported_ on, but spatialization is off, do nothing");
        return;
    }
    // Release ARM, try offload to DSP
    AUDIO_INFO_LOG("btOffloadSupported_ on, try offload effect on device %{public}d", deviceType_);
    AudioSpatializationState oldState = {spatializationEnabled_, headTrackingEnabled_};
    AudioSpatializationState offState = {false, false};
    UpdateSpatializationState(offState);
    btOffloadSupported_ = isSupported;
    UpdateSpatializationState(oldState);
    return;
}
} // namespace AudioStandard
} // namespace OHOS
