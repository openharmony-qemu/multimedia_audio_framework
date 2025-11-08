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

#include "audio_suite_capabilities_parser.h"

namespace OHOS {
namespace AudioStandard {
namespace AudioSuite {

bool AudioSuiteCapabilitiesParser::LoadConfiguration(
    std::unordered_map<AudioNodeType, NodeCapability> &audioSuiteCapabilities)
{
    std::shared_ptr<AudioXmlNode> curNode = AudioXmlNode::Create();
    CHECK_AND_RETURN_RET_LOG(curNode->Config(AUDIO_SUITE_CAPABILITIES_CONFIG_FILE, nullptr, 0) == SUCCESS,
        false, "audio_suite_capabilities.xml is not found!");
    bool result = ParseInternal(curNode->GetCopyNode(), audioSuiteCapabilities);
    CHECK_AND_RETURN_RET_LOG(result, false, "audio_suite_capabilities xml parse failed.");
    return true;
}

bool AudioSuiteCapabilitiesParser::ParseInternal(
    std::shared_ptr<AudioXmlNode> audioSuiteCapabilitiesXmlNode,
    std::unordered_map<AudioNodeType, NodeCapability> &audioSuiteCapabilities)
{
    std::shared_ptr<AudioXmlNode> nodeCapabilityXmlNode = audioSuiteCapabilitiesXmlNode->GetChildrenNode();
    nodeCapabilityXmlNode->MoveToNext();
    std::shared_ptr<AudioXmlNode> nodeTypeXmlNode = nodeCapabilityXmlNode->GetChildrenNode();
    for (; nodeTypeXmlNode->IsNodeValid(); nodeTypeXmlNode->MoveToNext()) {
        if (!nodeTypeXmlNode->IsElementNode()) {
            continue;
        }
        if (nodeTypeXmlNode->CompareName("nodeType")) {
            ParserNodeType(nodeTypeXmlNode->GetCopyNode(), audioSuiteCapabilities);
        }
    }
    return true;
}

void AudioSuiteCapabilitiesParser::ParserNodeType(
    std::shared_ptr<AudioXmlNode> curNode, std::unordered_map<AudioNodeType, NodeCapability> &audioSuiteCapabilities)
{
    std::string name;
    NodeCapability nodeCapability;

    curNode->GetProp("name", name);
    curNode->GetProp("soName", nodeCapability.soName);
    curNode->GetProp("soPath", nodeCapability.soPath);
    curNode->GetProp("general", nodeCapability.general);
    auto it = NODE_TYPE_MAP.find(name);
    CHECK_AND_RETURN_LOG(
        it != NODE_TYPE_MAP.end(), "parse node cabability error, unexpected type name: %{public}s.", name.c_str());
    audioSuiteCapabilities[it->second] = nodeCapability;
}

}  // namespace AudioSuite
}  // namespace AudioStandard
}  // namespace OHOS
