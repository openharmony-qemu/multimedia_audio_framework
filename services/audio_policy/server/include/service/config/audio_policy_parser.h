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
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <unordered_map>
#include <string>
#include <regex>

#include "audio_adapter_info.h"
#include "audio_device_info.h"
#include "audio_stream_info.h"
#include "iport_observer.h"
#include "parser.h"

namespace OHOS {
namespace AudioStandard {
class AudioPolicyParser : public Parser {
public:
    static constexpr char CHIP_PROD_CONFIG_FILE[] = "/chip_prod/etc/audio/audio_policy_config.xml";
    static constexpr char CONFIG_FILE[] = "vendor/etc/audio/audio_policy_config.xml";

    bool LoadConfiguration() final;
    bool Parse() final;
    void Destroy() final;

    explicit AudioPolicyParser(IPortObserver &observer)
        : portObserver_(observer),
          doc_(nullptr)
    {
    }

    virtual ~AudioPolicyParser()
    {
        Destroy();
    }

private:
    AdapterType GetAdapterTypeAsInt(xmlNode &node);
    PipeType GetPipeInfoTypeAsInt(xmlNode &node);
    GlobalConfigType GetGlobalConfigTypeAsInt(xmlNode &node);
    XmlNodeType GetXmlNodeTypeAsInt(xmlNode &node);
    DefaultMaxInstanceType GetDefaultMaxInstanceTypeAsInt(xmlNode &node);
    StreamType GetStreamTypeAsInt(xmlNode &node);

    bool ParseInternal(xmlNode &node);
    void ParseAdapters(xmlNode &node);
    void ParseAdapter(xmlNode &node);
    void ParsePipes(xmlNode &node, AudioAdapterInfo &adapterInfo);
    void ParsePipeInfos(xmlNode &node, PipeInfo &pipeInfo);
    void ParseStreamProps(xmlNode &node, PipeInfo &pipeInfo);
    void ParseConfigs(xmlNode &node, PipeInfo &pipeInfo);
    void HandleConfigFlagAndUsage(ConfigInfo &configInfo, PipeInfo &pipeInfo);
    void ParseDevices(xmlNode &node, AudioAdapterInfo &adapterInfo);
    void ParseGroups(xmlNode& node, XmlNodeType type);
    void ParseGroup(xmlNode& node, XmlNodeType type);
    void ParseGroupSink(xmlNode &node, XmlNodeType type, std::string &groupName);
    void ParseGlobalConfigs(xmlNode& node);
    void ParsePAConfigs(xmlNode& node);
    void ParseDefaultMaxInstances(xmlNode& node);
    void ParseOutputMaxInstances(xmlNode& node);
    void ParseInputMaxInstances(xmlNode& node);
    void ParseCommonConfigs(xmlNode& node);

    void HandleUpdateRouteSupportParsed(std::string &value);
    PAConfigType GetPaConfigType(std::string &name);

    void SplitStringToList(std::string &str, std::list<std::string> &result);
    void SplitChannelStringToSet(std::string &str, std::set<uint32_t> &result);

    std::string ExtractPropertyValue(const std::string &propName, xmlNode &node);
    AdaptersType GetAdaptersType(const std::string &adapterClass);

    std::string GetAudioModuleInfoName(std::string &pipeInfoName, std::list<AudioPipeDeviceInfo> &deviceInfos);
    void ConvertAdapterInfoToAudioModuleInfo();
    void ConvertAdapterInfoToGroupInfo(std::unordered_map<std::string, std::string> &volumeGroupMap,
        std::unordered_map<std::string, std::string> &interruptGroupMap);
    void GetCommontAudioModuleInfo(PipeInfo &pipeInfo, AudioModuleInfo &audioModuleInfo);
    ClassType GetClassTypeByAdapterType(AdaptersType adapterType);
    void GetOffloadAndOpenMicState(AudioAdapterInfo &adapterInfo, bool &shouldEnableOffload);

    IPortObserver &portObserver_;
    xmlDoc *doc_;
    std::unordered_map<AdaptersType, AudioAdapterInfo> adapterInfoMap_ {};
    std::unordered_map<ClassType, std::list<AudioModuleInfo>> xmlParsedDataMap_ {};
    std::unordered_map<std::string, std::string> volumeGroupMap_;
    std::unordered_map<std::string, std::string> interruptGroupMap_;
    GlobalConfigs globalConfigs_;
    bool shouldOpenMicSpeaker_ = false;
};
} // namespace AudioStandard
} // namespace OHOS

#endif // AUDIO_POLICY_PARSER_H
