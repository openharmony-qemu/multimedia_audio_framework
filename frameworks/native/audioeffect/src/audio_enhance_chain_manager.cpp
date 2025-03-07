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

#undef LOG_TAG
#define LOG_TAG "AudioEnhanceChainManager"

#include "audio_enhance_chain_manager.h"

#include <algorithm>
#include <charconv>
#include <system_error>

#include "securec.h"
#include "system_ability_definition.h"

#include "audio_effect_log.h"
#include "audio_errors.h"
#include "audio_effect.h"
#include "audio_enhance_chain.h"
#include "audio_enhance_chain_adapter.h"
#include "audio_setting_provider.h"
#include "audio_device_type.h"
#include "audio_effect_map.h"

using namespace OHOS::AudioStandard;

namespace OHOS {
namespace AudioStandard {
namespace {
constexpr uint32_t SCENE_TYPE_OFFSET = 32;
constexpr uint32_t CAPTURER_ID_OFFSET = 16;
constexpr uint64_t SCENE_TYPE_MASK = 0xFF00000000;
constexpr uint64_t CAPTURER_ID_MASK = 0x0000FFFF0000;
constexpr uint64_t RENDERER_ID_MASK = 0x00000000FFFF;
constexpr uint32_t VOLUME_FACTOR = 100;
const std::unordered_map<AudioEnhanceMode, std::string> AUDIO_ENHANCE_SUPPORTED_SCENE_MODES {
    {ENHANCE_NONE, "ENHANCE_NONE"},
    {ENHANCE_DEFAULT, "ENHANCE_DEFAULT"},
};
const std::vector<AudioEnhanceScene> AUDIO_WITH_DEVICE_ENHANCES {SCENE_VOIP_UP};
const std::string MAINKEY_DEVICE_STATUS = "device_status";
const std::string SUBKEY_FOLD_STATE = "fold_state";
}

static int32_t FindEnhanceLib(const std::string &enhance,
    const std::vector<std::shared_ptr<AudioEffectLibEntry>> &enhanceLibraryList,
    std::shared_ptr<AudioEffectLibEntry> &libEntry, std::string &libName)
{
    for (const std::shared_ptr<AudioEffectLibEntry> &lib : enhanceLibraryList) {
        if (std::any_of(lib->effectName.begin(), lib->effectName.end(),
            [&enhance](const std::string &effectName) {
                return effectName == enhance;
            })) {
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
    captureIdToDeviceMap_.clear();
    captureIdToDeviceNameMap_.clear();
    renderIdToDeviceMap_.clear();
    enhanceBuffer_ = nullptr;
    isInitialized_ = false;
    foldState_ = FOLD_STATE_MIDDLE;
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

void AudioEnhanceChainManager::ResetInfo()
{
    sceneTypeToEnhanceChainMap_.clear();
    sceneTypeToEnhanceChainCountMap_.clear();
    sceneTypeAndModeToEnhanceChainNameMap_.clear();
    enhanceChainToEnhancesMap_.clear();
    enhanceToLibraryEntryMap_.clear();
    enhanceToLibraryNameMap_.clear();
    enhancePropertyMap_.clear();
    defaultPropertyMap_.clear();
    captureIdToDeviceMap_.clear();
    captureIdToDeviceNameMap_.clear();
    renderIdToDeviceMap_.clear();
    FreeEnhanceBuffer();
    isInitialized_ = false;
    sessionId_ = 0;
    volumeType_ = STREAM_MUSIC;
    systemVol_ = 0.0f;
    streamVol_ = 0.0f;
    isMute_ = false;
}

void AudioEnhanceChainManager::ConstructEnhanceChainMgrMaps(std::vector<EffectChain> &enhanceChains,
    const EffectChainManagerParam &managerParam, std::vector<std::shared_ptr<AudioEffectLibEntry>> &enhanceLibraryList)
{
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
            if (enhanceToLibraryEntryMap_.count(enhanceName)) {
                enhances.emplace_back(enhanceName);
            }
        }
        enhanceChainToEnhancesMap_[key] = enhances;
    }
    // Construct sceneTypeAndModeToEnhanceChainNameMap_ that stores effectMode associated with the effectChainName
    sceneTypeAndModeToEnhanceChainNameMap_ = managerParam.sceneTypeToChainNameMap;
    // Construct enhancePropertyMap_ that stores effect's property
    enhancePropertyMap_ = managerParam.effectDefaultProperty;
    defaultPropertyMap_ = managerParam.effectDefaultProperty;
    ConstructDeviceEnhances();
}

void AudioEnhanceChainManager::ConstructDeviceEnhances()
{
    CHECK_AND_RETURN_LOG(sceneTypeAndModeToEnhanceChainNameMap_.size() != 0, "no enhance algos");
    const std::unordered_map<AudioEnhanceScene, std::string> &audioEnhanceSupportedSceneTypes =
        GetEnhanceSupportedSceneType();
    for (const auto& sceneType: AUDIO_WITH_DEVICE_ENHANCES) {
        std::string scene = audioEnhanceSupportedSceneTypes.find(sceneType)->second;
        std::string sceneAndMode = scene + "_&_" + "ENHANCE_DEFAULT";
        std::string enhanceChain = "";
        auto item = sceneTypeAndModeToEnhanceChainNameMap_.find(sceneAndMode);
        CHECK_AND_CONTINUE_LOG(item != sceneTypeAndModeToEnhanceChainNameMap_.end(),
            "no such sceneAndMode %{public}s", sceneAndMode.c_str());
        enhanceChain = item->second;
        auto mapIter = enhanceChainToEnhancesMap_.find(enhanceChain);
        std::vector<std::string> deviceEnhances;
        CHECK_AND_CONTINUE_LOG(mapIter != enhanceChainToEnhancesMap_.end(),
            "no such enhanceChain %{public}s", enhanceChain.c_str());
        deviceEnhances = mapIter->second;
        for (std::string enhance: deviceEnhances) {
            auto iter = enhancePropertyMap_.find(enhance);
            if (iter == enhancePropertyMap_.end()) {
                continue;
            }
            withDeviceEnhances_.insert(iter->first);
        }
    }
}

void AudioEnhanceChainManager::UpdateEnhancePropertyMapFromDb(DeviceType deviceType)
{
    std::string deviceTypeName = "";
    GetDeviceTypeName(deviceType, deviceTypeName);
    AudioSettingProvider &settingProvider = AudioSettingProvider::GetInstance(AUDIO_POLICY_SERVICE_ID);
    CHECK_AND_RETURN_LOG(settingProvider.CheckOsAccountReady(), "os account not ready");
    for (auto &[enhance, prop] : enhancePropertyMap_) {
        std::string property = "";
        if (deviceTypeName == "") {
            AUDIO_ERR_LOG("DeviceTypeName Null");
            return;
        }
        std::string key = "";
        if (withDeviceEnhances_.find(enhance) == withDeviceEnhances_.end()) {
            key = enhance;
        } else {
            key = enhance +  "_&_" + deviceTypeName;
        }
        ErrCode ret = settingProvider.GetStringValue(key, property, "system");
        if (ret == SUCCESS) {
            prop = property;
            AUDIO_INFO_LOG("Get Effect_&_DeviceType:%{public}s is Property:%{public}s",
                key.c_str(), property.c_str());
        } else {
            ret = settingProvider.PutStringValue(key, defaultPropertyMap_[enhance], "system");
            if (ret != SUCCESS) {
                AUDIO_ERR_LOG("set to default Property:%{public}s, failed, ErrCode : %{public}d",
                    defaultPropertyMap_[enhance].c_str(), ret);
                return;
            }
            prop = defaultPropertyMap_[enhance];
            AUDIO_INFO_LOG("Get prop failed,Effect_&_DeviceType:%{public}s is set to default Property:%{public}s",
                key.c_str(), prop.c_str());
        }
    }
}

void AudioEnhanceChainManager::InitAudioEnhanceChainManager(std::vector<EffectChain> &enhanceChains,
    const EffectChainManagerParam &managerParam, std::vector<std::shared_ptr<AudioEffectLibEntry>> &enhanceLibraryList)
{
    std::lock_guard<std::mutex> lock(chainManagerMutex_);
    normalSceneLimit_ = managerParam.maxExtraNum;
    chainNum_ = 0;
    priorSceneSet_.insert(managerParam.priorSceneList.begin(), managerParam.priorSceneList.end());
    defaultScene_ = managerParam.defaultSceneName;
    AUDIO_INFO_LOG("defaultSceneName is %{public}s", defaultScene_.c_str());
    for (const auto &priorScene : priorSceneSet_) {
        AUDIO_INFO_LOG("priorScene contains %{public}s", priorScene.c_str());
    }
    AUDIO_INFO_LOG("normalSceneLimit is %{public}u", normalSceneLimit_);
    ConstructEnhanceChainMgrMaps(enhanceChains, managerParam, enhanceLibraryList);

    AUDIO_INFO_LOG("enhanceToLibraryEntryMap_ size %{public}zu \
        enhanceChainToEnhancesMap_ size %{public}zu \
        sceneTypeAndModeToEnhanceChainNameMap_ size %{public}zu",
        enhanceToLibraryEntryMap_.size(),
        enhanceChainToEnhancesMap_.size(),
        sceneTypeAndModeToEnhanceChainNameMap_.size());
    isInitialized_ = true;
}

int32_t AudioEnhanceChainManager::InitEnhanceBuffer()
{
    std::lock_guard<std::mutex> lock(chainManagerMutex_);
    uint32_t len = 0;
    uint32_t lenEc = 0;
    uint32_t lenMicRef = 0;
    for (auto &[scode, chain] : sceneTypeToEnhanceChainMap_) {
        if (chain) {
            len = std::max(len, chain->GetAlgoBufferSize());
            lenEc = std::max(lenEc, chain->GetAlgoBufferSizeEc());
            lenMicRef = std::max(lenMicRef, chain->GetAlgoBufferSizeMicRef());
        }
    }
    if (enhanceBuffer_ == nullptr) {
        enhanceBuffer_ = std::make_unique<EnhanceBuffer>();
        enhanceBuffer_->micBufferIn.resize(len);
        enhanceBuffer_->micBufferOut.resize(len);
        enhanceBuffer_->ecBuffer.resize(lenEc);
        enhanceBuffer_->micRefBuffer.resize(lenMicRef);
        AUDIO_INFO_LOG("enhanceBuffer_ init len:%{public}u lenEc:%{public}u lenMicRef:%{public}u",
            len, lenEc, lenMicRef);
        return SUCCESS;
    }
    if ((len > enhanceBuffer_->micBufferIn.size())) {
        enhanceBuffer_->micBufferIn.resize(len);
        enhanceBuffer_->micBufferOut.resize(len);
    }
    if (lenEc > enhanceBuffer_->ecBuffer.size()) {
        enhanceBuffer_->ecBuffer.resize(lenEc);
    }
    if (lenMicRef > enhanceBuffer_->micRefBuffer.size()) {
        enhanceBuffer_->micRefBuffer.resize(lenMicRef);
    }
    AUDIO_INFO_LOG("enhanceBuffer_ update len:%{public}u lenEc:%{public}u lenMicRef:%{public}u",
        len, lenEc, lenMicRef);
    return SUCCESS;
}

int32_t AudioEnhanceChainManager::ParseSceneKeyCode(const uint64_t sceneKeyCode, std::string &sceneType,
    std::string &capturerDeviceStr, std::string &rendererDeviceStr)
{
    uint32_t sceneCode = (sceneKeyCode & SCENE_TYPE_MASK) >> SCENE_TYPE_OFFSET;
    AUDIO_INFO_LOG("sceneKeyCode = %{public}" PRIu64 ", sceneCode = %{public}u", sceneKeyCode, sceneCode);
    AudioEnhanceScene scene = static_cast<AudioEnhanceScene>(sceneCode);
    DeviceType capturerDevice = DEVICE_TYPE_INVALID;
    const std::unordered_map<DeviceType, std::string> &supportDeviceType = GetSupportedDeviceType();
    const std::unordered_map<AudioEnhanceScene, std::string> &audioEnhanceSupportedSceneTypes =
        GetEnhanceSupportedSceneType();
    auto item = audioEnhanceSupportedSceneTypes.find(scene);
    if (item != audioEnhanceSupportedSceneTypes.end()) {
        sceneType = item->second;
    } else {
        AUDIO_ERR_LOG("scene[%{public}d] not be supported", scene);
        return ERROR;
    }
    uint32_t captureId = (sceneKeyCode & CAPTURER_ID_MASK) >> CAPTURER_ID_OFFSET;
    uint32_t renderId = (sceneKeyCode & RENDERER_ID_MASK);
    auto capItem = captureIdToDeviceMap_.find(captureId);
    if (capItem != captureIdToDeviceMap_.end()) {
        capturerDevice = capItem->second;
    } else {
        AUDIO_ERR_LOG("can't find captureId[%{public}u] in captureIdToDeviceMap_", captureId);
        return ERROR;
    }

    DeviceType rendererDevice = renderIdToDeviceMap_[renderId];

    auto deviceItem = supportDeviceType.find(capturerDevice);
    if (deviceItem != supportDeviceType.end()) {
        if ((capturerDevice == DEVICE_TYPE_INVALID) || (capturerDevice == DEVICE_TYPE_NONE)) {
            capturerDeviceStr = "DEVICE_TYPE_MIC";
            AUDIO_ERR_LOG("capturerDevice not availd");
        } else {
            capturerDeviceStr = deviceItem->second;
        }
    } else {
        AUDIO_ERR_LOG("capturerDevice[%{public}d] not in supportDeviceType", capturerDevice);
        return ERROR;
    }
    deviceItem = supportDeviceType.find(rendererDevice);
    if (deviceItem != supportDeviceType.end()) {
        rendererDeviceStr = deviceItem->second;
    } else {
        AUDIO_ERR_LOG("rendererDevice[%{public}d] not in supportDeviceType", rendererDevice);
        return ERROR;
    }
    return SUCCESS;
}

int32_t AudioEnhanceChainManager::CreateAudioEnhanceChainDynamic(const uint64_t sceneKeyCode,
    const AudioEnhanceDeviceAttr &deviceAttr)
{
    std::lock_guard<std::mutex> lock(chainManagerMutex_);
    if (sceneTypeAndModeToEnhanceChainNameMap_.size() == 0) {
        AUDIO_INFO_LOG("no algo on audio_framework");
        return ERROR;
    }

    std::shared_ptr<AudioEnhanceChain> audioEnhanceChain = nullptr;
    auto it = sceneTypeToEnhanceChainMap_.find(sceneKeyCode);
    if (it != sceneTypeToEnhanceChainMap_.end() && it->second != nullptr) {
        sceneTypeToEnhanceChainCountMap_[sceneKeyCode]++;
        AUDIO_INFO_LOG("Now enhanceChain num is:%{public}u scenKey[%{public}" PRIu64 "] count: %{public}d", chainNum_,
            sceneKeyCode, sceneTypeToEnhanceChainCountMap_[sceneKeyCode]);
        audioEnhanceChain = it->second;
        if (audioEnhanceChain->IsEmptyEnhanceHandles()) {
            return ERROR;
        }
        return audioEnhanceChain->IsDefaultChain();
    }
    bool createFlag = false;
    bool defaultFlag = false;
    if (CreateEnhanceChainInner(audioEnhanceChain, sceneKeyCode, deviceAttr, createFlag, defaultFlag)) {
        AUDIO_ERR_LOG("CreateEnhanceChainInner failed!");
        return ERROR;
    }
    AUDIO_INFO_LOG("Now enhanceChain num is:%{public}u scenKey[%{public}" PRIu64 "] count: %{public}d", chainNum_,
        sceneKeyCode, sceneTypeToEnhanceChainCountMap_[sceneKeyCode]);
    // means map to default chain
    if (!createFlag) {
        return audioEnhanceChain->IsDefaultChain();
    }
    std::string effectChainName = GetEnhanceChainNameBySceneCode(sceneKeyCode, defaultFlag);
    if (AddAudioEnhanceChainHandles(audioEnhanceChain, effectChainName) != SUCCESS) {
        sceneTypeToEnhanceChainCountMap_.erase(sceneKeyCode);
        sceneTypeToEnhanceChainMap_.erase(sceneKeyCode);
        chainNum_--;
        AUDIO_ERR_LOG("%{public}" PRIu64 " create failed.", sceneKeyCode);
        return ERROR;
    }
    AUDIO_INFO_LOG("%{public}" PRIu64 " create success", sceneKeyCode);
    return audioEnhanceChain->IsDefaultChain();
}

std::string AudioEnhanceChainManager::GetEnhanceChainNameBySceneCode(const uint64_t sceneKeyCode,
    const bool defaultFlag)
{
    std::string enhanceNone = AUDIO_ENHANCE_SUPPORTED_SCENE_MODES.find(ENHANCE_NONE)->second;
    std::string sceneType = "";
    std::string capturerDevice = "";
    std::string rendererDeivce = "";
    if (ParseSceneKeyCode(sceneKeyCode, sceneType, capturerDevice, rendererDeivce) != SUCCESS) {
        return enhanceNone;
    }
    if (defaultFlag) {
        AUDIO_INFO_LOG("sceneType %{public}s set to defaultScene %{public}s", sceneType.c_str(),
            defaultScene_.c_str());
        sceneType = defaultScene_;
    }
    // first check specific device, then check no device
    std::string enhanceChainKey = sceneType + "_&_" + "ENHANCE_DEFAULT" + "_&_" + capturerDevice;
    auto mapIter = sceneTypeAndModeToEnhanceChainNameMap_.find(enhanceChainKey);
    if (mapIter == sceneTypeAndModeToEnhanceChainNameMap_.end()) {
        enhanceChainKey = sceneType + "_&_" + "ENHANCE_DEFAULT";
        mapIter = sceneTypeAndModeToEnhanceChainNameMap_.find(enhanceChainKey);
    }
    if (mapIter == sceneTypeAndModeToEnhanceChainNameMap_.end() ||
        !enhanceChainToEnhancesMap_.count(mapIter->second)) {
        AUDIO_ERR_LOG("EnhanceChain key [%{public}s] does not exist, auto set to %{public}s",
            enhanceChainKey.c_str(), enhanceNone.c_str());
        return enhanceNone;
    } else {
        return mapIter->second;
    }
}

void AudioEnhanceChainManager::GetDeviceNameByCaptureId(const uint32_t captureId, std::string &deviceName)
{
    auto item = captureIdToDeviceNameMap_.find(captureId);
    if (item != captureIdToDeviceNameMap_.end()) {
        deviceName = captureIdToDeviceNameMap_[captureId];
    }
}

int32_t AudioEnhanceChainManager::CreateEnhanceChainInner(std::shared_ptr<AudioEnhanceChain> &audioEnhanceChain,
    const uint64_t sceneKeyCode, const AudioEnhanceDeviceAttr &deviceAttr, bool &createFlag, bool &defaultFlag)
{
    std::string sceneType = "";
    std::string capturerDevice = "";
    std::string rendererDeivce = "";
    if (ParseSceneKeyCode(sceneKeyCode, sceneType, capturerDevice, rendererDeivce) != SUCCESS) {
        AUDIO_ERR_LOG("ParseSceneKeyCode failed!!!");
        return ERROR;
    }
    uint32_t captureId = (sceneKeyCode & CAPTURER_ID_MASK) >> 8;
    std::string deviceName = "";
    GetDeviceNameByCaptureId(captureId, deviceName);
    createFlag = true;
    // normal scene
    if (priorSceneSet_.find(sceneType) == priorSceneSet_.end()) {
        // effect instance exceeded
        if (captureId2SceneCount_[captureId] == normalSceneLimit_) {
            // default enhance chain not exsist
            if (captureId2DefaultChainCount_[captureId] != 0) {
                createFlag = false;
                audioEnhanceChain = captureId2DefaultChain_[captureId];
                // add sceneType change after integration supported
                AUDIO_INFO_LOG("sceneKey[%{public}" PRIu64 "] defaultChainExsist", sceneKeyCode);
            } else {
                AudioEnhanceParamAdapter algoParam = {(uint32_t)isMute_, (uint32_t)(systemVol_ * VOLUME_FACTOR),
                    foldState_, capturerDevice, rendererDeivce, defaultScene_, deviceName};
                audioEnhanceChain = std::make_shared<AudioEnhanceChain>(defaultScene_, algoParam, deviceAttr, 1);
                captureId2DefaultChain_[captureId] = audioEnhanceChain;
                AUDIO_INFO_LOG("captureId %{public}u defaultScene chain not exsist, create it", captureId);
                chainNum_++;
            }
            captureId2DefaultChainCount_[captureId]++;
            defaultFlag = true;
        } else {
            AudioEnhanceParamAdapter algoParam = {(uint32_t)isMute_, (uint32_t)(systemVol_ * VOLUME_FACTOR),
                foldState_, capturerDevice, rendererDeivce, sceneType, deviceName};
            audioEnhanceChain = std::make_shared<AudioEnhanceChain>(sceneType, algoParam, deviceAttr, 0);
            captureId2SceneCount_[captureId]++;
            AUDIO_INFO_LOG("captureId %{public}u create normalScene %{public}s chain", captureId, sceneType.c_str());
            chainNum_++;
        }
    } else {
        AudioEnhanceParamAdapter algoParam = {(uint32_t)isMute_, (uint32_t)(systemVol_ * VOLUME_FACTOR),
            foldState_, capturerDevice, rendererDeivce, sceneType, deviceName};
        audioEnhanceChain = std::make_shared<AudioEnhanceChain>(sceneType, algoParam, deviceAttr, 0);
        AUDIO_INFO_LOG("priorScene %{public}s chain created", sceneType.c_str());
        chainNum_++;
    }
    CHECK_AND_RETURN_RET_LOG(audioEnhanceChain != nullptr, ERROR, "AudioEnhanceChain construct failed.");
    sceneTypeToEnhanceChainMap_[sceneKeyCode] = audioEnhanceChain;
    sceneTypeToEnhanceChainCountMap_[sceneKeyCode] = 1;
    return SUCCESS;
}

int32_t AudioEnhanceChainManager::AddAudioEnhanceChainHandles(std::shared_ptr<AudioEnhanceChain> &audioEnhanceChain,
    const std::string &enhanceChain)
{
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
        auto propIter = enhancePropertyMap_.find(enhance);
        ret = audioEnhanceChain->AddEnhanceHandle(handle, enhanceToLibraryEntryMap_[enhance]->audioEffectLibHandle,
            enhance, propIter == enhancePropertyMap_.end() ? "" : propIter->second);
        if (ret != SUCCESS) {
            AUDIO_ERR_LOG("AddEnhanceHandle fail");
            enhanceToLibraryEntryMap_[enhance]->audioEffectLibHandle->releaseEffect(handle);
        }
    }

    if (audioEnhanceChain->IsEmptyEnhanceHandles()) {
        AUDIO_ERR_LOG("EnhanceChain is empty, copy bufIn to bufOut like EFFECT_NONE mode");
        return ERROR;
    }
    return SUCCESS;
}

int32_t AudioEnhanceChainManager::FreeEnhanceBuffer()
{
    if (enhanceBuffer_ != nullptr) {
        std::vector<uint8_t>().swap(enhanceBuffer_->ecBuffer);
        std::vector<uint8_t>().swap(enhanceBuffer_->micBufferIn);
        std::vector<uint8_t>().swap(enhanceBuffer_->micBufferOut);
        std::vector<uint8_t>().swap(enhanceBuffer_->micRefBuffer);
        enhanceBuffer_ = nullptr;
        AUDIO_INFO_LOG("release EnhanceBuffer success");
    }
    return SUCCESS;
}

int32_t AudioEnhanceChainManager::ReleaseAudioEnhanceChainDynamic(const uint64_t sceneKeyCode)
{
    std::lock_guard<std::mutex> lock(chainManagerMutex_);
    CHECK_AND_RETURN_RET_LOG(isInitialized_, ERROR, "has not been initialized");
    if (sceneTypeAndModeToEnhanceChainNameMap_.size() == 0) {
        AUDIO_INFO_LOG("no algo on audio_framework");
        return ERROR;
    }

    auto chainMapIter = sceneTypeToEnhanceChainMap_.find(sceneKeyCode);
    if (chainMapIter == sceneTypeToEnhanceChainMap_.end() || chainMapIter->second == nullptr) {
        AUDIO_INFO_LOG("Now enhanceChain num is:%{public}u cannot find scenKey[%{public}" PRIu64 "]",
            chainNum_, sceneKeyCode);
        sceneTypeToEnhanceChainCountMap_.erase(sceneKeyCode);
        sceneTypeToEnhanceChainMap_.erase(sceneKeyCode);
        return SUCCESS;
    }
    auto chainCountIter = sceneTypeToEnhanceChainCountMap_.find(sceneKeyCode);
    if (chainCountIter->second > 1) {
        chainCountIter->second--;
        AUDIO_INFO_LOG("Now enhanceChain num is:%{public}u scenKey[%{public}" PRIu64 "] count: %{public}d", chainNum_,
            sceneKeyCode, chainCountIter->second);
        return SUCCESS;
    }
    if (DeleteEnhanceChainInner(chainMapIter->second, sceneKeyCode) != SUCCESS) {
        return ERROR;
    }
    AUDIO_INFO_LOG("release %{public}" PRIu64, sceneKeyCode);
    if (sceneTypeToEnhanceChainMap_.size() == 0) {
        FreeEnhanceBuffer();
    }
    return SUCCESS;
}

int32_t AudioEnhanceChainManager::DeleteEnhanceChainInner(std::shared_ptr<AudioEnhanceChain> &audioEnhanceChain,
    const uint64_t sceneKeyCode)
{
    std::string sceneType = "";
    std::string capturerDevice = "";
    std::string rendererDeivce = "";
    if (ParseSceneKeyCode(sceneKeyCode, sceneType, capturerDevice, rendererDeivce) != SUCCESS) {
        AUDIO_ERR_LOG("ParseSceneKeyCode failed!!!");
        return ERROR;
    }
    uint32_t captureId = (sceneKeyCode & CAPTURER_ID_MASK) >> 8;
    // not prior scene
    if (priorSceneSet_.find(sceneType) == priorSceneSet_.end()) {
        // default chain
        if (audioEnhanceChain->IsDefaultChain()) {
            auto &defaultChainCount = captureId2DefaultChainCount_.find(captureId)->second;
            defaultChainCount--;
            // delete default chain
            if (!defaultChainCount) {
                captureId2DefaultChain_[captureId] = nullptr;
                chainNum_--;
                AUDIO_INFO_LOG("sceneKey[%{public}" PRIu64 "] defaultScene chain release", sceneKeyCode);
            } else {
                AUDIO_INFO_LOG("sceneKey[%{public}" PRIu64 "] defaultScene chain count:%{public}u", sceneKeyCode,
                    defaultChainCount);
            }
        } else {
            captureId2SceneCount_[captureId]--;
            chainNum_--;
            AUDIO_INFO_LOG("captureId %{public}u normalScene %{public}s chain release", captureId, sceneType.c_str());
        }
    } else {
        chainNum_--;
        AUDIO_INFO_LOG("priorScene %{public}s chain release", sceneType.c_str());
    }
    sceneTypeToEnhanceChainCountMap_.erase(sceneKeyCode);
    sceneTypeToEnhanceChainMap_.erase(sceneKeyCode);
    AUDIO_INFO_LOG("Now enhanceChain num is:%{public}u sceneKey[%{public}" PRIu64 "] count: %{public}d", chainNum_,
        sceneKeyCode, 0);
    return SUCCESS;
}

bool AudioEnhanceChainManager::ExistAudioEnhanceChain(const uint64_t sceneKeyCode)
{
    std::lock_guard<std::mutex> lock(chainManagerMutex_);
    CHECK_AND_RETURN_RET_LOG(isInitialized_, false, "has not been initialized");

    if (!sceneTypeToEnhanceChainMap_.count(sceneKeyCode)) {
        return false;
    }
    auto audioEnhanceChain = sceneTypeToEnhanceChainMap_[sceneKeyCode];
    CHECK_AND_RETURN_RET_LOG(audioEnhanceChain != nullptr, false,
        "null sceneTypeToEnhanceChainMap_[%{public}" PRIu64 "]", sceneKeyCode);
    return !audioEnhanceChain->IsEmptyEnhanceHandles();
}

int32_t AudioEnhanceChainManager::AudioEnhanceChainGetAlgoConfig(const uint64_t sceneKeyCode,
    AudioBufferConfig &micConfig, AudioBufferConfig &ecConfig, AudioBufferConfig &micRefConfig)
{
    std::lock_guard<std::mutex> lock(chainManagerMutex_);
    CHECK_AND_RETURN_RET_LOG(isInitialized_, ERROR, "has not been initialized");
    if (!sceneTypeToEnhanceChainMap_.count(sceneKeyCode)) {
        AUDIO_ERR_LOG("sceneTypeToEnhanceChainMap_ have not %{public}" PRIu64, sceneKeyCode);
        return ERROR;
    }
    auto audioEnhanceChain = sceneTypeToEnhanceChainMap_[sceneKeyCode];
    CHECK_AND_RETURN_RET_LOG(audioEnhanceChain != nullptr, ERROR, "[%{public}" PRIu64 "] get config faild",
        sceneKeyCode);
    audioEnhanceChain->GetAlgoConfig(micConfig, ecConfig, micRefConfig);
    return SUCCESS;
}

bool AudioEnhanceChainManager::IsEmptyEnhanceChain()
{
    std::lock_guard<std::mutex> lock(chainManagerMutex_);
    CHECK_AND_RETURN_RET_LOG(isInitialized_, ERROR, "has not been initialized");
    if (sceneTypeAndModeToEnhanceChainNameMap_.size() == 0) {
        return true;
    }
    for (auto &[scode, chain] : sceneTypeToEnhanceChainMap_) {
        if (chain != nullptr && !chain->IsEmptyEnhanceHandles()) {
            return false;
        }
    }
    return true;
}

int32_t AudioEnhanceChainManager::CopyToEnhanceBuffer(void *data, uint32_t length)
{
    std::lock_guard<std::mutex> lock(chainManagerMutex_);
    if (enhanceBuffer_ == nullptr) {
        return ERROR;
    }
    AUDIO_DEBUG_LOG("length: %{public}zu chunk length: %{public}u", enhanceBuffer_->micBufferIn.size(), length);
    CHECK_AND_RETURN_RET_LOG(memcpy_s(enhanceBuffer_->micBufferIn.data(), enhanceBuffer_->micBufferIn.size(), data,
        length) == 0, ERROR, "memcpy error in data to enhanceBuffer->micBufferIn");
    return SUCCESS;
}

int32_t AudioEnhanceChainManager::CopyEcToEnhanceBuffer(void *data, uint32_t length)
{
    std::lock_guard<std::mutex> lock(chainManagerMutex_);
    if (enhanceBuffer_ == nullptr) {
        return ERROR;
    }
    AUDIO_DEBUG_LOG("lengthEc: %{public}zu chunkEc length: %{public}u", enhanceBuffer_->ecBuffer.size(), length);
    CHECK_AND_RETURN_RET_LOG(memcpy_s(enhanceBuffer_->ecBuffer.data(), enhanceBuffer_->ecBuffer.size(), data,
        length) == 0, ERROR, "memcpy error in data to enhanceBuffer->ecBuffer");
    return SUCCESS;
}

int32_t AudioEnhanceChainManager::CopyMicRefToEnhanceBuffer(void *data, uint32_t length)
{
    std::lock_guard<std::mutex> lock(chainManagerMutex_);
    if (enhanceBuffer_ == nullptr) {
        return ERROR;
    }
    AUDIO_DEBUG_LOG("lengthMicRef: %{public}zu chunkMicRef length: %{public}u", enhanceBuffer_->micRefBuffer.size(),
        length);
    CHECK_AND_RETURN_RET_LOG(memcpy_s(enhanceBuffer_->micRefBuffer.data(), enhanceBuffer_->micRefBuffer.size(),
        data, length) == 0, ERROR, "memcpy error in data to enhanceBuffer->micBufferIn");
    return SUCCESS;
}

int32_t AudioEnhanceChainManager::CopyFromEnhanceBuffer(void *data, uint32_t length)
{
    std::lock_guard<std::mutex> lock(chainManagerMutex_);
    if (enhanceBuffer_ == nullptr) {
        return ERROR;
    }
    if (length > enhanceBuffer_->micBufferOut.size()) {
        return ERROR;
    }
    CHECK_AND_RETURN_RET_LOG(memcpy_s(data, length, enhanceBuffer_->micBufferOut.data(), length) == 0,
        ERROR, "memcpy error in micBufferOut to data");
    return SUCCESS;
}

int32_t AudioEnhanceChainManager::ApplyAudioEnhanceChain(const uint64_t sceneKeyCode, uint32_t length)
{
    std::lock_guard<std::mutex> lock(chainManagerMutex_);
    CHECK_AND_RETURN_RET_LOG(enhanceBuffer_ != nullptr, ERROR, "enhance buffer is null");
    auto chainMapIter = sceneTypeToEnhanceChainMap_.find(sceneKeyCode);
    if (chainMapIter == sceneTypeToEnhanceChainMap_.end() || chainMapIter->second == nullptr) {
        CHECK_AND_RETURN_RET_LOG(memcpy_s(enhanceBuffer_->micBufferOut.data(), enhanceBuffer_->micBufferOut.size(),
            enhanceBuffer_->micBufferIn.data(), length) == 0, ERROR, "memcpy error in apply enhance");
        AUDIO_ERR_LOG("Can not find %{public}" PRIu64 " in sceneTypeToEnhanceChainMap_", sceneKeyCode);
        return ERROR;
    }
    auto audioEnhanceChain = chainMapIter->second;
    if (audioEnhanceChain->ApplyEnhanceChain(enhanceBuffer_, length) != SUCCESS) {
        AUDIO_ERR_LOG("Apply %{public}" PRIu64 " failed.", sceneKeyCode);
        return ERROR;
    }
    AUDIO_DEBUG_LOG("Apply %{public}" PRIu64 " success", sceneKeyCode);
    return SUCCESS;
}

int32_t AudioEnhanceChainManager::UpdatePropertyAndSendToAlgo(const DeviceType &inputDevice)
{
    UpdateEnhancePropertyMapFromDb(inputDevice);
    for (const auto &[enhance, prop] : enhancePropertyMap_) {
        for (const auto &[sceneType, enhanceChain] : sceneTypeToEnhanceChainMap_) {
            if (enhanceChain) {
                int32_t ret = enhanceChain->SetEnhanceProperty(enhance, prop);
                CHECK_AND_RETURN_RET_LOG(ret == 0, ERR_OPERATION_FAILED, "set property failed");
            }
        }
    }
    return SUCCESS;
}

int32_t AudioEnhanceChainManager::SetInputDevice(const uint32_t &captureId, const DeviceType &inputDevice,
    const std::string &deviceName)
{
    const std::unordered_map<DeviceType, std::string> &supportDeviceType = GetSupportedDeviceType();
    std::lock_guard<std::mutex> lock(chainManagerMutex_);
    auto item = captureIdToDeviceMap_.find(captureId);
    if (item == captureIdToDeviceMap_.end()) {
        captureIdToDeviceMap_[captureId] = inputDevice;
        AUDIO_INFO_LOG("set new inputdevice, captureId: %{public}d, inputDevice: %{public}d", captureId, inputDevice);
        return SUCCESS;
    }
    if (item->second == inputDevice) {
        AUDIO_INFO_LOG("set same device, captureId: %{public}d, inputDevice: %{public}d", captureId, inputDevice);
        return SUCCESS;
    }
    // item->second != inputDevice
    captureIdToDeviceMap_[captureId] = inputDevice;
    captureIdToDeviceNameMap_.insert_or_assign(captureId, deviceName);
    std::string inputDeviceStr = "";
    auto deviceItem = supportDeviceType.find(inputDevice);
    if (deviceItem != supportDeviceType.end()) {
        inputDeviceStr = deviceItem->second;
    } else {
        return ERROR;
    }
    for (auto &[sceneKeyCode, chain] : sceneTypeToEnhanceChainMap_) {
        uint32_t tempId = (sceneKeyCode & CAPTURER_ID_MASK) >> 8;
        if ((tempId == captureId) && chain) {
            if (chain->SetInputDevice(inputDeviceStr, deviceName) != SUCCESS) {
                AUDIO_ERR_LOG("chain:%{public}u set input device failed", tempId);
            }
        }
    }
    if (UpdatePropertyAndSendToAlgo(inputDevice) != SUCCESS) {
        return ERROR;
    }
    AUDIO_INFO_LOG("success, captureId: %{public}d, inputDevice: %{public}d deviceName:%{public}s",
        captureId, inputDevice, deviceName.c_str());
    return SUCCESS;
}

int32_t AudioEnhanceChainManager::SetOutputDevice(const uint32_t &renderId, const DeviceType &outputDevice)
{
    std::lock_guard<std::mutex> lock(chainManagerMutex_);
    renderIdToDeviceMap_.insert_or_assign(renderId, outputDevice);
    AUDIO_INFO_LOG("success, renderId: %{public}d, outputDevice: %{public}d", renderId, outputDevice);
    return SUCCESS;
}

int32_t AudioEnhanceChainManager::SetVolumeInfo(const AudioVolumeType &volumeType, const float &systemVol)
{
    volumeType_ = volumeType;
    systemVol_ = systemVol;
    if (sceneTypeAndModeToEnhanceChainNameMap_.size() == 0 || sceneTypeToEnhanceChainMap_.size() == 0) {
        return SUCCESS;
    }
    AUDIO_DEBUG_LOG("success, volumeType: %{public}d, systemVol: %{public}f", volumeType_, systemVol_);
    return SUCCESS;
}

int32_t AudioEnhanceChainManager::SetMicrophoneMuteInfo(const bool &isMute)
{
    isMute_ = isMute;
    std::lock_guard<std::mutex> lock(chainManagerMutex_);
    int32_t ret = 0;
    for (const auto &[sceneType, enhanceChain] : sceneTypeToEnhanceChainMap_) {
        if (enhanceChain) {
            ret = enhanceChain->SetEnhanceParam(isMute_, systemVol_);
            CHECK_AND_RETURN_RET_LOG(ret == 0, ERROR,
                "[%{public}" PRIu64 "] set mute:%{public}d vol:%{public}f failed", sceneType, isMute_, systemVol_);
        }
    }
    AUDIO_INFO_LOG("success, isMute: %{public}d", isMute_);
    return SUCCESS;
}

int32_t AudioEnhanceChainManager::SetStreamVolumeInfo(const uint32_t &sessionId, const float &streamVol)
{
    sessionId_ = sessionId;
    streamVol_ = streamVol;
    if (sceneTypeAndModeToEnhanceChainNameMap_.size() == 0 || sceneTypeToEnhanceChainMap_.size() == 0) {
        return SUCCESS;
    }
    AUDIO_DEBUG_LOG("success, sessionId: %{public}d, streamVol: %{public}f", sessionId_, streamVol_);
    return SUCCESS;
}

int32_t AudioEnhanceChainManager::SetAudioEnhanceProperty(const AudioEffectPropertyArrayV3 &propertyArray,
    DeviceType deviceType)
{
    int32_t ret = SUCCESS;
    std::lock_guard<std::mutex> lock(chainManagerMutex_);
    for (const auto &property : propertyArray.property) {
        std::string key = "";
        enhancePropertyMap_.insert_or_assign(property.name, property.category);
        if (withDeviceEnhances_.find(property.name) == withDeviceEnhances_.end()) {
            key = property.name;
        } else {
            std::string deviceTypeName = "";
            GetDeviceTypeName(deviceType, deviceTypeName);
            key = property.name + "_&_" + deviceTypeName;
        }
        ret = WriteEnhancePropertyToDb(key, property.category);
        if (ret != SUCCESS) {
            AUDIO_ERR_LOG("fail, WriteEnhancePropertyToDb, ErrCode: %{public}d", ret);
            continue;
        }
        SetAudioEnhancePropertyToChains(property);
    }
    return SUCCESS;
}

int32_t AudioEnhanceChainManager::SetAudioEnhancePropertyToChains(AudioEffectPropertyV3 property)
{
    int32_t ret = SUCCESS;
    for (const auto &[sceneType, enhanceChain] : sceneTypeToEnhanceChainMap_) {
        if (enhanceChain) {
            AUDIO_DEBUG_LOG("effectClass->name %{public}s effectProp->category %{public}s",
                property.name.c_str(), property.category.c_str());
            ret = enhanceChain->SetEnhanceProperty(property.name, property.category);
            CHECK_AND_CONTINUE_LOG(ret == SUCCESS, "set property failed[%{public}d]", ret);
        }
    }
    return ret;
}

int32_t AudioEnhanceChainManager::SetAudioEnhanceProperty(const AudioEnhancePropertyArray &propertyArray,
    DeviceType deviceType)
{
    int32_t ret = SUCCESS;
    std::lock_guard<std::mutex> lock(chainManagerMutex_);
    for (const auto &property : propertyArray.property) {
        std::string key = "";
        enhancePropertyMap_.insert_or_assign(property.enhanceClass, property.enhanceProp);
        if (withDeviceEnhances_.find(property.enhanceClass) == withDeviceEnhances_.end()) {
            key = property.enhanceClass;
        } else {
            std::string deviceTypeName = "";
            GetDeviceTypeName(deviceType, deviceTypeName);
            key = property.enhanceClass + "_&_" + deviceTypeName;
        }
        ret = WriteEnhancePropertyToDb(key, property.enhanceProp);
        if (ret != SUCCESS) {
            AUDIO_ERR_LOG("fail, WriteEnhancePropertyToDb, ErrCode: %{public}d", ret);
            continue;
        }
        SetAudioEnhancePropertyToChains(property);
    }
    return SUCCESS;
}

int32_t AudioEnhanceChainManager::SetAudioEnhancePropertyToChains(AudioEnhanceProperty property)
{
    int32_t ret = SUCCESS;
    for (const auto &[sceneType, enhanceChain] : sceneTypeToEnhanceChainMap_) {
        if (enhanceChain) {
            ret = enhanceChain->SetEnhanceProperty(property.enhanceClass, property.enhanceProp);
            CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_OPERATION_FAILED, "set property failed");
        }
    }
    return ret;
}

int32_t AudioEnhanceChainManager::WriteEnhancePropertyToDb(const std::string &key, const std::string &property)
{
    AudioSettingProvider &settingProvider = AudioSettingProvider::GetInstance(AUDIO_POLICY_SERVICE_ID);
    CHECK_AND_RETURN_RET_LOG(settingProvider.CheckOsAccountReady(), ERROR, "os account not ready");
    ErrCode ret = settingProvider.PutStringValue(key, property, "system");
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_OPERATION_FAILED, "Write Enhance Property to Database failed");
    AUDIO_INFO_LOG("success, write Enhance_&_DeviceType:%{public}s is Property:%{public}s to Database",
        key.c_str(), property.c_str());
    return SUCCESS;
}

