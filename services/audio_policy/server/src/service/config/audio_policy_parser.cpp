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
#define LOG_TAG "AudioPolicyParser"
#endif

#include "audio_policy_parser.h"
#include "audio_utils.h"
#include "audio_errors.h"

#include <sstream>

namespace OHOS {
namespace AudioStandard {
constexpr int32_t AUDIO_MS_PER_S = 1000;
constexpr uint32_t LAYOUT_MONO_CHANNEL_ENUM = 1;
constexpr uint32_t LAYOUT_STEREO_CHANNEL_ENUM = 2;
constexpr uint32_t LAYOUT_QUAD_CHANNEL_ENUM = 4;
constexpr uint32_t LAYOUT_5POINT1_CHANNEL_ENUM = 6;
constexpr uint32_t LAYOUT_7POINT1_CHANNEL_ENUM = 8;
constexpr uint32_t S16LE_TO_BYTE = 2;
constexpr uint32_t S24LE_TO_BYTE = 3;
constexpr uint32_t S32LE_TO_BYTE = 4;

static std::map<std::string, uint32_t> layoutStrToChannels = {
    {"CH_LAYOUT_MONO", LAYOUT_MONO_CHANNEL_ENUM},
    {"CH_LAYOUT_STEREO", LAYOUT_STEREO_CHANNEL_ENUM},
    {"CH_LAYOUT_5POINT1", LAYOUT_5POINT1_CHANNEL_ENUM},
    {"CH_LAYOUT_QUAD", LAYOUT_QUAD_CHANNEL_ENUM},
    {"CH_LAYOUT_7POINT1", LAYOUT_7POINT1_CHANNEL_ENUM},
};

static std::map<std::string, uint32_t> formatStrToEnum = {
    {"s16le", S16LE_TO_BYTE},
    {"s24le", S24LE_TO_BYTE},
    {"s32le", S32LE_TO_BYTE},
};

static std::map<std::string, uint32_t> audioFlagStrToEnum = {
    {"AUDIO_FLAG_NORMAL", AUDIO_FLAG_NORMAL},
    {"AUDIO_FLAG_MMAP", AUDIO_FLAG_MMAP},
};

static std::map<std::string, uint32_t> audioUsageStrToEnum = {
    {"AUDIO_USAGE_NORMAL", AUDIO_USAGE_NORMAL},
    {"AUDIO_USAGE_VOIP", AUDIO_USAGE_VOIP},
};

// LCOV_EXCL_START
bool AudioPolicyParser::LoadConfiguration()
{
    AUDIO_INFO_LOG("Enter");
    if (curNode_->Config(CHIP_PROD_CONFIG_FILE, nullptr, 0) != SUCCESS) {
        if (curNode_->Config(CONFIG_FILE, nullptr, 0) != SUCCESS) {
            AUDIO_ERR_LOG("LoadConfiguration readFile Failed");
            return false;
        }
    }

    if (!ParseInternal(curNode_->GetCopyNode())) {
        AUDIO_ERR_LOG("Audio policy config xml parse failed.");
        return false;
    }
    std::unordered_map<std::string, std::string> volumeGroupMap {};
    std::unordered_map<std::string, std::string> interruptGroupMap {};

    ConvertAdapterInfoToGroupInfo(volumeGroupMap, interruptGroupMap);
    ConvertAdapterInfoToAudioModuleInfo();

    volumeGroupMap_ = volumeGroupMap;
    interruptGroupMap_ = interruptGroupMap;

    portObserver_.OnAudioPolicyXmlParsingCompleted(adapterInfoMap_);
    portObserver_.OnXmlParsingCompleted(xmlParsedDataMap_);
    portObserver_.OnVolumeGroupParsed(volumeGroupMap_);
    portObserver_.OnInterruptGroupParsed(interruptGroupMap_);
    portObserver_.OnGlobalConfigsParsed(globalConfigs_);

    AUDIO_INFO_LOG("Done");
    return true;
}

void AudioPolicyParser::Destroy()
{
    curNode_->FreeDoc();
}

bool AudioPolicyParser::ParseInternal(std::shared_ptr<AudioXmlNode> curNode)
{
    for (; curNode->IsNodeValid(); curNode->MoveToNext()) {
        if (curNode->IsElementNode()) {
            switch (GetXmlNodeTypeAsInt(curNode)) {
                case XmlNodeType::ADAPTERS:
                    ParseAdapters(curNode->GetCopyNode());
                    break;
                case XmlNodeType::VOLUME_GROUPS:
                    ParseGroups(curNode->GetCopyNode(), XmlNodeType::VOLUME_GROUPS);
                    break;
                case XmlNodeType::INTERRUPT_GROUPS:
                    ParseGroups(curNode->GetCopyNode(), XmlNodeType::INTERRUPT_GROUPS);
                    break;
                case XmlNodeType::GLOBAL_CONFIGS:
                    ParseGlobalConfigs(curNode->GetCopyNode());
                    break;
                default:
                    ParseInternal(curNode->GetChildrenNode());
                    break;
            }
        }
    }
    return true;
}

void AudioPolicyParser::ParseAdapters(std::shared_ptr<AudioXmlNode> curNode)
{
    curNode->MoveToChildren();

    while (curNode->IsNodeValid()) {
        if (curNode->IsElementNode()) {
            ParseAdapter(curNode->GetCopyNode());
        }
        curNode->MoveToNext();
    }
}

void AudioPolicyParser::ConvertAdapterInfoToGroupInfo(std::unordered_map<std::string, std::string> &volumeGroupMap,
    std::unordered_map<std::string, std::string> &interruptGroupMap)
{
    for (auto &[sinkName, groupName] : volumeGroupMap_) {
        volumeGroupMap["Speaker"] = groupName;
    }

    for (auto &[sinkName, groupName] : interruptGroupMap_) {
        interruptGroupMap["Speaker"] = groupName;
    }
}

void AudioPolicyParser::GetCommontAudioModuleInfo(PipeInfo &pipeInfo, AudioModuleInfo &audioModuleInfo)
{
    audioModuleInfo.role = pipeInfo.paPropRole_;

    for (auto sampleRate : pipeInfo.sampleRates_) {
        audioModuleInfo.supportedRate_.insert(sampleRate);
    }
    for (auto channelLayout : pipeInfo.channelLayouts_) {
        audioModuleInfo.supportedChannels_.insert(channelLayout);
    }

    audioModuleInfo.lib = pipeInfo.lib_;

    if (pipeInfo.streamPropInfos_.size() != 0) {
        audioModuleInfo.rate = std::to_string((*pipeInfo.streamPropInfos_.begin()).sampleRate_);
        audioModuleInfo.format = (*pipeInfo.streamPropInfos_.begin()).format_;
        audioModuleInfo.channels = std::to_string((*pipeInfo.streamPropInfos_.begin()).channelLayout_);
        audioModuleInfo.bufferSize = std::to_string((*pipeInfo.streamPropInfos_.begin()).bufferSize_);
    }

    for (auto &config : pipeInfo.configInfos_) {
        if (config.name_ == "filePath") {
            audioModuleInfo.fileName = config.value_;
        }
    }

    audioModuleInfo.fixedLatency = pipeInfo.fixedLatency_;
    audioModuleInfo.renderInIdleState = pipeInfo.renderInIdleState_;
}

ClassType AudioPolicyParser::GetClassTypeByAdapterType(AdaptersType adapterType)
{
    if (adapterType == AdaptersType::TYPE_PRIMARY) {
        return ClassType::TYPE_PRIMARY;
    } else if (adapterType == AdaptersType::TYPE_A2DP) {
        return ClassType::TYPE_A2DP;
    } else if (adapterType == AdaptersType::TYPE_REMOTE_AUDIO) {
        return ClassType::TYPE_REMOTE_AUDIO;
    } else if (adapterType == AdaptersType::TYPE_FILE_IO) {
        return ClassType::TYPE_FILE_IO;
    } else if (adapterType == AdaptersType::TYPE_USB) {
        return ClassType::TYPE_USB;
    }  else if (adapterType == AdaptersType::TYPE_DP) {
        return ClassType::TYPE_DP;
    } else {
        return ClassType::TYPE_INVALID;
    }
}

// LCOV_EXCL_STOP
void AudioPolicyParser::GetOffloadAndOpenMicState(AudioAdapterInfo &adapterInfo,
    bool &shouldEnableOffload)
{
    for (auto &pipeInfo : adapterInfo.pipeInfos_) {
        if (pipeInfo.paPropRole_ == MODULE_TYPE_SINK &&
            pipeInfo.name_.find(MODULE_SINK_OFFLOAD) != std::string::npos) {
            shouldEnableOffload = true;
        }
    }
}

std::string AudioPolicyParser::GetAudioModuleInfoName(std::string &pipeInfoName,
    std::list<AudioPipeDeviceInfo> &deviceInfos)
{
    for (auto &deviceInfo : deviceInfos) {
        if (std::find(deviceInfo.supportPipes_.begin(), deviceInfo.supportPipes_.end(), pipeInfoName) !=
            deviceInfo.supportPipes_.end()) {
            return deviceInfo.name_;
        }
    }
    return "";
}

// LCOV_EXCL_START
void AudioPolicyParser::ConvertAdapterInfoToAudioModuleInfo()
{
    for (auto &[adapterType, adapterInfo] : adapterInfoMap_) {
        std::list<AudioModuleInfo> audioModuleList = {};
        bool shouldEnableOffload = false;
        if (adapterType == AdaptersType::TYPE_PRIMARY) {
            GetOffloadAndOpenMicState(adapterInfo, shouldEnableOffload);
        }

        std::string currentRole = "";
        for (auto &pipeInfo : adapterInfo.pipeInfos_) {
            if (currentRole == pipeInfo.pipeRole_) {
                continue;
            }
            currentRole = pipeInfo.pipeRole_;
            CHECK_AND_CONTINUE_LOG(pipeInfo.name_.find(MODULE_SINK_OFFLOAD) == std::string::npos,
                "skip offload out sink.");
            AudioModuleInfo audioModuleInfo = {};
            GetCommontAudioModuleInfo(pipeInfo, audioModuleInfo);

            audioModuleInfo.className = adapterInfo.adapterName_;
            // The logic here strongly depends on the moduleName in the XML
            if (pipeInfo.moduleName_ != "") {
                audioModuleInfo.name = pipeInfo.moduleName_;
            } else {
                audioModuleInfo.name = GetAudioModuleInfoName(pipeInfo.name_, adapterInfo.deviceInfos_);
            }

            audioModuleInfo.adapterName = adapterInfo.adapterName_;
            if (adapterType == AdaptersType::TYPE_FILE_IO) {
                audioModuleInfo.adapterName = STR_INIT;
                audioModuleInfo.format = STR_INIT;
                audioModuleInfo.className = FILE_CLASS;
            }
            audioModuleInfo.sinkLatency = globalConfigs_.globalPaConfigs_.sinkLatency_;

            shouldOpenMicSpeaker_ ? audioModuleInfo.OpenMicSpeaker = "1" : audioModuleInfo.OpenMicSpeaker = "0";
            audioModuleInfo.defaultAdapterEnable = shouldSetDefaultAdapter_ ? "1" : "0";
            if (adapterType == AdaptersType::TYPE_PRIMARY &&
                shouldEnableOffload && pipeInfo.paPropRole_ == MODULE_TYPE_SINK) {
                audioModuleInfo.offloadEnable = "1";
            }
            audioModuleList.push_back(audioModuleInfo);
        }
        std::list<AudioModuleInfo> audioModuleListTmp = audioModuleList;
        std::list<AudioModuleInfo> audioModuleListData = {};
        for (auto audioModuleInfo : audioModuleList) {
            audioModuleInfo.ports = audioModuleListTmp;
            audioModuleListData.push_back(audioModuleInfo);
            AUDIO_WARNING_LOG("name:%{public}s, adapter name:%{public}s, adapter type:%{public}d",
                audioModuleInfo.name.c_str(), audioModuleInfo.adapterName.c_str(), adapterType);
        }
        ClassType classType = GetClassTypeByAdapterType(adapterType);
        xmlParsedDataMap_[classType] = audioModuleListData;
    }
}

void AudioPolicyParser::ParseAdapter(std::shared_ptr<AudioXmlNode> curNode)
{
    std::string adapterName;
    CHECK_AND_RETURN_LOG(curNode->GetProp("name", adapterName) == SUCCESS, "get prop name fail!");
    AdaptersType adaptersType = GetAdaptersType(adapterName);
    adapterInfoMap_[adaptersType] = {};

    std::string supportScene;
    curNode->GetProp("supportSelectScene", supportScene);

    AudioAdapterInfo adapterInfo = {};
    adapterInfo.adapterName_ = adapterName;
    adapterInfo.adaptersupportScene_ = supportScene;

    curNode->MoveToChildren();
    while (curNode->IsNodeValid()) {
        if (curNode->IsElementNode()) {
            switch (GetAdapterTypeAsInt(curNode)) {
                case AdapterType::PIPES:
                    ParsePipes(curNode->GetCopyNode(), adapterInfo);
                    break;
                case AdapterType::DEVICES:
                    ParseDevices(curNode->GetCopyNode(), adapterInfo);
                    break;
                default:
                    ParseAdapter(curNode->GetChildrenNode());
                    break;
            }
        }
        curNode->MoveToNext();
    }
    adapterInfoMap_[adaptersType] = adapterInfo;
}

void AudioPolicyParser::ParsePipes(std::shared_ptr<AudioXmlNode> curNode, AudioAdapterInfo &adapterInfo)
{
    curNode->MoveToChildren();
    std::list<PipeInfo> pipeInfos;

    while (curNode->IsNodeValid()) {
        if (curNode->IsElementNode()) {
            PipeInfo pipeInfo {};

            int32_t ret = 0;
            curNode->GetProp("name", pipeInfo.name_);
            curNode->GetProp("role", pipeInfo.pipeRole_);
            curNode->GetProp("flags", pipeInfo.pipeFlags_);
            ParsePipeInfos(curNode->GetCopyNode(), pipeInfo);
            pipeInfos.push_back(pipeInfo);
        }
        curNode->MoveToNext();
    }
    adapterInfo.pipeInfos_ = pipeInfos;
}

void AudioPolicyParser::SplitChannelStringToSet(std::string &str, std::set<uint32_t> &result)
{
    std::stringstream ss(str);
    std::string token;

    while (std::getline(ss, token, ',')) {
        result.insert(layoutStrToChannels[token]);
    }
}

void AudioPolicyParser::ParsePipeInfos(std::shared_ptr<AudioXmlNode> curNode, PipeInfo &pipeInfo)
{
    curNode->MoveToChildren();
    while (curNode->IsNodeValid()) {
        if (curNode->IsElementNode()) {
            switch (GetPipeInfoTypeAsInt(curNode)) {
                case PipeType::PA_PROP:
                    curNode->GetProp("lib", pipeInfo.lib_);
                    curNode->GetProp("role", pipeInfo.paPropRole_);
                    curNode->GetProp("fixed_latency", pipeInfo.fixedLatency_);
                    curNode->GetProp("render_in_idle_state", pipeInfo.renderInIdleState_);
                    curNode->GetProp("moduleName", pipeInfo.moduleName_);
                    break;
                case PipeType::STREAM_PROP:
                    ParseStreamProps(curNode->GetCopyNode(), pipeInfo);
                    break;
                case PipeType::CONFIGS:
                    ParseConfigs(curNode->GetCopyNode(), pipeInfo);
                    break;
                default:
                    ParsePipeInfos(curNode->GetChildrenNode(), pipeInfo);
                    break;
            }
        }
        curNode->MoveToNext();
    }
}

AdapterType AudioPolicyParser::GetAdapterTypeAsInt(std::shared_ptr<AudioXmlNode> curNode)
{
    if (curNode->CompareName("pipes")) {
        return AdapterType::PIPES;
    } else if (curNode->CompareName("devices")) {
        return AdapterType::DEVICES;
    } else {
        return AdapterType::UNKNOWN;
    }
}

PipeType AudioPolicyParser::GetPipeInfoTypeAsInt(std::shared_ptr<AudioXmlNode> curNode)
{
    if (curNode->CompareName("paProp")) {
        return PipeType::PA_PROP;
    } else if (curNode->CompareName("streamProps")) {
        return PipeType::STREAM_PROP;
    } else if (curNode->CompareName("attributes")) {
        return PipeType::CONFIGS;
    } else {
        return PipeType::UNKNOWN;
    }
}

void AudioPolicyParser::ParseStreamProps(std::shared_ptr<AudioXmlNode> curNode, PipeInfo &pipeInfo)
{
    curNode->MoveToChildren();
    std::list<StreamPropInfo> streamPropInfos;

    while (curNode->IsNodeValid()) {
        if (curNode->IsElementNode()) {
            StreamPropInfo streamPropInfo = {};
            curNode->GetProp("format", streamPropInfo.format_);

            std::string sampleRateStr;
            curNode->GetProp("sampleRates", sampleRateStr);
            StringConverter(sampleRateStr, streamPropInfo.sampleRate_);
            pipeInfo.sampleRates_.push_back(streamPropInfo.sampleRate_);

            std::string periodInMsStr;
            curNode->GetProp("periodInMs", periodInMsStr);
            StringConverter(periodInMsStr, streamPropInfo.periodInMs_);

            std::string channelLayoutStr;
            curNode->GetProp("channelLayout", channelLayoutStr);
            streamPropInfo.channelLayout_ = layoutStrToChannels[channelLayoutStr];
            pipeInfo.channelLayouts_.push_back(streamPropInfo.channelLayout_);

            std::string bufferSizeStr;
            int32_t ret = curNode->GetProp("bufferSize", bufferSizeStr);
            if (ret == SUCCESS) {
                StringConverter(bufferSizeStr, streamPropInfo.bufferSize_);
            } else {
                streamPropInfo.bufferSize_ = formatStrToEnum[streamPropInfo.format_] * streamPropInfo.sampleRate_ *
                    streamPropInfo.periodInMs_ * streamPropInfo.channelLayout_ / AUDIO_MS_PER_S;
            }
            streamPropInfos.push_back(streamPropInfo);
        }
        curNode->MoveToNext();
    }
    pipeInfo.streamPropInfos_ = streamPropInfos;
}

void AudioPolicyParser::ParseConfigs(std::shared_ptr<AudioXmlNode> curNode, PipeInfo &pipeInfo)
{
    curNode->MoveToChildren();
    std::list<ConfigInfo> configInfos;

    while (curNode->IsNodeValid()) {
        if (curNode->IsElementNode()) {
            ConfigInfo configInfo = {};
            curNode->GetProp("name", configInfo.name_);
            curNode->GetProp("value", configInfo.value_);
            configInfos.push_back(configInfo);
            HandleConfigFlagAndUsage(configInfo, pipeInfo);
        }
        curNode->MoveToNext();
    }
    if (pipeInfo.audioUsage_ == AUDIO_USAGE_VOIP && pipeInfo.audioFlag_ == AUDIO_FLAG_MMAP) {
        portObserver_.OnVoipConfigParsed(true);
    }
    pipeInfo.configInfos_ = configInfos;
}

void AudioPolicyParser::HandleConfigFlagAndUsage(ConfigInfo &configInfo, PipeInfo &pipeInfo)
{
    if (configInfo.name_ == "flag") {
        auto it = audioFlagStrToEnum.find(configInfo.value_);
        if (it != audioFlagStrToEnum.end()) {
            pipeInfo.audioFlag_ = static_cast<int32_t>(it->second);
        }
    } else if (configInfo.name_ == "usage") {
        auto it = audioUsageStrToEnum.find(configInfo.value_);
        if (it != audioUsageStrToEnum.end()) {
            pipeInfo.audioUsage_ = static_cast<int32_t>(it->second);
        }
    }
}

void AudioPolicyParser::ParseDevices(std::shared_ptr<AudioXmlNode> curNode, AudioAdapterInfo &adapterInfo)
{
    curNode->MoveToChildren();
    std::list<AudioPipeDeviceInfo> deviceInfos = {};

    while (curNode->IsNodeValid()) {
        if (curNode->IsElementNode()) {
            AudioPipeDeviceInfo deviceInfo = {};
            curNode->GetProp("name", deviceInfo.name_);
            curNode->GetProp("type", deviceInfo.type_);
            curNode->GetProp("pin", deviceInfo.pin_);
            curNode->GetProp("role", deviceInfo.role_);

            std::string supportPipeInStr;
            curNode->GetProp("supportPipes", supportPipeInStr);
            SplitStringToList(supportPipeInStr, deviceInfo.supportPipes_);
            deviceInfos.push_back(deviceInfo);
        }
        curNode->MoveToNext();
    }
    adapterInfo.deviceInfos_ = deviceInfos;
}

void AudioPolicyParser::SplitStringToList(std::string &str, std::list<std::string> &result)
{
    char *token = std::strtok(&str[0], ",");
    while (token != nullptr) {
        result.push_back(token);
        token = std::strtok(nullptr, ",");
    }
}

void AudioPolicyParser::ParseGroups(std::shared_ptr<AudioXmlNode> curNode, XmlNodeType type)
{
    curNode->MoveToChildren();

    while (curNode->IsNodeValid()) {
        if (curNode->IsElementNode()) {
            ParseGroup(curNode->GetCopyNode(), type);
        }
        curNode->MoveToNext();
    }
}

void AudioPolicyParser::ParseGroup(std::shared_ptr<AudioXmlNode> curNode, XmlNodeType type)
{
    curNode->MoveToChildren();

    while (curNode->IsNodeValid()) {
        if (curNode->IsElementNode()) {
            std::string groupName;
            curNode->GetProp("name", groupName);
            ParseGroupSink(curNode->GetCopyNode(), type, groupName);
        }
        curNode->MoveToNext();
    }
}

void AudioPolicyParser::ParseGroupSink(std::shared_ptr<AudioXmlNode> curNode, XmlNodeType type, std::string &groupName)
{
    curNode->MoveToChildren();

    while (curNode->IsNodeValid()) {
        if (curNode->IsElementNode()) {
            std::string sinkName;
            curNode->GetProp("name", sinkName);
            if (type == XmlNodeType::VOLUME_GROUPS) {
                volumeGroupMap_[sinkName] = groupName;
            } else if (type == XmlNodeType::INTERRUPT_GROUPS) {
                interruptGroupMap_[sinkName] = groupName;
            }
        }
        curNode->MoveToNext();
    }
}

void AudioPolicyParser::ParseGlobalConfigs(std::shared_ptr<AudioXmlNode> curNode)
{
    curNode->MoveToChildren();
    while (curNode->IsNodeValid()) {
        if (curNode->IsElementNode()) {
            switch (GetGlobalConfigTypeAsInt(curNode)) {
                case GlobalConfigType::DEFAULT_OUTPUT:
                    curNode->GetProp("adapter", globalConfigs_.adapter_);
                    curNode->GetProp("pipe", globalConfigs_.pipe_);
                    curNode->GetProp("device", globalConfigs_.device_);
                    break;
                case GlobalConfigType::COMMON_CONFIGS:
                    ParseCommonConfigs(curNode->GetCopyNode());
                    break;
                case GlobalConfigType::PA_CONFIGS:
                    ParsePAConfigs(curNode->GetCopyNode());
                    break;
                case GlobalConfigType::DEFAULT_MAX_CON_CURRENT_INSTANCE:
                    ParseDefaultMaxInstances(curNode->GetCopyNode());
                    break;
                default:
                    ParseGlobalConfigs(curNode->GetChildrenNode());
                    break;
            }
        }
        curNode->MoveToNext();
    }
}

GlobalConfigType AudioPolicyParser::GetGlobalConfigTypeAsInt(std::shared_ptr<AudioXmlNode> curNode)
{
    if (curNode->CompareName("defaultOutput")) {
        return GlobalConfigType::DEFAULT_OUTPUT;
    } else if (curNode->CompareName("commonConfigs")) {
        return GlobalConfigType::COMMON_CONFIGS;
    } else if (curNode->CompareName("paConfigs")) {
        return GlobalConfigType::PA_CONFIGS;
    } else if (curNode->CompareName("maxConcurrentInstances")) {
        return GlobalConfigType::DEFAULT_MAX_CON_CURRENT_INSTANCE;
    } else {
        return GlobalConfigType::UNKNOWN;
    }
}

void AudioPolicyParser::ParsePAConfigs(std::shared_ptr<AudioXmlNode> curNode)
{
    curNode->MoveToChildren();

    while (curNode->IsNodeValid()) {
        if (curNode->IsElementNode()) {
            std::string name;
            std::string value;
            curNode->GetProp("name", name);
            curNode->GetProp("value", value);
            uint64_t convertValue = 0;

            switch (GetPaConfigType(name)) {
                case PAConfigType::AUDIO_LATENCY:
                    CHECK_AND_RETURN_LOG(StringConverter(value, convertValue),
                        "convert invalid value: %{public}s", value.c_str());
                    portObserver_.OnAudioLatencyParsed(convertValue);
                    globalConfigs_.globalPaConfigs_.audioLatency_ = value;
                    break;
                case PAConfigType::SINK_LATENCY:
                    CHECK_AND_RETURN_LOG(StringConverter(value, convertValue),
                        "convert invalid value: %{public}s", value.c_str());
                    portObserver_.OnSinkLatencyParsed(convertValue);
                    globalConfigs_.globalPaConfigs_.sinkLatency_ = value;
                    break;
                default:
                    ParsePAConfigs(curNode->GetChildrenNode());
                    break;
            }
        }
        curNode->MoveToNext();
    }
}

void AudioPolicyParser::ParseDefaultMaxInstances(std::shared_ptr<AudioXmlNode> curNode)
{
    curNode->MoveToChildren();
    while (curNode->IsNodeValid()) {
        if (curNode->IsElementNode()) {
            std::string sExtendInfo;
            curNode->GetContent(sExtendInfo);
            switch (GetDefaultMaxInstanceTypeAsInt(curNode)) {
                case DefaultMaxInstanceType::OUTPUT:
                    ParseOutputMaxInstances(curNode->GetCopyNode());
                    break;
                case DefaultMaxInstanceType::INPUT:
                    ParseInputMaxInstances(curNode->GetCopyNode());
                    break;
                default:
                    ParseDefaultMaxInstances(curNode->GetChildrenNode());
                    break;
            }
        }
        curNode->MoveToNext();
    }
}

void AudioPolicyParser::ParseOutputMaxInstances(std::shared_ptr<AudioXmlNode> curNode)
{
    curNode->MoveToChildren();
    std::list<ConfigInfo> configInfos;

    while (curNode->IsNodeValid()) {
        if (curNode->IsElementNode()) {
            ConfigInfo configInfo = {};
            curNode->GetProp("name", configInfo.name_);
            curNode->GetProp("flag", configInfo.type_);
            curNode->GetProp("value", configInfo.value_);
            configInfos.push_back(configInfo);
        }
        curNode->MoveToNext();
    }
    globalConfigs_.outputConfigInfos_ = configInfos;
}

void AudioPolicyParser::ParseInputMaxInstances(std::shared_ptr<AudioXmlNode> curNode)
{
    curNode->MoveToChildren();
    std::list<ConfigInfo> configInfos;

    while (curNode->IsNodeValid()) {
        if (curNode->IsElementNode()) {
            ConfigInfo configInfo = {};
            curNode->GetProp("name", configInfo.name_);
            curNode->GetProp("flag", configInfo.type_);
            curNode->GetProp("value", configInfo.value_);
            configInfos.push_back(configInfo);
        }
        curNode->MoveToNext();
    }
    globalConfigs_.inputConfigInfos_ = configInfos;
}

void AudioPolicyParser::ParseCommonConfigs(std::shared_ptr<AudioXmlNode> curNode)
{
    curNode->MoveToChildren();
    std::list<ConfigInfo> configInfos;

    while (curNode->IsNodeValid()) {
        if (curNode->IsElementNode()) {
            ConfigInfo configInfo = {};
            curNode->GetProp("name", configInfo.name_);
            curNode->GetProp("value", configInfo.value_);
            configInfos.push_back(configInfo);
            if (configInfo.name_ == "updateRouteSupport") {
                AUDIO_INFO_LOG("update route support: %{public}s", configInfo.value_.c_str());
                HandleUpdateRouteSupportParsed(configInfo.value_);
            } else if (configInfo.name_ == "anahsShowType") {
                AUDIO_INFO_LOG("anahs pc support: %{public}s", configInfo.value_.c_str());
                HandleUpdateAnahsSupportParsed(configInfo.value_);
            } else if (configInfo.name_ == "setDefaultAdapter") {
                AUDIO_INFO_LOG("default adapter support: %{public}s", configInfo.value_.c_str());
                HandleDefaultAdapterSupportParsed(configInfo.value_);
            }
        }
        curNode->MoveToNext();
    }
    globalConfigs_.commonConfigs_ = configInfos;
}

void AudioPolicyParser::HandleUpdateRouteSupportParsed(std::string &value)
{
    if (value == "true") {
        portObserver_.OnUpdateRouteSupport(true);
        shouldOpenMicSpeaker_ = true;
    } else {
        portObserver_.OnUpdateRouteSupport(false);
        shouldOpenMicSpeaker_ = false;
    }
}

void AudioPolicyParser::HandleUpdateAnahsSupportParsed(std::string &value)
{
    std::string anahsShowType = "Dialog";
    anahsShowType = value;
    AUDIO_INFO_LOG("HandleUpdateAnahsSupportParsed show type: %{public}s", anahsShowType.c_str());
    portObserver_.OnUpdateAnahsSupport(anahsShowType);
}

void AudioPolicyParser::HandleDefaultAdapterSupportParsed(std::string &value)
{
    if (value == "true") {
        portObserver_.OnUpdateDefaultAdapter(true);
        shouldSetDefaultAdapter_ = true;
    } else {
        portObserver_.OnUpdateDefaultAdapter(false);
        shouldSetDefaultAdapter_ = false;
    }
}

XmlNodeType AudioPolicyParser::GetXmlNodeTypeAsInt(std::shared_ptr<AudioXmlNode> curNode)
{
    if (curNode->CompareName("adapters")) {
        return XmlNodeType::ADAPTERS;
    } else if (curNode->CompareName("volumeGroups")) {
        return XmlNodeType::VOLUME_GROUPS;
    } else if (curNode->CompareName("interruptGroups")) {
        return XmlNodeType::INTERRUPT_GROUPS;
    } else if (curNode->CompareName("globalConfigs")) {
        return XmlNodeType::GLOBAL_CONFIGS;
    } else {
        return XmlNodeType::XML_UNKNOWN;
    }
}

AdaptersType AudioPolicyParser::GetAdaptersType(const std::string &adapterName)
{
    if (adapterName == ADAPTER_PRIMARY_TYPE)
        return AdaptersType::TYPE_PRIMARY;
    else if (adapterName == ADAPTER_A2DP_TYPE)
        return AdaptersType::TYPE_A2DP;
    else if (adapterName == ADAPTER_REMOTE_TYPE)
        return AdaptersType::TYPE_REMOTE_AUDIO;
    else if (adapterName == ADAPTER_FILE_TYPE)
        return AdaptersType::TYPE_FILE_IO;
    else if (adapterName == ADAPTER_USB_TYPE)
        return AdaptersType::TYPE_USB;
    else if (adapterName == ADAPTER_DP_TYPE)
        return AdaptersType::TYPE_DP;
    else
        return AdaptersType::TYPE_INVALID;
}

PAConfigType AudioPolicyParser::GetPaConfigType(std::string &name)
{
    if (name =="audioLatency") {
        return PAConfigType::AUDIO_LATENCY;
    } else if (name =="sinkLatency") {
        return PAConfigType::SINK_LATENCY;
    } else {
        return PAConfigType::UNKNOWN;
    }
}

DefaultMaxInstanceType AudioPolicyParser::GetDefaultMaxInstanceTypeAsInt(std::shared_ptr<AudioXmlNode> curNode)
{
    if (curNode->CompareName("output")) {
        return DefaultMaxInstanceType::OUTPUT;
    } else if (curNode->CompareName("input")) {
        return DefaultMaxInstanceType::INPUT;
    } else {
        return DefaultMaxInstanceType::UNKNOWN;
    }
}
// LCOV_EXCL_STOP
} // namespace AudioStandard
} // namespace OHOS
