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
#define LOG_TAG "AudioSuiteCapabilitiesParser"
#endif

#include "audio_utils.h"
#include "audio_suite_capabilities_parser.h"
#include "audio_suite_log.h"

namespace OHOS {
namespace AudioStandard {
namespace AudioSuite {

AudioSuiteCapabilitiesParser::AudioSuiteCapabilitiesParser()
{
    AUDIO_DEBUG_LOG("AudioSuiteCapabilitiesParser ctor");
}

AudioSuiteCapabilitiesParser::~AudioSuiteCapabilitiesParser()
{
    AUDIO_DEBUG_LOG("AudioSuiteCapabilitiesParser dtor");
}

bool AudioSuiteCapabilitiesParser::LoadConfiguration(
    std::unordered_map<AudioNodeType, NodeParameter> &audioSuiteCapabilities)
{
    std::shared_ptr<AudioXmlNode> curNode = AudioXmlNode::Create();
    CHECK_AND_RETURN_RET_LOG(curNode->Config(AUDIO_SUITE_CAPABILITIES_CONFIG_FILE, nullptr, 0) == SUCCESS,
        false, "audio_suite_capabilities.xml is not found!");
    bool result = ParseInternal(curNode->GetCopyNode(), audioSuiteCapabilities);
    CHECK_AND_RETURN_RET_LOG(result, false, "audio_suite_capabilities xml parse failed.");
    return true;
}

bool AudioSuiteCapabilitiesParser::ParseInternal(
    std::shared_ptr<AudioXmlNode> curNode, std::unordered_map<AudioNodeType, NodeParameter> &audioSuiteCapabilities)
{
    for (; curNode && curNode->IsNodeValid(); curNode->MoveToNext()) {
        if (!curNode->IsElementNode()) {
            continue;
        }
        if (curNode->CompareName("nodeType")) {
            ParserNodeType(curNode->GetCopyNode(), audioSuiteCapabilities);
        } else {
            ParseInternal(curNode->GetChildrenNode(), audioSuiteCapabilities);
        }
    }
    return true;
}

void AudioSuiteCapabilitiesParser::ParserNodeType(
    std::shared_ptr<AudioXmlNode> curNode, std::unordered_map<AudioNodeType, NodeParameter> &audioSuiteCapabilities)
{
    std::string name;
    std::string realtimeFactorStr;
    std::string frameLenStr;
    NodeParameter nodeParameter;

    curNode->GetProp("name", name);
    curNode->GetProp("soName", nodeParameter.soName);
    curNode->GetProp("soPath", nodeParameter.soPath);
    curNode->GetProp("general", nodeParameter.general);
    curNode->GetProp("realtimeFactor", realtimeFactorStr);
    curNode->GetProp("frameLen", frameLenStr);

    // convert to float
    nodeParameter.realtimeFactor = GetRealtimeFactor(realtimeFactorStr);
    AUDIO_INFO_LOG(
        "Get node capability, name:%{public}s, realtimeFactor:%{public}f", name.c_str(), nodeParameter.realtimeFactor);

    // convert to uint32_t
    bool ret = StringConverter(frameLenStr, nodeParameter.frameLen);
    CHECK_AND_RETURN_LOG(ret, "convert string to uint32_t error, invalid frameLenStr =%{public}s", frameLenStr.c_str());
    auto it = NODE_TYPE_MAP.find(name);
    CHECK_AND_RETURN_LOG(
        it != NODE_TYPE_MAP.end(), "parse node cabability error, unexpected type name: %{public}s.", name.c_str());
    audioSuiteCapabilities[it->second] = nodeParameter;
}

float AudioSuiteCapabilitiesParser::GetRealtimeFactor(std::string valueStr)
{
    float defaultValue = 1.0f;  // default value when get config from XML failed.
    float value = 1.0f;
    CHECK_AND_RETURN_RET_LOG(StringConverterFloat(valueStr, value), defaultValue,
        "convert string to float value error, invalid valueStr =%{public}s", valueStr.c_str());
    return value;
}

}  // namespace AudioSuite
}  // namespace AudioStandard
}  // namespace OHOS