void AudioEnhanceChainManager::GetDeviceTypeName(DeviceType deviceType, std::string &deviceName)
{
    const std::unordered_map<DeviceType, std::string> &supportDeviceType = GetSupportedDeviceType();
    auto item = supportDeviceType.find(deviceType);
    if (item != supportDeviceType.end()) {
        deviceName = item->second;
    }
}

int32_t AudioEnhanceChainManager::GetAudioEnhanceProperty(AudioEffectPropertyArrayV3 &propertyArray,
    DeviceType deviceType)
{
    std::lock_guard<std::mutex> lock(chainManagerMutex_);
    propertyArray.property.clear();
    if (deviceType != DEVICE_TYPE_NONE) {
        UpdateEnhancePropertyMapFromDb(deviceType);
    }
    for (const auto &[effect, prop] : enhancePropertyMap_) {
        if (!prop.empty()) {
            AUDIO_DEBUG_LOG("effect->name %{public}s prop->category %{public}s", effect.c_str(), prop.c_str());
            propertyArray.property.emplace_back(AudioEffectPropertyV3{effect, prop, CAPTURE_EFFECT_FLAG});
        }
    }
    return SUCCESS;
}

int32_t AudioEnhanceChainManager::GetAudioEnhanceProperty(AudioEnhancePropertyArray &propertyArray,
    DeviceType deviceType)
{
    std::lock_guard<std::mutex> lock(chainManagerMutex_);
    propertyArray.property.clear();
    if (deviceType != DEVICE_TYPE_NONE) {
        UpdateEnhancePropertyMapFromDb(deviceType);
    }
    for (const auto &[effect, prop] : enhancePropertyMap_) {
        if (!prop.empty()) {
            propertyArray.property.emplace_back(AudioEnhanceProperty{effect, prop});
            AUDIO_INFO_LOG("effect %{public}s is now %{public}s mode",
                effect.c_str(), prop.c_str());
        }
    }
    return SUCCESS;
}

