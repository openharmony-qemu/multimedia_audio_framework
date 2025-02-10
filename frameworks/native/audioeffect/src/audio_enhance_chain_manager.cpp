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
#define LOG_TAG "AudioEnhanceChainManager"
#endif

#include "audio_enhance_chain_manager.h"

#include "securec.h"
#include "audio_effect_log.h"
#include "audio_errors.h"
#include "audio_enhance_chain_adapter.h"

using namespace OHOS::AudioStandard;

namespace OHOS {
namespace AudioStandard {

static int32_t FindEnhanceLib(const std::string &enhance,
    const std::vector<std::shared_ptr<AudioEffectLibEntry>> &enhanceLibraryList,
    std::shared_ptr<AudioEffectLibEntry> &libEntry, std::string &libName)
{
    for (const std::shared_ptr<AudioEffectLibEntry> &lib : enhanceLibraryList) {
        if (lib->libraryName == enhance) {
            libName = lib->libraryName;
            libEntry = lib;
            return SUCCESS;
        }
    }
    return ERROR;
}

static int32_t CheckValidEnhanceLibEntry(const std::shared_ptr<AudioEffectLibEntry> &libEntry,
    const std::string &enhance, const std::string &libName)
{
    CHECK_AND_RETURN_RET_LOG(libEntry, ERROR, "Enhance [%{public}s] in lib [%{public}s] is nullptr",
        enhance.c_str(), libName.c_str());
    CHECK_AND_RETURN_RET_LOG(libEntry->audioEffectLibHandle, ERROR,
        "AudioEffectLibHandle of Enhance [%{public}s] in lib [%{public}s] is nullptr",
        enhance.c_str(), libName.c_str());
    CHECK_AND_RETURN_RET_LOG(libEntry->audioEffectLibHandle->createEffect, ERROR,
        "CreateEffect function of Enhance [%{public}s] in lib [%{public}s] is nullptr",
        enhance.c_str(), libName.c_str());
    CHECK_AND_RETURN_RET_LOG(libEntry->audioEffectLibHandle->releaseEffect, ERROR,
        "ReleaseEffect function of Enhance [%{public}s] in lib [%{public}s] is nullptr",
        enhance.c_str(), libName.c_str());
    return SUCCESS;
}

AudioEnhanceChainManager::AudioEnhanceChainManager()
{
    sceneTypeToEnhanceChainMap_.clear();
    sceneTypeToEnhanceChainCountMap_.clear();
    sceneTypeAndModeToEnhanceChainNameMap_.clear();
    enhanceChainToEnhancesMap_.clear();
    enhanceToLibraryEntryMap_.clear();
    enhanceToLibraryNameMap_.clear();
    isInitialized_ = false;
    upAndDownDevice_ = "";
}

AudioEnhanceChainManager::~AudioEnhanceChainManager()
{
    AUDIO_INFO_LOG("~AudioEnhanceChainManager destroy");
}

AudioEnhanceChainManager *AudioEnhanceChainManager::GetInstance()
{
    static AudioEnhanceChainManager audioEnhanceChainManager;
    return &audioEnhanceChainManager;
}

void AudioEnhanceChainManager::InitAudioEnhanceChainManager(std::vector<EffectChain> &enhanceChains,
    const EffectChainManagerParam &managerParam, std::vector<std::shared_ptr<AudioEffectLibEntry>> &enhanceLibraryList)
{
    const std::unordered_map<std::string, std::string> &enhanceChainNameMap = managerParam.sceneTypeToChainNameMap;
    std::lock_guard<std::mutex> lock(chainManagerMutex_);
    std::set<std::string> enhanceSet;
    for (EffectChain enhanceChain : enhanceChains) {
        for (std::string enhance : enhanceChain.apply) {
            enhanceSet.insert(enhance);
        }
    }
    // Construct enhanceToLibraryEntryMap_ that stores libEntry for each effect name
    std::shared_ptr<AudioEffectLibEntry> libEntry = nullptr;
    std::string libName;
    for (std::string enhance : enhanceSet) {
        int32_t ret = FindEnhanceLib(enhance, enhanceLibraryList, libEntry, libName);
        CHECK_AND_CONTINUE_LOG(ret != ERROR, "Couldn't find libEntry of effect %{public}s", enhance.c_str());
        ret = CheckValidEnhanceLibEntry(libEntry, enhance, libName);
        enhanceToLibraryEntryMap_[enhance] = libEntry;
        enhanceToLibraryNameMap_[enhance] = libName;
    }
    // Construct enhanceChainToEnhancesMap_ that stores all effect names of each effect chain
    for (EffectChain enhanceChain : enhanceChains) {
        std::string key = enhanceChain.name;
        std::vector<std::string> enhances;
        for (std::string enhanceName : enhanceChain.apply) {
            enhances.emplace_back(enhanceName);
        }
        enhanceChainToEnhancesMap_[key] = enhances;
    }
    // Construct sceneTypeAndModeToEnhanceChainNameMap_ that stores effectMode associated with the effectChainName
    for (auto item = enhanceChainNameMap.begin(); item != enhanceChainNameMap.end(); item++) {
        sceneTypeAndModeToEnhanceChainNameMap_[item->first] = item->second;
    }
    AUDIO_INFO_LOG("enhanceToLibraryEntryMap_ size %{public}zu \
        enhanceToLibraryNameMap_ size %{public}zu \
        sceneTypeAndModeToEnhanceChainNameMap_ size %{public}zu",
        enhanceToLibraryEntryMap_.size(),
        enhanceChainToEnhancesMap_.size(),
        sceneTypeAndModeToEnhanceChainNameMap_.size());
    isInitialized_ = true;
}

int32_t AudioEnhanceChainManager::CreateAudioEnhanceChainDynamic(const std::string &sceneType,
    const std::string &enhanceMode, const std::string &upAndDownDevice)
{
    std::lock_guard<std::mutex> lock(chainManagerMutex_);
    upAndDownDevice_ = upAndDownDevice;
    std::string sceneTypeAndDeviceKey = sceneType + "_&_" + upAndDownDevice;
    std::shared_ptr<AudioEnhanceChain> audioEnhanceChain = nullptr;
    if (sceneTypeToEnhanceChainMap_.count(sceneTypeAndDeviceKey)) {
        if (!sceneTypeToEnhanceChainCountMap_.count(sceneTypeAndDeviceKey) ||
            sceneTypeToEnhanceChainCountMap_[sceneTypeAndDeviceKey] < 1) {
            sceneTypeToEnhanceChainCountMap_.erase(sceneTypeAndDeviceKey);
            sceneTypeToEnhanceChainMap_.erase(sceneTypeAndDeviceKey);
            return ERROR;
            }
        sceneTypeToEnhanceChainCountMap_[sceneTypeAndDeviceKey]++;
        return SUCCESS;
    } else {
        audioEnhanceChain = std::make_shared<AudioEnhanceChain>(sceneType);
        sceneTypeToEnhanceChainMap_.insert(std::make_pair(sceneTypeAndDeviceKey, audioEnhanceChain));
        if (!sceneTypeToEnhanceChainCountMap_.count(sceneTypeAndDeviceKey)) {
            sceneTypeToEnhanceChainCountMap_.insert(std::make_pair(sceneTypeAndDeviceKey, 1));
        } else {
            sceneTypeToEnhanceChainCountMap_[sceneTypeAndDeviceKey] = 1;
        }
    }
    if (SetAudioEnhanceChainDynamic(sceneType, enhanceMode, upAndDownDevice) != SUCCESS) {
        return ERROR;
    }
    AUDIO_INFO_LOG("%{public}s create success", sceneTypeAndDeviceKey.c_str());
    return SUCCESS;
}

int32_t AudioEnhanceChainManager::SetAudioEnhanceChainDynamic(const std::string &sceneType,
    const std::string &enhanceMode, const std::string &upAndDownDevice)
{
    std::string sceneTypeAndDeviceKey = sceneType + "_&_" + upAndDownDevice;
    CHECK_AND_RETURN_RET_LOG(sceneTypeToEnhanceChainMap_.count(sceneTypeAndDeviceKey), ERROR,
        "SceneType [%{public}s] does not exist, fail to set.", sceneTypeAndDeviceKey.c_str());
    
    std::shared_ptr<AudioEnhanceChain> audioEnhanceChain = sceneTypeToEnhanceChainMap_[sceneTypeAndDeviceKey];

    std::string enhanceChain;
    std::string enhanceChainKey = sceneType + "_&_" + enhanceMode;
    std::string enhanceNone = AUDIO_SUPPORTED_SCENE_MODES.find(EFFECT_NONE)->second;
    if (!sceneTypeAndModeToEnhanceChainNameMap_.count(enhanceChainKey)) {
        AUDIO_ERR_LOG("EnhanceChain key [%{public}s] does not exist, auto set to %{public}s",
            enhanceChainKey.c_str(), enhanceNone.c_str());
        enhanceChain = enhanceNone;
    } else {
        enhanceChain = sceneTypeAndModeToEnhanceChainNameMap_[enhanceChainKey];
    }

    if (enhanceChain != enhanceNone && !enhanceChainToEnhancesMap_.count(enhanceChain)) {
        AUDIO_ERR_LOG("EnhanceChain name [%{public}s] does not exist, auto set to %{public}s",
            enhanceChain.c_str(), enhanceNone.c_str());
            enhanceChain = enhanceNone;
    }

    audioEnhanceChain->SetEnhanceMode(enhanceMode);
    for (std::string enhance : enhanceChainToEnhancesMap_[enhanceChain]) {
        AudioEffectHandle handle = nullptr;
        AudioEffectDescriptor descriptor;
        descriptor.libraryName = enhanceToLibraryNameMap_[enhance];
        descriptor.effectName = enhance;

        AUDIO_INFO_LOG("libraryName: %{public}s effectName:%{public}s",
            descriptor.libraryName.c_str(), descriptor.effectName.c_str());
        int32_t ret = enhanceToLibraryEntryMap_[enhance]->audioEffectLibHandle->createEffect(descriptor, &handle);
        CHECK_AND_CONTINUE_LOG(ret == 0, "EnhanceToLibraryEntryMap[%{public}s] createEffect fail",
            enhance.c_str());
        audioEnhanceChain->AddEnhanceHandle(handle, enhanceToLibraryEntryMap_[enhance]->audioEffectLibHandle);
    }

    if (audioEnhanceChain->IsEmptyEnhanceHandles()) {
        AUDIO_ERR_LOG("EnhanceChain is empty, copy bufIn to bufOut like EFFECT_NONE mode");
        return ERROR;
    }
    return SUCCESS;
}

int32_t AudioEnhanceChainManager::ReleaseAudioEnhanceChainDynamic(const std::string &sceneType,
    const std::string &upAndDownDevice)
{
    std::lock_guard<std::mutex> lock(chainManagerMutex_);
    CHECK_AND_RETURN_RET_LOG(isInitialized_, ERROR, "has not been initialized");
    CHECK_AND_RETURN_RET_LOG(sceneType != "", ERROR, "null sceneType");

    std::string sceneTypeAndDeviceKey = sceneType + "_&_" + upAndDownDevice;
    if (!sceneTypeToEnhanceChainMap_.count(sceneTypeAndDeviceKey)) {
        sceneTypeToEnhanceChainCountMap_.erase(sceneTypeAndDeviceKey);
        return SUCCESS;
    } else if (sceneTypeToEnhanceChainCountMap_.count(sceneTypeAndDeviceKey) &&
        sceneTypeToEnhanceChainCountMap_[sceneTypeAndDeviceKey] > 1) {
        sceneTypeToEnhanceChainCountMap_[sceneTypeAndDeviceKey]--;
        return SUCCESS;
    }
    sceneTypeToEnhanceChainCountMap_.erase(sceneTypeAndDeviceKey);
    sceneTypeToEnhanceChainMap_.erase(sceneTypeAndDeviceKey);
    AUDIO_INFO_LOG("release %{public}s", sceneTypeAndDeviceKey.c_str());
    return SUCCESS;
}

int32_t AudioEnhanceChainManager::ApplyAudioEnhanceChain(const std::string &sceneType,
    EnhanceBufferAttr *enhanceBufferAttr)
{
    CHECK_AND_RETURN_RET_LOG(enhanceBufferAttr != nullptr, ERROR, "enhance buffer is null");
    std::lock_guard<std::mutex> lock(chainManagerMutex_);
    std::string sceneTypeAndDeviceKey = sceneType + "_&_" + GetUpAndDownDevice();
    if (!sceneTypeToEnhanceChainMap_.count(sceneTypeAndDeviceKey)) {
        uint32_t totalLen = enhanceBufferAttr->byteLenPerFrame * enhanceBufferAttr->outNum;
        CHECK_AND_RETURN_RET_LOG(memcpy_s(enhanceBufferAttr->output, totalLen, enhanceBufferAttr->input,
            totalLen) == 0, ERROR, "memcpy error in apply enhance");
        AUDIO_ERR_LOG("Can not find %{public}s in sceneTypeToEnhanceChainMap_", sceneTypeAndDeviceKey.c_str());
        return ERROR;
    }
    auto audioEnhanceChain = sceneTypeToEnhanceChainMap_[sceneTypeAndDeviceKey];
    if (audioEnhanceChain->ApplyEnhanceChain(enhanceBufferAttr) != SUCCESS) {
        AUDIO_ERR_LOG("Apply %{public}s failed.", sceneTypeAndDeviceKey.c_str());
        return ERROR;
    }
    AUDIO_INFO_LOG("Apply %{public}s success", sceneTypeAndDeviceKey.c_str());
    return SUCCESS;
}

bool AudioEnhanceChainManager::ExistAudioEnhanceChain(const std::string &sceneType)
{
    std::lock_guard<std::mutex> lock(chainManagerMutex_);
    if (!isInitialized_) {
        AUDIO_ERR_LOG("audioEnhanceChainManager has not been initialized.");
        return false;
    }
    CHECK_AND_RETURN_RET(sceneType != "", false);
    CHECK_AND_RETURN_RET_LOG(GetUpAndDownDevice() != "", false, "null device");

    std::string sceneTypeAndDeviceKey = sceneType + "_&_" + GetUpAndDownDevice();
    if (!sceneTypeToEnhanceChainMap_.count(sceneTypeAndDeviceKey)) {
        return false;
    }
    auto audioEnhanceChain = sceneTypeToEnhanceChainMap_[sceneTypeAndDeviceKey];
    CHECK_AND_RETURN_RET_LOG(audioEnhanceChain != nullptr, false, "null sceneTypeToEnhanceChainMap_[%{public}s]",
        sceneTypeAndDeviceKey.c_str());
    return !audioEnhanceChain->IsEmptyEnhanceHandles();
}

int32_t AudioEnhanceChainManager::SetAudioEnhanceProperty(const AudioEffectPropertyArrayV3 &propertyArray,
    DeviceType deviceType)
{
    return AUDIO_OK;
}

int32_t AudioEnhanceChainManager::GetAudioEnhanceProperty(AudioEffectPropertyArrayV3 &propertyArray,
    DeviceType deviceType)
{
    return AUDIO_OK;
}

std::string AudioEnhanceChainManager::GetUpAndDownDevice()
{
    return upAndDownDevice_;
}

} // namespace AudioStandard
} // namespace OHOS