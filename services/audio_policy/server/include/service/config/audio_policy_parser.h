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

#ifndef AUDIO_POLICY_PARSER_H
#define AUDIO_POLICY_PARSER_H

#include <list>
#include <unordered_map>
#include <string>
#include <regex>

#include "audio_adapter_info.h"
#include "audio_device_info.h"
#include "audio_stream_info.h"
#include "iport_observer.h"
#include "parser.h"
#include "audio_xml_parser.h"

namespace OHOS {
namespace AudioStandard {
class AudioPolicyParser : public Parser {
public:
    static constexpr char CHIP_PROD_CONFIG_FILE[] = "/chip_prod/etc/audio/audio_policy_config.xml";
    static constexpr char CONFIG_FILE[] = "/vendor/etc/audio/audio_policy_config.xml";

    bool LoadConfiguration() final;
    void Destroy() final;

    explicit AudioPolicyParser(IPortObserver &observer): portObserver_(observer)
    {
        curNode_ = AudioXmlNode::Create();
    }

    virtual ~AudioPolicyParser()
    {
        Destroy();
        curNode_ = nullptr;
    }

private:
    AdapterType GetAdapterTypeAsInt(std::shared_ptr<AudioXmlNode> curNode);
    PipeType GetPipeInfoTypeAsInt(std::shared_ptr<AudioXmlNode> curNode);
    GlobalConfigType GetGlobalConfigTypeAsInt(std::shared_ptr<AudioXmlNode> curNode);
    XmlNodeType GetXmlNodeTypeAsInt(std::shared_ptr<AudioXmlNode> curNode);
    DefaultMaxInstanceType GetDefaultMaxInstanceTypeAsInt(std::shared_ptr<AudioXmlNode> curNode);

    bool ParseInternal(std::shared_ptr<AudioXmlNode> curNode);
    void ParseAdapters(std::shared_ptr<AudioXmlNode> curNode);
    void ParseAdapter(std::shared_ptr<AudioXmlNode> curNode);
    void ParsePipes(std::shared_ptr<AudioXmlNode> curNode, AudioAdapterInfo &adapterInfo);
    void ParsePipeInfos(std::shared_ptr<AudioXmlNode> curNode, PipeInfo &pipeInfo);
    void ParseStreamProps(std::shared_ptr<AudioXmlNode> curNode, PipeInfo &pipeInfo);
    void ParseConfigs(std::shared_ptr<AudioXmlNode> curNode, PipeInfo &pipeInfo);
    void HandleConfigFlagAndUsage(ConfigInfo &configInfo, PipeInfo &pipeInfo);
    void ParseDevices(std::shared_ptr<AudioXmlNode> curNode, AudioAdapterInfo &adapterInfo);
    void ParseGroups(std::shared_ptr<AudioXmlNode> curNode, XmlNodeType type);
    void ParseGroup(std::shared_ptr<AudioXmlNode> curNode, XmlNodeType type);
    void ParseGroupSink(std::shared_ptr<AudioXmlNode> curNode, XmlNodeType type, std::string &groupName);
    void ParseGlobalConfigs(std::shared_ptr<AudioXmlNode> curNode);
    void ParsePAConfigs(std::shared_ptr<AudioXmlNode> curNode);
    void ParseDefaultMaxInstances(std::shared_ptr<AudioXmlNode> curNode);
    void ParseOutputMaxInstances(std::shared_ptr<AudioXmlNode> curNode);
    void ParseInputMaxInstances(std::shared_ptr<AudioXmlNode> curNode);
    void ParseCommonConfigs(std::shared_ptr<AudioXmlNode> curNode);

    void HandleUpdateRouteSupportParsed(std::string &value);
    void HandleUpdateAnahsSupportParsed(std::string &value);
    void HandleDefaultAdapterSupportParsed(std::string &value);
    PAConfigType GetPaConfigType(std::string &name);

    void SplitStringToList(std::string &str, std::list<std::string> &result);
    void SplitChannelStringToSet(std::string &str, std::set<uint32_t> &result);

    AdaptersType GetAdaptersType(const std::string &adapterClass);

    std::string GetAudioModuleInfoName(std::string &pipeInfoName, std::list<AudioPipeDeviceInfo> &deviceInfos);
    void ConvertAdapterInfoToAudioModuleInfo();
    void ConvertAdapterInfoToGroupInfo(std::unordered_map<std::string, std::string> &volumeGroupMap,
        std::unordered_map<std::string, std::string> &interruptGroupMap);
    void GetCommontAudioModuleInfo(PipeInfo &pipeInfo, AudioModuleInfo &audioModuleInfo);
    ClassType GetClassTypeByAdapterType(AdaptersType adapterType);
    void GetOffloadAndOpenMicState(AudioAdapterInfo &adapterInfo, bool &shouldEnableOffload);

    IPortObserver &portObserver_;
    std::shared_ptr<AudioXmlNode> curNode_ = nullptr;
    std::unordered_map<AdaptersType, AudioAdapterInfo> adapterInfoMap_ {};
    std::unordered_map<ClassType, std::list<AudioModuleInfo>> xmlParsedDataMap_ {};
    std::unordered_map<std::string, std::string> volumeGroupMap_;
    std::unordered_map<std::string, std::string> interruptGroupMap_;
    GlobalConfigs globalConfigs_;
    bool shouldOpenMicSpeaker_ = false;
    bool shouldSetDefaultAdapter_ = false;
};
} // namespace AudioStandard
} // namespace OHOS

#endif // AUDIO_POLICY_PARSER_H
