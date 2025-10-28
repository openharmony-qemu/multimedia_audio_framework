/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
#define LOG_TAG "AudioEnhanceChainManagerImpl"

#include "audio_enhance_chain_manager_impl.h"

#include <algorithm>
#include <charconv>
#include <system_error>

#include "securec.h"
#include "system_ability_definition.h"

#include "audio_effect_log.h"
#include "audio_errors.h"
#include "audio_effect.h"
#include "audio_enhance_chain.h"
#include "audio_setting_provider.h"
#include "audio_device_type.h"
#include "audio_effect_map.h"
#include "audio_utils.h"
#include "chain_pool.h"

namespace OHOS {
namespace AudioStandard {
namespace {
constexpr uint32_t SCENE_TYPE_OFFSET = 32;
constexpr uint32_t CAPTURER_ID_OFFSET = 16;
constexpr uint64_t SCENE_TYPE_MASK = 0xFF00000000;
constexpr uint64_t CAPTURER_ID_MASK = 0x0000FFFF0000;
constexpr uint32_t VOLUME_FACTOR = 100;
constexpr uint32_t BATTERY_CAPACITY = 100;
const std::vector<AudioEnhanceScene> AUDIO_WITH_DEVICE_ENHANCES = { SCENE_VOIP_UP };
const std::string MAINKEY_DEVICE_STATUS = "device_status";
const std::string SUBKEY_FOLD_STATE = "fold_state";
const std::string SUBKEY_POWER_STATE = "power_state";

const std::map<AudioEnhanceScene, uint32_t> SCENE_THREAD_ID_MAP = {
    { SCENE_VOIP_UP, 1 },
    { SCENE_RECORD, 2 },
    { SCENE_PRE_ENHANCE, 3 },
    { SCENE_ASR, 4 },
    { SCENE_VOICE_MESSAGE, 4 },
    { SCENE_NONE, 4 },
};

uint32_t GetThreadIdByScene(AudioEnhanceScene scene)
{
    auto iter = SCENE_THREAD_ID_MAP.find(scene);
    if (iter != SCENE_THREAD_ID_MAP.end()) {
        return iter->second;
    }

    return SCENE_THREAD_ID_MAP.at(SCENE_NONE);
}

std::shared_ptr<AudioEffectLibEntry> FindEnhanceLib(const std::string &enhanceName,
    const std::vector<std::shared_ptr<AudioEffectLibEntry>> &libList)
{
    for (const auto &lib : libList) {
        if (lib == nullptr) {
            continue;
        }
        auto iter = std::find(lib->effectName.begin(), lib->effectName.end(), enhanceName);
        if (iter != lib->effectName.end()) {
            return lib;
        }
    }

    return nullptr;
}
} // namespace

void AudioEnhanceChainManagerImpl::ResetInfo()
{
    enhanceConfigInfoMap_.clear();
    chainConfigInfoMap_.clear();
    enhancePropertyMap_.clear();
    captureIdToDeviceMap_.clear();
    threadHandlerMap_.clear();
    maxNormalInstanceNum_ = 0;
    defaultScene_ = "";
    priorSceneSet_.clear();
    enhancePara_ = {};
}

void AudioEnhanceChainManagerImpl::SetRelateWithDevicePropForEnhance()
{
    const auto &audioEnhanceSupportedSceneTypes = GetEnhanceSupportedSceneType();
    for (const auto& sceneType : AUDIO_WITH_DEVICE_ENHANCES) {
        std::string scene = audioEnhanceSupportedSceneTypes.find(sceneType)->second;
        std::string sceneAndMode = scene + "_&_" + "ENHANCE_DEFAULT";
        auto iter = chainConfigInfoMap_.find(sceneAndMode);
        CHECK_AND_CONTINUE_LOG(iter != chainConfigInfoMap_.end(),
            "no such sceneAndMode %{public}s", sceneAndMode.c_str());
        for (const auto &enhanceName : iter->second.enhanceNames) {
            auto propIter = enhancePropertyMap_.find(enhanceName);
            if (propIter == enhancePropertyMap_.end()) {
                continue;
            }
            auto enhanceIter = enhanceConfigInfoMap_.find(enhanceName);
            if (enhanceIter != enhanceConfigInfoMap_.end()) {
                enhanceIter->second.relateWithDevice = true;
            }
        }
    }
}

void AudioEnhanceChainManagerImpl::UpdateEnhancePropertyMapFromDb(DeviceType deviceType)
{
    std::string deviceTypeName = "";
    GetDeviceTypeName(deviceType, deviceTypeName);
    CHECK_AND_RETURN_LOG(deviceTypeName != "", "get deviceTypeName fail");
    AudioSettingProvider &settingProvider = AudioSettingProvider::GetInstance(AUDIO_POLICY_SERVICE_ID);
    CHECK_AND_RETURN_LOG(settingProvider.CheckOsAccountReady(), "os account not ready");
    for (auto &[enhance, prop] : enhancePropertyMap_) {
        std::string property = "";
        std::string key = enhance;
        auto iter = enhanceConfigInfoMap_.find(enhance);
        if (iter != enhanceConfigInfoMap_.end() && iter->second.relateWithDevice) {
            key = enhance + "_&_" + deviceTypeName;
        }
        ErrCode ret = settingProvider.GetStringValue(key, property, "system");
        if (ret == SUCCESS) {
            prop = property;
            AUDIO_INFO_LOG("Get Effect_&_DeviceType:%{public}s is Property:%{public}s", key.c_str(), property.c_str());
        } else {
            std::string defaultProp = "";
            if (iter != enhanceConfigInfoMap_.end()) {
                defaultProp = iter->second.defaultProp;
            }
            ret = settingProvider.PutStringValue(key, defaultProp, "system");
            if (ret != SUCCESS) {
                AUDIO_ERR_LOG("set default Property:%{public}s failed, ErrCode: %{public}d", defaultProp.c_str(), ret);
                return;
            }
            prop = defaultProp;
            AUDIO_INFO_LOG("Get prop failed,Effect_&_DeviceType:%{public}s is set to default Property:%{public}s",
                key.c_str(), prop.c_str());
        }
    }
}

void AudioEnhanceChainManagerImpl::InitAudioEnhanceChainManager(const std::vector<EffectChain> &enhanceChains,
    const EffectChainManagerParam &managerParam, const std::vector<std::shared_ptr<AudioEffectLibEntry>> &libList)
{
    std::set<std::string> enhanceNameSet;
    for (const auto &it : managerParam.sceneTypeToChainNameMap) {
        EnhanceChainConfigInfo info = {};
        info.chainName = it.second;
        for (const auto &chain : enhanceChains) {
            if (chain.name == info.chainName) {
                info.chainLabel = chain.label;
                info.enhanceNames = chain.apply;
                break;
            }
        }
        chainConfigInfoMap_.emplace(it.first, info);
        for (const auto &enhanceName : info.enhanceNames) {
            enhanceNameSet.emplace(enhanceName);
        }
    }

    for (const auto &enhanceName : enhanceNameSet) {
        EnhanceConfigInfo info = {};
        info.enhanceLib = FindEnhanceLib(enhanceName, libList);
        if (info.enhanceLib == nullptr) {
            continue;
        }
        auto iter = managerParam.effectDefaultProperty.find(enhanceName);
        if (iter != managerParam.effectDefaultProperty.end()) {
            info.defaultProp = iter->second;
        }
        info.relateWithDevice = false;
        enhanceConfigInfoMap_.emplace(enhanceName, info);
    }

    priorSceneSet_.insert(managerParam.priorSceneList.begin(), managerParam.priorSceneList.end());
    enhancePropertyMap_.insert(managerParam.effectDefaultProperty.begin(), managerParam.effectDefaultProperty.end());
    defaultScene_ = managerParam.defaultSceneName;
    maxNormalInstanceNum_ = managerParam.maxExtraNum;
    SetRelateWithDevicePropForEnhance();
}

int32_t AudioEnhanceChainManagerImpl::ParseSceneKeyCode(uint64_t sceneKeyCode, std::string &sceneType,
    std::string &capturerDeviceStr, std::string &rendererDeviceStr)
{
    uint32_t sceneCode = (sceneKeyCode & SCENE_TYPE_MASK) >> SCENE_TYPE_OFFSET;
    AUDIO_INFO_LOG("sceneKeyCode = %{public}" PRIu64 ", sceneCode = %{public}u", sceneKeyCode, sceneCode);
    AudioEnhanceScene scene = static_cast<AudioEnhanceScene>(sceneCode);
    DeviceType capturerDevice = DEVICE_TYPE_INVALID;
    const std::unordered_map<DeviceType, std::string> &supportDeviceType = GetSupportedDeviceType();
    const auto &audioEnhanceSupportedSceneTypes = GetEnhanceSupportedSceneType();
    auto item = audioEnhanceSupportedSceneTypes.find(scene);
    if (item != audioEnhanceSupportedSceneTypes.end()) {
        sceneType = item->second;
    } else {
        AUDIO_ERR_LOG("scene[%{public}d] not be supported", scene);
        return ERROR;
    }
    uint32_t captureId = (sceneKeyCode & CAPTURER_ID_MASK) >> CAPTURER_ID_OFFSET;
    auto capItem = captureIdToDeviceMap_.find(captureId);
    if (capItem != captureIdToDeviceMap_.end()) {
        capturerDevice = capItem->second;
    } else {
        AUDIO_ERR_LOG("can't find captureId[%{public}u] in captureIdToDeviceMap_", captureId);
        return ERROR;
    }

    DeviceType rendererDevice = DEVICE_TYPE_NONE;

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

std::shared_ptr<ThreadHandler> AudioEnhanceChainManagerImpl::GetThreadHandlerByScene(AudioEnhanceScene scene)
{
    auto threadId = GetThreadIdByScene(scene);
    auto iter = threadHandlerMap_.find(threadId);
    if (iter != threadHandlerMap_.end()) {
        ++iter->second.second;
        return iter->second.first;
    }

    std::string threadName = "OS_ProCap" + std::to_string(scene);
    Trace trace("CreateThread: " + std::to_string(scene));
    auto threadHandler = ThreadHandler::NewInstance(threadName);
    if (threadHandler == nullptr) {
        AUDIO_ERR_LOG("create thread handler fail");
        return nullptr;
    }
    uint32_t useCount = 0;
    threadHandlerMap_.emplace(threadId, std::make_pair(threadHandler, ++useCount));

    return threadHandler;
}

void AudioEnhanceChainManagerImpl::ReleaseThreadHandlerByScene(AudioEnhanceScene scene)
{
    auto threadId = GetThreadIdByScene(scene);
    auto iter = threadHandlerMap_.find(threadId);
    if (iter != threadHandlerMap_.end() && --iter->second.second > 0) {
        AUDIO_INFO_LOG("threadId: %{public}u useCount: %{public}u", threadId, iter->second.second);
    } else {
        Trace trace("DeleteThread: " + std::to_string(scene));
        threadHandlerMap_.erase(threadId);
    }
}

int32_t AudioEnhanceChainManagerImpl::CreateAudioEnhanceChainDynamic(uint64_t sceneKeyCode,
    const AudioEnhanceDeviceAttr &deviceAttr)
{
    CHECK_AND_RETURN_RET_LOG(chainConfigInfoMap_.size() != 0, ERROR, "no enhance chain config info");

    std::lock_guard<std::mutex> lock(chainManagerMutex_);
    auto chain = ChainPool::GetInstance().GetChainById(sceneKeyCode);
    if (chain != nullptr) {
        AUDIO_INFO_LOG("chain: %{public}" PRIu64 " is exist", sceneKeyCode);
        return SUCCESS;
    }

    auto newChain = CreateEnhanceChainInner(sceneKeyCode, deviceAttr);
    CHECK_AND_RETURN_RET_LOG(newChain != nullptr, ERROR, "CreateEnhanceChainInner fail");

    bool defaultFlag = (newChain->GetScenePriority() == DEFAULT_SCENE);
    auto enhanceNames = GetEnhanceNamesBySceneCode(sceneKeyCode, defaultFlag);
    CHECK_AND_RETURN_RET_LOG(enhanceNames.size() != 0, ERROR, "enhanceNames is empty");

    auto scene = static_cast<AudioEnhanceScene>((sceneKeyCode & SCENE_TYPE_MASK) >> SCENE_TYPE_OFFSET);
    auto handler = GetThreadHandlerByScene(scene);
    CHECK_AND_RETURN_RET_LOG(handler != nullptr, ERROR, "handler is null");
    newChain->SetThreadHandler(handler);

    if (AddAudioEnhanceChainHandles(newChain, enhanceNames) != SUCCESS) {
        AUDIO_ERR_LOG("chain: %{public}" PRIu64 " create failed", sceneKeyCode);
        ReleaseThreadHandlerByScene(scene);
        return ERROR;
    }

    ChainPool::GetInstance().AddChain(newChain);
    AUDIO_INFO_LOG("chain: %{public}" PRIu64 " create success", sceneKeyCode);

    return SUCCESS;
}

std::vector<std::string> AudioEnhanceChainManagerImpl::GetEnhanceNamesBySceneCode(uint64_t sceneKeyCode,
    bool defaultFlag)
{
    std::string sceneType = "";
    std::string capturerDevice = "";
    std::string rendererDeivce = "";
    auto parseSceneKeyCodeRet = ParseSceneKeyCode(sceneKeyCode, sceneType, capturerDevice, rendererDeivce);
    CHECK_AND_RETURN_RET_LOG(parseSceneKeyCodeRet == SUCCESS, {}, "ParseSceneKeyCode fail");

    if (defaultFlag) {
        AUDIO_INFO_LOG("sceneType %{public}s set to defaultScene %{public}s", sceneType.c_str(), defaultScene_.c_str());
        sceneType = defaultScene_;
    }
    // first check specific device, then check no device
    std::string enhanceChainKey = sceneType + "_&_" + "ENHANCE_DEFAULT" + "_&_" + capturerDevice;
    auto mapIter = chainConfigInfoMap_.find(enhanceChainKey);
    if (mapIter == chainConfigInfoMap_.end()) {
        enhanceChainKey = sceneType + "_&_" + "ENHANCE_DEFAULT";
        mapIter = chainConfigInfoMap_.find(enhanceChainKey);
    }
    if (mapIter == chainConfigInfoMap_.end()) {
        AUDIO_ERR_LOG("EnhanceChain key [%{public}s] does not exist", enhanceChainKey.c_str());
        return {};
    } else {
        return mapIter->second.enhanceNames;
    }
}

std::shared_ptr<AudioEnhanceChain> AudioEnhanceChainManagerImpl::CreateEnhanceChainInner(uint64_t sceneKeyCode,
    const AudioEnhanceDeviceAttr &deviceAttr)
{
    std::string sceneType = "";
    std::string capturerDevice = "";
    std::string rendererDeivce = "";
    auto parseSceneKeyCodeRet = ParseSceneKeyCode(sceneKeyCode, sceneType, capturerDevice, rendererDeivce);
    CHECK_AND_RETURN_RET_LOG(parseSceneKeyCodeRet == SUCCESS, nullptr, "ParseSceneKeyCode fail");

    AudioEnhanceParamAdapter enhancePara = { enhancePara_.muteInfo, enhancePara_.volumeInfo, enhancePara_.foldState,
        enhancePara_.powerState, capturerDevice, rendererDeivce, sceneType, "" };
    if (priorSceneSet_.find(sceneType) != priorSceneSet_.end()) {
        AUDIO_INFO_LOG("scene: %{public}s create prior enhance chain", sceneType.c_str());
        return std::make_shared<AudioEnhanceChain>(sceneKeyCode, sceneType, PRIOR_SCENE, enhancePara, deviceAttr);
    }

    uint32_t normalSceneNum = 0;
    auto chainArray = ChainPool::GetInstance().GetAllChain();
    for (const auto &chain : chainArray) {
        if (chain != nullptr && chain->GetScenePriority() == NORMAL_SCENE) {
            ++normalSceneNum;
        }
    }

    std::shared_ptr<AudioEnhanceChain> chain = nullptr;
    if (normalSceneNum >= maxNormalInstanceNum_) {
        AUDIO_INFO_LOG("scene: %{public}s create default enhance chain", sceneType.c_str());
        enhancePara.sceneType = defaultScene_;
        chain = std::make_shared<AudioEnhanceChain>(sceneKeyCode, defaultScene_, DEFAULT_SCENE,
            enhancePara, deviceAttr);
    } else {
        AUDIO_INFO_LOG("scene: %{public}s create normal enhance chain", sceneType.c_str());
        chain = std::make_shared<AudioEnhanceChain>(sceneKeyCode, sceneType, NORMAL_SCENE, enhancePara, deviceAttr);
    }

    return chain;
}

int32_t AudioEnhanceChainManagerImpl::AddAudioEnhanceChainHandles(std::shared_ptr<AudioEnhanceChain> &audioEnhanceChain,
    const std::vector<std::string> &enhanceNames)
{
    std::vector<EnhanceModulePara> moduleParas;
    for (const auto &enhance : enhanceNames) {
        EnhanceModulePara para = {};
        para.enhanceName = enhance;
        auto enhanceIter = enhanceConfigInfoMap_.find(enhance);
        if (enhanceIter != enhanceConfigInfoMap_.end() && enhanceIter->second.enhanceLib != nullptr) {
            para.libName = enhanceIter->second.enhanceLib->libraryName;
            para.libHandle = enhanceIter->second.enhanceLib->audioEffectLibHandle;
        }
        if (auto iter = enhancePropertyMap_.find(enhance); iter != enhancePropertyMap_.end()) {
            para.enhanceProp = iter->second;
        }

        moduleParas.emplace_back(para);
    }

    int32_t createAllModuleRet = audioEnhanceChain->CreateAllEnhanceModule(moduleParas);
    CHECK_AND_RETURN_RET_LOG(createAllModuleRet == SUCCESS, ERROR, "CreateAllEnhanceModule fail");

    return SUCCESS;
}

int32_t AudioEnhanceChainManagerImpl::ReleaseAudioEnhanceChainDynamic(uint64_t sceneKeyCode)
{
    std::lock_guard<std::mutex> lock(chainManagerMutex_);

    auto chain = ChainPool::GetInstance().GetChainById(sceneKeyCode);
    if (chain != nullptr) {
        chain->ReleaseAllEnhanceModule();
    }
    AUDIO_INFO_LOG("release chain: %{public}" PRIu64, sceneKeyCode);
    ChainPool::GetInstance().DeleteChain(sceneKeyCode);

    auto scene = static_cast<AudioEnhanceScene>((sceneKeyCode & SCENE_TYPE_MASK) >> SCENE_TYPE_OFFSET);
    ReleaseThreadHandlerByScene(scene);

    return SUCCESS;
}

int32_t AudioEnhanceChainManagerImpl::AudioEnhanceChainGetAlgoConfig(uint64_t sceneKeyCode,
    AudioBufferConfig &micConfig, AudioBufferConfig &ecConfig, AudioBufferConfig &micRefConfig)
{
    auto chain = ChainPool::GetInstance().GetChainById(sceneKeyCode);
    if (chain == nullptr) {
        AUDIO_ERR_LOG("chain: %{public}" PRIu64 " is not found", sceneKeyCode);
        return ERROR;
    }

    chain->GetAlgoConfig(micConfig, ecConfig, micRefConfig);

    return SUCCESS;
}

int32_t AudioEnhanceChainManagerImpl::ApplyEnhanceChainById(uint64_t sceneKeyCode, const EnhanceTransBuffer &transBuf)
{
    auto chain = ChainPool::GetInstance().GetChainById(sceneKeyCode);
    if (chain == nullptr) {
        AUDIO_ERR_LOG("chain: %{public}" PRIu64 " is not found", sceneKeyCode);
        return ERROR;
    }

    auto applyChainRet = chain->ApplyEnhanceChain(transBuf);
    if (applyChainRet != SUCCESS) {
        AUDIO_ERR_LOG("apply chain: %{public}" PRIu64 " fail", sceneKeyCode);
        return ERROR;
    }

    return SUCCESS;
}

int32_t AudioEnhanceChainManagerImpl::GetChainOutputDataById(uint64_t sceneKeyCode, void *buf, size_t bufSize)
{
    auto chain = ChainPool::GetInstance().GetChainById(sceneKeyCode);
    if (chain == nullptr) {
        AUDIO_ERR_LOG("chain: %{public}" PRIu64 " is not found", sceneKeyCode);
        return ERROR;
    }

    auto getChainOutputRet = chain->GetOutputDataFromChain(buf, bufSize);
    if (getChainOutputRet != SUCCESS) {
        AUDIO_ERR_LOG("get chain %{public}" PRIu64 " output fail", sceneKeyCode);
        return ERROR;
    }

    return SUCCESS;
}

int32_t AudioEnhanceChainManagerImpl::UpdatePropertyAndSendToAlgo(DeviceType inputDevice)
{
    UpdateEnhancePropertyMapFromDb(inputDevice);
    auto chainArray = ChainPool::GetInstance().GetAllChain();
    for (const auto &[enhance, prop] : enhancePropertyMap_) {
        for (const auto &chain : chainArray) {
            if (chain != nullptr) {
                int32_t ret = chain->SetEnhanceProperty(enhance, prop);
                CHECK_AND_RETURN_RET_LOG(ret == 0, ERR_OPERATION_FAILED, "set property failed");
            }
        }
    }
    return SUCCESS;
}

int32_t AudioEnhanceChainManagerImpl::SetInputDevice(uint32_t captureId, DeviceType inputDevice,
    const std::string &deviceName)
{
    const auto &supportDeviceType = GetSupportedDeviceType();
    std::lock_guard<std::mutex> lock(chainManagerMutex_);
    auto item = captureIdToDeviceMap_.find(captureId);
    if (item == captureIdToDeviceMap_.end()) {
        captureIdToDeviceMap_[captureId] = inputDevice;
        AUDIO_INFO_LOG("set new inputdevice, captureId: %{public}u, inputDevice: %{public}d", captureId, inputDevice);
        return SUCCESS;
    }
    if (item->second == inputDevice) {
        AUDIO_INFO_LOG("set same device, captureId: %{public}u, inputDevice: %{public}d", captureId, inputDevice);
        return SUCCESS;
    }

    captureIdToDeviceMap_[captureId] = inputDevice;
    std::string inputDeviceStr = "";

    if (auto iter = supportDeviceType.find(inputDevice); iter != supportDeviceType.end()) {
        inputDeviceStr = iter->second;
    } else {
        return ERROR;
    }

    auto chainArray = ChainPool::GetInstance().GetAllChain();
    for (const auto &chain : chainArray) {
        if (chain == nullptr) {
            continue;
        }
        uint64_t sceneKeyCode = chain->GetChainId();
        uint32_t tempCaptureId = (sceneKeyCode & CAPTURER_ID_MASK) >> CAPTURER_ID_OFFSET;
        if (tempCaptureId == captureId) {
            if (chain->SetInputDevice(inputDeviceStr, deviceName) != SUCCESS) {
                AUDIO_ERR_LOG("chain:%{public}" PRIu64 " set input device failed", sceneKeyCode);
            }
        }
    }

    if (UpdatePropertyAndSendToAlgo(inputDevice) != SUCCESS) {
        return ERROR;
    }
    AUDIO_INFO_LOG("success, captureId: %{public}u, inputDevice: %{public}d deviceName:%{public}s",
        captureId, inputDevice, deviceName.c_str());
    return SUCCESS;
}

int32_t AudioEnhanceChainManagerImpl::SetOutputDevice(uint32_t renderId, DeviceType outputDevice)
{
    static_cast<void>(renderId);
    static_cast<void>(outputDevice);

    return SUCCESS;
}

int32_t AudioEnhanceChainManagerImpl::SetVolumeInfo(AudioVolumeType volumeType, float systemVol)
{
    AUDIO_DEBUG_LOG("success, volumeType: %{public}d, systemVol: %{public}f", volumeType, systemVol);

    std::lock_guard<std::mutex> lock(chainManagerMutex_);
    enhancePara_.volumeInfo = static_cast<uint32_t>(systemVol * VOLUME_FACTOR);
    static_cast<void>(volumeType);

    return SUCCESS;
}

int32_t AudioEnhanceChainManagerImpl::SetMicrophoneMuteInfo(bool isMute)
{
    {
        std::lock_guard<std::mutex> lock(chainManagerMutex_);
        enhancePara_.muteInfo = static_cast<uint32_t>(isMute);
    }
    uint32_t volumeInfo = enhancePara_.volumeInfo;
    auto chainArray = ChainPool::GetInstance().GetAllChain();
    for (const auto &chain : chainArray) {
        if (chain != nullptr) {
            int32_t ret = chain->SetEnhanceParam(isMute, volumeInfo);
            CHECK_AND_RETURN_RET_LOG(ret == 0, ERROR,
                "[%{public}" PRIu64 "] set mute:%{public}d fail", chain->GetChainId(), static_cast<int32_t>(isMute));
        }
    }
    AUDIO_INFO_LOG("success, isMute: %{public}d", static_cast<int32_t>(isMute));

    return SUCCESS;
}

int32_t AudioEnhanceChainManagerImpl::SetStreamVolumeInfo(uint32_t sessionId, float streamVol)
{
    AUDIO_DEBUG_LOG("success, sessionId: %{public}u, streamVol: %{public}f", sessionId, streamVol);

    return SUCCESS;
}

int32_t AudioEnhanceChainManagerImpl::SetAudioEnhanceProperty(const AudioEffectPropertyArrayV3 &propertyArray,
    DeviceType deviceType)
{
    int32_t ret = ERROR;
    std::lock_guard<std::mutex> lock(chainManagerMutex_);
    for (const auto &property : propertyArray.property) {
        std::string key = "";
        enhancePropertyMap_.insert_or_assign(property.name, property.category);
        auto iter = enhanceConfigInfoMap_.find(property.name);
        if (iter == enhanceConfigInfoMap_.end() || !iter->second.relateWithDevice) {
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

int32_t AudioEnhanceChainManagerImpl::SetAudioEnhancePropertyToChains(const AudioEffectPropertyV3 &property)
{
    auto chainArray = ChainPool::GetInstance().GetAllChain();
    for (const auto &chain : chainArray) {
        if (chain != nullptr) {
            AUDIO_DEBUG_LOG("effectClass->name %{public}s effectProp->category %{public}s",
                property.name.c_str(), property.category.c_str());
            int32_t ret = chain->SetEnhanceProperty(property.name, property.category);
            CHECK_AND_CONTINUE_LOG(ret == SUCCESS, "set property failed[%{public}d]", ret);
        }
    }

    return SUCCESS;
}

int32_t AudioEnhanceChainManagerImpl::SetAudioEnhanceProperty(const AudioEnhancePropertyArray &propertyArray,
    DeviceType deviceType)
{
    AudioEffectPropertyArrayV3 localPropertyArray = {};
    for (const auto &prop : propertyArray.property) {
        AudioEffectPropertyV3 propertyV3 = { prop.enhanceClass, prop.enhanceProp, CAPTURE_EFFECT_FLAG };
        localPropertyArray.property.emplace_back(std::move(propertyV3));
    }

    return SetAudioEnhanceProperty(localPropertyArray, deviceType);
}

int32_t AudioEnhanceChainManagerImpl::WriteEnhancePropertyToDb(const std::string &key, const std::string &property)
{
    AudioSettingProvider &settingProvider = AudioSettingProvider::GetInstance(AUDIO_POLICY_SERVICE_ID);
    CHECK_AND_RETURN_RET_LOG(settingProvider.CheckOsAccountReady(), ERROR, "os account not ready");
    ErrCode ret = settingProvider.PutStringValue(key, property, "system");
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_OPERATION_FAILED, "Write Enhance Property to Database failed");
    AUDIO_INFO_LOG("success, write Enhance_&_DeviceType:%{public}s is Property:%{public}s to Database",
        key.c_str(), property.c_str());
    return SUCCESS;
}

void AudioEnhanceChainManagerImpl::GetDeviceTypeName(DeviceType deviceType, std::string &deviceName)
{
    const std::unordered_map<DeviceType, std::string> &supportDeviceType = GetSupportedDeviceType();
    auto item = supportDeviceType.find(deviceType);
    if (item != supportDeviceType.end()) {
        deviceName = item->second;
    }
}

int32_t AudioEnhanceChainManagerImpl::GetAudioEnhanceProperty(AudioEffectPropertyArrayV3 &propertyArray,
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

int32_t AudioEnhanceChainManagerImpl::GetAudioEnhanceProperty(AudioEnhancePropertyArray &propertyArray,
    DeviceType deviceType)
{
    propertyArray.property.clear();
    AudioEffectPropertyArrayV3 localPropertyArray = {};
    GetAudioEnhanceProperty(localPropertyArray, deviceType);
    for (const auto &prop : localPropertyArray.property) {
        AudioEnhanceProperty property = { prop.name, prop.category };
        propertyArray.property.emplace_back(std::move(property));
    }

    return SUCCESS;
}

void AudioEnhanceChainManagerImpl::UpdateExtraSceneType(const std::string &mainkey, const std::string &subkey,
    const std::string &extraSceneType)
{
    uint32_t tempState = 0;
    if (extraSceneType.empty()) {
        AUDIO_WARNING_LOG("extraSceneTypen is empty string!");
        return;
    }
    auto result = std::from_chars(extraSceneType.data(), extraSceneType.data() + extraSceneType.size(), tempState);
    if (result.ec != std::errc() || result.ptr != (extraSceneType.data() + extraSceneType.size())) {
        AUDIO_ERR_LOG("extraSceneType: %{public}s is invalid", extraSceneType.c_str());
        return;
    }

    if (mainkey == MAINKEY_DEVICE_STATUS && subkey == SUBKEY_FOLD_STATE &&
        tempState >= FOLD_STATE_EXPAND && tempState <= FOLD_STATE_MIDDLE) {
        AUDIO_INFO_LOG("Set fold state: %{public}s to arm", extraSceneType.c_str());
        {
            std::lock_guard<std::mutex> lockFold(chainManagerMutex_);
            enhancePara_.foldState = tempState;
        }
        SendFoldStateToChain(tempState);
    } else if (mainkey == MAINKEY_DEVICE_STATUS && subkey == SUBKEY_POWER_STATE &&
        tempState >= 0 && tempState <= BATTERY_CAPACITY) {
        {
            std::lock_guard<std::mutex> lockPower(chainManagerMutex_);
            enhancePara_.powerState = tempState;
        }
        SendPowerStateToChain(tempState);
    } else {
        AUDIO_INFO_LOG("UpdateExtraSceneType failed, mainkey is %{public}s, subkey is %{public}s, "
            "extraSceneType is %{public}s", mainkey.c_str(), subkey.c_str(), extraSceneType.c_str());
        return;
    }
}

void AudioEnhanceChainManagerImpl::SendFoldStateToChain(uint32_t foldState)
{
    auto chainArray = ChainPool::GetInstance().GetAllChain();
    for (const auto &chain : chainArray) {
        CHECK_AND_CONTINUE(chain != nullptr);
        JUDGE_AND_WARNING_LOG(chain->SetFoldState(foldState) != SUCCESS,
            "Set fold state to enhance chain failed");
    }
}

void AudioEnhanceChainManagerImpl::SendPowerStateToChain(uint32_t powerState)
{
    auto chainArray = ChainPool::GetInstance().GetAllChain();
    for (const auto &chain : chainArray) {
        CHECK_AND_CONTINUE(chain != nullptr);
        JUDGE_AND_WARNING_LOG(chain->SetPowerState(powerState) != SUCCESS,
            "Set power state to enhance chain failed");
    }
}

int32_t AudioEnhanceChainManagerImpl::SendInitCommand()
{
    auto chainArray = ChainPool::GetInstance().GetAllChain();
    for (const auto &chain : chainArray) {
        if (chain != nullptr) {
            int32_t ret = chain->InitCommand();
            CHECK_AND_RETURN_RET_LOG(ret == 0, ERROR,
                "chain: [%{public}" PRIu64 "] set init command fail", chain->GetChainId());
        }
    }
    AUDIO_INFO_LOG("success");
    return SUCCESS;
}

AudioEnhanceChainManager *AudioEnhanceChainManager::GetInstance()
{
    static AudioEnhanceChainManagerImpl impl;
    return &impl;
}
} // namespace AudioStandard
} // namespace OHOS