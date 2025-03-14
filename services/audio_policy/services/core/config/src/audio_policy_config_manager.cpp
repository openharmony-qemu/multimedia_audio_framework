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
#define LOG_TAG "AudioPolicyConfigManager"
#endif

#include "audio_policy_config_manager.h"
#include "audio_policy_config_parser.h"
#include "audio_policy_utils.h"
#include "audio_policy_service.h"


namespace OHOS {
namespace AudioStandard {

static const char* MAX_RENDERERS_NAME = "maxRenderers";
static const char* MAX_CAPTURERS_NAME = "maxCapturers";
static const char* MAX_FAST_RENDERERS_NAME = "maxFastRenderers";

const int32_t DEFAULT_MAX_OUTPUT_NORMAL_INSTANCES = 128;
const int32_t DEFAULT_MAX_INPUT_NORMAL_INSTANCES = 16;
const int32_t DEFAULT_MAX_FAST_NORMAL_INSTANCES = 6;

bool AudioPolicyConfigManager::Init()
{
    std::unique_ptr<AudioPolicyConfigParser> audioPolicyConfigParser = make_unique<AudioPolicyConfigParser>(this);
    CHECK_AND_RETURN_RET_LOG(audioPolicyConfigParser != nullptr, false, "AudioPolicyConfigParser create failed");
    bool ret = audioPolicyConfigParser->LoadConfiguration();
    if (!ret) {
        AudioPolicyUtils::GetInstance().WriteServiceStartupError("Audio Policy Config Load Configuration failed");
        AUDIO_ERR_LOG("Audio Policy Config Load Configuration failed");
        return ret;
    }
    return ret;
}

void AudioPolicyConfigManager::OnAudioPolicyConfigXmlParsingCompleted()
{
    AUDIO_INFO_LOG("AdapterInfo num [%{public}zu]", audioPolicyCOnfig_.adapterInfoMap.size());
    CHECK_AND_RETURN_LOG(!audioPolicyConfig_.adapterInfoMap.empty(),
        "Parse audio policy xml failed, received data is empty");

    audioPolicyConfig_.Reorganize();

    isAdapterInfoMap_.store(true);

    OnHasEarpiece();
}

void AudioPolicyConfigManager::OnXmlParsingCompleted(
    const std::unordered_map<ClassType, std::list<AudioModuleInfo>> &xmlData)
{
    AUDIO_INFO_LOG("device class num [%{public}zu]", xmlData.size());
    CHECK_AND_RETURN_LOG(!xmlData.empty(), "failed to parse xml file. Received data is empty");

    deviceClassInfo_ = xmlData;
}

void AudioPolicyConfigManager::OnAudioLatencyParsed(uint64_t latency)
{
    audioLatencyInMsec_ = latency;
}

void AudioPolicyConfigManager::OnSinkLatencyParsed(uint32_t latency)
{
    sinkLatencyInMsec_ = latency;
}

void AudioPolicyConfigManager::OnVolumeGroupParsed(std::unordered_map<std::string, std::string>& volumeGroupData)
{
    AUDIO_INFO_LOG("group data num [%{public}zu]", volumeGroupData.size());
    CHECK_AND_RETURN_LOG(!volumeGroupData.empty(), "failed to parse xml file. Received data is empty");

    volumeGroupData_ = volumeGroupData;
}

void AudioPolicyConfigManager::OnInterruptGroupParsed(std::unordered_map<std::string, std::string>& interruptGroupData)
{
    AUDIO_INFO_LOG("group data num [%{public}zu]", interruptGroupData.size());
    CHECK_AND_RETURN_LOG(!interruptGroupData.empty(), "failed to parse xml file. Received data is empty");

    interruptGroupData_ = interruptGroupData;
}

void AudioPolicyConfigManager::OnUpdateRouteSupport(bool isSupported)
{
    isUpdateRouteSupported_ = isSupported;
}

void AudioPolicyConfigManager::OnUpdateDefaultAdapter(bool isEnable)
{
    isDefaultAdapterEnable_ = isEnable;
}

bool AudioPolicyConfigManager::GetDefaultAdapterEnable()
{
    return isDefaultAdapterEnable_;
}

void AudioPolicyConfigManager::OnGlobalConfigsParsed(PolicyGlobalConfigs &globalConfigs)
{
    globalConfigs_ = globalConfigs;
}

void AudioPolicyConfigManager::OnVoipConfigParsed(bool enableFastVoip)
{
    enableFastVoip_ = enableFastVoip;
}

void AudioPolicyConfigManager::OnUpdateAnahsSupport(std::string anahsShowType)
{
    AUDIO_INFO_LOG("OnUpdateAnahsSupport show type: %{public}s", anahsShowType.c_str());
    AudioPolicyService::GetAudioPolicyService().OnUpdateAnahsSupport(anahsShowType);
}

void AudioPolicyConfigManager::OnHasEarpiece()
{
    for (const auto &adapterInfo : audioPolicyConfig_.adapterInfoMap) {
        hasEarpiece_ = std::any_of(adapterInfo.second->deviceInfos.begin(), adapterInfo.second->deviceInfos.end(),
            [](const auto& deviceInfo) {
                return deviceInfo->type_ == DEVICE_TYPE_EARPIECE;
            });
        if (hasEarpiece_) {
            break;
        }
    }
    audioDeviceManager_.UpdateEarpieceStatus(hasEarpiece_);
}

void AudioPolicyConfigManager::SetNormalVoipFlag(const bool &normalVoipFlag)
{
    normalVoipFlag_ = normalVoipFlag;
}

bool AudioPolicyConfigManager::GetModuleListByType(ClassType type, std::list<AudioModuleInfo>& moduleList)
{
    auto modulesPos = deviceClassInfo_.find(type);
    if (modulesPos != deviceClassInfo_.end()) {
        moduleList = modulesPos->second;
        return true;
    }
    return false;
}

void AudioPolicyConfigManager::GetDeviceClassInfo(
    std::unordered_map<ClassType, std::list<AudioModuleInfo>> &deviceClassInfo)
{
    deviceClassInfo = deviceClassInfo_;
}

std::string AudioPolicyConfigManager::GetGroupName(const std::string& deviceName, const GroupType type)
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

int32_t AudioPolicyConfigManager::GetMaxRendererInstances()
{
    for (auto commonConfig : globalConfigs_.commonConfigs_) {
        if (commonConfig.name_ != MAX_RENDERERS_NAME) {
            continue;
        }
        int32_t convertValue = 0;
        AUDIO_INFO_LOG("Max output normal instance is %{public}s", commonConfig.value_.c_str());
        CHECK_AND_RETURN_RET_LOG(StringConverter(commonConfig.value_, convertValue),
            DEFAULT_MAX_OUTPUT_NORMAL_INSTANCES,
            "convert invalid configInfo.value_: %{public}s", commonConfig.value_.c_str());
        return convertValue;
    }
    return DEFAULT_MAX_OUTPUT_NORMAL_INSTANCES;
}

int32_t AudioPolicyConfigManager::GetMaxCapturersInstances()
{
    for (auto commonConfig : globalConfigs_.commonConfigs_) {
        if (commonConfig.name_ != MAX_CAPTURERS_NAME) {
            continue;
        }
        int32_t convertValue = 0;
        AUDIO_INFO_LOG("Max input normal instance is %{public}s", commonConfig.value_.c_str());
        CHECK_AND_RETURN_RET_LOG(StringConverter(commonConfig.value_, convertValue),
            DEFAULT_MAX_INPUT_NORMAL_INSTANCES,
            "convert invalid configInfo.value_: %{public}s", commonConfig.value_.c_str());
        return convertValue;
    }
    return DEFAULT_MAX_INPUT_NORMAL_INSTANCES;
}

int32_t AudioPolicyConfigManager::GetMaxFastRenderersInstances()
{
    for (auto commonConfig : globalConfigs_.commonConfigs_) {
        if (commonConfig.name_ != MAX_FAST_RENDERERS_NAME) {
            continue;
        }
        int32_t convertValue = 0;
        AUDIO_INFO_LOG("Max Fast Renderer instance is %{public}s", commonConfig.value_.c_str());
        CHECK_AND_RETURN_RET_LOG(StringConverter(commonConfig.value_, convertValue),
            DEFAULT_MAX_FAST_NORMAL_INSTANCES,
            "convert invalid configInfo.value_: %{public}s", commonConfig.value_.c_str());
        return convertValue;
    }
    return DEFAULT_MAX_FAST_NORMAL_INSTANCES;
}

int32_t AudioPolicyConfigManager::GetVoipRendererFlag(const std::string &sinkPortName, const std::string &networkId,
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

int32_t AudioPolicyConfigManager::GetAudioLatencyFromXml() const
{
    return audioLatencyInMsec_;
}

uint32_t AudioPolicyConfigManager::GetSinkLatencyFromXml() const
{
    return sinkLatencyInMsec_;
}

void AudioPolicyConfigManager::GetAudioAdapterInfos(
    std::unordered_map<AudioAdapterType, std::shared_ptr<PolicyAdapterInfo>> &adapterInfoMap)
{
    adapterInfoMap = audioPolicyConfig_.adapterInfoMap;
}

void AudioPolicyConfigManager::GetVolumeGroupData(std::unordered_map<std::string, std::string>& volumeGroupData)
{
    volumeGroupData = volumeGroupData_;
}

void AudioPolicyConfigManager::GetInterruptGroupData(std::unordered_map<std::string, std::string>& interruptGroupData)
{
    interruptGroupData = interruptGroupData_;
}

void AudioPolicyConfigManager::GetGlobalConfigs(PolicyGlobalConfigs &globalConfigs)
{
    globalConfigs = globalConfigs_;
}

bool AudioPolicyConfigManager::GetVoipConfig()
{
    return enableFastVoip_;
}

bool AudioPolicyConfigManager::GetUpdateRouteSupport()
{
    return isUpdateRouteSupported_;
}

bool AudioPolicyConfigManager::GetAdapterInfoFlag()
{
    return isAdapterInfoMap_.load();
}

bool AudioPolicyConfigManager::GetAdapterInfoByType(AudioAdapterType type, std::shared_ptr<PolicyAdapterInfo> &info)
{
    auto it = audioPolicyConfig_.adapterInfoMap.find(type);
    if (it == audioPolicyConfig_.adapterInfoMap.end()) {
        AUDIO_ERR_LOG("can not find adapter info");
        return false;
    }
    info = it->second;
    return true;
}

bool AudioPolicyConfigManager::GetHasEarpiece()
{
    return hasEarpiece_;
}

uint32_t AudioPolicyConfigManager::GetRouteFlag(std::shared_ptr<AudioStreamDescriptor> &desc)
{
    // device -> adapter -> flag -> stream
    uint32_t flag = AUDIO_FLAG_NONE; // input or output? default?
    auto newDeviceDesc = desc->newDeviceDescs_.front();
    std::shared_ptr<AdapterDeviceInfo> deviceInfo = audioPolicyConfig_.GetAdapterDeviceInfo(
        newDeviceDesc->deviceType_, newDeviceDesc->deviceRole_, newDeviceDesc->networkId_, desc->audioFlag_);
    CHECK_AND_RETURN_RET_LOG(deviceInfo != nullptr, flag, "Find device failed; use none flag");

    for (auto &pipeIt : deviceInfo->supportPipeMap_) {
        if ((desc->audioMode_ == static_cast<AudioMode>(pipeIt.second->role_)) && (desc->audioFlag_ & pipeIt.first)) {
            flag = pipeIt.first;
            break;
        }
    }
    if (flag == AUDIO_FLAG_NONE) {
        flag = desc->audioMode_ == AUDIO_MODE_PLAYBACK ?
            AUDIO_OUTPUT_FLAG_NORMAL : AUDIO_INPUT_FLAG_NORMAL;
    }
    AUDIO_INFO_LOG("flag:0x%{public}x, target flag:0x%{public}x", desc->audioFlag_, flag);
    return flag;
}

void AudioPolicyConfigManager::GetStreamPropInfo(std::shared_ptr<AudioStreamDescriptor> &desc,
    std::shared_ptr<PipeStreamPropInfo> &info)
{
    auto newDeviceDesc = desc->newDeviceDescs_.front();
    std::shared_ptr<AdapterDeviceInfo> deviceInfo = audioPolicyConfig_.GetDeviceInfo(
        newDeviceDesc->deviceType_, newDeviceDesc->deviceRole_, newDeviceDesc->networkId_, desc->audioFlag_);
    CHECK_AND_RETURN_LOG(deviceInfo != nullptr, flag, "Find device failed;none streamProp");

    auto pipeIt = deviceInfo->supportPipeMap_.find(desc->routeFlag_);
    CHECK_AND_RETURN_LOG(pipeIt != deviceInfo->supportPipeMap_.end(), "Find pipeInfo failed;none streamProp");

    AudioChannel tempChannel = desc->streamInfo_.channels;
    if ((desc->routeFlag_ == (AUDIO_INPUT_FLAG_VOIP | AUDIO_INPUT_FLAG_FAST)) ||
        (desc->routeFlag_ == (AUDIO_OUTPUT_FLAG_VOIP | AUDIO_OUTPUT_FLAG_FAST))) {
        tempChannel = desc->streamInfo_.channels == MONO ? STEREO : desc->streamInfo_.channels;
    }

    for (auto &streamProp : pipeIt->second->streamPropInfos_) {
        if (streamProp->format_ == desc->streamInfo_.format &&
            streamProp->sampleRate_ == desc->streamInfo_.samplingRate &&
            streamProp->channels_ == tempChannel) {
            info = streamProp;
            AUDIO_INFO_LOG("format:%{public}u, sampleRate:%{public}u, channelLayout:%{public}u, channels:%{public}u,"
                " desc channels:%{public}u", info->format_, info->sampleRate_, info->channelLayout_, info->channels_,
                tempChannel);
            return;
        }
    }

    if (SupportImplicitConversion(desc->routeFlag_)) {
        AUDIO_INFO_LOG("Select first attribute");
        info = pipeIt->second->streamPropInfos_.front();
    }

    if (info->format_ == INVALID_WIDTH && info->sampleRate_ == 0 && info->channelLayout_ == CH_LAYOUT_UNKNOWN &&
        desc->routeFlag_ != (AUDIO_OUTPUT_FLAG_NORMAL || AUDIO_INPUT_FLAG_NORMAL)) {
        AUDIO_INFO_LOG("Find streamPropInfo failed, choose normal flag");
        desc->routeFlag_ = desc->audioMode_ == AUDIO_MODE_PLAYBACK ?
            AUDIO_OUTPUT_FLAG_NORMAL : AUDIO_INPUT_FLAG_NORMAL;
        auto pipeIt = deviceInfo->supportPipeMap_.find(desc->routeFlag_);
        for (auto &streamProp : pipeIt->second->streamPropInfos_) {
            if (streamProp->format_ == desc->streamInfo_.format &&
                streamProp->sampleRate_ == desc->streamInfo_.samplingRate &&
                streamProp->channels_ == desc->streamInfo_.channels) {
                info = streamProp;
                AUDIO_INFO_LOG("format:%{public}u, sampleRate:%{public}u, channelLayout:%{public}u, "
                    "channels:%{public}u, desc channels:%{public}u", info->format_, info->sampleRate_,
                    info->channelLayout_, info->channels_, desc->streamInfo_.channels);
                return;
            }
        }
    }
    if (info->format_ == INVALID_WIDTH && info->sampleRate_ == 0 && info->channelLayout_ == CH_LAYOUT_UNKNOWN &&
        desc->routeFlag_ == (AUDIO_OUTPUT_FLAG_NORMAL || AUDIO_INPUT_FLAG_NORMAL) &&
        !pipeIt->second->streamPropInfos_.empty()) {
        info = pipeIt->second->streamPropInfos_.front(); // if not match, choose first?
    }
    AUDIO_INFO_LOG("format:%{public}u, sampleRate:%{public}u, channelLayout:%{public}lu, channels:%{public}u",
        info->format_, info->sampleRate_, info->channelLayout_, info->channels_, desc->streamInfo_.channels);
}

bool AudioPolicyConfigManager::SupportImplicitConversion(uint32_t routeFlag)
{
    if ((routeFlag & AUDIO_OUTPUT_FLAG_NORMAL) ||
        (routeFlag & AUDIO_OUTPUT_FLAG_DIRECT) ||
        (routeFlag & AUDIO_OUTPUT_FLAG_MULTICHANNEL) ||
        (routeFlag & AUDIO_OUTPUT_FLAG_LOWPOWER) ||
        (routeFlag & AUDIO_INPUT_FLAG_NORMAL) ||
        (routeFlag & AUDIO_INPUT_FLAG_WAKEUP)) {
        return true;
    }
    return false
}

}
}
