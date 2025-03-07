/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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
#define LOG_TAG "AudioConfigManager"
#endif

#include "audio_config_manager.h"
#include <ability_manager_client.h>
#include "iservice_registry.h"
#include "parameter.h"
#include "parameters.h"
#include "audio_policy_log.h"
#include "audio_manager_listener_stub.h"
#include "audio_inner_call.h"
#include "media_monitor_manager.h"

#include "audio_policy_utils.h"
#include "audio_policy_service.h"

namespace OHOS {
namespace AudioStandard {

const int32_t DEFAULT_MAX_OUTPUT_NORMAL_INSTANCES = 128;
static const std::string EARPIECE_TYPE_NAME = "DEVICE_TYPE_EARPIECE";

bool AudioConfigManager::Init()
{
    std::unique_ptr<AudioPolicyParser> audioPolicyConfigParser = make_unique<AudioPolicyParser>(*this);
    bool ret = audioPolicyConfigParser->LoadConfiguration();
    if (!ret) {
        AudioPolicyUtils::GetInstance().WriteServiceStartupError("Audio Policy Config Load Configuration failed");
        AUDIO_ERR_LOG("Audio Policy Config Load Configuration failed");
        return ret;
    }
    return ret;
}

void AudioConfigManager::OnAudioPolicyXmlParsingCompleted(
    const std::unordered_map<AdaptersType, AudioAdapterInfo> adapterInfoMap)
{
    AUDIO_INFO_LOG("adapterInfo num [%{public}zu]", adapterInfoMap.size());
    CHECK_AND_RETURN_LOG(!adapterInfoMap.empty(), "failed to parse audiopolicy xml file. Received data is empty");
    adapterInfoMap_ = adapterInfoMap;

    for (const auto &adapterInfo : adapterInfoMap_) {
        hasEarpiece_ = std::any_of((adapterInfo.second).deviceInfos_.begin(), (adapterInfo.second).deviceInfos_.end(),
            [](const auto& deviceInfos) {
                return deviceInfos.type_ == EARPIECE_TYPE_NAME;
            });
        if (hasEarpiece_) {
            break;
        }
    }
    isAdapterInfoMap_.store(true);

    audioDeviceManager_.UpdateEarpieceStatus(hasEarpiece_);
}

bool AudioConfigManager::GetHasEarpiece()
{
    return hasEarpiece_;
}

bool AudioConfigManager::GetAdapterInfoFlag()
{
    return isAdapterInfoMap_.load();
}

bool AudioConfigManager::GetAdapterInfoByType(AdaptersType type, AudioAdapterInfo &info)
{
    auto it = adapterInfoMap_.find(type);
    if (it == adapterInfoMap_.end()) {
        AUDIO_ERR_LOG("can not find adapter info");
        return false;
    }
    info = it->second;
    return true;
}

// Parser callbacks
void AudioConfigManager::OnXmlParsingCompleted(const std::unordered_map<ClassType, std::list<AudioModuleInfo>> &xmlData)
{
    AUDIO_INFO_LOG("device class num [%{public}zu]", xmlData.size());
    CHECK_AND_RETURN_LOG(!xmlData.empty(), "failed to parse xml file. Received data is empty");

    deviceClassInfo_ = xmlData;
}

void AudioConfigManager::GetDeviceClassInfo(std::unordered_map<ClassType, std::list<AudioModuleInfo>> &deviceClassInfo)
{
    deviceClassInfo = deviceClassInfo_;
}

bool AudioConfigManager::GetModuleListByType(ClassType type, std::list<AudioModuleInfo>& moduleList)
{
    auto modulesPos = deviceClassInfo_.find(type);
    if (modulesPos != deviceClassInfo_.end()) {
        moduleList = modulesPos->second;
        return true;
    }
    return false;
}

void AudioConfigManager::OnUpdateRouteSupport(bool isSupported)
{
    isUpdateRouteSupported_ = isSupported;
}

bool AudioConfigManager::GetUpdateRouteSupport()
{
    return isUpdateRouteSupported_;
}

void AudioConfigManager::OnUpdateDefaultAdapter(bool isEnable)
{
    isDefaultAdapterEnable_ = isEnable;
}

bool AudioConfigManager::GetDefaultAdapterEnable()
{
    return isDefaultAdapterEnable_;
}

void AudioConfigManager::OnUpdateAnahsSupport(std::string anahsShowType)
{
    AUDIO_INFO_LOG("OnUpdateAnahsSupport show type: %{public}s", anahsShowType.c_str());
    AudioPolicyService::GetAudioPolicyService().OnUpdateAnahsSupport(anahsShowType);
}

void AudioConfigManager::OnVolumeGroupParsed(std::unordered_map<std::string, std::string>& volumeGroupData)
{
    AUDIO_INFO_LOG("group data num [%{public}zu]", volumeGroupData.size());
    CHECK_AND_RETURN_LOG(!volumeGroupData.empty(), "failed to parse xml file. Received data is empty");

    volumeGroupData_ = volumeGroupData;
}

void AudioConfigManager::OnInterruptGroupParsed(std::unordered_map<std::string, std::string>& interruptGroupData)
{
    AUDIO_INFO_LOG("group data num [%{public}zu]", interruptGroupData.size());
    CHECK_AND_RETURN_LOG(!interruptGroupData.empty(), "failed to parse xml file. Received data is empty");

    interruptGroupData_ = interruptGroupData;
}

std::string AudioConfigManager::GetGroupName(const std::string& deviceName, const GroupType type)
{
    std::string groupName = GROUP_NAME_NONE;
    if (type == VOLUME_TYPE) {
        auto iter = volumeGroupData_.find(deviceName);
        if (iter != volumeGroupData_.end()) {
            groupName = iter->second;
        }
    } else {
        auto iter = interruptGroupData_.find(deviceName);
        if (iter != interruptGroupData_.end()) {
            groupName = iter->second;
        }
    }
    return groupName;
}

void AudioConfigManager::OnGlobalConfigsParsed(GlobalConfigs &globalConfigs)
{
    globalConfigs_ = globalConfigs;
}

int32_t AudioConfigManager::GetMaxRendererInstances()
{
    auto configIter = std::find_if(globalConfigs_.outputConfigInfos_.begin(), globalConfigs_.outputConfigInfos_.end(),
        [](const auto& configInfo) {
            return configInfo.name_ == "normal" && configInfo.value_ != "";
        });
    if (configIter != globalConfigs_.outputConfigInfos_.end()) {
        AUDIO_INFO_LOG("Max output normal instance is %{public}s", configIter->value_.c_str());
        int32_t convertValue = 0;
        CHECK_AND_RETURN_RET_LOG(StringConverter(configIter->value_, convertValue),
            DEFAULT_MAX_OUTPUT_NORMAL_INSTANCES,
            "convert invalid configInfo.value_: %{public}s", configIter->value_.c_str());
        return convertValue;
    }
    return DEFAULT_MAX_OUTPUT_NORMAL_INSTANCES;
}

void AudioConfigManager::OnVoipConfigParsed(bool enableFastVoip)
{
    enableFastVoip_ = enableFastVoip;
}


void AudioConfigManager::SetNormalVoipFlag(const bool &normalVoipFlag)
{
    normalVoipFlag_ = normalVoipFlag;
}

bool AudioConfigManager::GetNormalVoipFlag()
{
    return normalVoipFlag_;
}

int32_t AudioConfigManager::GetVoipRendererFlag(const std::string &sinkPortName, const std::string &networkId,
    const AudioSamplingRate &samplingRate)
{
    // VoIP stream has three mode for different products.
    if (enableFastVoip_ && (sinkPortName == PRIMARY_SPEAKER && networkId == LOCAL_NETWORK_ID)) {
        if (samplingRate != SAMPLE_RATE_48000 && samplingRate != SAMPLE_RATE_16000) {
            return AUDIO_FLAG_NORMAL;
        }
        return AUDIO_FLAG_VOIP_FAST;
    } else if (!normalVoipFlag_ && (sinkPortName == PRIMARY_SPEAKER) && (networkId == LOCAL_NETWORK_ID)) {
        AUDIO_INFO_LOG("Direct VoIP mode is supported for the device");
        return AUDIO_FLAG_VOIP_DIRECT;
    }

    return AUDIO_FLAG_NORMAL;
}

void AudioConfigManager::OnAudioLatencyParsed(uint64_t latency)
{
    audioLatencyInMsec_ = latency;
}

void AudioConfigManager::OnSinkLatencyParsed(uint32_t latency)
{
    sinkLatencyInMsec_ = latency;
}

int32_t AudioConfigManager::GetAudioLatencyFromXml() const
{
    return audioLatencyInMsec_;
}

uint32_t AudioConfigManager::GetSinkLatencyFromXml() const
{
    return sinkLatencyInMsec_;
}

void AudioConfigManager::GetAudioAdapterInfos(std::unordered_map<AdaptersType, AudioAdapterInfo> &adapterInfoMap)
{
    adapterInfoMap = adapterInfoMap_;
}

void AudioConfigManager::GetVolumeGroupData(std::unordered_map<std::string, std::string>& volumeGroupData)
{
    volumeGroupData = volumeGroupData_;
}

void AudioConfigManager::GetInterruptGroupData(std::unordered_map<std::string, std::string>& interruptGroupData)
{
    interruptGroupData = interruptGroupData_;
}

void AudioConfigManager::GetGlobalConfigs(GlobalConfigs &globalConfigs)
{
    globalConfigs = globalConfigs_;
}

bool AudioConfigManager::GetVoipConfig()
{
    return enableFastVoip_;
}

}
}