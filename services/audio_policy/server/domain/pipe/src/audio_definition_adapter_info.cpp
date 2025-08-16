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
#ifndef LOG_TAG
#define LOG_TAG "AudioDefinitionAdapterInfo"
#endif

#include "audio_definition_adapter_info.h"
#include "audio_device_type.h"
#include "audio_effect.h"

namespace OHOS {
namespace AudioStandard {
AudioPolicyConfigData& AudioPolicyConfigData::GetInstance()
{
    static AudioPolicyConfigData instance;
    return instance;
}

void AudioPolicyConfigData::SetDeviceInfoMap(std::list<std::shared_ptr<AdapterDeviceInfo>> &deviceInfos,
    std::unordered_map<std::string, std::shared_ptr<AdapterDeviceInfo>> &tmpDeviceInfoMap_)
{
    if (deviceInfos.size() <= 0) {
        AUDIO_ERR_LOG("SetDeviceInfoMap failed, deviceInfos is empty");
    }

    for (auto &deviceInfo : deviceInfos) {
        std::pair<DeviceType, DeviceRole> deviceMapKey = std::make_pair(deviceInfo->type_, deviceInfo->role_);
        auto deviceInfoIt = deviceInfoMap.find(deviceMapKey);
        if (deviceInfoIt != deviceInfoMap.end()) {
            deviceInfoIt->second.insert(deviceInfo);
        } else {
            std::set<std::shared_ptr<AdapterDeviceInfo>> deviceSet;
            deviceSet.insert(deviceInfo);
            deviceInfoMap.insert({deviceMapKey, deviceSet});
        }
        tmpDeviceInfoMap_.insert({deviceInfo->name_, deviceInfo});
    }
}

void AudioPolicyConfigData::SetSupportDeviceAndPipeMap(std::shared_ptr<AdapterPipeInfo> &pipeInfo_,
    std::unordered_map<std::string, std::shared_ptr<AdapterDeviceInfo>> &tmpDeviceInfoMap_)
{
    if (pipeInfo_->streamPropInfos_.size() == 0) {
        AUDIO_INFO_LOG("dynamic setting");
        for (auto deviceName : pipeInfo_->supportDevices_) {
            auto uniqueDeviceIt = tmpDeviceInfoMap_.find(deviceName);
            CHECK_AND_CONTINUE_LOG(uniqueDeviceIt != tmpDeviceInfoMap_.end(),
                "pipe needed device:%{public}s not exists.", deviceName.c_str());
            uniqueDeviceIt->second->supportPipeMap_.insert({pipeInfo_->supportFlags_, pipeInfo_});
        }
        return;
    }
    for (auto &streamPropInfo : pipeInfo_->streamPropInfos_) {
        for (auto deviceName : streamPropInfo->supportDevices_) {
            auto uniqueDeviceIt = tmpDeviceInfoMap_.find(deviceName);
            if (uniqueDeviceIt == tmpDeviceInfoMap_.end()) {
                AUDIO_WARNING_LOG("streamProp needed device:%{public}s not exists.", deviceName.c_str());
                continue;
            }
            streamPropInfo->supportDeviceMap_.insert({uniqueDeviceIt->second->type_, uniqueDeviceIt->second});
            uniqueDeviceIt->second->supportPipeMap_.insert({pipeInfo_->supportFlags_, pipeInfo_});
        }
    }
}

void AudioPolicyConfigData::Reorganize()
{
    std::unordered_map<std::string, std::shared_ptr<AdapterDeviceInfo>> tmpDeviceInfoMap;

    for (auto &pair : adapterInfoMap) {
        SetDeviceInfoMap(pair.second->deviceInfos, tmpDeviceInfoMap);
    }

    for (auto &pair : adapterInfoMap) {
        if (pair.second->pipeInfos.size() <= 0) {
            AUDIO_ERR_LOG("Set Pipes failed, pipeInfos is empty");
        }
        for (auto &pipeInfo_ : pair.second->pipeInfos) {
            SetSupportDeviceAndPipeMap(pipeInfo_, tmpDeviceInfoMap);
        }
    }

    SelfCheck();
}

void AudioPolicyConfigData::SelfCheck()
{
    CHECK_AND_RETURN_LOG(adapterInfoMap.size() != 0, "SelfCheck Failled! Config No Adapter!");
    CHECK_AND_RETURN_LOG(deviceInfoMap.size() != 0, "SelfCheck Failled! Config No Device!");

    for (auto &pair : adapterInfoMap) {
        pair.second->SelfCheck();
    }
}

void AudioPolicyConfigData::SetVersion(const std::string &version)
{
    if (!version.empty()) {
        version_ = version;
    } else {
        AUDIO_ERR_LOG("Set version failed, data is empty");
    }
}

std::string AudioPolicyConfigData::GetVersion()
{
    return version_;
}

std::shared_ptr<AdapterDeviceInfo> AudioPolicyConfigData::GetAdapterDeviceInfo(
    DeviceType type_, DeviceRole role_, const std::string &networkId_, uint32_t flags, int32_t a2dpOffloadFlag)
{
    // use primary to select device when in remote cast;
    DeviceType tempType = (type_ == DEVICE_TYPE_REMOTE_CAST ? DEVICE_TYPE_SPEAKER : type_);
    std::pair<DeviceType, DeviceRole> deviceMapKey = std::make_pair(tempType, role_);
    auto deviceSetIt = deviceInfoMap.find(deviceMapKey);
    if (deviceSetIt == deviceInfoMap.end()) {
        AUDIO_ERR_LOG("Device Not Configured!");
        return nullptr;
    }

    if (deviceSetIt->second.empty()) {
        AUDIO_ERR_LOG("Device Set Is Empty!");
        return nullptr;
    }

    if (deviceSetIt->second.size() == 1) {
        return *(deviceSetIt->second.begin());
    }

    std::string targetAdapterName = "";
    if (networkId_ != LOCAL_NETWORK_ID) {
        targetAdapterName = "remote";
    } else {
        if (type_ == DEVICE_TYPE_BLUETOOTH_A2DP && a2dpOffloadFlag != A2DP_OFFLOAD) {
            targetAdapterName = "a2dp";
        } else {
            targetAdapterName = "primary";
        }
    }

    for (auto &deviceInfo : deviceSetIt->second) {
        std::shared_ptr<PolicyAdapterInfo> adapterInfoPtr = deviceInfo->adapterInfo_.lock();
        if (adapterInfoPtr == nullptr) {
            AUDIO_ERR_LOG("AdapterInfo is nullptr!");
            continue;
        }
        if (adapterInfoPtr->adapterName == targetAdapterName) {
            return deviceInfo;
        }
    }
    AUDIO_ERR_LOG("Can not match any Device!");
    return nullptr;
}

void AudioPolicyConfigData::UpdateDynamicStreamProps(const std::string adapterName, const std::string &pipeName,
    const std::list<std::shared_ptr<PipeStreamPropInfo>> &streamProps)
{
    CHECK_AND_RETURN_LOG(!streamProps.empty(), "streamProps is empty");
    AudioAdapterType adapterType = PolicyAdapterInfo::GetAdapterType(adapterName);
    CHECK_AND_RETURN_LOG(adapterInfoMap.count(adapterType) != 0, "adapter not exist");
    std::shared_ptr<PolicyAdapterInfo> adapterInfo = adapterInfoMap[adapterType];
    CHECK_AND_RETURN_LOG(adapterInfo != nullptr, "adapterInfo is nullptr");
    std::shared_ptr<AdapterPipeInfo> pipeInfo = adapterInfo->GetPipeInfoByName(pipeName);
    CHECK_AND_RETURN_LOG(pipeInfo != nullptr, "pipeInfo is nullptr");

    std::unordered_map<std::string, std::shared_ptr<AdapterDeviceInfo>> tmpDeviceInfoMap;
    for (auto &deviceInfo : adapterInfo->deviceInfos) {
        tmpDeviceInfoMap.insert({deviceInfo->name_, deviceInfo});
    }
    for (auto &streamProp : streamProps) {
        CHECK_AND_RETURN_LOG(streamProp != nullptr, "streamProp is nullptr");
        streamProp->pipeInfo_ = pipeInfo;
        for (auto deviceName : streamProp->supportDevices_) {
            auto uniqueDeviceIt = tmpDeviceInfoMap.find(deviceName);
            CHECK_AND_CONTINUE_LOG(uniqueDeviceIt != tmpDeviceInfoMap.end(),
                "streamProp needed device %{public}s not exist", deviceName.c_str());
            streamProp->supportDeviceMap_.insert({uniqueDeviceIt->second->type_, uniqueDeviceIt->second});
        }
    }
    pipeInfo->UpdateDynamicStreamProps(streamProps);
}

void AudioPolicyConfigData::ClearDynamicStreamProps(const std::string adapterName, const std::string &pipeName)
{
    AudioAdapterType adapterType = PolicyAdapterInfo::GetAdapterType(adapterName);
    CHECK_AND_RETURN_LOG(adapterInfoMap.count(adapterType) != 0, "adapter not exist");
    std::shared_ptr<PolicyAdapterInfo> adapterInfo = adapterInfoMap[adapterType];
    CHECK_AND_RETURN_LOG(adapterInfo != nullptr, "adapterInfo is nullptr");
    std::shared_ptr<AdapterPipeInfo> pipeInfo = adapterInfo->GetPipeInfoByName(pipeName);
    CHECK_AND_RETURN_LOG(pipeInfo != nullptr, "pipeInfo is nullptr");
    pipeInfo->ClearDynamicStreamProps();
}

uint32_t AudioPolicyConfigData::GetConfigStreamPropsSize(const std::string adapterName,
    const std::string &pipeName) const
{
    AudioAdapterType adapterType = PolicyAdapterInfo::GetAdapterType(adapterName);
    auto it = adapterInfoMap.find(adapterType);
    CHECK_AND_RETURN_RET_LOG(it != adapterInfoMap.end(), 0, "adapter not exist");
    std::shared_ptr<PolicyAdapterInfo> adapterInfo = it->second;
    CHECK_AND_RETURN_RET_LOG(adapterInfo != nullptr, 0, "adapterInfo is nullptr");
    std::shared_ptr<AdapterPipeInfo> pipeInfo = adapterInfo->GetPipeInfoByName(pipeName);
    CHECK_AND_RETURN_RET_LOG(pipeInfo != nullptr, 0, "pipeInfo is nullptr");
    return pipeInfo->streamPropInfos_.size();
}

uint32_t AudioPolicyConfigData::GetDynamicStreamPropsSize(const std::string adapterName,
    const std::string &pipeName) const
{
    AudioAdapterType adapterType = PolicyAdapterInfo::GetAdapterType(adapterName);
    auto it = adapterInfoMap.find(adapterType);
    CHECK_AND_RETURN_RET_LOG(it != adapterInfoMap.end(), 0, "adapter not exist");
    std::shared_ptr<PolicyAdapterInfo> adapterInfo = it->second;
    CHECK_AND_RETURN_RET_LOG(adapterInfo != nullptr, 0, "adapterInfo is nullptr");
    std::shared_ptr<AdapterPipeInfo> pipeInfo = adapterInfo->GetPipeInfoByName(pipeName);
    CHECK_AND_RETURN_RET_LOG(pipeInfo != nullptr, 0, "pipeInfo is nullptr");
    return pipeInfo->dynamicStreamPropInfos_.size();
}

PolicyAdapterInfo::PolicyAdapterInfo()
{
    AUDIO_INFO_LOG("in");
}
PolicyAdapterInfo::~PolicyAdapterInfo()
{
    AUDIO_INFO_LOG("in");
}

AudioAdapterType PolicyAdapterInfo::GetTypeEnum()
{
    return GetAdapterType(adapterName);
}

AudioAdapterType PolicyAdapterInfo::GetAdapterType(const std::string &adapterName)
{
    if (adapterName == ADAPTER_TYPE_PRIMARY) {
        return AudioAdapterType::TYPE_PRIMARY;
    } else if (adapterName == ADAPTER_TYPE_A2DP) {
        return AudioAdapterType::TYPE_A2DP;
    } else if (adapterName == ADAPTER_TYPE_HEARING_AID) {
        return AudioAdapterType::TYPE_HEARING_AID;
    } else if (adapterName == ADAPTER_TYPE_REMOTE) {
        return AudioAdapterType::TYPE_REMOTE_AUDIO;
    } else if (adapterName == ADAPTER_TYPE_FILE) {
        return AudioAdapterType::TYPE_FILE_IO;
    } else if (adapterName == ADAPTER_TYPE_USB) {
        return AudioAdapterType::TYPE_USB;
    } else if (adapterName == ADAPTER_TYPE_DP) {
        return AudioAdapterType::TYPE_DP;
    } else if (adapterName == ADAPTER_TYPE_ACCESSORY) {
        return AudioAdapterType::TYPE_ACCESSORY;
    } else if (adapterName == ADAPTER_TYPE_SLE) {
        return AudioAdapterType::TYPE_SLE;
    } else {
        return AudioAdapterType::TYPE_INVALID;
    }
}

std::shared_ptr<AdapterPipeInfo> PolicyAdapterInfo::GetPipeInfoByName(const std::string &pipeName)
{
    for (auto &pipeInfo : pipeInfos) {
        if (pipeInfo == nullptr) {
            AUDIO_ERR_LOG("pipeInfo is null!");
            continue;
        }
        if (pipeInfo->name_ == pipeName) {
            return pipeInfo;
        }
    }
    AUDIO_ERR_LOG("Can not match pipe:%{public}s!", pipeName.c_str());
    return nullptr;
}

std::shared_ptr<AdapterDeviceInfo> PolicyAdapterInfo::GetDeviceInfoByType(DeviceType deviceType, DeviceRole role)
{
    for (auto &deviceInfo : deviceInfos) {
        if (deviceInfo->type_ == deviceType && deviceInfo->role_ == role) {
            return deviceInfo;
        }
    }
    return nullptr;
}

void PolicyAdapterInfo::SelfCheck()
{
    CHECK_AND_RETURN_LOG(pipeInfos.size() != 0, "SelfCheck Failled! Adapter:%{public}s No Pipe!",
        adapterName.c_str());
    CHECK_AND_RETURN_LOG(deviceInfos.size() != 0, "SelfCheck Failled! Adapter:%{public}s No Device!",
        adapterName.c_str());

    for (auto &pipeInfo : pipeInfos) {
        pipeInfo->SelfCheck();
    }
    for (auto &deviceInfo : deviceInfos) {
        deviceInfo->SelfCheck();
    }
}

void AdapterDeviceInfo::SelfCheck()
{
    std::shared_ptr<PolicyAdapterInfo> adapterInfoPtr = adapterInfo_.lock();
    CHECK_AND_RETURN_LOG(adapterInfoPtr != nullptr, "SelfCheck Failled! Device:%{public}s adapterInfo is null!",
        name_.c_str());
    CHECK_AND_RETURN_LOG(supportPipeMap_.size() != 0, "SelfCheck Failled! Device:%{public}s Not Support Any Pipe!",
        name_.c_str());

    for (auto &pipeName : supportPipes_) {
        bool flag = false;
        for (auto &pair : supportPipeMap_) {
            if (pair.second->name_ == pipeName) {
                flag = true;
            }
        }
        if (!flag) {
            AUDIO_ERR_LOG("SelfCheck Failled! Device:%{public}s Not Support Pipe:%{public}s!",
                name_.c_str(), pipeName.c_str());
        }
    }
}

void AdapterPipeInfo::SelfCheck()
{
    std::shared_ptr<PolicyAdapterInfo> adapterInfoPtr = adapterInfo_.lock();
    CHECK_AND_RETURN_LOG(adapterInfoPtr != nullptr, "SelfCheck Failled! Pipe:%{public}s adapterInfo is null!",
        name_.c_str());
    CHECK_AND_RETURN_LOG(streamPropInfos_.size() != 0, "SelfCheck Failled! Pipe:%{public}s No streamProp!",
        name_.c_str());
    CHECK_AND_RETURN_LOG(attributeInfos_.size() != 0, "SelfCheck Failled! Pipe:%{public}s No Attribute!",
        name_.c_str());

    for (auto &streamPropInfo : streamPropInfos_) {
        streamPropInfo->SelfCheck();
    }
}

void AdapterPipeInfo::UpdateDynamicStreamProps(const std::list<std::shared_ptr<PipeStreamPropInfo>> &streamProps)
{
    CHECK_AND_RETURN_LOG(streamProps.size() != 0, "streamProps is empty");
    std::lock_guard<std::mutex> lock(dynamicMtx_);
    dynamicStreamPropInfos_ = streamProps;
}

void AdapterPipeInfo::ClearDynamicStreamProps()
{
    std::lock_guard<std::mutex> lock(dynamicMtx_);
    dynamicStreamPropInfos_.clear();
}

void PipeStreamPropInfo::SelfCheck()
{
    std::shared_ptr<AdapterPipeInfo> pipeInfoPtr = pipeInfo_.lock();
    CHECK_AND_RETURN_LOG(pipeInfoPtr != nullptr, "SelfCheck Failled! pipeinfo is null!");
    CHECK_AND_RETURN_LOG(supportDeviceMap_.size() != 0, "SelfCheck Failled! Pipe:%{public}s Not support Any Device!",
        pipeInfoPtr->name_.c_str());

    for (auto &deviceName : supportDevices_) {
        bool flag = false;
        for (auto &pair : supportDeviceMap_) {
            if (pair.second->name_ == deviceName) {
                flag = true;
            }
        }
        if (!flag) {
            AUDIO_ERR_LOG("SelfCheck Failled! Pipe:%{public}s Not Support Device:%{public}s!",
                pipeInfoPtr->name_.c_str(), deviceName.c_str());
        }
    }
}

}
}