int32_t AudioEnhanceChainManager::ApplyAudioEnhanceChainDefault(const uint32_t captureId, uint32_t length)
{
    std::lock_guard<std::mutex> lock(chainManagerMutex_);
    CHECK_AND_RETURN_RET_LOG(enhanceBuffer_ != nullptr, ERROR, "enhance buffer is null");
    auto chainMapIter = captureId2DefaultChain_.find(captureId);
    if (chainMapIter == captureId2DefaultChain_.end() || chainMapIter->second == nullptr) {
        CHECK_AND_RETURN_RET_LOG(memcpy_s(enhanceBuffer_->micBufferOut.data(), enhanceBuffer_->micBufferOut.size(),
            enhanceBuffer_->micBufferIn.data(), length) == 0, ERROR, "memcpy error in apply enhance");
        AUDIO_ERR_LOG("Can not find %{public}u in captureId2DefaultChain_", captureId);
        return ERROR;
    }
    auto audioEnhanceChain = chainMapIter->second;
    if (audioEnhanceChain->ApplyEnhanceChain(enhanceBuffer_, length) != SUCCESS) {
        AUDIO_ERR_LOG("Apply default chain failed with captureId %{public}u.", captureId);
        return ERROR;
    }
    AUDIO_DEBUG_LOG("Apply default chain success with captureId %{public}u.", captureId);
    return SUCCESS;
}

