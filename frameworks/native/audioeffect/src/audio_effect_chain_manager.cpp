/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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
#include "system_ability_definition.h"
#include "audio_setting_provider.h"
#include "audio_device_type.h"
#include "audio_effect_map.h"
#include "audio_effect_hdi_param.h"

namespace OHOS {
namespace AudioStandard {
namespace {
constexpr int32_t RELEASE_WAIT_TIME_MS = 3000; // 3000ms
constexpr float INITIAL_DSP_VOLUME = -1.0f;
constexpr int32_t INITIAL_DSP_STREAMUSAGE = -2;
constexpr float EPSILON = 1e-6f;

const std::unordered_map<std::string, std::string> AUDIO_PERSISTENCE_EFFECT_KEY {
    {"voip_down", "settings.sound_ai_voip_down_selection"},
};
const std::vector<std::string> AUDIO_PERSISTENCE_SCENE {"SCENE_VOIP_DOWN"};
}

// LCOV_EXCL_START
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
// LCOV_EXCL_STOP

static int32_t FindEffectLib(const std::string &effect,
    const std::vector<std::shared_ptr<AudioEffectLibEntry>> &effectLibraryList,
    std::shared_ptr<AudioEffectLibEntry> &libEntry, std::string &libName)
{
    for (const std::shared_ptr<AudioEffectLibEntry> &lib : effectLibraryList) {
        if (std::any_of(lib->effectName.begin(), lib->effectName.end(),
            [&effect](const std::string &effectName) {
                return effectName == effect;
            })) {
            libName = lib->libraryName;
            libEntry = lib;
            return SUCCESS;
        }
    }
    return ERROR;
}

AudioEffectChainManager::AudioEffectChainManager()
{
    effectToLibraryEntryMap_.clear();
    effectToLibraryNameMap_.clear();
    effectChainToEffectsMap_.clear();
    sceneTypeAndModeToEffectChainNameMap_.clear();
    sceneTypeToEffectChainMap_.clear();
    sceneTypeToEffectChainCountMap_.clear();
    sessionIDSet_.clear();
    sceneTypeToSessionIDMap_.clear();
    sessionIDToEffectInfoMap_.clear();
    effectPropertyMap_.clear();
    deviceType_ = DEVICE_TYPE_SPEAKER;
    deviceSink_ = DEFAULT_DEVICE_SINK;
    spatialDeviceType_ = EARPHONE_TYPE_OTHERS;
    isInitialized_ = false;
    defaultPropertyMap_.clear();
#ifdef SENSOR_ENABLE
    headTracker_ = std::make_shared<HeadTracker>();
#endif

    audioEffectHdiParam_ = std::make_shared<AudioEffectHdiParam>();
    memset_s(static_cast<void *>(effectHdiInput_), sizeof(effectHdiInput_), 0, sizeof(effectHdiInput_));
    GetSysPara("const.build.product", deviceClass_);
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

// LCOV_EXCL_START
int32_t AudioEffectChainManager::UpdateDeviceInfo(int32_t device, const std::string &sinkName)
{
    Trace trace("AudioEffectChainManager::UpdateDeviceInfo");
    if (!isInitialized_) {
        deviceType_ = (DeviceType)device;
        deviceSink_ = sinkName;
        AUDIO_INFO_LOG("has not beed initialized");
        return ERROR;
    }
    deviceSink_ = sinkName;

    if (deviceType_ == (DeviceType)device) {
        return ERROR;
    }
    // Delete effectChain in AP and store in backup map
    AUDIO_PRERELEASE_LOGI("delete all chains when device type change");
    DeleteAllChains();
    deviceType_ = (DeviceType)device;

    return SUCCESS;
}
// LCOV_EXCL_STOP

// LCOV_EXCL_START
void AudioEffectChainManager::SetSpkOffloadState()
{
    Trace trace("AudioEffectChainManager::SetSpkOffloadState");
    int32_t ret;
    if (deviceType_ == DEVICE_TYPE_SPEAKER) {
        if (!spkOffloadEnabled_) {
            effectHdiInput_[0] = HDI_INIT;
            CHECK_AND_RETURN_LOG(audioEffectHdiParam_ != nullptr, "audioEffectHdiParam_ is nullptr");
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
            CHECK_AND_RETURN_LOG(audioEffectHdiParam_ != nullptr, "audioEffectHdiParam_ is nullptr");
            ret = audioEffectHdiParam_->UpdateHdiState(effectHdiInput_, DEVICE_TYPE_SPEAKER);
            if (ret != SUCCESS) {
                AUDIO_WARNING_LOG("set hdi destroy failed, backup speaker entered");
            }
            spkOffloadEnabled_ = false;
        }

        AUDIO_INFO_LOG("recover all chains if device type not bt.");
        RecoverAllChains();
    }
}
// LCOV_EXCL_STOP

// LCOV_EXCL_START
void AudioEffectChainManager::SetOutputDeviceSink(int32_t device, const std::string &sinkName)
{
    Trace trace("AudioEffectChainManager::SetOutputDeviceSink device: " + std::to_string(device) +
        " sinkName: " + sinkName);
    std::lock_guard<std::mutex> lock(dynamicMutex_);
    if (UpdateDeviceInfo(device, sinkName) != SUCCESS) {
        return;
    }
    // recover effectChain in speaker mode
    SetSpkOffloadState();
    return;
}
// LCOV_EXCL_STOP

std::string AudioEffectChainManager::GetDeviceTypeName()
{
    std::string name = "";
    const std::unordered_map<DeviceType, std::string> &supportDeviceType = GetSupportedDeviceType();
    auto device = supportDeviceType.find(deviceType_);
    if (device != supportDeviceType.end()) {
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
    std::lock_guard<std::mutex> lock(dynamicMutex_);
    InitHdiStateInner();
}

void AudioEffectChainManager::InitHdiStateInner()
{
    CHECK_AND_RETURN_LOG(audioEffectHdiParam_ != nullptr, "audioEffectHdiParam_ is nullptr");
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
void AudioEffectChainManager::InitAudioEffectChainManager(const std::vector<EffectChain> &effectChains,
    const EffectChainManagerParam &effectChainManagerParam,
    const std::vector<std::shared_ptr<AudioEffectLibEntry>> &effectLibraryList)
{
    std::lock_guard<std::mutex> lock(dynamicMutex_);
    maxEffectChainCount_ = effectChainManagerParam.maxExtraNum + 1;
    priorSceneList_ = effectChainManagerParam.priorSceneList;
    ConstructEffectChainMgrMaps(effectChains, effectChainManagerParam, effectLibraryList);
    AUDIO_INFO_LOG("EffectToLibraryEntryMap size %{public}zu", effectToLibraryEntryMap_.size());
    AUDIO_DEBUG_LOG("EffectChainToEffectsMap size %{public}zu, SceneTypeAndModeToEffectChainNameMap size %{public}zu",
        effectChainToEffectsMap_.size(), sceneTypeAndModeToEffectChainNameMap_.size());
    InitHdiStateInner();
    isInitialized_ = true;
    RecoverAllChains();
}

void AudioEffectChainManager::ConstructEffectChainMgrMaps(const std::vector<EffectChain> &effectChains,
    const EffectChainManagerParam &effectChainManagerParam,
    const std::vector<std::shared_ptr<AudioEffectLibEntry>> &effectLibraryList)
{
    const std::unordered_map<std::string, std::string> &map = effectChainManagerParam.sceneTypeToChainNameMap;
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
        effectToLibraryEntryMap_[effect] = libEntry;
        effectToLibraryNameMap_[effect] = libName;
    }
    // Construct EffectChainToEffectsMap that stores all effect names of each effect chain
    for (EffectChain efc: effectChains) {
        std::string key = efc.name;
        std::vector <std::string> effects;
        for (std::string effectName: efc.apply) {
            if (effectToLibraryEntryMap_.count(effectName)) {
                effects.emplace_back(effectName);
            }
        }
        effectChainToEffectsMap_[key] = effects;
    }

    // Constrcut SceneTypeAndModeToEffectChainNameMap that stores effectMode associated with the effectChainName
    for (auto item = map.begin(); item != map.end(); ++item) {
        sceneTypeAndModeToEffectChainNameMap_[item->first] = item->second;
        if (item->first.substr(0, effectChainManagerParam.defaultSceneName.size()) ==
            effectChainManagerParam.defaultSceneName) {
            sceneTypeAndModeToEffectChainNameMap_[DEFAULT_SCENE_TYPE + item->first.substr(
                effectChainManagerParam.defaultSceneName.size())] = item->second;
        }
    }
    // Construct effectPropertyMap_ that stores effect's property
    effectPropertyMap_ = effectChainManagerParam.effectDefaultProperty;
    defaultPropertyMap_ = effectChainManagerParam.effectDefaultProperty;
}

bool AudioEffectChainManager::CheckAndAddSessionID(const std::string &sessionID)
{
    std::lock_guard<std::mutex> lock(dynamicMutex_);
    if (sessionIDSet_.count(sessionID)) {
        return false;
    }
    sessionIDSet_.insert(sessionID);
    return true;
}

int32_t AudioEffectChainManager::CreateAudioEffectChainDynamic(const std::string &sceneType)
{
    Trace trace("AudioEffectChainManager::CreateAudioEffectChainDynamic: " + sceneType);
    std::lock_guard<std::mutex> lock(dynamicMutex_);
    return CreateAudioEffectChainDynamicInner(sceneType);
}

int32_t AudioEffectChainManager::SetAudioEffectChainDynamic(std::string &sceneType, const std::string &effectMode)
{
    Trace trace("AudioEffectChainManager::SetAudioEffectChainDynamic: " + sceneType);
    std::string sceneTypeAndDeviceKey = sceneType + "_&_" + GetDeviceTypeName();
    CHECK_AND_RETURN_RET_LOG(sceneTypeToEffectChainMap_.count(sceneTypeAndDeviceKey) &&
        sceneTypeToEffectChainMap_[sceneTypeAndDeviceKey] != nullptr, ERROR,
        "SceneType [%{public}s] does not exist, failed to set", sceneType.c_str());

    std::shared_ptr<AudioEffectChain> audioEffectChain = sceneTypeToEffectChainMap_[sceneTypeAndDeviceKey];
    std::string effectChainKey = sceneType + "_&_" + effectMode + "_&_" + GetDeviceTypeName();
    std::string effectChain = GetEffectChainByMode(effectChainKey);

    ConfigureAudioEffectChain(audioEffectChain, effectMode);
    bool exists = std::find(AUDIO_PERSISTENCE_SCENE.begin(), AUDIO_PERSISTENCE_SCENE.end(), sceneType) !=
        AUDIO_PERSISTENCE_SCENE.end();
    if (exists && !hasLoadedEffectProperties_) {
        LoadEffectProperties();
    }
    std::string tSceneType = (sceneType == DEFAULT_SCENE_TYPE ? DEFAULT_PRESET_SCENE : sceneType);
    for (std::string effect: effectChainToEffectsMap_[effectChain]) {
        AudioEffectHandle handle = nullptr;
        AudioEffectDescriptor descriptor;
        descriptor.libraryName = effectToLibraryNameMap_[effect];
        descriptor.effectName = effect;
        CHECK_AND_CONTINUE_LOG(effectToLibraryEntryMap_.count(effect) && effectToLibraryEntryMap_[effect] != nullptr,
            "null AudioEffectLibEntry");
        int32_t ret = effectToLibraryEntryMap_[effect]->audioEffectLibHandle->createEffect(descriptor, &handle);
        CHECK_AND_CONTINUE_LOG(ret == 0, "EffectToLibraryEntryMap[%{public}s] createEffect fail", effect.c_str());
        CHECK_AND_CONTINUE_LOG(handle != nullptr, "handle is null");
        AUDIO_INFO_LOG("createEffect, EffectToLibraryEntryMap [%{public}s], effectChainKey [%{public}s]",
            effect.c_str(), effectChainKey.c_str());
        AudioEffectScene currSceneType;
        UpdateCurrSceneType(currSceneType, tSceneType);
        auto propIter = effectPropertyMap_.find(effect);
        audioEffectChain->AddEffectHandle(handle, effectToLibraryEntryMap_[effect]->audioEffectLibHandle,
            currSceneType, effect, propIter == effectPropertyMap_.end() ? "" : propIter->second);
    }
    audioEffectChain->ResetIoBufferConfig();

    if (audioEffectChain->IsEmptyEffectHandles()) {
        AUDIO_PRERELEASE_LOGI("Effectchain is empty, copy bufIn to bufOut like EFFECT_NONE mode");
    }

    AUDIO_INFO_LOG("SceneType %{public}s delay %{public}u", sceneType.c_str(), audioEffectChain->GetLatency());
    return SUCCESS;
}

std::string AudioEffectChainManager::GetEffectChainByMode(std::string effectChainKey)
{
    std::string effectChain;
    const std::unordered_map<AudioEffectMode, std::string> &audioSupportedSceneModes = GetAudioSupportedSceneModes();
    std::string effectNone = audioSupportedSceneModes.find(EFFECT_NONE)->second;
    if (!sceneTypeAndModeToEffectChainNameMap_.count(effectChainKey)) {
        AUDIO_ERR_LOG("EffectChain key [%{public}s] does not exist, auto set to %{public}s",
            effectChainKey.c_str(), effectNone.c_str());
        effectChain = effectNone;
    } else {
        effectChain = sceneTypeAndModeToEffectChainNameMap_[effectChainKey];
    }

    if (effectChain != effectNone && !effectChainToEffectsMap_.count(effectChain)) {
        AUDIO_ERR_LOG("EffectChain name [%{public}s] does not exist, auto set to %{public}s",
            effectChain.c_str(), effectNone.c_str());
        effectChain = effectNone;
    }
    return effectChain;
}

void AudioEffectChainManager::ConfigureAudioEffectChain(std::shared_ptr<AudioEffectChain> audioEffectChain,
    const std::string &effectMode)
{
    audioEffectChain->SetEffectMode(effectMode);
    audioEffectChain->SetExtraSceneType(extraSceneType_);
    audioEffectChain->SetSpatialDeviceType(spatialDeviceType_);
    audioEffectChain->SetSpatializationSceneType(spatializationSceneType_);
    audioEffectChain->SetSpatializationEnabled(spatializationEnabled_);
    audioEffectChain->SetLidState(lidState_);
    audioEffectChain->SetFoldState(foldState_);
    audioEffectChain->SetAbsVolumeStateToEffectChain(absVolumeState_);
}

bool AudioEffectChainManager::CheckAndRemoveSessionID(const std::string &sessionID)
{
    std::lock_guard<std::mutex> lock(dynamicMutex_);
    if (!sessionIDSet_.count(sessionID)) {
        return false;
    }
    sessionIDSet_.erase(sessionID);
    return true;
}

int32_t AudioEffectChainManager::ReleaseAudioEffectChainDynamic(const std::string &sceneType)
{
    Trace trace("AudioEffectChainManager::ReleaseAudioEffectChainDynamic: " + sceneType);
    std::lock_guard<std::mutex> lock(dynamicMutex_);
    return ReleaseAudioEffectChainDynamicInner(sceneType);
}

bool AudioEffectChainManager::ExistAudioEffectChain(const std::string &sceneType, const std::string &effectMode)
{
    std::lock_guard<std::mutex> lock(dynamicMutex_);
    return ExistAudioEffectChainInner(sceneType, effectMode);
}

int32_t AudioEffectChainManager::GetOutputChannelInfo(const std::string &sceneType,
    uint32_t &channels, uint64_t &channelLayout)
{
    std::lock_guard<std::mutex> lock(dynamicMutex_);
    std::string sceneTypeAndDeviceKey = sceneType + "_&_" + GetDeviceTypeName();

    auto it = sceneTypeToEffectChainMap_.find(sceneTypeAndDeviceKey);
    CHECK_AND_RETURN_RET_LOG(it != sceneTypeToEffectChainMap_.end() && it->second != nullptr,
        ERROR, "effect chain not found for scene type: %{public}s", sceneTypeAndDeviceKey.c_str());

    auto audioEffectChain = it->second;
    audioEffectChain->UpdateBufferConfig(channels, channelLayout);
    return SUCCESS;
}

int32_t AudioEffectChainManager::ApplyAudioEffectChain(const std::string &sceneType,
    std::unique_ptr<EffectBufferAttr> &bufferAttr)
{
    Trace trace("AudioEffectChainManager::ApplyAudioEffectChain: " + sceneType + " frameLen: " +
        std::to_string(bufferAttr->frameLen) + " numChans: " + std::to_string(bufferAttr->numChans));
    std::string sceneTypeAndDeviceKey = sceneType + "_&_" + GetDeviceTypeName();
    size_t totLen = static_cast<size_t>(bufferAttr->frameLen * bufferAttr->numChans * sizeof(float));
    std::lock_guard<std::mutex> lock(dynamicMutex_);

    auto it = sceneTypeToEffectChainMap_.find(sceneTypeAndDeviceKey);
    if (it == sceneTypeToEffectChainMap_.end() || it->second == nullptr) {
        CHECK_AND_RETURN_RET_LOG(memcpy_s(bufferAttr->bufOut, totLen, bufferAttr->bufIn, totLen) == 0, ERROR,
            "memcpy error when no effect applied");
        return ERROR;
    }

    auto audioEffectChain = it->second;
    AudioEffectProcInfo procInfo = {headTrackingEnabled_, btOffloadEnabled_};
    audioEffectChain->ApplyEffectChain(bufferAttr->bufIn, bufferAttr->bufOut, bufferAttr->frameLen, procInfo);
    audioEffectChain->UpdateBufferConfig(bufferAttr->outChannels, bufferAttr->outChannelLayout);
    return SUCCESS;
}

// LCOV_EXCL_START
int32_t AudioEffectChainManager::EffectDspVolumeUpdate(std::shared_ptr<AudioEffectVolume> audioEffectVolume)
{
    CHECK_AND_RETURN_RET_LOG(audioEffectVolume != nullptr, ERROR, "null audioEffectVolume");
    float volumeMax = 0.0f;
    bool needSendDspVolumeFlag = false;
    for (auto it = sceneTypeToSessionIDMap_.begin(); it != sceneTypeToSessionIDMap_.end(); ++it) {
        std::set<std::string> sessions = sceneTypeToSessionIDMap_[it->first];
        for (auto s = sessions.begin(); s != sessions.end(); s++) {
            if (sessionIDToEffectInfoMap_.find(*s) == sessionIDToEffectInfoMap_.end()) {
                continue;
            }
            CHECK_AND_CONTINUE_LOG(sessionIDToEffectInfoMap_[*s].sceneMode != "EFFECT_NONE",
                "sessionID:%{public}s sceneType:%{public}s, sceneMode is EFFECT_NONE, no send volume",
                (*s).c_str(), it->first.c_str());
            float streamVolumeTemp = audioEffectVolume->GetStreamVolume(*s);
            float systemVolumeTemp = audioEffectVolume->GetSystemVolume(
                sessionIDToEffectInfoMap_[*s].systemVolumeType);
            volumeMax = fmax((streamVolumeTemp * systemVolumeTemp), volumeMax);
            needSendDspVolumeFlag = true;
        }
    }
    if (static_cast<int32_t>(audioEffectVolume->GetDspVolume() * MAX_UINT_VOLUME_NUM) !=
        static_cast<int32_t>(volumeMax * MAX_UINT_VOLUME_NUM) && needSendDspVolumeFlag == true) {
        effectHdiInput_[0] = HDI_VOLUME;
        int32_t dspVolumeMax = static_cast<int32_t>(volumeMax * MAX_UINT_DSP_VOLUME);
        int32_t ret = memcpy_s(&effectHdiInput_[1], SEND_HDI_COMMAND_LEN - sizeof(int8_t),
            &dspVolumeMax, sizeof(int32_t));
        CHECK_AND_RETURN_RET_LOG(ret == 0, ERROR, "memcpy volume failed");
        AUDIO_INFO_LOG("set hdi finalVolume: %{public}u", *(reinterpret_cast<uint32_t *>(&effectHdiInput_[1])));
        CHECK_AND_RETURN_RET_LOG(audioEffectHdiParam_ != nullptr, ERROR, "audioEffectHdiParam_ is nullptr");
        if (audioEffectHdiParam_->UpdateHdiState(effectHdiInput_) != SUCCESS) {
            AUDIO_WARNING_LOG("set hdi volume failed");
            return ERROR;
        }
        audioEffectVolume->SetDspVolume(volumeMax);
    }
    return SUCCESS;
}
// LCOV_EXCL_STOP

int32_t AudioEffectChainManager::EffectApVolumeUpdate(std::shared_ptr<AudioEffectVolume> audioEffectVolume)
{
    CHECK_AND_RETURN_RET_LOG(audioEffectVolume != nullptr, ERROR, "null audioEffectVolume");
    for (auto sessionId = sessionIDSet_.begin(); sessionId != sessionIDSet_.end(); ++sessionId) {
        if (sessionIDToEffectInfoMap_.find(*sessionId) == sessionIDToEffectInfoMap_.end()) {
            continue;
        }
        CHECK_AND_CONTINUE_LOG(sessionIDToEffectInfoMap_[*sessionId].sceneMode != "EFFECT_NONE",
            "sessionID:%{public}s, sceneMode is EFFECT_NONE, no send volume", (*sessionId).c_str());
        std::string sceneTypeTemp = sessionIDToEffectInfoMap_[*sessionId].sceneType;
        std::string sceneTypeAndDeviceKey = sceneTypeTemp + "_&_" + GetDeviceTypeName();
        CHECK_AND_CONTINUE_LOG(sceneTypeToEffectChainMap_.count(sceneTypeAndDeviceKey) != 0 &&
            sceneTypeToEffectChainMap_[sceneTypeAndDeviceKey] != nullptr,
            "null audioEffectChain, sceneType: %{public}s", sceneTypeTemp.c_str());
        auto audioEffectChain = sceneTypeToEffectChainMap_[sceneTypeAndDeviceKey];
        float streamVolumeTemp = audioEffectVolume->GetStreamVolume(*sessionId);
        float systemVolumeTemp = audioEffectVolume->GetSystemVolume(
            sessionIDToEffectInfoMap_[*sessionId].systemVolumeType);
        float currVolumeTemp = audioEffectChain->GetCurrVolume();
        float volumeMax = fmax((streamVolumeTemp * systemVolumeTemp), currVolumeTemp);
        if (volumeMax > currVolumeTemp) {
            audioEffectChain->SetCurrVolume(volumeMax);
        }
        audioEffectChain->SetFinalVolumeState(true);
    }
    return SendEffectApVolume(audioEffectVolume);
}

// LCOV_EXCL_START
int32_t AudioEffectChainManager::SendEffectApVolume(std::shared_ptr<AudioEffectVolume> audioEffectVolume)
{
    CHECK_AND_RETURN_RET_LOG(audioEffectVolume != nullptr, ERROR, "null audioEffectVolume");
    for (auto it = sceneTypeToEffectChainMap_.begin(); it != sceneTypeToEffectChainMap_.end(); ++it) {
        if (it->second == nullptr) {
            AUDIO_INFO_LOG("null audioEffectChain, sceneType: %{public}s", it->first.c_str());
            continue;
        }
        auto audioEffectChain = it->second;
        float volumeMax = audioEffectChain->GetCurrVolume();
        if (static_cast<int32_t>(audioEffectChain->GetFinalVolume() * MAX_UINT_VOLUME_NUM) ==
            static_cast<int32_t>(volumeMax * MAX_UINT_VOLUME_NUM)) {
            audioEffectChain->SetFinalVolumeState(false);
        } else {
            if (audioEffectChain->GetFinalVolumeState() == true) {
                audioEffectChain->SetFinalVolume(volumeMax);
                int32_t ret = audioEffectChain->UpdateEffectParam();
                CHECK_AND_CONTINUE_LOG(ret == 0, "set ap volume failed, ret: %{public}d", ret);
                AUDIO_INFO_LOG("The delay of SceneType %{public}s is %{public}u, finalVolume changed to %{public}f",
                    it->first.c_str(), audioEffectChain->GetLatency(), volumeMax);
                audioEffectChain->SetFinalVolumeState(false);
            }
        }
    }
    for (auto it = sceneTypeToEffectChainMap_.begin(); it != sceneTypeToEffectChainMap_.end(); ++it) {
        CHECK_AND_CONTINUE_LOG(it->second != nullptr, "null audioEffectChain, sceneType: %{public}s",
            it->first.c_str());
        auto audioEffectChain = it->second;
        float volume = 0.0f;
        audioEffectChain->SetCurrVolume(volume);
    }
    return SUCCESS;
}
// LCOV_EXCL_STOP

int32_t AudioEffectChainManager::EffectVolumeUpdate()
{
    std::lock_guard<std::mutex> lock(dynamicMutex_);
    std::shared_ptr<AudioEffectVolume> audioEffectVolume = AudioEffectVolume::GetInstance();
    CHECK_AND_RETURN_RET_LOG(audioEffectVolume != nullptr, ERROR, "null audioEffectVolume");
    return EffectVolumeUpdateInner(audioEffectVolume);
}

int32_t AudioEffectChainManager::StreamVolumeUpdate(const std::string sessionIDString, const float streamVolume)
{
    std::lock_guard<std::mutex> lock(dynamicMutex_);
    // update streamVolume
    std::shared_ptr<AudioEffectVolume> audioEffectVolume = AudioEffectVolume::GetInstance();
    CHECK_AND_RETURN_RET_LOG(audioEffectVolume != nullptr, ERROR, "null audioEffectVolume");
    audioEffectVolume->SetStreamVolume(sessionIDString, streamVolume);
    int32_t ret;
    AUDIO_INFO_LOG("sessionId: %{public}s, set streamVolume: %{public}f",
        sessionIDString.c_str(), audioEffectVolume->GetStreamVolume(sessionIDString));
    ret = EffectVolumeUpdateInner(audioEffectVolume);
    return ret;
}

int32_t AudioEffectChainManager::DeleteStreamVolume(const std::string StringSessionID)
{
    // delete streamVolume by sessionId, but don't update volume
    std::lock_guard<std::mutex> lock(dynamicMutex_);
    return DeleteStreamVolumeInner(StringSessionID);
}

int32_t AudioEffectChainManager::DeleteStreamVolumeInner(const std::string StringSessionID)
{
    std::shared_ptr<AudioEffectVolume> audioEffectVolume = AudioEffectVolume::GetInstance();
    CHECK_AND_RETURN_RET_LOG(audioEffectVolume != nullptr, ERROR, "null audioEffectVolume");
    audioEffectVolume->StreamVolumeDelete(StringSessionID);
    AUDIO_INFO_LOG("delete streamVolume, sessionId: %{public}s", StringSessionID.c_str());
    return SUCCESS;
}

int32_t AudioEffectChainManager::SetEffectSystemVolume(const int32_t systemVolumeType, const float systemVolume)
{
    std::lock_guard<std::mutex> lock(dynamicMutex_);
    // set systemVolume by systemVolumeType
    std::shared_ptr<AudioEffectVolume> audioEffectVolume = AudioEffectVolume::GetInstance();
    CHECK_AND_RETURN_RET_LOG(audioEffectVolume != nullptr, ERROR, "null audioEffectVolume");
    audioEffectVolume->SetSystemVolume(systemVolumeType, systemVolume);
    AUDIO_INFO_LOG("systemVolumeType: %{public}d, systemVolume: %{public}f", systemVolumeType,
        audioEffectVolume->GetSystemVolume(systemVolumeType));

    return EffectVolumeUpdateInner(audioEffectVolume);
}

#ifdef WINDOW_MANAGER_ENABLE
int32_t AudioEffectChainManager::EffectDspRotationUpdate(std::shared_ptr<AudioEffectRotation> audioEffectRotation,
    const uint32_t rotationState)
{
    // send rotation to dsp
    AUDIO_DEBUG_LOG("send rotation to dsp.");
    CHECK_AND_RETURN_RET_LOG(audioEffectRotation != nullptr, ERROR, "null audioEffectRotation");
    AUDIO_DEBUG_LOG("rotationState change, new state: %{public}d, previous state: %{public}d",
        rotationState, audioEffectRotation->GetRotation());
    effectHdiInput_[0] = HDI_ROTATION;
    effectHdiInput_[1] = rotationState;
    AUDIO_INFO_LOG("set hdi rotation: %{public}d", effectHdiInput_[1]);
    CHECK_AND_RETURN_RET_LOG(audioEffectHdiParam_ != nullptr, ERROR, "audioEffectHdiParam_ is nullptr");
    if (audioEffectHdiParam_->UpdateHdiState(effectHdiInput_) != SUCCESS) {
        AUDIO_WARNING_LOG("set hdi rotation failed");
        return ERROR;
    }

    return SUCCESS;
}

// LCOV_EXCL_START
int32_t AudioEffectChainManager::EffectApRotationUpdate(std::shared_ptr<AudioEffectRotation> audioEffectRotation,
    const uint32_t rotationState)
{
    // send rotation to ap
    AUDIO_DEBUG_LOG("send rotation to ap.");
    CHECK_AND_RETURN_RET_LOG(audioEffectRotation != nullptr, ERROR, "null audioEffectRotation");
    AUDIO_DEBUG_LOG("rotationState change, new state: %{public}d, previous state: %{public}d",
        rotationState, audioEffectRotation->GetRotation());
    for (auto it = sceneTypeToSessionIDMap_.begin(); it != sceneTypeToSessionIDMap_.end(); it++) {
        std::string sceneTypeAndDeviceKey = it->first + "_&_" + GetDeviceTypeName();
        if (!sceneTypeToEffectChainMap_.count(sceneTypeAndDeviceKey)) {
            return ERROR;
        }
        auto audioEffectChain = sceneTypeToEffectChainMap_[sceneTypeAndDeviceKey];
        if (audioEffectChain == nullptr) {
            return ERROR;
        }
        int32_t ret = audioEffectChain->UpdateEffectParam();
        CHECK_AND_RETURN_RET_LOG(ret == 0, ERROR, "set ap rotation failed");
        AUDIO_INFO_LOG("The delay of SceneType %{public}s is %{public}u, rotation changed to %{public}u",
            it->first.c_str(), audioEffectChain->GetLatency(), rotationState);
        }

    return SUCCESS;
}
// LCOV_EXCL_STOP

int32_t AudioEffectChainManager::EffectRotationUpdate(const uint32_t rotationState)
{
    std::lock_guard<std::mutex> lock(dynamicMutex_);
    std::shared_ptr<AudioEffectRotation> audioEffectRotation = AudioEffectRotation::GetInstance();
    AUDIO_INFO_LOG("rotation update to %{public}u", rotationState);
    if (audioEffectRotation->GetRotation() != rotationState) {
        audioEffectRotation->SetRotation(rotationState);
        EffectDspRotationUpdate(audioEffectRotation, rotationState);
        EffectApRotationUpdate(audioEffectRotation, rotationState);
    }

    return SUCCESS;
}
#endif

int32_t AudioEffectChainManager::UpdateMultichannelConfig(const std::string &sceneType)
{
    std::lock_guard<std::mutex> lock(dynamicMutex_);
    return UpdateMultichannelConfigInner(sceneType);
}

int32_t AudioEffectChainManager::InitAudioEffectChainDynamic(const std::string &sceneType)
{
    Trace trace("AudioEffectChainManager::InitAudioEffectChainDynamic: " + sceneType);
    std::lock_guard<std::mutex> lock(dynamicMutex_);
    return InitAudioEffectChainDynamicInner(sceneType);
}

// LCOV_EXCL_START
int32_t AudioEffectChainManager::InitAudioEffectChainDynamicInner(const std::string &sceneType)
{
    CHECK_AND_RETURN_RET_LOG(isInitialized_, ERROR, "has not been initialized");
    CHECK_AND_RETURN_RET_LOG(sceneType != "", ERROR, "null sceneType");

    std::shared_ptr<AudioEffectChain> audioEffectChain = nullptr;
    std::string sceneTypeAndDeviceKey = sceneType + "_&_" + GetDeviceTypeName();
    if (!sceneTypeToEffectChainMap_.count(sceneTypeAndDeviceKey)) {
        return SUCCESS;
    } else {
        audioEffectChain = sceneTypeToEffectChainMap_[sceneTypeAndDeviceKey];
    }
    if (audioEffectChain != nullptr) {
        audioEffectChain->InitEffectChain();
        AUDIO_INFO_LOG("init effect buffer");
    }

    return SUCCESS;
}
// LCOV_EXCL_STOP

int32_t AudioEffectChainManager::UpdateSpatializationState(AudioSpatializationState spatializationState)
{
    Trace trace("AudioEffectChainManager::UpdateSpatializationState");
    std::lock_guard<std::mutex> lock(dynamicMutex_);
    return UpdateSpatializationStateInner(spatializationState);
}

int32_t AudioEffectChainManager::UpdateSpatialDeviceType(AudioSpatialDeviceType spatialDeviceType)
{
    std::lock_guard<std::mutex> lock(dynamicMutex_);
    int32_t ret{ SUCCESS };
    spatialDeviceType_ = spatialDeviceType;

    effectHdiInput_[0] = HDI_UPDATE_SPATIAL_DEVICE_TYPE;
    effectHdiInput_[1] = spatialDeviceType_;
    AUDIO_INFO_LOG("set hdi spatialDeviceType: %{public}d", effectHdiInput_[1]);
    CHECK_AND_RETURN_RET_LOG(audioEffectHdiParam_ != nullptr, ERROR, "audioEffectHdiParam_ is nullptr");
    ret = audioEffectHdiParam_->UpdateHdiState(effectHdiInput_, DEVICE_TYPE_BLUETOOTH_A2DP);
    if (ret != SUCCESS) {
        AUDIO_WARNING_LOG("set hdi update spatial device type failed");
    }

    for (const auto& sceneType2EffectChain : sceneTypeToEffectChainMap_) {
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
    std::lock_guard<std::mutex> lock(dynamicMutex_);
    return ReturnEffectChannelInfoInner(sceneType, channels, channelLayout);
}

int32_t AudioEffectChainManager::SessionInfoMapAdd(const std::string &sessionID, const SessionEffectInfo &info)
{
    std::lock_guard<std::mutex> lock(dynamicMutex_);
    CHECK_AND_RETURN_RET_LOG(sessionID != "", ERROR, "null sessionID");
    if (!sessionIDToEffectInfoMap_.count(sessionID)) {
        sceneTypeToSessionIDMap_[info.sceneType].insert(sessionID);
        sessionIDToEffectInfoMap_[sessionID] = info;
    } else if (sessionIDToEffectInfoMap_[sessionID].sceneMode != info.sceneMode) {
        sessionIDToEffectInfoMap_[sessionID] = info;
    } else {
        return ERROR;
    }
    return SUCCESS;
}

int32_t AudioEffectChainManager::SessionInfoMapDelete(const std::string &sceneType, const std::string &sessionID)
{
    std::lock_guard<std::mutex> lock(dynamicMutex_);
    if (!sceneTypeToSessionIDMap_.count(sceneType)) {
        return ERROR;
    }
    if (sceneTypeToSessionIDMap_[sceneType].erase(sessionID)) {
        if (sceneTypeToSessionIDMap_[sceneType].empty()) {
            sceneTypeToSessionIDMap_.erase(sceneType);
        }
    } else {
        return ERROR;
    }
    if (!sessionIDToEffectInfoMap_.erase(sessionID)) {
        return ERROR;
    }
    return SUCCESS;
}

int32_t AudioEffectChainManager::SetHdiParam(const AudioEffectScene &sceneType)
{
    if (!isInitialized_) {
        if (initializedLogFlag_) {
            AUDIO_ERR_LOG("audioEffectChainManager has not been initialized");
            initializedLogFlag_ = false;
        }
        return ERROR;
    }
    memset_s(static_cast<void *>(effectHdiInput_), sizeof(effectHdiInput_), 0, sizeof(effectHdiInput_));

    if (currDspSceneType_ != sceneType) {
        effectHdiInput_[0] = HDI_ROOM_MODE;
        effectHdiInput_[1] = sceneType;
        AUDIO_PRERELEASE_LOGI("set hdi room mode sceneType: %{public}d", effectHdiInput_[1]);
        CHECK_AND_RETURN_RET_LOG(audioEffectHdiParam_ != nullptr, ERROR, "audioEffectHdiParam_ is nullptr");
        if (audioEffectHdiParam_->UpdateHdiState(effectHdiInput_) != SUCCESS) {
            AUDIO_WARNING_LOG("set hdi room mode failed");
            return ERROR;
        }
        currDspSceneType_ = sceneType;
    }
    return SUCCESS;
}

// LCOV_EXCL_START
void AudioEffectChainManager::UpdateSensorState()
{
    effectHdiInput_[0] = HDI_HEAD_MODE;
    effectHdiInput_[1] = headTrackingEnabled_ == true ? 1 : 0;
    AUDIO_INFO_LOG("set hdi head mode: %{public}d", effectHdiInput_[1]);
    CHECK_AND_RETURN_LOG(audioEffectHdiParam_ != nullptr, "audioEffectHdiParam_ is nullptr");
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
    for (auto it = sceneTypeToEffectChainMap_.begin(); it != sceneTypeToEffectChainMap_.end(); ++it) {
        auto audioEffectChain = it->second;
        if (audioEffectChain == nullptr) {
            continue;
        }
        audioEffectChain->SetHeadTrackingDisabled();
    }
#endif
}
// LCOV_EXCL_STOP

void AudioEffectChainManager::DeleteAllChains()
{
    Trace trace("AudioEffectChainManager::DeleteAllChains");
    std::map<std::string, int32_t> sceneTypeToEffectChainCountBackupMap;
    for (auto it = sceneTypeToEffectChainCountMap_.begin(); it != sceneTypeToEffectChainCountMap_.end(); ++it) {
        AUDIO_INFO_LOG("sceneTypeAndDeviceKey %{public}s count:%{public}d", it->first.c_str(), it->second);
        sceneTypeToEffectChainCountBackupMap.insert(std::make_pair(it->first, it->second));
    }

    for (auto it = sceneTypeToEffectChainCountBackupMap.begin(); it != sceneTypeToEffectChainCountBackupMap.end();
        ++it) {
        std::string sceneType = it->first.substr(0, static_cast<size_t>(it->first.find("_&_")));
        for (int32_t k = 0; k < it->second; ++k) {
            ReleaseAudioEffectChainDynamicInner(sceneType);
        }
    }
    return;
}

void AudioEffectChainManager::RecoverAllChains()
{
    Trace trace("AudioEffectChainManager::RecoverAllChains");
    for (auto item : sceneTypeCountList_) {
        AUDIO_INFO_LOG("sceneType %{public}s count:%{public}d", item.first.c_str(), item.second);
        for (int32_t k = 0; k < item.second; ++k) {
            CreateAudioEffectChainDynamicInner(item.first);
        }
        UpdateMultichannelConfigInner(item.first);
    }
    UpdateDefaultAudioEffectInner();
    UpdateStreamUsageInner();

    std::shared_ptr<AudioEffectVolume> audioEffectVolume = AudioEffectVolume::GetInstance();
    CHECK_AND_RETURN_LOG(audioEffectVolume != nullptr, "null audioEffectVolume");
    EffectVolumeUpdateInner(audioEffectVolume);
}

// LCOV_EXCL_START
uint32_t AudioEffectChainManager::GetLatency(const std::string &sessionId)
{
    Trace trace("AudioEffectChainManager::GetLatency: " + sessionId);
    std::lock_guard<std::mutex> lock(dynamicMutex_);
    if (((deviceType_ == DEVICE_TYPE_SPEAKER) && (spkOffloadEnabled_)) ||
        ((deviceType_ == DEVICE_TYPE_BLUETOOTH_A2DP) && (btOffloadEnabled_))) {
        AUDIO_DEBUG_LOG("offload enabled, return 0");
        return 0;
    }

    CHECK_AND_RETURN_RET(sessionIDToEffectInfoMap_.count(sessionId), 0);
    if (sessionIDToEffectInfoMap_[sessionId].sceneMode == "" ||
        sessionIDToEffectInfoMap_[sessionId].sceneMode == "None") {
        AUDIO_DEBUG_LOG("seceneMode is None, return 0");
        return 0;
    }

    std::string sceneTypeAndDeviceKey = sessionIDToEffectInfoMap_[sessionId].sceneType + "_&_" + GetDeviceTypeName();
    CHECK_AND_RETURN_RET(sceneTypeToEffectChainMap_.count(sceneTypeAndDeviceKey) &&
        sceneTypeToEffectChainMap_[sceneTypeAndDeviceKey] != nullptr, 0);
    return sceneTypeToEffectChainMap_[sceneTypeAndDeviceKey]->GetLatency();
}
// LCOV_EXCL_STOP

int32_t AudioEffectChainManager::SetSpatializationSceneType(AudioSpatializationSceneType spatializationSceneType)
{
    std::lock_guard<std::mutex> lock(dynamicMutex_);
    AUDIO_INFO_LOG("spatialization scene type is set to be %{public}d", spatializationSceneType);
    spatializationSceneType_ = spatializationSceneType;

    if (!spatializationEnabled_ || !IsDeviceTypeSupportingSpatialization()) {
        return SUCCESS;
    }

    effectHdiInput_[0] = HDI_SPATIALIZATION_SCENE_TYPE;
    effectHdiInput_[1] = static_cast<int32_t>(spatializationSceneType_);
    CHECK_AND_RETURN_RET_LOG(audioEffectHdiParam_ != nullptr, ERROR, "audioEffectHdiParam_ is nullptr");
    if (audioEffectHdiParam_->UpdateHdiState(effectHdiInput_) != SUCCESS) {
        AUDIO_WARNING_LOG("set hdi spatialization scene type failed");
    }

    SetSpatializationSceneTypeToChains();

    return SUCCESS;
}

// LCOV_EXCL_START
void AudioEffectChainManager::SendAudioParamToHDI(
    HdiSetParamCommandCode code, const std::string &value, DeviceType device)
{
    effectHdiInput_[0] = code;
    CHECK_AND_RETURN_LOG(StringConverter(value, effectHdiInput_[1]),
        "convert invalid bufferSize: %{public}s", value.c_str());
    CHECK_AND_RETURN_LOG(audioEffectHdiParam_ != nullptr, "audioEffectHdiParam_ is nullptr");
    if (audioEffectHdiParam_->UpdateHdiState(effectHdiInput_) != SUCCESS) {
        AUDIO_WARNING_LOG("set hdi parameter failed for code %{public}d and value %{public}s", code, value.c_str());
    }
}
// LCOV_EXCL_STOP

// LCOV_EXCL_START
void AudioEffectChainManager::SendAudioParamToARM(HdiSetParamCommandCode code, const std::string &value)
{
    for (const auto &[scene, audioEffectChain] : sceneTypeToEffectChainMap_) {
        if (audioEffectChain == nullptr) {
            continue;
        }

        bool paramUpdated = false;
        switch (code) {
            case HDI_EXTRA_SCENE_TYPE:
                audioEffectChain->SetExtraSceneType(value);
                paramUpdated = true;
                break;
            case HDI_FOLD_STATE:
                audioEffectChain->SetFoldState(value);
                paramUpdated = true;
                break;
            case HDI_LID_STATE:
                audioEffectChain->SetLidState(value);
                paramUpdated = true;
                break;
            default:
                break;
        }

        if (paramUpdated && audioEffectChain->UpdateEffectParam() != SUCCESS) {
            AUDIO_WARNING_LOG("Update effect chain failed for code %{public}d and value %{public}s",
                              code, value.c_str());
        }
    }
}
// LCOV_EXCL_STOP

// LCOV_EXCL_START
void AudioEffectChainManager::UpdateParamExtra(
    const std::string &mainkey, const std::string &subkey, const std::string &value)
{
    std::lock_guard<std::mutex> lock(dynamicMutex_);
    auto updateParam = [&](std::string &param, HdiSetParamCommandCode code) {
        AUDIO_INFO_LOG("Set %{public}s: %{public}s to hdi and arm", subkey.c_str(), value.c_str());
        param = value;
        SendAudioParamToHDI(code, value, DEVICE_TYPE_SPEAKER);
        SendAudioParamToARM(code, value);
    };

    if (mainkey == "audio_effect" && subkey == "update_audio_effect_type") {
        updateParam(extraSceneType_, HDI_EXTRA_SCENE_TYPE);
    } else if (mainkey == "device_status" && subkey == "fold_state") {
        updateParam(foldState_, HDI_FOLD_STATE);
    } else if (mainkey == "device_status" && subkey == "lid_state") {
        updateParam(lidState_, HDI_LID_STATE);
    } else {
        AUDIO_INFO_LOG("UpdateParamExtra failed, mainkey is %{public}s, subkey is %{public}s, "
            "value is %{public}s", mainkey.c_str(), subkey.c_str(), value.c_str());
        return;
    }
}
// LCOV_EXCL_STOP

// LCOV_EXCL_START
void AudioEffectChainManager::SetSpatializationSceneTypeToChains()
{
    for (auto it = sceneTypeToEffectChainMap_.begin(); it != sceneTypeToEffectChainMap_.end(); ++it) {
        auto audioEffectChain = it->second;
        if (audioEffectChain == nullptr) {
            continue;
        }
        audioEffectChain->SetSpatializationSceneType(spatializationSceneType_);
        if (audioEffectChain->UpdateEffectParam() != SUCCESS) {
            AUDIO_WARNING_LOG("Update param to effect chain failed");
            continue;
        }
    }
}
// LCOV_EXCL_STOP

// LCOV_EXCL_START
void AudioEffectChainManager::SetSpatializationEnabledToChains()
{
    for (auto it = sceneTypeToEffectChainMap_.begin(); it != sceneTypeToEffectChainMap_.end(); ++it) {
        auto audioEffectChain = it->second;
        if (audioEffectChain == nullptr) {
            continue;
        }

        if (btOffloadEnabled_ == false) {
            audioEffectChain->SetSpatializationEnabledForFading(spatializationEnabled_);
        } else {
            audioEffectChain->SetSpatializationEnabledForFading(false);
        }
    }
}
// LCOV_EXCL_STOP

void AudioEffectChainManager::ResetInfo()
{
    std::lock_guard<std::mutex> lock(dynamicMutex_);
    effectToLibraryEntryMap_.clear();
    effectToLibraryNameMap_.clear();
    effectChainToEffectsMap_.clear();
    sceneTypeAndModeToEffectChainNameMap_.clear();
    sceneTypeToEffectChainMap_.clear();
    sceneTypeToEffectChainCountMap_.clear();
    sessionIDSet_.clear();
    sceneTypeToSessionIDMap_.clear();
    sessionIDToEffectInfoMap_.clear();
    sceneTypeToSpecialEffectSet_.clear();
    effectPropertyMap_.clear();
    deviceType_ = DEVICE_TYPE_SPEAKER;
    deviceSink_ = DEFAULT_DEVICE_SINK;
    isInitialized_ = false;
    spatializationEnabled_ = false;
    headTrackingEnabled_ = false;
    btOffloadEnabled_ = false;
    spkOffloadEnabled_ = false;
    initializedLogFlag_ = true;
    spatializationSceneType_ = SPATIALIZATION_SCENE_TYPE_MUSIC;
    isDefaultEffectChainExisted_ = false;
    currDspStreamUsage_ = INITIAL_DSP_STREAMUSAGE;
    currDspSceneType_ = SCENE_INITIAL;
    std::shared_ptr<AudioEffectVolume> audioEffectVolume = AudioEffectVolume::GetInstance();
    CHECK_AND_RETURN_LOG(audioEffectVolume != nullptr, "null audioEffectVolume");
    audioEffectVolume->SetDspVolume(INITIAL_DSP_VOLUME);
}

void AudioEffectChainManager::UpdateDefaultAudioEffect()
{
    Trace trace("AudioEffectChainManager::UpdateDefaultAudioEffect");
    std::lock_guard<std::mutex> lock(dynamicMutex_);
    UpdateDefaultAudioEffectInner();
}

void AudioEffectChainManager::UpdateDefaultAudioEffectInner()
{
    Trace trace("AudioEffectChainManager::UpdateDefaultAudioEffectInner");
    // for default scene type
    uint32_t maxDefaultSessionID = 0;
    uint32_t maxSessionID = 0;
    for (auto& scenePair : sceneTypeToSessionIDMap_) {
        std::set<std::string> &sessions = scenePair.second;
        if (!sceneTypeToSpecialEffectSet_.count(scenePair.first) &&
            std::find(priorSceneList_.begin(), priorSceneList_.end(),
            scenePair.first) == priorSceneList_.end()) {
            FindMaxSessionID(maxDefaultSessionID, maxDefaultSessionIDToSceneType_, scenePair.first, sessions);
        }
        FindMaxSessionID(maxSessionID, maxSessionIDToSceneType_, scenePair.first, sessions);
    }
    maxSessionID_ = maxSessionID;
    AUDIO_INFO_LOG("newest stream, maxDefaultSessionID: %{public}u, sceneType: %{public}s,"
        "maxSessionID: %{public}u, sceneType: %{public}s",
        maxDefaultSessionID, maxDefaultSessionIDToSceneType_.c_str(),
        maxSessionID_, maxSessionIDToSceneType_.c_str());

    std::string key = maxDefaultSessionIDToSceneType_ + "_&_" + GetDeviceTypeName();
    std::string maxDefaultSession = std::to_string(maxDefaultSessionID);
    AudioEffectScene currDefaultSceneType;
    UpdateCurrSceneType(currDefaultSceneType, maxDefaultSessionIDToSceneType_);
    if (!maxDefaultSessionIDToSceneType_.empty() && sessionIDToEffectInfoMap_.count(maxDefaultSession) &&
        sceneTypeToEffectChainMap_.count(key) && sceneTypeToEffectChainMap_[key] != nullptr) {
        SessionEffectInfo info = sessionIDToEffectInfoMap_[maxDefaultSession];
        std::shared_ptr<AudioEffectChain> audioEffectChain = sceneTypeToEffectChainMap_[key];
        audioEffectChain->SetEffectCurrSceneType(currDefaultSceneType);
        audioEffectChain->SetStreamUsage(info.streamUsage);
        audioEffectChain->UpdateEffectParam();
    }
}

void AudioEffectChainManager::UpdateStreamUsage()
{
    Trace trace("AudioEffectChainManager::UpdateStreamUsage");
    std::lock_guard<std::mutex> lock(dynamicMutex_);
    UpdateStreamUsageInner();
}

// LCOV_EXCL_START
void AudioEffectChainManager::UpdateStreamUsageInner()
{
    Trace trace("AudioEffectChainManager::UpdateStreamUsageInner");
    // for special scene type
    for (auto& specialSceneType : sceneTypeToSpecialEffectSet_) {
        uint32_t maxSpecialSessionID = 0;
        std::string maxSpecialSceneType = "";
        auto it = sceneTypeToSessionIDMap_.find(specialSceneType);
        if (it != sceneTypeToSessionIDMap_.end()) {
            std::set<std::string> &sessions = it->second;
            FindMaxSessionID(maxSpecialSessionID, maxSpecialSceneType, specialSceneType, sessions);
        }
        std::string maxSpecialSession = std::to_string(maxSpecialSessionID);
        std::string key = maxSpecialSceneType + "_&_" + GetDeviceTypeName();
        if (!maxSpecialSceneType.empty() && sessionIDToEffectInfoMap_.count(maxSpecialSession) &&
            sceneTypeToEffectChainMap_.count(key) && sceneTypeToEffectChainMap_[key] != nullptr) {
            SessionEffectInfo info = sessionIDToEffectInfoMap_[maxSpecialSession];
            std::shared_ptr<AudioEffectChain> audioEffectChain = sceneTypeToEffectChainMap_[key];
            audioEffectChain->SetStreamUsage(info.streamUsage);
            audioEffectChain->UpdateEffectParam();
        }
        AUDIO_INFO_LOG("newest stream, maxSpecialSessionID: %{public}u, sceneType: %{public}s",
            maxSpecialSessionID, maxSpecialSceneType.c_str());
    }
    // for prior scene type
    for (auto& priorSceneType : priorSceneList_) {
        uint32_t maxPriorSessionID = 0;
        std::string maxPriorSceneType = "";
        auto it = sceneTypeToSessionIDMap_.find(priorSceneType);
        if (it != sceneTypeToSessionIDMap_.end()) {
            std::set<std::string> &sessions = it->second;
            FindMaxSessionID(maxPriorSessionID, maxPriorSceneType, priorSceneType, sessions);
        }
        std::string key = maxPriorSceneType + "_&_" + GetDeviceTypeName();
        std::string maxPriorSession = std::to_string(maxPriorSessionID);
        if (!maxPriorSceneType.empty() && sessionIDToEffectInfoMap_.count(maxPriorSession) &&
            sceneTypeToEffectChainMap_.count(key) && sceneTypeToEffectChainMap_[key] != nullptr) {
            SessionEffectInfo info = sessionIDToEffectInfoMap_[maxPriorSession];
            std::shared_ptr<AudioEffectChain> audioEffectChain = sceneTypeToEffectChainMap_[key];
            audioEffectChain->SetStreamUsage(info.streamUsage);
            audioEffectChain->UpdateEffectParam();
        }
        AUDIO_INFO_LOG("newest stream, maxSpecialSessionID: %{public}u, sceneType: %{public}s",
            maxPriorSessionID, maxPriorSceneType.c_str());
    }
    // update dsp scene type and stream usage
    UpdateCurrSceneTypeAndStreamUsageForDsp();
}
// LCOV_EXCL_STOP

// LCOV_EXCL_START
bool AudioEffectChainManager::CheckSceneTypeMatch(const std::string &sinkSceneType, const std::string &sceneType)
{
    std::lock_guard<std::mutex> lock(dynamicMutex_);
    std::string sceneTypeAndDeviceKey = sceneType + "_&_" + GetDeviceTypeName();
    std::string sinkSceneTypeAndDeviceKey = sinkSceneType + "_&_" + GetDeviceTypeName();
    std::string defaultSceneTypeAndDeviceKey = DEFAULT_SCENE_TYPE + "_&_" + GetDeviceTypeName();
    if (!sceneTypeToEffectChainMap_.count(sceneTypeAndDeviceKey) ||
        !sceneTypeToEffectChainMap_.count(sinkSceneTypeAndDeviceKey)) {
        return false;
    }
    if (sceneType == sinkSceneType && (sceneTypeToSpecialEffectSet_.count(sinkSceneType) ||
        std::find(priorSceneList_.begin(), priorSceneList_.end(), sceneType) != priorSceneList_.end())) {
        return true;
    }
    if (sceneTypeToEffectChainMap_[sceneTypeAndDeviceKey] ==
        sceneTypeToEffectChainMap_[sinkSceneTypeAndDeviceKey]) {
        return sceneTypeAndDeviceKey == defaultSceneTypeAndDeviceKey;
    }
    return false;
}
// LCOV_EXCL_STOP

void AudioEffectChainManager::UpdateCurrSceneType(AudioEffectScene &currSceneType, const std::string &sceneType)
{
    const std::unordered_map<AudioEffectScene, std::string> &audioSupportedSceneTypes = GetSupportedSceneType();
    currSceneType = static_cast<AudioEffectScene>(GetKeyFromValue(audioSupportedSceneTypes, sceneType));
}

// LCOV_EXCL_START
void AudioEffectChainManager::FindMaxEffectChannels(const std::string &sceneType,
    const std::set<std::string> &sessions, uint32_t &channels, uint64_t &channelLayout)
{
    for (auto s = sessions.begin(); s != sessions.end(); ++s) {
        SessionEffectInfo info = sessionIDToEffectInfoMap_[*s];
        uint32_t tmpChannelCount;
        uint64_t tmpChannelLayout;
        std::string deviceType = GetDeviceTypeName();
        if (((deviceType == "DEVICE_TYPE_BLUETOOTH_A2DP") || (deviceType == "DEVICE_TYPE_SPEAKER"))
            && ExistAudioEffectChainInner(sceneType, info.sceneMode)) {
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
// LCOV_EXCL_STOP

std::shared_ptr<AudioEffectChain> AudioEffectChainManager::CreateAudioEffectChain(const std::string &sceneType,
    bool isPriorScene)
{
    std::shared_ptr<AudioEffectChain> audioEffectChain = nullptr;
    std::string defaultSceneTypeAndDeviceKey = DEFAULT_SCENE_TYPE + "_&_" + GetDeviceTypeName();
    defaultEffectChainCreated_ = false;
    if (isPriorScene) {
        HILOG_COMM_INFO("create prior effect chain: %{public}s", sceneType.c_str());
#ifdef SENSOR_ENABLE
        audioEffectChain = std::make_shared<AudioEffectChain>(sceneType, headTracker_);
#else
        audioEffectChain = std::make_shared<AudioEffectChain>(sceneType);
#endif
        return audioEffectChain;
    }
    if ((maxEffectChainCount_ - static_cast<int32_t>(sceneTypeToSpecialEffectSet_.size())) > 1) {
        HILOG_COMM_INFO("max audio effect chain count not reached, create special effect chain: %{public}s",
            sceneType.c_str());
        sceneTypeToSpecialEffectSet_.insert(sceneType);
#ifdef SENSOR_ENABLE
        audioEffectChain = std::make_shared<AudioEffectChain>(sceneType, headTracker_);
#else
        audioEffectChain = std::make_shared<AudioEffectChain>(sceneType);
#endif
    } else {
        if (!isDefaultEffectChainExisted_) {
            HILOG_COMM_INFO("max audio effect chain count reached, create current and default effect chain");
#ifdef SENSOR_ENABLE
            audioEffectChain = std::make_shared<AudioEffectChain>(DEFAULT_SCENE_TYPE, headTracker_);
#else
            audioEffectChain = std::make_shared<AudioEffectChain>(DEFAULT_SCENE_TYPE);
#endif
            defaultEffectChainCount_ = 1;
            isDefaultEffectChainExisted_ = true;
            defaultEffectChainCreated_ = true;
        } else {
            audioEffectChain = sceneTypeToEffectChainMap_[defaultSceneTypeAndDeviceKey];
            defaultEffectChainCount_++;
            HILOG_COMM_INFO("max audio effect chain count reached and default effect chain already exist: %{public}d",
                defaultEffectChainCount_);
        }
    }
    return audioEffectChain;
}

// LCOV_EXCL_START
int32_t AudioEffectChainManager::CheckAndReleaseCommonEffectChain(const std::string &sceneType)
{
    Trace trace("AudioEffectChainManager::CheckAndReleaseCommonEffectChain: " + sceneType);
    HILOG_COMM_INFO("release effect chain for scene type: %{public}s", sceneType.c_str());
    std::string sceneTypeAndDeviceKey = sceneType + "_&_" + GetDeviceTypeName();
    std::string defaultSceneTypeAndDeviceKey = DEFAULT_SCENE_TYPE + "_&_" + GetDeviceTypeName();
    sceneTypeToEffectChainCountMap_[sceneTypeAndDeviceKey] = 0;
    if (sceneTypeToSpecialEffectSet_.erase(sceneType) > 0) {
        sceneTypeToEffectChainMap_[sceneTypeAndDeviceKey]->InitEffectChain();
    }
    if (!isDefaultEffectChainExisted_) {
        return ERROR;
    }
    if (sceneTypeToEffectChainMap_.count(defaultSceneTypeAndDeviceKey) != 0 &&
        sceneTypeToEffectChainMap_[defaultSceneTypeAndDeviceKey] != nullptr &&
        sceneTypeToEffectChainMap_[defaultSceneTypeAndDeviceKey] ==
        sceneTypeToEffectChainMap_[sceneTypeAndDeviceKey]) {
        if (defaultEffectChainCount_ <= 1) {
            defaultEffectChainCount_= 0;
            isDefaultEffectChainExisted_ = false;
            sceneTypeToEffectChainMap_[defaultSceneTypeAndDeviceKey]->InitEffectChain();
            HILOG_COMM_INFO("default effect chain will be released");
            return SUCCESS;
        } else {
            defaultEffectChainCount_--;
            HILOG_COMM_INFO("default effect chain still exist, count is %{public}d", defaultEffectChainCount_);
        }
    }
    return ERROR;
}
// LCOV_EXCL_STOP

// LCOV_EXCL_START
void AudioEffectChainManager::UpdateSpatializationEnabled(AudioSpatializationState spatializationState)
{
    spatializationEnabled_ = spatializationState.spatializationEnabled;

    memset_s(static_cast<void *>(effectHdiInput_), sizeof(effectHdiInput_), 0, sizeof(effectHdiInput_));
    if (spatializationEnabled_) {
        if (((deviceType_ == DEVICE_TYPE_BLUETOOTH_A2DP) || (deviceType_ == DEVICE_TYPE_BLUETOOTH_SCO)) &&
            (!btOffloadSupported_)) {
            AUDIO_INFO_LOG("A2dp-hal, enter ARM processing");
            btOffloadEnabled_ = false;
            SetSpatializationEnabledToChains();
            return;
        }
        effectHdiInput_[0] = HDI_INIT;
        CHECK_AND_RETURN_LOG(audioEffectHdiParam_ != nullptr, "audioEffectHdiParam_ is nullptr");
        int32_t ret = audioEffectHdiParam_->UpdateHdiState(effectHdiInput_, DEVICE_TYPE_BLUETOOTH_A2DP);
        if (ret != SUCCESS) {
            AUDIO_WARNING_LOG("set hdi init failed, enter route of escape in ARM");
            btOffloadEnabled_ = false;
        } else {
            AUDIO_INFO_LOG("set hdi init succeeded, normal spatialization entered");
            btOffloadEnabled_ = true;
        }
    } else {
        effectHdiInput_[0] = HDI_DESTROY;
        AUDIO_INFO_LOG("set hdi destroy.");
        CHECK_AND_RETURN_LOG(audioEffectHdiParam_ != nullptr, "audioEffectHdiParam_ is nullptr");
        int32_t ret = audioEffectHdiParam_->UpdateHdiState(effectHdiInput_, DEVICE_TYPE_BLUETOOTH_A2DP);
        if (ret != SUCCESS) {
            AUDIO_WARNING_LOG("set hdi destroy failed");
        }
        btOffloadEnabled_ = false;
    }
    SetSpatializationEnabledToChains();
}
// LCOV_EXCL_STOP

// for AISS temporarily
bool AudioEffectChainManager::CheckIfSpkDsp()
{
    return deviceType_ == DEVICE_TYPE_SPEAKER;
}

void AudioEffectChainManager::UpdateEffectBtOffloadSupported(const bool &isSupported)
{
    Trace trace("AudioEffectChainManager::UpdateEffectBtOffloadSupported: " + std::to_string(isSupported));
    std::lock_guard<std::mutex> lock(dynamicMutex_);
    if (isSupported == btOffloadSupported_) {
        return;
    }

    if (!spatializationEnabled_) {
        btOffloadSupported_ = isSupported;
        AUDIO_INFO_LOG("btOffloadSupported_ %{public}d, but spatialization is off, do nothing", btOffloadSupported_);
        return;
    }
    // Release ARM, try offload to DSP
    AUDIO_INFO_LOG("btOffloadSupported_ %{public}d, try offload effect on device %{public}d",
        btOffloadSupported_, deviceType_);
    AudioSpatializationState oldState = {spatializationEnabled_, headTrackingEnabled_};
    AudioSpatializationState offState = {false, false};
    UpdateSpatializationStateInner(offState);
    btOffloadSupported_ = isSupported;
    UpdateSpatializationStateInner(oldState);
    return;
}

int32_t AudioEffectChainManager::SetAudioEffectProperty(const AudioEffectPropertyArrayV3 &propertyArray)
{
    int32_t ret = AUDIO_OK;
    std::lock_guard<std::mutex> lock(dynamicMutex_);
    for (const auto &property : propertyArray.property) {
        effectPropertyMap_.insert_or_assign(property.name, property.category);
        for (const auto &[sceneType, effectChain] : sceneTypeToEffectChainMap_) {
            if (effectChain) {
                AUDIO_DEBUG_LOG("effectClass->name %{public}s effectProp->category %{public}s",
                    property.name.c_str(), property.category.c_str());
                ret = effectChain->SetEffectProperty(property.name, property.category);
                CHECK_AND_CONTINUE_LOG(ret == AUDIO_OK, "set property failed[%{public}d]", ret);
            }
        }
    }
    return ret;
}

// LCOV_EXCL_START
void AudioEffectChainManager::LoadEffectProperties()
{
    hasLoadedEffectProperties_ = false;
    AudioSettingProvider &settingProvider = AudioSettingProvider::GetInstance(AUDIO_POLICY_SERVICE_ID);
    CHECK_AND_RETURN_LOG(settingProvider.CheckOsAccountReady(), "os account not ready");
    for (const auto &[effect, key] : AUDIO_PERSISTENCE_EFFECT_KEY) {
        std::string prop = "";
        ErrCode ret = settingProvider.GetStringValue(key, prop, "system");
        if (!prop.empty() && ret == SUCCESS) {
            AUDIO_INFO_LOG("effect->name %{public}s prop %{public}s", effect.c_str(), prop.c_str());
            effectPropertyMap_[effect] = prop;
        } else {
            AUDIO_ERR_LOG("get prop failed for key %{public}s", key.c_str());
            if (defaultPropertyMap_.count(effect) != 0) {
                AUDIO_INFO_LOG("effect->name %{public}s defaultProp %{public}s", effect.c_str(), prop.c_str());
                effectPropertyMap_[effect] = defaultPropertyMap_[effect];
            }
        }
    }
    hasLoadedEffectProperties_ = true;
}
// LCOV_EXCL_STOP

// LCOV_EXCL_START
int32_t AudioEffectChainManager::SetAudioEffectProperty(const AudioEffectPropertyArray &propertyArray)
{
    std::lock_guard<std::mutex> lock(dynamicMutex_);
    for (const auto &property : propertyArray.property) {
        effectPropertyMap_.insert_or_assign(property.effectClass, property.effectProp);
        for (const auto &[sceneType, effectChain] : sceneTypeToEffectChainMap_) {
            if (effectChain) {
                effectChain->SetEffectProperty(property.effectClass, property.effectProp);
            }
        }
    }
    return AUDIO_OK;
}
// LCOV_EXCL_STOP

int32_t AudioEffectChainManager::GetAudioEffectProperty(AudioEffectPropertyArrayV3 &propertyArray)
{
    std::lock_guard<std::mutex> lock(dynamicMutex_);
    propertyArray.property.clear();
    for (const auto &[effect, prop] : effectPropertyMap_) {
        if (!prop.empty()) {
            AUDIO_DEBUG_LOG("effect->name %{public}s prop->category %{public}s",
                effect.c_str(), prop.c_str());
            propertyArray.property.emplace_back(AudioEffectPropertyV3{effect, prop, RENDER_EFFECT_FLAG});
        }
    }
    return AUDIO_OK;
}

int32_t AudioEffectChainManager::GetAudioEffectProperty(AudioEffectPropertyArray &propertyArray)
{
    std::lock_guard<std::mutex> lock(dynamicMutex_);
    propertyArray.property.clear();
    for (const auto &[effect, prop] : effectPropertyMap_) {
        if (!prop.empty()) {
            propertyArray.property.emplace_back(AudioEffectProperty{effect, prop});
        }
    }
    return AUDIO_OK;
}

int32_t AudioEffectChainManager::UpdateSceneTypeList(const std::string &sceneType, SceneTypeOperation operation)
{
    Trace trace("AudioEffectChainManager::UpdateSceneTypeList: " + sceneType + " operation: " +
        std::to_string(static_cast<int32_t>(operation)));
    std::lock_guard<std::mutex> lock(dynamicMutex_);
    CHECK_AND_RETURN_RET_LOG(sceneType != "", ERROR, "null sceneType");
    if (operation == ADD_SCENE_TYPE) {
        auto it = std::find_if(sceneTypeCountList_.begin(), sceneTypeCountList_.end(),
            [sceneType](const std::pair<std::string, int32_t> &element) {
                return element.first == sceneType;
            });
        if (it == sceneTypeCountList_.end()) {
            sceneTypeCountList_.push_back(std::make_pair(sceneType, 1));
            AUDIO_INFO_LOG("scene Type %{public}s is added", sceneType.c_str());
        } else {
            it->second++;
            AUDIO_INFO_LOG("scene Type %{public}s count is increased to %{public}d", sceneType.c_str(), it->second);
        }
    } else if (operation == REMOVE_SCENE_TYPE) {
        auto it = std::find_if(sceneTypeCountList_.begin(), sceneTypeCountList_.end(),
            [sceneType](const std::pair<std::string, int32_t> &element) {
                return element.first == sceneType;
            });
        if (it == sceneTypeCountList_.end()) {
            AUDIO_WARNING_LOG("scene type %{public}s to be removed is not found", sceneType.c_str());
            return ERROR;
        }
        if (it->second <= 1) {
            sceneTypeCountList_.erase(it);
            AUDIO_INFO_LOG("scene Type %{public}s is removed", sceneType.c_str());
        } else {
            it->second--;
            AUDIO_INFO_LOG("scene Type %{public}s count is decreased to %{public}d", sceneType.c_str(), it->second);
        }
    } else {
        AUDIO_ERR_LOG("Wrong operation to sceneTypeToEffectChainCountBackupMap.");
        return ERROR;
    }
    return SUCCESS;
}

// LCOV_EXCL_START
uint32_t AudioEffectChainManager::GetSceneTypeToChainCount(const std::string &sceneType)
{
    std::lock_guard<std::mutex> lock(dynamicMutex_);

    if (sceneType == DEFAULT_SCENE_TYPE) {
        return defaultEffectChainCount_;
    }
    std::string sceneTypeAndDeviceKey = sceneType + "_&_" + GetDeviceTypeName();
    std::string defaultSceneTypeAndDeviceKey = DEFAULT_SCENE_TYPE + "_&_" + GetDeviceTypeName();

    if (sceneTypeToEffectChainMap_.count(sceneTypeAndDeviceKey)) {
        if (sceneTypeToEffectChainMap_.count(defaultSceneTypeAndDeviceKey) &&
            (sceneTypeToEffectChainMap_[sceneTypeAndDeviceKey] ==
            sceneTypeToEffectChainMap_[defaultSceneTypeAndDeviceKey])) {
            return 0;
        } else {
            return sceneTypeToEffectChainCountMap_[sceneTypeAndDeviceKey];
        }
    }
    return 0;
}
// LCOV_EXCL_STOP

void AudioEffectChainManager::FindMaxSessionID(uint32_t &maxSessionID, std::string &sceneType,
    const std::string &scenePairType, std::set<std::string> &sessions)
{
    std::shared_ptr<AudioEffectVolume> audioEffectVolume = AudioEffectVolume::GetInstance();
    CHECK_AND_RETURN_LOG(audioEffectVolume != nullptr, "null audioEffectVolume");
    for (auto &sessionID : sessions) {
        if (sessionIDToEffectInfoMap_[sessionID].sceneMode == "EFFECT_NONE" ||
            fabs(audioEffectVolume->GetStreamVolume(sessionID)) < EPSILON) {
            continue;
        }
        uint32_t sessionIDInt = static_cast<uint32_t>(std::stoul(sessionID));
        if (sessionIDInt > maxSessionID) {
            maxSessionID = sessionIDInt;
            sceneType = scenePairType;
        }
    }
}

int32_t AudioEffectChainManager::UpdateCurrSceneTypeAndStreamUsageForDsp()
{
    AudioEffectScene currSceneType;
    std::string maxSession = std::to_string(maxSessionID_);
    UpdateCurrSceneType(currSceneType, maxSessionIDToSceneType_);
    SetHdiParam(currSceneType);
    if (sessionIDToEffectInfoMap_.count(maxSession) &&
        sessionIDToEffectInfoMap_[maxSession].streamUsage != currDspStreamUsage_) {
        SessionEffectInfo info = sessionIDToEffectInfoMap_[maxSession];
        effectHdiInput_[0] = HDI_STREAM_USAGE;
        effectHdiInput_[1] = info.streamUsage;
        CHECK_AND_RETURN_RET_LOG(audioEffectHdiParam_ != nullptr, ERROR, "audioEffectHdiParam_ is nullptr");
        int32_t ret = audioEffectHdiParam_->UpdateHdiState(effectHdiInput_);
        AUDIO_INFO_LOG("set hdi streamUsage: %{public}d", info.streamUsage);
        if (ret != SUCCESS) {
            AUDIO_WARNING_LOG("set hdi streamUsage failed");
            return ERROR;
        }
        currDspStreamUsage_ = info.streamUsage;
    }
    return SUCCESS;
}

int32_t AudioEffectChainManager::NotifyAndCreateAudioEffectChain(const std::string &sceneType)
{
    Trace trace("AudioEffectChainManager::NotifyAndCreateAudioEffectChain: " + sceneType);
    std::shared_ptr<AudioEffectChain> audioEffectChain = nullptr;
    std::string sceneTypeAndDeviceKey = sceneType + "_&_" + GetDeviceTypeName();
    std::string defaultSceneTypeAndDeviceKey = DEFAULT_SCENE_TYPE + "_&_" + GetDeviceTypeName();
    const std::unordered_map<AudioEffectMode, std::string> &audioSupportedSceneModes = GetAudioSupportedSceneModes();

    bool isPriorScene = std::find(priorSceneList_.begin(), priorSceneList_.end(), sceneType) != priorSceneList_.end();
    audioEffectChain = CreateAudioEffectChain(sceneType, isPriorScene);

    if (sceneTypeToEffectChainMap_.count(sceneTypeAndDeviceKey) &&
        sceneTypeToEffectChainMap_[sceneTypeAndDeviceKey] != nullptr &&
        sceneTypeToEffectChainCountMap_.count(sceneTypeAndDeviceKey) &&
        sceneTypeToEffectChainCountMap_[sceneTypeAndDeviceKey] == 0) {
        if (defaultEffectChainCreated_ == true) {
            sceneTypeToEffectChainMap_[defaultSceneTypeAndDeviceKey] =
                sceneTypeToEffectChainMap_[sceneTypeAndDeviceKey];
        } else if (isDefaultEffectChainExisted_ == true) {
            sceneTypeToEffectChainMap_[sceneTypeAndDeviceKey] = audioEffectChain;
        }
        sceneTypeToEffectChainCountMap_[sceneTypeAndDeviceKey] = 1;
        AUDIO_INFO_LOG("Reusing existing sceneTypeAndDeviceKey [%{public}s]", sceneTypeAndDeviceKey.c_str());
        cv_.notify_all();
        return SUCCESS;
    }
    if (defaultEffectChainCreated_ == true) {
        sceneTypeToEffectChainMap_[defaultSceneTypeAndDeviceKey] = audioEffectChain;
    }
    sceneTypeToEffectChainMap_[sceneTypeAndDeviceKey] = audioEffectChain;
    sceneTypeToEffectChainCountMap_[sceneTypeAndDeviceKey] = 1;
    AUDIO_INFO_LOG("Create new sceneTypeAndDeviceKey [%{public}s]", sceneTypeAndDeviceKey.c_str());
    if (!audioSupportedSceneModes.count(EFFECT_DEFAULT)) {
        return ERROR;
    }
    std::string effectMode = audioSupportedSceneModes.find(EFFECT_DEFAULT)->second;
    if (!isPriorScene && !sceneTypeToSpecialEffectSet_.count(sceneType) && defaultEffectChainCount_ > 1) {
        return SUCCESS;
    }
    std::string createSceneType = (isPriorScene || sceneTypeToSpecialEffectSet_.count(sceneType) > 0) ?
        sceneType : DEFAULT_SCENE_TYPE;
    if (SetAudioEffectChainDynamic(createSceneType, effectMode) != SUCCESS) {
        return ERROR;
    }
    return SUCCESS;
}

int32_t AudioEffectChainManager::CreateAudioEffectChainDynamicInner(const std::string &sceneType)
{
    Trace trace("AudioEffectChainManager::CreateAudioEffectChainDynamicInner: " + sceneType);
    CHECK_AND_RETURN_RET_LOG(isInitialized_, ERROR, "has not been initialized");
    CHECK_AND_RETURN_RET_LOG(sceneType != "", ERROR, "null sceneType");

    std::string sceneTypeAndDeviceKey = sceneType + "_&_" + GetDeviceTypeName();
    std::string defaultSceneTypeAndDeviceKey = DEFAULT_SCENE_TYPE + "_&_" + GetDeviceTypeName();
    if (sceneTypeToEffectChainMap_.count(sceneTypeAndDeviceKey) &&
        sceneTypeToEffectChainCountMap_.count(sceneTypeAndDeviceKey) &&
        sceneTypeToEffectChainCountMap_[sceneTypeAndDeviceKey] > 0) {
        if (sceneTypeToEffectChainMap_[sceneTypeAndDeviceKey] == nullptr) {
            sceneTypeToEffectChainMap_.erase(sceneTypeAndDeviceKey);
            sceneTypeToEffectChainCountMap_.erase(sceneTypeAndDeviceKey);
            AUDIO_WARNING_LOG("scene type %{public}s has null effect chain", sceneTypeAndDeviceKey.c_str());
        } else {
            sceneTypeToEffectChainCountMap_[sceneTypeAndDeviceKey]++;
            if (isDefaultEffectChainExisted_ && sceneTypeToEffectChainMap_.count(defaultSceneTypeAndDeviceKey) != 0 &&
                sceneTypeToEffectChainMap_[sceneTypeAndDeviceKey] ==
                sceneTypeToEffectChainMap_[defaultSceneTypeAndDeviceKey]) {
                defaultEffectChainCount_++;
            }
            HILOG_COMM_INFO("effect chain %{public}s still exist, current count: %{public}d, default count: %{public}d",
                sceneType.c_str(), sceneTypeToEffectChainCountMap_[sceneTypeAndDeviceKey], defaultEffectChainCount_);
            return SUCCESS;
        }
    }
    return NotifyAndCreateAudioEffectChain(sceneType);
}

void AudioEffectChainManager::WaitAndReleaseEffectChain(const std::string &sceneType,
    const std::string &sceneTypeAndDeviceKey, const std::string &defaultSceneTypeAndDeviceKey, int32_t ret)
{
    Trace trace("AudioEffectChainManager::WaitAndReleaseEffectChain: " + sceneType);
    std::unique_lock<std::mutex> lock(dynamicMutex_);

    auto condition = [this, sceneType, sceneTypeAndDeviceKey, defaultSceneTypeAndDeviceKey]() {
        return sceneTypeToEffectChainCountMap_.count(sceneTypeAndDeviceKey) &&
            sceneTypeToEffectChainCountMap_[sceneTypeAndDeviceKey] > 0;
    };
    if (cv_.wait_for(lock, std::chrono::milliseconds(RELEASE_WAIT_TIME_MS), condition)) {
        AUDIO_INFO_LOG("New sceneTypeAndDeviceKey [%{public}s] is being created, cancelling previous release",
            sceneTypeAndDeviceKey.c_str());
        return;
    }

    if (sceneTypeToEffectChainCountMap_.count(sceneTypeAndDeviceKey) &&
        sceneTypeToEffectChainCountMap_[sceneTypeAndDeviceKey] == 0) {
        sceneTypeToEffectChainCountMap_.erase(sceneTypeAndDeviceKey);
        if (ret == SUCCESS && defaultEffectChainCount_ == 0 &&
            sceneTypeToEffectChainMap_[defaultSceneTypeAndDeviceKey] ==
            sceneTypeToEffectChainMap_[sceneTypeAndDeviceKey]) {
            sceneTypeToEffectChainMap_.erase(defaultSceneTypeAndDeviceKey);
            AUDIO_INFO_LOG("default effect chain is released");
        }
        sceneTypeToEffectChainMap_.erase(sceneTypeAndDeviceKey);
        AUDIO_INFO_LOG("sceneTypeAndDeviceKey [%{public}s] is being released", sceneTypeAndDeviceKey.c_str());
    }
}

int32_t AudioEffectChainManager::ReleaseAudioEffectChainDynamicInner(const std::string &sceneType)
{
    Trace trace("AudioEffectChainManager::ReleaseAudioEffectChainDynamicInner: " + sceneType);
    CHECK_AND_RETURN_RET_LOG(isInitialized_, ERROR, "has not been initialized");
    CHECK_AND_RETURN_RET_LOG(sceneType != "", ERROR, "null sceneType");

    std::string sceneTypeAndDeviceKey = sceneType + "_&_" + GetDeviceTypeName();
    std::string defaultSceneTypeAndDeviceKey = DEFAULT_SCENE_TYPE + "_&_" + GetDeviceTypeName();
    if (!sceneTypeToEffectChainMap_.count(sceneTypeAndDeviceKey) ||
        (sceneTypeToEffectChainMap_[sceneTypeAndDeviceKey] == nullptr) ||
        (sceneTypeToEffectChainCountMap_.count(sceneTypeAndDeviceKey) &&
        sceneTypeToEffectChainCountMap_[sceneTypeAndDeviceKey] == 0)) {
        sceneTypeToEffectChainCountMap_.erase(sceneTypeAndDeviceKey);
        AUDIO_WARNING_LOG("scene type %{public}s has null effect chain", sceneTypeAndDeviceKey.c_str());
        return SUCCESS;
    } else if (sceneTypeToEffectChainCountMap_.count(sceneTypeAndDeviceKey) &&
        sceneTypeToEffectChainCountMap_[sceneTypeAndDeviceKey] > 1) {
        sceneTypeToEffectChainCountMap_[sceneTypeAndDeviceKey]--;
        if (sceneTypeToEffectChainMap_.count(defaultSceneTypeAndDeviceKey) != 0 &&
            sceneTypeToEffectChainMap_[sceneTypeAndDeviceKey] ==
            sceneTypeToEffectChainMap_[defaultSceneTypeAndDeviceKey]) {
            defaultEffectChainCount_--;
        }
        AUDIO_INFO_LOG("effect chain %{public}s still exist, current count: %{public}d, default count: %{public}d",
            sceneType.c_str(), sceneTypeToEffectChainCountMap_[sceneTypeAndDeviceKey], defaultEffectChainCount_);
        return SUCCESS;
    }
    int32_t ret = CheckAndReleaseCommonEffectChain(sceneType);
    std::thread([this, sceneType, sceneTypeAndDeviceKey, defaultSceneTypeAndDeviceKey, ret]() {
        WaitAndReleaseEffectChain(sceneType, sceneTypeAndDeviceKey, defaultSceneTypeAndDeviceKey, ret);
    }).detach();
    return SUCCESS;
}

bool AudioEffectChainManager::ExistAudioEffectChainInner(const std::string &sceneType, const std::string &effectMode)
{
    if (!isInitialized_) {
        if (initializedLogFlag_) {
            AUDIO_ERR_LOG("audioEffectChainManager has not been initialized");
            initializedLogFlag_ = false;
        }
        return false;
    }
    initializedLogFlag_ = true;
    CHECK_AND_RETURN_RET(sceneType != "", false);
    CHECK_AND_RETURN_RET(GetDeviceTypeName() != "", false);

    if ((deviceType_ == DEVICE_TYPE_SPEAKER) && (spkOffloadEnabled_)) {
        return false;
    }

    if ((deviceType_ == DEVICE_TYPE_BLUETOOTH_A2DP) && (btOffloadEnabled_)) {
        return false;
    }

    std::string effectChainKey = sceneType + "_&_" + effectMode + "_&_" + GetDeviceTypeName();
    if (!sceneTypeAndModeToEffectChainNameMap_.count(effectChainKey)) {
        return false;
    }
    std::string sceneTypeAndDeviceKey = sceneType + "_&_" + GetDeviceTypeName();
    // if the effectChain exist, see if it is empty
    if (!sceneTypeToEffectChainMap_.count(sceneTypeAndDeviceKey) ||
        sceneTypeToEffectChainMap_[sceneTypeAndDeviceKey] == nullptr) {
        return false;
    }
    if (sceneTypeToEffectChainCountMap_.count(sceneTypeAndDeviceKey) &&
        sceneTypeToEffectChainCountMap_[sceneTypeAndDeviceKey] == 0) {
        return false;
    }
    auto audioEffectChain = sceneTypeToEffectChainMap_[sceneTypeAndDeviceKey];
    return !audioEffectChain->IsEmptyEffectHandles();
}

int32_t AudioEffectChainManager::UpdateMultichannelConfigInner(const std::string &sceneType)
{
    std::string sceneTypeAndDeviceKey = sceneType + "_&_" + GetDeviceTypeName();
    if (!sceneTypeToEffectChainMap_.count(sceneTypeAndDeviceKey)) {
        return ERROR;
    }
    uint32_t inputChannels = DEFAULT_NUM_CHANNEL;
    uint64_t inputChannelLayout = DEFAULT_NUM_CHANNELLAYOUT;
    ReturnEffectChannelInfoInner(sceneType, inputChannels, inputChannelLayout);

    auto audioEffectChain = sceneTypeToEffectChainMap_[sceneTypeAndDeviceKey];
    if (audioEffectChain == nullptr) {
        return ERROR;
    }
    audioEffectChain->UpdateMultichannelIoBufferConfig(inputChannels, inputChannelLayout);
    return SUCCESS;
}

int32_t AudioEffectChainManager::UpdateSpatializationStateInner(AudioSpatializationState spatializationState)
{
    Trace trace("AudioEffectChainManager::UpdateSpatializationStateInner previous state: " +
        std::to_string(spatializationEnabled_.load()) + std::to_string(headTrackingEnabled_) +
        " current state: " + std::to_string(spatializationState.spatializationEnabled) +
        std::to_string(spatializationState.headTrackingEnabled));
    HILOG_COMM_INFO("begin to update spatialization state, current state: %{public}d and %{public}d, previous state: \
        %{public}d and %{public}d", spatializationState.spatializationEnabled, spatializationState.headTrackingEnabled,
        spatializationEnabled_.load(), headTrackingEnabled_);

    if (spatializationEnabled_ != spatializationState.spatializationEnabled) {
        UpdateSpatializationEnabled(spatializationState);
    }
    if (headTrackingEnabled_ != spatializationState.headTrackingEnabled) {
        headTrackingEnabled_ = spatializationState.headTrackingEnabled;
        UpdateSensorState();
    }

    std::shared_ptr<AudioEffectVolume> audioEffectVolume = AudioEffectVolume::GetInstance();
    CHECK_AND_RETURN_RET_LOG(audioEffectVolume != nullptr, ERROR, "null audioEffectVolume");
    EffectVolumeUpdateInner(audioEffectVolume);
    return SUCCESS;
}

int32_t AudioEffectChainManager::ReturnEffectChannelInfoInner(const std::string &sceneType, uint32_t &channels,
    uint64_t &channelLayout)
{
    std::string sceneTypeAndDeviceKey = sceneType + "_&_" + GetDeviceTypeName();
    if (!sceneTypeToEffectChainMap_.count(sceneTypeAndDeviceKey) ||
        sceneTypeToEffectChainMap_[sceneTypeAndDeviceKey] == nullptr) {
        return ERROR;
    }
    for (auto& scenePair : sceneTypeToSessionIDMap_) {
        std::string pairSceneTypeAndDeviceKey = scenePair.first + "_&_" + GetDeviceTypeName();
        if (sceneTypeToEffectChainMap_.count(pairSceneTypeAndDeviceKey) > 0 &&
            sceneTypeToEffectChainMap_[pairSceneTypeAndDeviceKey] != nullptr &&
            sceneTypeToEffectChainMap_[sceneTypeAndDeviceKey] ==
            sceneTypeToEffectChainMap_[pairSceneTypeAndDeviceKey]) {
            FindMaxEffectChannels(scenePair.first, scenePair.second, channels, channelLayout);
        }
    }
    auto audioEffectChain = sceneTypeToEffectChainMap_[sceneTypeAndDeviceKey];
    audioEffectChain->SetCurrChannelNoCheck(channels);
    audioEffectChain->SetCurrChannelLayoutNoCheck(channelLayout);
    return SUCCESS;
}

int32_t AudioEffectChainManager::EffectVolumeUpdateInner(std::shared_ptr<AudioEffectVolume> audioEffectVolume)
{
    int32_t ret;
    if (((deviceType_ == DEVICE_TYPE_SPEAKER) && (spkOffloadEnabled_)) ||
        ((deviceType_ == DEVICE_TYPE_BLUETOOTH_A2DP) && (btOffloadEnabled_))) {
        ret = EffectDspVolumeUpdate(audioEffectVolume);
    } else {
        ret = EffectApVolumeUpdate(audioEffectVolume);
    }
    return ret;
}

int32_t AudioEffectChainManager::InitEffectBuffer(const std::string &sessionID)
{
    Trace trace("AudioEffectChainManager::InitEffectBuffer: " + sessionID);
    std::lock_guard<std::mutex> lock(dynamicMutex_);
    return InitEffectBufferInner(sessionID);
}

int32_t AudioEffectChainManager::InitEffectBufferInner(const std::string &sessionID)
{
    if (sessionIDToEffectInfoMap_.find(sessionID) == sessionIDToEffectInfoMap_.end() ||
        sessionIDToEffectInfoMap_[sessionID].sceneMode == "EFFECT_NONE") {
        AUDIO_INFO_LOG("sessionIDToEffectInfoMap not find sessionID or sceneMode is EFFECT_NONE: %{public}s",
            sessionID.c_str());
        return SUCCESS;
    }
    std::string sceneTypeTemp = sessionIDToEffectInfoMap_[sessionID].sceneType;
    if (IsEffectChainStop(sceneTypeTemp, sessionID)) {
        AUDIO_INFO_LOG("sessionID: %{public}s sceneType: %{public}s: make init effect buffer",
            sessionID.c_str(), sceneTypeTemp.c_str());
        return InitAudioEffectChainDynamicInner(sceneTypeTemp);
    }
    AUDIO_INFO_LOG("sessionID: %{public}s, don't need init effect buffer", sessionID.c_str());
    return SUCCESS;
}

bool AudioEffectChainManager::IsEffectChainStop(const std::string &sceneType, const std::string &sessionID)
{
    std::string sceneTypeAndDeviceKey = sceneType + "_&_" + GetDeviceTypeName();
    CHECK_AND_RETURN_RET_LOG(sceneTypeToEffectChainMap_.count(sceneTypeAndDeviceKey) > 0 &&
        sceneTypeToEffectChainMap_[sceneTypeAndDeviceKey] != nullptr, false, "null audioEffectChain");
    auto audioEffectChain = sceneTypeToEffectChainMap_[sceneTypeAndDeviceKey];
    for (auto it = sessionIDToEffectInfoMap_.begin(); it != sessionIDToEffectInfoMap_.end(); ++it) {
        if (it->first == sessionID || it->second.sceneMode == "EFFECT_NONE") {
            continue;
        }
        std::string sceneTypeTemp = it->second.sceneType;
        std::string sceneTypeAndDeviceKeyTemp = sceneTypeTemp + "_&_" + GetDeviceTypeName();
        CHECK_AND_RETURN_RET_LOG(sceneTypeToEffectChainMap_.count(sceneTypeAndDeviceKeyTemp) > 0 &&
            sceneTypeToEffectChainMap_[sceneTypeAndDeviceKeyTemp] != nullptr, false, "null audioEffectChain");
        auto audioEffectChainTemp = sceneTypeToEffectChainMap_[sceneTypeAndDeviceKeyTemp];
        if (audioEffectChainTemp == audioEffectChain) {
            return false;
        }
    }
    return true;
}

ProcessClusterOperation AudioEffectChainManager::CheckProcessClusterInstances(const std::string &sceneType)
{
    Trace trace("AudioEffectChainManager::CheckProcessClusterInstances: " + sceneType);
    std::lock_guard<std::mutex> lock(dynamicMutex_);
    CHECK_AND_RETURN_RET(sceneType != "SCENE_EXTRA", CREATE_EXTRA_PROCESSCLUSTER);
    CHECK_AND_RETURN_RET(!GetOffloadEnabled(), USE_NONE_PROCESSCLUSTER);
    std::string sceneTypeAndDeviceKey = sceneType + "_&_" + GetDeviceTypeName();
    std::string defaultSceneTypeAndDeviceKey = DEFAULT_SCENE_TYPE + "_&_" + GetDeviceTypeName();

    if (sceneTypeToEffectChainMap_.count(sceneTypeAndDeviceKey) &&
        sceneTypeToEffectChainCountMap_.count(sceneTypeAndDeviceKey) &&
        sceneTypeToEffectChainCountMap_[sceneTypeAndDeviceKey] > 0) {
        if (sceneTypeToEffectChainMap_[sceneTypeAndDeviceKey] == nullptr) {
            AUDIO_WARNING_LOG("scene type %{public}s has null process cluster", sceneTypeAndDeviceKey.c_str());
        } else if (isDefaultEffectChainExisted_ && sceneTypeToEffectChainMap_[sceneTypeAndDeviceKey] ==
            sceneTypeToEffectChainMap_[defaultSceneTypeAndDeviceKey]) {
            return USE_DEFAULT_PROCESSCLUSTER;
        } else {
            return NO_NEED_TO_CREATE_PROCESSCLUSTER;
        }
    }

    bool isPriorScene = std::find(priorSceneList_.begin(), priorSceneList_.end(), sceneType) != priorSceneList_.end();
    if (isPriorScene) {
        return CREATE_NEW_PROCESSCLUSTER;
    }
    if ((maxEffectChainCount_ - static_cast<int32_t>(sceneTypeToSpecialEffectSet_.size())) > 1) {
        return CREATE_NEW_PROCESSCLUSTER;
    } else if (!isDefaultEffectChainExisted_) {
        return CREATE_DEFAULT_PROCESSCLUSTER;
    } else {
        return USE_DEFAULT_PROCESSCLUSTER;
    }
}

int32_t AudioEffectChainManager::QueryEffectChannelInfo(const std::string &sceneType, uint32_t &channels,
    uint64_t &channelLayout)
{
    Trace trace("AudioEffectChainManager::QueryEffectChannelInfo: " + sceneType);
    std::lock_guard<std::mutex> lock(dynamicMutex_);
    return QueryEffectChannelInfoInner(sceneType, channels, channelLayout);
}

int32_t AudioEffectChainManager::QueryEffectChannelInfoInner(const std::string &sceneType, uint32_t &channels,
    uint64_t &channelLayout)
{
    channels = DEFAULT_NUM_CHANNEL;
    channelLayout = DEFAULT_NUM_CHANNELLAYOUT;
    std::string sceneTypeAndDeviceKey = sceneType + "_&_" + GetDeviceTypeName();
    if (sceneTypeToEffectChainMap_.count(sceneTypeAndDeviceKey) == 0 ||
        sceneTypeToEffectChainMap_[sceneTypeAndDeviceKey] == nullptr) {
        return ERROR;
    }
    auto audioEffectChain = sceneTypeToEffectChainMap_[sceneTypeAndDeviceKey];
    audioEffectChain->GetInputChannelInfo(channels, channelLayout);
    return SUCCESS;
}

int32_t AudioEffectChainManager::SetAbsVolumeStateToEffect(const bool absVolumeState)
{
    std::lock_guard<std::mutex> lock(dynamicMutex_);
    return SetAbsVolumeStateToEffectInner(absVolumeState);
}

int32_t AudioEffectChainManager::SetAbsVolumeStateToEffectInner(const bool absVolumeState)
{
    if (absVolumeState_ != absVolumeState) {
        EffectDspAbsVolumeStateUpdate(absVolumeState);
        EffectApAbsVolumeStateUpdate(absVolumeState);
        absVolumeState_ = absVolumeState;
        AUDIO_INFO_LOG("absVolumeStateUpdate absVolumeState: %{public}d", absVolumeState_);
    } else {
        AUDIO_INFO_LOG("absVolumeState is not changed");
    }

    return SUCCESS;
}

int32_t AudioEffectChainManager::EffectDspAbsVolumeStateUpdate(const bool absVolumeState)
{
    //send absVolumeState to dsp, but no use now
    
    effectHdiInput_[0] = HDI_ABS_VOLUME_STATE;
    effectHdiInput_[1] = static_cast<int8_t>(absVolumeState);
    CHECK_AND_RETURN_RET_LOG(audioEffectHdiParam_ != nullptr, ERROR, "audioEffectHdiParam_ is nullptr");
    int32_t ret = audioEffectHdiParam_->UpdateHdiState(effectHdiInput_);
    AUDIO_INFO_LOG("absVolumeState change, new state: %{public}d, previous state: %{public}d, ret: %{public}d",
        effectHdiInput_[1], absVolumeState_, ret);
    return SUCCESS;
}

int32_t AudioEffectChainManager::EffectApAbsVolumeStateUpdate(const bool absVolumeState)
{
    //send absVolumeState to ap

    for (auto it = sceneTypeToEffectChainMap_.begin(); it != sceneTypeToEffectChainMap_.end(); it++) {
        auto audioEffectChain = it->second;
        if (audioEffectChain == nullptr) {
            continue;
        }

        audioEffectChain->SetAbsVolumeStateToEffectChain(absVolumeState);
        int32_t ret = audioEffectChain->UpdateEffectParam();
        CHECK_AND_CONTINUE_LOG(ret == 0, "set ap absVolumeState failed");
        AUDIO_INFO_LOG("The delay of SceneType %{public}s is %{public}u, new state: %{public}d, "
            "previous state: %{public}d",
            it->first.c_str(), audioEffectChain->GetLatency(), absVolumeState, absVolumeState_);
    }

    return SUCCESS;
}

bool AudioEffectChainManager::IsDeviceTypeSupportingSpatialization()
{
    return (deviceType_ == DEVICE_TYPE_BLUETOOTH_SCO) || (deviceType_ == DEVICE_TYPE_BLUETOOTH_A2DP) ||
        (deviceType_ == DEVICE_TYPE_NEARLINK);
}

bool AudioEffectChainManager::ExistAudioEffectChainArm(const std::string sceneType, const AudioEffectMode effectMode)
{
    Trace trace("AudioEffectChainManager::ExistAudioEffectChainArm: " + sceneType);
    std::lock_guard<std::mutex> lock(dynamicMutex_);
    if (effectMode == EFFECT_NONE) {
        return false;
    }
    const std::unordered_map<AudioEffectMode, std::string> &audioSupportedSceneModes = GetAudioSupportedSceneModes();
    CHECK_AND_RETURN_RET_LOG(audioSupportedSceneModes.count(effectMode), false, "invalid effectMode");
    std::string sceneMode = audioSupportedSceneModes.find(effectMode)->second;
    std::string effectChainKey = sceneType + "_&_" + sceneMode + "_&_" + GetDeviceTypeName();
    if (!sceneTypeAndModeToEffectChainNameMap_.count(effectChainKey)) {
        AUDIO_INFO_LOG("EffectChain key [%{public}s] does not exist in arm", effectChainKey.c_str());
        return false;
    }
    return true;
}

bool AudioEffectChainManager::IsChannelLayoutSupportedForDspEffect(AudioChannelLayout channelLayout)
{
    std::lock_guard<std::mutex> lock(dynamicMutex_);
    effectHdiInput_[0] = HDI_QUERY_CHANNELLAYOUT;
    uint64_t* tempChannelLayout = reinterpret_cast<uint64_t *>(effectHdiInput_ + 1);
    *tempChannelLayout = channelLayout;
    CHECK_AND_RETURN_RET_LOG(audioEffectHdiParam_ != nullptr, false, "audioEffectHdiParam_ is nullptr");
    if (audioEffectHdiParam_->UpdateHdiState(effectHdiInput_, deviceType_) != SUCCESS) {
        AUDIO_WARNING_LOG("query channel layout support failed :%{public}" PRIu64, channelLayout);
        return false;
    }
    return true;
}
} // namespace AudioStandard
} // namespace OHOS
