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

#include <sstream>

namespace OHOS {
namespace AudioStandard {
constexpr int32_t AUDIO_MS_PER_S = 1000;
constexpr uint32_t LAYOUT_MONO_CHANNEL_ENUM = 1;
constexpr uint32_t LAYOUT_STEREO_CHANNEL_ENUM = 2;
constexpr uint32_t LAYOUT_4POINT0_CHANNEL_ENUM = 4;
constexpr uint32_t LAYOUT_QUAD_CHANNEL_ENUM = 4;
constexpr uint32_t LAYOUT_5POINT1_CHANNEL_ENUM = 6;
constexpr uint32_t LAYOUT_7POINT1_CHANNEL_ENUM = 8;
constexpr uint32_t S16LE_TO_BYTE = 2;
constexpr uint32_t S24LE_TO_BYTE = 3;
constexpr uint32_t S32LE_TO_BYTE = 4;

static std::map<std::string, uint32_t> layoutStrToChannels = {
    {"CH_LAYOUT_MONO", LAYOUT_MONO_CHANNEL_ENUM},
    {"CH_LAYOUT_STEREO", LAYOUT_STEREO_CHANNEL_ENUM},
    {"CH_LAYOUT_4POINT0", LAYOUT_4POINT0_CHANNEL_ENUM},
    {"CH_LAYOUT_QUAD", LAYOUT_QUAD_CHANNEL_ENUM},
    {"CH_LAYOUT_5POINT1", LAYOUT_5POINT1_CHANNEL_ENUM},
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

bool AudioPolicyParser::LoadConfiguration()
{
    AUDIO_INFO_LOG("Enter");
    doc_ = xmlReadFile(CHIP_PROD_CONFIG_FILE, nullptr, 0);
    if (doc_ == nullptr) {
        doc_ = xmlReadFile(CONFIG_FILE, nullptr, 0);
        if (doc_ == nullptr) {
            AUDIO_ERR_LOG("xmlReadFile Failed");
            return false;
        }
    }
    AUDIO_INFO_LOG("Done");
    return true;
}

// LCOV_EXCL_START
bool AudioPolicyParser::Parse()
{
    AUDIO_INFO_LOG("Enter");
    xmlNode *root = xmlDocGetRootElement(doc_);
    if (root == nullptr) {
        AUDIO_ERR_LOG("xmlDocGetRootElement Failed");
        return false;
    }
    if (!ParseInternal(*root)) {
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
    if (doc_ != nullptr) {
        xmlFreeDoc(doc_);
    }
}

bool AudioPolicyParser::ParseInternal(xmlNode &node)
{
    xmlNode *currNode = &node;
    for (; currNode; currNode = currNode->next) {
        if (XML_ELEMENT_NODE == currNode->type) {
            switch (GetXmlNodeTypeAsInt(*currNode)) {
                case XmlNodeType::ADAPTERS:
                    ParseAdapters(*currNode);
                    break;
                case XmlNodeType::VOLUME_GROUPS:
                    ParseGroups(*currNode, XmlNodeType::VOLUME_GROUPS);
                    break;
                case XmlNodeType::INTERRUPT_GROUPS:
                    ParseGroups(*currNode, XmlNodeType::INTERRUPT_GROUPS);
                    break;
                case XmlNodeType::GLOBAL_CONFIGS:
                    ParseGlobalConfigs(*currNode);
                    break;
                default:
                    ParseInternal(*(currNode->children));
                    break;
            }
        }
    }
    return true;
}

void AudioPolicyParser::ParseAdapters(xmlNode &node)
{
    xmlNode *currNode = nullptr;
    currNode = node.xmlChildrenNode;

    while (currNode != nullptr) {
        if (currNode->type == XML_ELEMENT_NODE) {
            ParseAdapter(*currNode);
        }
        currNode = currNode->next;
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
        }
        ClassType classType = GetClassTypeByAdapterType(adapterType);
        xmlParsedDataMap_[classType] = audioModuleListData;
    }
}

void AudioPolicyParser::ParseAdapter(xmlNode &node)
{
    std::string adapterName = ExtractPropertyValue("name", node);
    if (adapterName.empty()) {
        AUDIO_ERR_LOG("No name provided for the adapter class %{public}s", node.name);
        return;
    }

    AdaptersType adaptersType = GetAdaptersType(adapterName);
    adapterInfoMap_[adaptersType] = {};

    AudioAdapterInfo adapterInfo = {};
    adapterInfo.adapterName_ = adapterName;
    adapterInfo.adaptersupportScene_ = ExtractPropertyValue("supportScene", node);

    xmlNode *currNode = node.xmlChildrenNode;
    while (currNode != nullptr) {
        if (currNode->type == XML_ELEMENT_NODE) {
            switch (GetAdapterTypeAsInt(*currNode)) {
                case AdapterType::PIPES:
                    ParsePipes(*currNode, adapterInfo);
                    break;
                case AdapterType::DEVICES:
                    ParseDevices(*currNode, adapterInfo);
                    break;
                default:
                    ParseAdapter(*(currNode->children));
                    break;
            }
        }
        currNode = currNode->next;
    }
    adapterInfoMap_[adaptersType] = adapterInfo;
}

void AudioPolicyParser::ParsePipes(xmlNode &node, AudioAdapterInfo &adapterInfo)
{
    xmlNode *currNode = node.xmlChildrenNode;
    std::list<PipeInfo> pipeInfos;

    while (currNode != nullptr) {
        if (currNode->type == XML_ELEMENT_NODE) {
            PipeInfo pipeInfo {};
            pipeInfo.name_ = ExtractPropertyValue("name", *currNode);
            pipeInfo.pipeRole_ = ExtractPropertyValue("role", *currNode);
            pipeInfo.pipeFlags_ = ExtractPropertyValue("flags", *currNode);
            ParsePipeInfos(*currNode, pipeInfo);
            pipeInfos.push_back(pipeInfo);
        }
        currNode = currNode->next;
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

void AudioPolicyParser::ParsePipeInfos(xmlNode &node, PipeInfo &pipeInfo)
{
    xmlNode *currNode = node.xmlChildrenNode;
    while (currNode != nullptr) {
        if (currNode->type == XML_ELEMENT_NODE) {
            switch (GetPipeInfoTypeAsInt(*currNode)) {
                case PipeType::PA_PROP:
                    pipeInfo.lib_ = ExtractPropertyValue("lib", *currNode);
                    pipeInfo.paPropRole_ = ExtractPropertyValue("role", *currNode);
                    pipeInfo.fixedLatency_ = ExtractPropertyValue("fixed_latency", *currNode);
                    pipeInfo.renderInIdleState_ = ExtractPropertyValue("render_in_idle_state", *currNode);
                    pipeInfo.moduleName_ = ExtractPropertyValue("moduleName", *currNode);
                    break;
                case PipeType::STREAM_PROP:
                    ParseStreamProps(*currNode, pipeInfo);
                    break;
                case PipeType::CONFIGS:
                    ParseConfigs(*currNode, pipeInfo);
                    break;
                default:
                    ParsePipeInfos(*(currNode->children), pipeInfo);
                    break;
            }
        }
        currNode = currNode->next;
    }
}

AdapterType AudioPolicyParser::GetAdapterTypeAsInt(xmlNode &node)
{
    if (!xmlStrcmp(node.name, reinterpret_cast<const xmlChar*>("pipes"))) {
        return AdapterType::PIPES;
    } else if (!xmlStrcmp(node.name, reinterpret_cast<const xmlChar*>("devices"))) {
        return AdapterType::DEVICES;
    } else {
        return AdapterType::UNKNOWN;
    }
}

PipeType AudioPolicyParser::GetPipeInfoTypeAsInt(xmlNode &node)
{
    if (!xmlStrcmp(node.name, reinterpret_cast<const xmlChar*>("paProp"))) {
        return PipeType::PA_PROP;
    } else if (!xmlStrcmp(node.name, reinterpret_cast<const xmlChar*>("streamProps"))) {
        return PipeType::STREAM_PROP;
    } else if (!xmlStrcmp(node.name, reinterpret_cast<const xmlChar*>("attributes"))) {
        return PipeType::CONFIGS;
    } else {
        return PipeType::UNKNOWN;
    }
}

void AudioPolicyParser::ParseStreamProps(xmlNode &node, PipeInfo &pipeInfo)
{
    xmlNode *currNode = node.xmlChildrenNode;
    std::list<StreamPropInfo> streamPropInfos;

    while (currNode != nullptr) {
        if (currNode->type == XML_ELEMENT_NODE) {
            StreamPropInfo streamPropInfo = {};
            streamPropInfo.format_ = ExtractPropertyValue("format", *currNode);
            std::string sampleRateStr = ExtractPropertyValue("sampleRates", *currNode);
            if (sampleRateStr != "") {
                streamPropInfo.sampleRate_ = (uint32_t)std::stoi(sampleRateStr);
                pipeInfo.sampleRates_.push_back(streamPropInfo.sampleRate_);
            }
            std::string periodInMsStr = ExtractPropertyValue("periodInMs", *currNode);
            if (periodInMsStr != "") {
                streamPropInfo.periodInMs_ = (uint32_t)std::stoi(periodInMsStr);
            }
            std::string channelLayoutStr = ExtractPropertyValue("channelLayout", *currNode);
            if (channelLayoutStr != "") {
                streamPropInfo.channelLayout_ = layoutStrToChannels[channelLayoutStr];
                pipeInfo.channelLayouts_.push_back(streamPropInfo.channelLayout_);
            }

            std::string bufferSizeStr = ExtractPropertyValue("bufferSize", *currNode);
            if (bufferSizeStr != "") {
                streamPropInfo.bufferSize_ = (uint32_t)std::stoi(bufferSizeStr);
            } else {
                streamPropInfo.bufferSize_ = formatStrToEnum[streamPropInfo.format_] * streamPropInfo.sampleRate_ *
                    streamPropInfo.periodInMs_ * streamPropInfo.channelLayout_ / AUDIO_MS_PER_S;
            }

            streamPropInfos.push_back(streamPropInfo);
        }
        currNode = currNode->next;
    }
    pipeInfo.streamPropInfos_ = streamPropInfos;
}

void AudioPolicyParser::ParseConfigs(xmlNode &node, PipeInfo &pipeInfo)
{
    xmlNode *configNode = nullptr;
    configNode = node.xmlChildrenNode;
    std::list<ConfigInfo> configInfos;

    while (configNode != nullptr) {
        if (configNode->type == XML_ELEMENT_NODE) {
            ConfigInfo configInfo = {};
            configInfo.name_ = ExtractPropertyValue("name", *configNode);
            configInfo.value_ = ExtractPropertyValue("value", *configNode);
            configInfos.push_back(configInfo);
            HandleConfigFlagAndUsage(configInfo, pipeInfo);
        }
        configNode = configNode->next;
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

void AudioPolicyParser::ParseDevices(xmlNode &node, AudioAdapterInfo &adapterInfo)
{
    xmlNode *currNode = nullptr;
    currNode = node.xmlChildrenNode;
    std::list<AudioPipeDeviceInfo> deviceInfos = {};

    while (currNode != nullptr) {
        if (currNode->type == XML_ELEMENT_NODE) {
            AudioPipeDeviceInfo deviceInfo = {};
            deviceInfo.name_ = ExtractPropertyValue("name", *currNode);
            deviceInfo.type_ = ExtractPropertyValue("type", *currNode);
            deviceInfo.pin_ = ExtractPropertyValue("pin", *currNode);
            deviceInfo.role_ = ExtractPropertyValue("role", *currNode);
            std::string supportPipeInStr = ExtractPropertyValue("supportPipes", *currNode);
            SplitStringToList(supportPipeInStr, deviceInfo.supportPipes_);
            deviceInfos.push_back(deviceInfo);
        }
        currNode = currNode->next;
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

void AudioPolicyParser::ParseGroups(xmlNode &node, XmlNodeType type)
{
    xmlNode *currNode = nullptr;
    currNode = node.xmlChildrenNode;

    while (currNode != nullptr) {
        if (currNode->type == XML_ELEMENT_NODE) {
            ParseGroup(*currNode, type);
        }
        currNode = currNode->next;
    }
}

void AudioPolicyParser::ParseGroup(xmlNode &node, XmlNodeType type)
{
    xmlNode *currNode = nullptr;
    currNode = node.xmlChildrenNode;

    while (currNode != nullptr) {
        if (currNode->type == XML_ELEMENT_NODE) {
            std::string groupName = ExtractPropertyValue("name", *currNode);
            ParseGroupSink(*currNode, type, groupName);
        }
        currNode = currNode->next;
    }
}

void AudioPolicyParser::ParseGroupSink(xmlNode &node, XmlNodeType type, std::string &groupName)
{
    xmlNode *currNode = nullptr;
    currNode = node.xmlChildrenNode;

    while (currNode != nullptr) {
        if (currNode->type == XML_ELEMENT_NODE) {
            std::string sinkName = ExtractPropertyValue("name", *currNode);
            if (type == XmlNodeType::VOLUME_GROUPS) {
                volumeGroupMap_[sinkName] = groupName;
            } else if (type == XmlNodeType::INTERRUPT_GROUPS) {
                interruptGroupMap_[sinkName] = groupName;
            }
        }
        currNode = currNode->next;
    }
}

void AudioPolicyParser::ParseGlobalConfigs(xmlNode &node)
{
    xmlNode *currNode = node.xmlChildrenNode;
    while (currNode != nullptr) {
        if (currNode->type == XML_ELEMENT_NODE) {
            switch (GetGlobalConfigTypeAsInt(*currNode)) {
                case GlobalConfigType::DEFAULT_OUTPUT:
                    globalConfigs_.adapter_ = ExtractPropertyValue("adapter", *currNode);
                    globalConfigs_.pipe_ = ExtractPropertyValue("pipe", *currNode);
                    globalConfigs_.device_ = ExtractPropertyValue("device", *currNode);
                    break;
                case GlobalConfigType::COMMON_CONFIGS:
                    ParseCommonConfigs(*currNode);
                    break;
                case GlobalConfigType::PA_CONFIGS:
                    ParsePAConfigs(*currNode);
                    break;
                case GlobalConfigType::DEFAULT_MAX_CON_CURRENT_INSTANCE:
                    ParseDefaultMaxInstances(*currNode);
                    break;
                default:
                    ParseGlobalConfigs(*(currNode->children));
                    break;
            }
        }
        currNode = currNode->next;
    }
}

GlobalConfigType AudioPolicyParser::GetGlobalConfigTypeAsInt(xmlNode &node)
{
    if (!xmlStrcmp(node.name, reinterpret_cast<const xmlChar*>("defaultOutput"))) {
        return GlobalConfigType::DEFAULT_OUTPUT;
    } else if (!xmlStrcmp(node.name, reinterpret_cast<const xmlChar*>("commonConfigs"))) {
        return GlobalConfigType::COMMON_CONFIGS;
    } else if (!xmlStrcmp(node.name, reinterpret_cast<const xmlChar*>("paConfigs"))) {
        return GlobalConfigType::PA_CONFIGS;
    } else if (!xmlStrcmp(node.name, reinterpret_cast<const xmlChar*>("maxConcurrentInstances"))) {
        return GlobalConfigType::DEFAULT_MAX_CON_CURRENT_INSTANCE;
    } else {
        return GlobalConfigType::UNKNOWN;
    }
}

void AudioPolicyParser::ParsePAConfigs(xmlNode &node)
{
    xmlNode *currNode = nullptr;
    currNode = node.xmlChildrenNode;

    while (currNode != nullptr) {
        if (currNode->type == XML_ELEMENT_NODE) {
            std::string name = ExtractPropertyValue("name", *currNode);
            std::string value = ExtractPropertyValue("value", *currNode);

            switch (GetPaConfigType(name)) {
                case PAConfigType::AUDIO_LATENCY:
                    portObserver_.OnAudioLatencyParsed((uint64_t)std::stoi(value));
                    globalConfigs_.globalPaConfigs_.audioLatency_ = value;
                    break;
                case PAConfigType::SINK_LATENCY:
                    portObserver_.OnSinkLatencyParsed((uint64_t)std::stoi(value));
                    globalConfigs_.globalPaConfigs_.sinkLatency_ = value;
                    break;
                default:
                    ParsePAConfigs(*(currNode->children));
                    break;
            }
        }
        currNode = currNode->next;
    }
}

void AudioPolicyParser::ParseDefaultMaxInstances(xmlNode &node)
{
    xmlNode *currNode = node.xmlChildrenNode;
    while (currNode != nullptr) {
        if (currNode->type == XML_ELEMENT_NODE) {
            xmlChar *extendInfo = xmlNodeGetContent(currNode);
            std::string sExtendInfo(reinterpret_cast<char *>(extendInfo));
            switch (GetDefaultMaxInstanceTypeAsInt(*currNode)) {
                case DefaultMaxInstanceType::OUTPUT:
                    ParseOutputMaxInstances(*currNode);
                    break;
                case DefaultMaxInstanceType::INPUT:
                    ParseInputMaxInstances(*currNode);
                    break;
                default:
                    ParseDefaultMaxInstances(*(currNode->children));
                    break;
            }
        }
        currNode = currNode->next;
    }
}

void AudioPolicyParser::ParseOutputMaxInstances(xmlNode &node)
{
    xmlNode *currNode = nullptr;
    currNode = node.xmlChildrenNode;
    std::list<ConfigInfo> configInfos;

    while (currNode != nullptr) {
        if (currNode->type == XML_ELEMENT_NODE) {
            ConfigInfo configInfo = {};
            configInfo.name_ = ExtractPropertyValue("name", *currNode);
            configInfo.type_ = ExtractPropertyValue("flag", *currNode);
            configInfo.value_ = ExtractPropertyValue("value", *currNode);
            configInfos.push_back(configInfo);
        }
        currNode = currNode->next;
    }
    globalConfigs_.outputConfigInfos_ = configInfos;
}

void AudioPolicyParser::ParseInputMaxInstances(xmlNode &node)
{
    xmlNode *currNode = nullptr;
    currNode = node.xmlChildrenNode;
    std::list<ConfigInfo> configInfos;

    while (currNode != nullptr) {
        if (currNode->type == XML_ELEMENT_NODE) {
            ConfigInfo configInfo = {};
            configInfo.name_ = ExtractPropertyValue("name", *currNode);
            configInfo.type_ = ExtractPropertyValue("flag", *currNode);
            configInfo.value_ = ExtractPropertyValue("value", *currNode);
            configInfos.push_back(configInfo);
        }
        currNode = currNode->next;
    }
    globalConfigs_.inputConfigInfos_ = configInfos;
}

void AudioPolicyParser::ParseCommonConfigs(xmlNode &node)
{
    xmlNode *currNode = nullptr;
    currNode = node.xmlChildrenNode;
    std::list<ConfigInfo> configInfos;

    while (currNode != nullptr) {
        if (currNode->type == XML_ELEMENT_NODE) {
            ConfigInfo configInfo = {};
            configInfo.name_ = ExtractPropertyValue("name", *currNode);
            configInfo.value_ = ExtractPropertyValue("value", *currNode);
            configInfos.push_back(configInfo);
            if (configInfo.name_ == "updateRouteSupport") {
                AUDIO_INFO_LOG("update route support: %{public}s", configInfo.value_.c_str());
                HandleUpdateRouteSupportParsed(configInfo.value_);
            }
        }
        currNode = currNode->next;
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

XmlNodeType AudioPolicyParser::GetXmlNodeTypeAsInt(xmlNode &node)
{
    if (!xmlStrcmp(node.name, reinterpret_cast<const xmlChar*>("adapters"))) {
        return XmlNodeType::ADAPTERS;
    } else if (!xmlStrcmp(node.name, reinterpret_cast<const xmlChar*>("volumeGroups"))) {
        return XmlNodeType::VOLUME_GROUPS;
    } else if (!xmlStrcmp(node.name, reinterpret_cast<const xmlChar*>("interruptGroups"))) {
        return XmlNodeType::INTERRUPT_GROUPS;
    } else if (!xmlStrcmp(node.name, reinterpret_cast<const xmlChar*>("globalConfigs"))) {
        return XmlNodeType::GLOBAL_CONFIGS;
    } else {
        return XmlNodeType::XML_UNKNOWN;
    }
}

std::string AudioPolicyParser::ExtractPropertyValue(const std::string &propName, xmlNode &node)
{
    std::string propValue = "";
    xmlChar *tempValue = nullptr;

    if (xmlHasProp(&node, reinterpret_cast<const xmlChar*>(propName.c_str()))) {
        tempValue = xmlGetProp(&node, reinterpret_cast<const xmlChar*>(propName.c_str()));
    }

    if (tempValue != nullptr) {
        propValue = reinterpret_cast<const char*>(tempValue);
        xmlFree(tempValue);
    }

    return propValue;
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

DefaultMaxInstanceType AudioPolicyParser::GetDefaultMaxInstanceTypeAsInt(xmlNode &node)
{
    if (!xmlStrcmp(node.name, reinterpret_cast<const xmlChar*>("output"))) {
        return DefaultMaxInstanceType::OUTPUT;
    } else if (!xmlStrcmp(node.name, reinterpret_cast<const xmlChar*>("input"))) {
        return DefaultMaxInstanceType::INPUT;
    } else {
        return DefaultMaxInstanceType::UNKNOWN;
    }
}

StreamType AudioPolicyParser::GetStreamTypeAsInt(xmlNode &node)
{
    if (!xmlStrcmp(node.name, reinterpret_cast<const xmlChar*>("normal"))) {
        return StreamType::NORMAL;
    } else if (!xmlStrcmp(node.name, reinterpret_cast<const xmlChar*>("fast"))) {
        return StreamType::FAST;
    } else {
        return StreamType::UNKNOWN;
    }
}
// LCOV_EXCL_STOP
} // namespace AudioStandard
} // namespace OHOS
