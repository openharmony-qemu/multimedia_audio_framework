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

#ifndef AUDIO_SUITE_CAPABILITIES_PARSER_H
#define AUDIO_SUITE_CAPABILITIES_PARSER_H

#include <unordered_map>
#include <string>
#include "audio_errors.h"
#include "audio_xml_parser.h"
#include "audio_suite_base.h"

namespace OHOS {
namespace AudioStandard {
namespace AudioSuite {
static constexpr char AUDIO_SUITE_CAPABILITIES_CONFIG_FILE[] = "/system/etc/audio/audio_suite_capabilities.xml";

struct NodeParameter {
    std::string soName;
    std::string soPath;
    std::string general;
    bool isLoaded = false;
    bool supportedOnThisDevice = false;
    uint32_t frameLen;
    uint32_t inSampleRate;
    uint32_t inChannels;
    uint32_t inFormat;
    uint32_t outSampleRate;
    uint32_t outChannels;
    uint32_t outFormat;
    float realtimeFactor = 1.0f;
};

class AudioSuiteCapabilitiesParser {
public:
    AudioSuiteCapabilitiesParser();
    ~AudioSuiteCapabilitiesParser();

    bool LoadConfiguration(
        std::unordered_map<AudioNodeType, NodeParameter> &audioSuiteCapabilities);

private:
    bool ParseInternal(std::shared_ptr<AudioXmlNode> audioSuiteCapabilitiesXmlNode,
        std::unordered_map<AudioNodeType, NodeParameter> &audioSuiteCapabilities);
    void ParserNodeType(std::shared_ptr<AudioXmlNode> curNode,
        std::unordered_map<AudioNodeType, NodeParameter> &audioSuiteCapabilities);
    float GetRealtimeFactor(std::string valueStr);
};
}  // namespace AudioSuite
}  // namespace AudioStandard
}  // namespace OHOS

#endif // AUDIO_SUITE_CAPABILITIES_PARSER_H
