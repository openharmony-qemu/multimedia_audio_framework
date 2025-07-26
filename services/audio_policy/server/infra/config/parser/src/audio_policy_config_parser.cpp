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
#define LOG_TAG "AudioPolicyConfigParser"
#endif

#include <sstream>
#include "audio_policy_config_parser.h"

#include "audio_errors.h"
#include "audio_adapter_info.h"

namespace OHOS {
namespace AudioStandard {
static const char *ENCODING_EAC3_NAME = "eac3";
static const char *FAST_DISTRIBUTE_TAG = "fast_distributed";

// LCOV_EXCL_START
bool AudioPolicyConfigParser::LoadConfiguration()
{
    AUDIO_INFO_LOG("Enter");
    if (curNode_->Config(CHIP_PROD_CONFIG_FILE, nullptr, 0) != SUCCESS) {
        if (curNode_->Config(CONFIG_FILE, nullptr, 0) != SUCCESS) {
            AUDIO_ERR_LOG("LoadConfiguration readFile failed");
            return false;
        }
    }
    
    AudioPolicyConfigData &config = AudioPolicyConfigData::GetInstance();
    if (curNode_->CompareName("audioPolicyConfiguration")) {
        std::string version;
        curNode_->GetProp("version", version);
        config.SetVersion(version);
    }

    if (!ParseInternal(curNode_->GetChildrenNode())) {
        AUDIO_ERR_LOG("Audio policy config xml parse failed");
        return false;
    }

    std::unordered_map<std::string, std::string> volumeGroupMap {};
    std::unordered_map<std::string, std::string> interruptGroupMap {};

    ConvertAdapterInfoToGroupInfo(volumeGroupMap, interruptGroupMap);
    ConvertAdapterInfoToAudioModuleInfo();

    volumeGroupMap_ = volumeGroupMap;
    interruptGroupMap_ = interruptGroupMap;

    configManager_->OnAudioPolicyConfigXmlParsingCompleted();
    configManager_->OnXmlParsingCompleted(xmlParsedDataMap_);
    configManager_->OnVolumeGroupParsed(volumeGroupMap_);
    configManager_->OnInterruptGroupParsed(interruptGroupMap_);
    configManager_->OnGlobalConfigsParsed(globalConfigs_);

    AUDIO_INFO_LOG("Done");
    return true;
}

void AudioPolicyConfigParser::Destroy()
{
    curNode_->FreeDoc();
}

bool AudioPolicyConfigParser::ParseInternal(std::shared_ptr<AudioXmlNode> curNode)
{
    for (; curNode->IsNodeValid(); curNode->MoveToNext()) {
        if (curNode->IsElementNode()) {
            switch (GetXmlNodeTypeAsInt(curNode)) {
                case PolicyXmlNodeType::ADAPTERS:
                    ParseAdapters(curNode->GetCopyNode());
                    break;
                case PolicyXmlNodeType::VOLUME_GROUPS:
                    ParseGroups(curNode->GetCopyNode(), PolicyXmlNodeType::VOLUME_GROUPS);
                    break;
                case PolicyXmlNodeType::INTERRUPT_GROUPS:
                    ParseGroups(curNode->GetCopyNode(), PolicyXmlNodeType::INTERRUPT_GROUPS);
                    break;
                case PolicyXmlNodeType::GLOBAL_CONFIGS:
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

void AudioPolicyConfigParser::ParseAdapters(std::shared_ptr<AudioXmlNode> curNode)
{
    curNode->MoveToChildren();
    AudioPolicyConfigData &config = AudioPolicyConfigData::GetInstance();

    while (curNode->IsNodeValid()) {
        if (curNode->IsElementNode()) {
            PolicyAdapterInfo adapterInfo {};
            std::shared_ptr<PolicyAdapterInfo> adapterInfoPtr = std::make_shared<PolicyAdapterInfo>(adapterInfo);
            ParseAdapter(curNode->GetCopyNode(), adapterInfoPtr);
            config.adapterInfoMap.insert({adapterInfoPtr->GetTypeEnum(), adapterInfoPtr});
        }
        curNode->MoveToNext();
    }
}

void AudioPolicyConfigParser::ParseAdapter(std::shared_ptr<AudioXmlNode> curNode,
    std::shared_ptr<PolicyAdapterInfo> &adapterInfo)
{
    std::string adapterName;
    CHECK_AND_RETURN_LOG(curNode->GetProp("name", adapterName) == SUCCESS, "Get prop name failed");
    std::string supportScene;
    curNode->GetProp("supportSelectScene", supportScene);

    adapterInfo->adapterName = adapterName;
    adapterInfo->adapterSupportScene = supportScene;

    curNode->MoveToChildren();
    while (curNode->IsNodeValid()) {
        if (curNode->IsElementNode()) {
            switch (GetAdapterInfoTypeAsInt(curNode)) {
                case AdapterInfoType::PIPES:
                    ParsePipes(curNode->GetCopyNode(), adapterInfo);
                    break;
                case AdapterInfoType::DEVICES:
                    ParseDevices(curNode->GetCopyNode(), adapterInfo);
                    break;
                default:
                    ParseAdapter(curNode->GetChildrenNode(), adapterInfo);
                    break;
            }
        }
        curNode->MoveToNext();
    }
}

void AudioPolicyConfigParser::ParsePipes(std::shared_ptr<AudioXmlNode> curNode,
    std::shared_ptr<PolicyAdapterInfo> &adapterInfo)
{
    curNode->MoveToChildren();
    std::list<std::shared_ptr<AdapterPipeInfo>> pipeInfos = {};

    while (curNode->IsNodeValid()) {
        if (curNode->IsElementNode()) {
            std::shared_ptr<AdapterPipeInfo> pipeInfoPtr = std::make_shared<AdapterPipeInfo>();
            pipeInfoPtr->adapterInfo_ = adapterInfo;
            curNode->GetProp("name", pipeInfoPtr->name_);
            if (pipeInfoPtr->name_.find(FAST_DISTRIBUTE_TAG) != string::npos) {
                AUDIO_WARNING_LOG("Fast distribute is not supported");
                curNode->MoveToNext();
                continue;
            }
            std::string pipeRole;
            curNode->GetProp("role", pipeRole);
            pipeInfoPtr->role_ = AudioDefinitionPolicyUtils::pipeRoleStrToEnum[pipeRole];
            std::string supportDevicesStr;
            curNode->GetProp("supportDevices", supportDevicesStr);
            if (supportDevicesStr != "") {
                SplitStringToList(supportDevicesStr, pipeInfoPtr->supportDevices_, ", ");
            }
            ParsePipeInfos(curNode->GetCopyNode(), pipeInfoPtr);
            pipeInfos.push_back(pipeInfoPtr);
        }
        curNode->MoveToNext();
    }
    adapterInfo->pipeInfos = std::move(pipeInfos);
}

void AudioPolicyConfigParser::ParsePipeInfos(std::shared_ptr<AudioXmlNode> curNode,
    std::shared_ptr<AdapterPipeInfo> &pipeInfo)
{
    curNode->MoveToChildren();
    while (curNode->IsNodeValid()) {
        if (curNode->IsElementNode()) {
            switch (GetPipeInfoTypeAsInt(curNode)) {
                case PipeInfoType::PA_PROP:
                    ParsePaProp(curNode->GetCopyNode(), pipeInfo);
                    break;
                case PipeInfoType::STREAM_PROP:
                    ParseStreamProps(curNode->GetCopyNode(), pipeInfo);
                    break;
                case PipeInfoType::ATTRIBUTE:
                    ParseAttributes(curNode->GetCopyNode(), pipeInfo);
                    break;
                default:
                    ParsePipeInfos(curNode->GetChildrenNode(), pipeInfo);
                    break;
            }
        }
        curNode->MoveToNext();
    }
}

void AudioPolicyConfigParser::ParsePaProp(std::shared_ptr<AudioXmlNode> curNode,
    std::shared_ptr<AdapterPipeInfo> &pipeInfo)
{
    PaPropInfo paProp = {};

    if (!curNode->IsNodeValid() || !curNode->IsElementNode()) {
        pipeInfo->paProp_ = paProp;
        return;
    }

    curNode->GetProp("lib", paProp.lib_);
    curNode->GetProp("role", paProp.role_);
    curNode->GetProp("moduleName", paProp.moduleName_);
    curNode->GetProp("fixed_latency", paProp.fixedLatency_);
    curNode->GetProp("render_in_idle_state", paProp.renderInIdleState_);
    pipeInfo->paProp_ = std::move(paProp);
}

void AudioPolicyConfigParser::ParseStreamProps(std::shared_ptr<AudioXmlNode> curNode,
    std::shared_ptr<AdapterPipeInfo> &pipeInfo)
{
    curNode->MoveToChildren();
    std::list<std::shared_ptr<PipeStreamPropInfo>> streamPropInfos = {};

    while (curNode->IsNodeValid()) {
        if (curNode->IsElementNode()) {
            PipeStreamPropInfo streamPropInfo = {};
            streamPropInfo.pipeInfo_ = pipeInfo;
            std::string formatStr;
            curNode->GetProp("format", formatStr);
            HandleEncodingEac3SupportParsed(pipeInfo, formatStr);
            streamPropInfo.format_ = AudioDefinitionPolicyUtils::formatStrToEnum[formatStr];
            std::string sampleRateStr;
            curNode->GetProp("sampleRates", sampleRateStr);
            StringConverter(sampleRateStr, streamPropInfo.sampleRate_);
            std::string channelLayoutStr;
            curNode->GetProp("channelLayout", channelLayoutStr);
            streamPropInfo.channelLayout_ = AudioDefinitionPolicyUtils::layoutStrToEnum[channelLayoutStr];
            streamPropInfo.channels_ = AudioDefinitionPolicyUtils::ConvertLayoutToAudioChannel(
                streamPropInfo.channelLayout_);
            std::string bufferSizeStr;
            curNode->GetProp("bufferSize", bufferSizeStr);
            StringConverter(bufferSizeStr, streamPropInfo.bufferSize_);
            std::string supportDevicesStr;
            curNode->GetProp("supportDevices", supportDevicesStr);
            if (supportDevicesStr != "") {
                SplitStringToList(supportDevicesStr, streamPropInfo.supportDevices_, ", ");
            }
            streamPropInfos.push_back(std::make_shared<PipeStreamPropInfo>(streamPropInfo));
        }
        curNode->MoveToNext();
    }
    pipeInfo->streamPropInfos_ = std::move(streamPropInfos);
}

void AudioPolicyConfigParser::ParseAttributes(std::shared_ptr<AudioXmlNode> curNode,
    std::shared_ptr<AdapterPipeInfo> &pipeInfo)
{
    curNode->MoveToChildren();
    std::list<std::shared_ptr<AttributeInfo>> attributeInfos = {};

    while (curNode->IsNodeValid()) {
        if (curNode->IsElementNode()) {
            AttributeInfo attributeInfo = {};
            curNode->GetProp("name", attributeInfo.name_);
            curNode->GetProp("value", attributeInfo.value_);
            ParseAttributeByName(attributeInfo, pipeInfo);
            attributeInfos.push_back(std::make_shared<AttributeInfo>(attributeInfo));
        }
        curNode->MoveToNext();
    }
    pipeInfo->attributeInfos_ = std::move(attributeInfos);
}

void AudioPolicyConfigParser::ParseAttributeByName(AttributeInfo &attributeInfo,
    std::shared_ptr<AdapterPipeInfo> &pipeInfo)
{
    if (attributeInfo.name_ == "flag") {
        std::list<std::string> supportFlags = {};
        std::string tmpValue = attributeInfo.value_;
        SplitStringToList(tmpValue, supportFlags, "|");
        for (auto flag : supportFlags) {
            pipeInfo->supportFlags_ |= AudioDefinitionPolicyUtils::flagStrToEnum[flag];
        }

        if (pipeInfo->supportFlags_ & AUDIO_OUTPUT_FLAG_VOIP) {
            AUDIO_INFO_LOG("Use fast voip");
            configManager_->OnVoipConfigParsed(true);
        }
    } else if (attributeInfo.name_ == "preload") {
        pipeInfo->preloadAttr_ = AudioDefinitionPolicyUtils::preloadStrToEnum[attributeInfo.value_];
    }
}

void AudioPolicyConfigParser::ParseDevices(std::shared_ptr<AudioXmlNode> curNode,
    std::shared_ptr<PolicyAdapterInfo> &adapterInfo)
{
    curNode->MoveToChildren();
    std::list<std::shared_ptr<AdapterDeviceInfo>> deviceInfos = {};

    while (curNode->IsNodeValid()) {
        if (curNode->IsElementNode()) {
            AdapterDeviceInfo deviceInfo {};
            deviceInfo.adapterInfo_ = adapterInfo;
            curNode->GetProp("name", deviceInfo.name_);
            std::string type;
            curNode->GetProp("type", type);
            deviceInfo.type_ = AudioDefinitionPolicyUtils::deviceTypeStrToEnum[type];
            std::string pin;
            curNode->GetProp("pin", pin);
            deviceInfo.pin_ = AudioDefinitionPolicyUtils::pinStrToEnum[pin];
            std::string role;
            curNode->GetProp("role", role);
            deviceInfo.role_ = AudioDefinitionPolicyUtils::deviceRoleStrToEnum[role];
            std::string supportPipeInStr;
            curNode->GetProp("supportPipes", supportPipeInStr);
            SplitStringToList(supportPipeInStr, deviceInfo.supportPipes_, ",");
            deviceInfos.push_back(std::make_shared<AdapterDeviceInfo>(deviceInfo));
        }
        curNode->MoveToNext();
    }
    adapterInfo->deviceInfos = std::move(deviceInfos);
}

void AudioPolicyConfigParser::ParseGroups(std::shared_ptr<AudioXmlNode> curNode, PolicyXmlNodeType type)
{
    curNode->MoveToChildren();

    while (curNode->IsNodeValid()) {
        if (curNode->IsElementNode()) {
            ParseGroup(curNode->GetCopyNode(), type);
        }
        curNode->MoveToNext();
    }
}

void AudioPolicyConfigParser::ParseGroup(std::shared_ptr<AudioXmlNode> curNode, PolicyXmlNodeType type)
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

void AudioPolicyConfigParser::ParseGroupSink(
    std::shared_ptr<AudioXmlNode> curNode, PolicyXmlNodeType type, std::string &groupName)
{
    curNode->MoveToChildren();

    while (curNode->IsNodeValid()) {
        if (curNode->IsElementNode()) {
            std::string sinkName;
            curNode->GetProp("name", sinkName);
            if (type == PolicyXmlNodeType::VOLUME_GROUPS) {
                volumeGroupMap_[sinkName] = groupName;
            } else if (type == PolicyXmlNodeType::INTERRUPT_GROUPS) {
                interruptGroupMap_[sinkName] = groupName;
            }
        }
        curNode->MoveToNext();
    }
}

void AudioPolicyConfigParser::ParseGlobalConfigs(std::shared_ptr<AudioXmlNode> curNode)
{
    curNode->MoveToChildren();
    while (curNode->IsNodeValid()) {
        if (curNode->IsElementNode()) {
            switch (GetGlobalConfigTypeAsInt(curNode)) {
                case PolicyGlobalConfigType::DEFAULT_OUTPUT:
                    curNode->GetProp("adapter", globalConfigs_.adapter_);
                    curNode->GetProp("pipe", globalConfigs_.pipe_);
                    curNode->GetProp("device", globalConfigs_.device_);
                    break;
                case PolicyGlobalConfigType::COMMON_CONFIGS:
                    ParseCommonConfigs(curNode->GetCopyNode());
                    break;
                case PolicyGlobalConfigType::PA_CONFIGS:
                    ParsePAConfigs(curNode->GetCopyNode());
                    break;
                default:
                    ParseGlobalConfigs(curNode->GetChildrenNode());
                    break;
            }
        }
        curNode->MoveToNext();
    }
}

PolicyGlobalConfigType AudioPolicyConfigParser::GetGlobalConfigTypeAsInt(std::shared_ptr<AudioXmlNode> curNode)
{
    if (curNode->CompareName("defaultOutput")) {
        return PolicyGlobalConfigType::DEFAULT_OUTPUT;
    } else if (curNode->CompareName("commonConfigs")) {
        return PolicyGlobalConfigType::COMMON_CONFIGS;
    } else if (curNode->CompareName("paConfigs")) {
        return PolicyGlobalConfigType::PA_CONFIGS;
    } else {
        return PolicyGlobalConfigType::UNKNOWN;
    }
}

void AudioPolicyConfigParser::ParsePAConfigs(std::shared_ptr<AudioXmlNode> curNode)
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
                case PolicyPAConfigType::FAST_FORMAT:
                    configManager_->OnFastFormatParsed(ConvertToFormat(value));
                    break;
                case PolicyPAConfigType::AUDIO_LATENCY:
                    CHECK_AND_RETURN_LOG(StringConverter(value, convertValue),
                        "convert invalid value: %{public}s", value.c_str());
                    configManager_->OnAudioLatencyParsed(convertValue);
                    globalConfigs_.globalPaConfigs_.audioLatency_ = value;
                    break;
                case PolicyPAConfigType::SINK_LATENCY:
                    CHECK_AND_RETURN_LOG(StringConverter(value, convertValue),
                        "convert invalid value: %{public}s", value.c_str());
                    configManager_->OnSinkLatencyParsed(convertValue);
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

// only support s16le and s32le
AudioSampleFormat AudioPolicyConfigParser::ConvertToFormat(std::string value)
{
    AudioSampleFormat format = SAMPLE_S16LE;
    if (value == "s16le") {
        format = SAMPLE_S16LE;
    }

    if (value == "s32le") {
        format = SAMPLE_S32LE;
    }
    return format;
}

PolicyPAConfigType AudioPolicyConfigParser::GetPaConfigType(std::string &name)
{
    if (name =="audioLatency") {
        return PolicyPAConfigType::AUDIO_LATENCY;
    } else if (name =="sinkLatency") {
        return PolicyPAConfigType::SINK_LATENCY;
    } else if (name =="fastFormat") {
        return PolicyPAConfigType::FAST_FORMAT;
    } else {
        return PolicyPAConfigType::UNKNOWN;
    }
}


void AudioPolicyConfigParser::ParseCommonConfigs(std::shared_ptr<AudioXmlNode> curNode)
{
    curNode->MoveToChildren();
    std::list<PolicyConfigInfo> configInfos;

    while (curNode->IsNodeValid()) {
        if (curNode->IsElementNode()) {
            PolicyConfigInfo configInfo = {};
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

void AudioPolicyConfigParser::HandleUpdateRouteSupportParsed(std::string &value)
{
    if (value == "true") {
        configManager_->OnUpdateRouteSupport(true);
        shouldOpenMicSpeaker_ = true;
    } else {
        configManager_->OnUpdateRouteSupport(false);
        shouldOpenMicSpeaker_ = false;
    }
}

void AudioPolicyConfigParser::HandleUpdateAnahsSupportParsed(std::string &value)
{
    std::string anahsShowType = "Dialog";
    anahsShowType = value;
    AUDIO_INFO_LOG("HandleUpdateAnahsSupportParsed show type: %{public}s", anahsShowType.c_str());
    configManager_->OnUpdateAnahsSupport(anahsShowType);
}

void AudioPolicyConfigParser::HandleDefaultAdapterSupportParsed(std::string &value)
{
    if (value == "true") {
        configManager_->OnUpdateDefaultAdapter(true);
        shouldSetDefaultAdapter_ = true;
    } else {
        configManager_->OnUpdateDefaultAdapter(false);
        shouldSetDefaultAdapter_ = false;
    }
}

void AudioPolicyConfigParser::HandleEncodingEac3SupportParsed(std::shared_ptr<AdapterPipeInfo> pipeInfo,
    const std::string &value)
{
    CHECK_AND_RETURN_LOG(pipeInfo != nullptr, "pipeInfo is nullptr");
    if (value == ENCODING_EAC3_NAME) {
        pipeInfo->supportEncodingEac3_ = true;
        configManager_->OnUpdateEac3Support(true);
    }
}

void AudioPolicyConfigParser::SplitStringToList(std::string &str, std::list<std::string> &result, const char *delim)
{
    char *token = std::strtok(&str[0], delim);
    while (token != nullptr) {
        result.push_back(token);
        token = std::strtok(nullptr, delim);
    }
}

PolicyXmlNodeType AudioPolicyConfigParser::GetXmlNodeTypeAsInt(std::shared_ptr<AudioXmlNode> curNode)
{
    if (curNode->CompareName("adapters")) {
        return PolicyXmlNodeType::ADAPTERS;
    } else if (curNode->CompareName("volumeGroups")) {
        return PolicyXmlNodeType::VOLUME_GROUPS;
    } else if (curNode->CompareName("interruptGroups")) {
        return PolicyXmlNodeType::INTERRUPT_GROUPS;
    } else if (curNode->CompareName("globalConfigs")) {
        return PolicyXmlNodeType::GLOBAL_CONFIGS;
    } else {
        return PolicyXmlNodeType::XML_UNKNOWN;
    }
}

AdapterInfoType AudioPolicyConfigParser::GetAdapterInfoTypeAsInt(std::shared_ptr<AudioXmlNode> curNode)
{
    if (curNode->CompareName("pipes")) {
        return AdapterInfoType::PIPES;
    } else if (curNode->CompareName("devices")) {
        return AdapterInfoType::DEVICES;
    } else {
        return AdapterInfoType::UNKNOWN;
    }
}

PipeInfoType AudioPolicyConfigParser::GetPipeInfoTypeAsInt(std::shared_ptr<AudioXmlNode> curNode)
{
    if (curNode->CompareName("paProp")) {
        return PipeInfoType::PA_PROP;
    } else if (curNode->CompareName("streamProps")) {
        return PipeInfoType::STREAM_PROP;
    } else if (curNode->CompareName("attributes")) {
        return PipeInfoType::ATTRIBUTE;
    } else {
        return PipeInfoType::UNKNOWN;
    }
}


void AudioPolicyConfigParser::ConvertAdapterInfoToGroupInfo(
    std::unordered_map<std::string, std::string> &volumeGroupMap,
    std::unordered_map<std::string, std::string> &interruptGroupMap)
{
    for (auto &[sinkName, groupName] : volumeGroupMap_) {
        volumeGroupMap["Speaker"] = groupName;
    }

    for (auto &[sinkName, groupName] : interruptGroupMap_) {
        interruptGroupMap["Speaker"] = groupName;
    }
}

void AudioPolicyConfigParser::ConvertAdapterInfoToAudioModuleInfo()
{
    AudioPolicyConfigData &config = AudioPolicyConfigData::GetInstance();
    for (auto &adapterInfoIt : config.adapterInfoMap) {
        std::list<AudioModuleInfo> audioModuleList = {};
        bool shouldEnableOffload = false;
        if (adapterInfoIt.first == AudioAdapterType::TYPE_PRIMARY) {
            GetOffloadAndOpenMicState(adapterInfoIt.second, shouldEnableOffload);
        }

        AudioPipeRole currentRole = PIPE_ROLE_NONE;
        for (auto &pipeInfo : adapterInfoIt.second->pipeInfos) {
            if (currentRole == pipeInfo->role_) {
                continue;
            }
            currentRole = pipeInfo->role_;
            CHECK_AND_CONTINUE_LOG(pipeInfo->name_.find(MODULE_SINK_OFFLOAD) == std::string::npos,
                "skip offload out sink.");
            AudioModuleInfo audioModuleInfo = {};
            GetCommontAudioModuleInfo(pipeInfo, audioModuleInfo);

            audioModuleInfo.className = adapterInfoIt.second->adapterName;
            // The logic here strongly depends on the moduleName in the XML
            if (pipeInfo->paProp_.moduleName_ != "") {
                audioModuleInfo.name = pipeInfo->paProp_.moduleName_;
            } else {
                audioModuleInfo.name = GetAudioModuleInfoName(pipeInfo->name_, adapterInfoIt.second->deviceInfos);
            }

            audioModuleInfo.adapterName = adapterInfoIt.second->adapterName;
            if (adapterInfoIt.first == AudioAdapterType::TYPE_FILE_IO) {
                audioModuleInfo.adapterName = STR_INIT;
                audioModuleInfo.format = STR_INIT;
                audioModuleInfo.className = FILE_CLASS;
            }
            audioModuleInfo.sinkLatency = globalConfigs_.globalPaConfigs_.sinkLatency_;

            audioModuleInfo.OpenMicSpeaker = shouldOpenMicSpeaker_ ? "1" : "0";
            if (adapterInfoIt.first == AudioAdapterType::TYPE_PRIMARY &&
                shouldEnableOffload && pipeInfo->paProp_.role_ == MODULE_TYPE_SINK) {
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
                audioModuleInfo.name.c_str(), audioModuleInfo.adapterName.c_str(), adapterInfoIt.first);
        }
        ClassType classType = GetClassTypeByAdapterType(adapterInfoIt.first);
        xmlParsedDataMap_[classType] = audioModuleListData;
    }
}

void AudioPolicyConfigParser::GetOffloadAndOpenMicState(std::shared_ptr<PolicyAdapterInfo> &adapterInfo,
    bool &shouldEnableOffload)
{
    for (auto &pipeInfo : adapterInfo->pipeInfos) {
        if (pipeInfo->paProp_.role_ == MODULE_TYPE_SINK &&
            pipeInfo->name_.find(MODULE_SINK_OFFLOAD) != std::string::npos) {
            shouldEnableOffload = true;
        }
    }
}

void AudioPolicyConfigParser::GetCommontAudioModuleInfo(std::shared_ptr<AdapterPipeInfo> &pipeInfo,
    AudioModuleInfo &audioModuleInfo)
{
    audioModuleInfo.role = pipeInfo->paProp_.role_;

    for (auto &streamPropInfo : pipeInfo->streamPropInfos_) {
        audioModuleInfo.supportedRate_.insert(streamPropInfo->sampleRate_);
        audioModuleInfo.supportedChannelLayout_.insert(streamPropInfo->channelLayout_);
    }

    audioModuleInfo.lib = pipeInfo->paProp_.lib_;

    if (pipeInfo->streamPropInfos_.size() != 0) {
        audioModuleInfo.rate = std::to_string(pipeInfo->streamPropInfos_.front()->sampleRate_);
        audioModuleInfo.format = AudioDefinitionPolicyUtils::enumToFormatStr[
            pipeInfo->streamPropInfos_.front()->format_];
        audioModuleInfo.channels = std::to_string(pipeInfo->streamPropInfos_.front()->channels_);
        audioModuleInfo.bufferSize = std::to_string(pipeInfo->streamPropInfos_.front()->bufferSize_);
    }

    for (auto &attributeInfo : pipeInfo->attributeInfos_) {
        if (attributeInfo->name_ == "filePath") {
            audioModuleInfo.fileName = attributeInfo->value_;
        }
    }

    audioModuleInfo.fixedLatency = pipeInfo->paProp_.fixedLatency_;
    audioModuleInfo.renderInIdleState = pipeInfo->paProp_.renderInIdleState_;
}

std::string AudioPolicyConfigParser::GetAudioModuleInfoName(std::string &pipeInfoName,
    std::list<std::shared_ptr<AdapterDeviceInfo>> &deviceInfos)
{
    for (auto &deviceInfo : deviceInfos) {
        if (std::find(deviceInfo->supportPipes_.begin(), deviceInfo->supportPipes_.end(), pipeInfoName) !=
            deviceInfo->supportPipes_.end()) {
            return deviceInfo->name_;
        }
    }
    return "";
}

ClassType AudioPolicyConfigParser::GetClassTypeByAdapterType(AudioAdapterType adapterType)
{
    if (adapterType == AudioAdapterType::TYPE_PRIMARY) {
        return ClassType::TYPE_PRIMARY;
    } else if (adapterType == AudioAdapterType::TYPE_A2DP) {
        return ClassType::TYPE_A2DP;
    } else if (adapterType == AudioAdapterType::TYPE_HEARING_AID) {
        return ClassType::TYPE_HEARING_AID;
    } else if (adapterType == AudioAdapterType::TYPE_REMOTE_AUDIO) {
        return ClassType::TYPE_REMOTE_AUDIO;
    } else if (adapterType == AudioAdapterType::TYPE_FILE_IO) {
        return ClassType::TYPE_FILE_IO;
    } else if (adapterType == AudioAdapterType::TYPE_USB) {
        return ClassType::TYPE_USB;
    }  else if (adapterType == AudioAdapterType::TYPE_DP) {
        return ClassType::TYPE_DP;
    } else if (adapterType == AudioAdapterType::TYPE_ACCESSORY) {
        return ClassType::TYPE_ACCESSORY;
    } else {
        return ClassType::TYPE_INVALID;
    }
}

}
}