void AudioEnhanceChainManager::UpdateExtraSceneType(const std::string &mainkey, const std::string &subkey,
    const std::string &extraSceneType)
{
    std::lock_guard<std::mutex> lock(chainManagerMutex_);
    if (mainkey == MAINKEY_DEVICE_STATUS && subkey == SUBKEY_FOLD_STATE) {
        AUDIO_INFO_LOG("Set fold state: %{public}s to arm", extraSceneType.c_str());
        uint32_t tempState = 0;
        auto result = std::from_chars(extraSceneType.data(), extraSceneType.data() + extraSceneType.size(), tempState);
        if (result.ec == std::errc() && result.ptr == (extraSceneType.data() + extraSceneType.size())) {
            foldState_ = tempState;
        } else {
            AUDIO_ERR_LOG("extraSceneType: %{public}s is invalid", extraSceneType.c_str());
            return;
        }

        for (const auto &[sceneType, enhanceChain] : sceneTypeToEnhanceChainMap_) {
            if (enhanceChain == nullptr) {
                continue;
            }
            if (enhanceChain->SetFoldState(foldState_) != SUCCESS) {
                AUDIO_WARNING_LOG("Set fold state to enhance chain failed");
                continue;
            }
        }
    } else {
        AUDIO_INFO_LOG("UpdateExtraSceneType failed, mainkey is %{public}s, subkey is %{public}s, "
            "extraSceneType is %{public}s", mainkey.c_str(), subkey.c_str(), extraSceneType.c_str());
        return;
    }
}

int32_t AudioEnhanceChainManager::SendInitCommand()
{
    std::lock_guard<std::mutex> lock(chainManagerMutex_);
    int32_t ret = 0;
    for (const auto &[sceneType, enhanceChain] : sceneTypeToEnhanceChainMap_) {
        if (enhanceChain) {
            ret = enhanceChain->InitCommand();
            CHECK_AND_RETURN_RET_LOG(ret == 0, ERROR, "[%{public}" PRIu64 "] set init enhance command failed",
                sceneType);
        }
    }
    AUDIO_INFO_LOG("SendInitCommand success");
    return SUCCESS;
}
} // namespace AudioStandard
} // namespace OHOS
