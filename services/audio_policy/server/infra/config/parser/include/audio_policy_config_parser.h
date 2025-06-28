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
#ifndef AUDIO_POLICY_CONFIG_PARSER_H
#define AUDIO_POLICY_CONFIG_PARSER_H

#include <list>
#include <unordered_map>
#include <string>
#include <regex>

#include "audio_device_info.h"
#include "audio_stream_info.h"
#include "parser.h"
#include "audio_xml_parser.h"
#include "audio_policy_config_manager.h"

namespace OHOS {
namespace AudioStandard {
class AudioPolicyConfigParser : public Parser {
public:
    static constexpr char CHIP_PROD_CONFIG_FILE[] = "/chip_prod/etc/audio/audio_policy_config.xml";
    static constexpr char CONFIG_FILE[] = "/vendor/etc/audio/audio_policy_config.xml";

    bool LoadConfiguration() final;
    void Destroy() final;

    explicit AudioPolicyConfigParser(AudioPolicyConfigManager *manager): configManager_(manager)
    {
        curNode_ = AudioXmlNode::Create();
    }

    virtual ~AudioPolicyConfigParser()
    {
        Destroy();
        curNode_ = nullptr;
    }

private:
    PolicyXmlNodeType GetXmlNodeTypeAsInt(std::shared_ptr<AudioXmlNode> curNode);
    AdapterInfoType GetAdapterInfoTypeAsInt(std::shared_ptr<AudioXmlNode> curNode);
    PipeInfoType GetPipeInfoTypeAsInt(std::shared_ptr<AudioXmlNode> curNode);
    PolicyGlobalConfigType GetGlobalConfigTypeAsInt(std::shared_ptr<AudioXmlNode> curNode);
    PolicyPAConfigType GetPaConfigType(std::string &name);
    ClassType GetClassTypeByAdapterType(AudioAdapterType adapterType);

    bool ParseInternal(std::shared_ptr<AudioXmlNode> curNode);
    void ParseAdapters(std::shared_ptr<AudioXmlNode> curNode);
    void ParseAdapter(std::shared_ptr<AudioXmlNode> curNode, std::shared_ptr<PolicyAdapterInfo> &adapterInfo);
    void ParsePipes(std::shared_ptr<AudioXmlNode> curNode, std::shared_ptr<PolicyAdapterInfo> &adapterInfo);
    void ParsePipeInfos(std::shared_ptr<AudioXmlNode> curNode, std::shared_ptr<AdapterPipeInfo> &pipeInfo);
    void ParsePaProp(std::shared_ptr<AudioXmlNode> curNode, std::shared_ptr<AdapterPipeInfo> &pipeInfo);
    void ParseStreamProps(std::shared_ptr<AudioXmlNode> curNode, std::shared_ptr<AdapterPipeInfo> &pipeInfo);
    void ParseAttributes(std::shared_ptr<AudioXmlNode> curNode, std::shared_ptr<AdapterPipeInfo> &pipeInfo);
    void ParseAttributeByName(AttributeInfo &attributeInfo, std::shared_ptr<AdapterPipeInfo> &pipeInfo);
    void ParseDevices(std::shared_ptr<AudioXmlNode> curNode, std::shared_ptr<PolicyAdapterInfo> &adapterInfo);
    void SplitStringToList(std::string &str, std::list<std::string> &result, const char *delim);
    void ParseGroups(std::shared_ptr<AudioXmlNode> curNode, PolicyXmlNodeType type);
    void ParseGroup(std::shared_ptr<AudioXmlNode> curNode, PolicyXmlNodeType type);
    void ParseGroupSink(std::shared_ptr<AudioXmlNode> curNode, PolicyXmlNodeType type, std::string &groupName);
    void ParseGlobalConfigs(std::shared_ptr<AudioXmlNode> curNode);
    void ParsePAConfigs(std::shared_ptr<AudioXmlNode> curNode);
    void ParseCommonConfigs(std::shared_ptr<AudioXmlNode> curNode);
    AudioSampleFormat ConvertToFormat(std::string value);
    void HandleUpdateRouteSupportParsed(std::string &value);
    void HandleUpdateAnahsSupportParsed(std::string &value);
    void HandleDefaultAdapterSupportParsed(std::string &value);
    void HandleEncodingEac3SupportParsed(std::shared_ptr<AdapterPipeInfo> pipeInfo, const std::string &value);
    
    void ConvertAdapterInfoToGroupInfo(std::unordered_map<std::string, std::string> &volumeGroupMap,
        std::unordered_map<std::string, std::string> &interruptGroupMap);
    void ConvertAdapterInfoToAudioModuleInfo();
    void GetOffloadAndOpenMicState(std::shared_ptr<PolicyAdapterInfo> &adapterInfo, bool &shouldEnableOffload);
    void GetCommontAudioModuleInfo(std::shared_ptr<AdapterPipeInfo> &pipeInfo, AudioModuleInfo &audioModuleInfo);
    std::string GetAudioModuleInfoName(std::string &pipeInfoName,
            std::list<std::shared_ptr<AdapterDeviceInfo>> &deviceInfos);

    std::shared_ptr<AudioXmlNode> curNode_ = nullptr;
    AudioPolicyConfigManager *configManager_ = nullptr;
    
    std::unordered_map<ClassType, std::list<AudioModuleInfo>> xmlParsedDataMap_ {};
    std::unordered_map<std::string, std::string> volumeGroupMap_;
    std::unordered_map<std::string, std::string> interruptGroupMap_;
    PolicyGlobalConfigs globalConfigs_;
    bool shouldOpenMicSpeaker_ = false;
    bool shouldSetDefaultAdapter_ = false;
};
} // namespace AudioStandard
} // namespace OHOS

#endif // AUDIO_POLICY_CONFIG_PARSER_H
