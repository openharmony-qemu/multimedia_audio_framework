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
#include "audio_suite_log.h"
#include "audio_xml_parser.h"
#include "audio_suite_base.h"

namespace OHOS {
namespace AudioStandard {
namespace AudioSuite {
static constexpr char AUDIO_SUITE_CAPABILITIES_CONFIG_FILE[] = "/system/etc/audio/audio_suite_capabilities.xml";

struct NodeCapability {
    std::string soName;
    std::string soPath;
    std::string general;
    bool isLoaded = false;
    bool supportedOnThisDevice = false;
    bool isSupportRealtime = false;
};

static const std::map<std::string, AudioNodeType>
    NODE_TYPE_MAP = {{"EQUALIZER", NODE_TYPE_EQUALIZER},
        {"NOISE_REDUCTION", NODE_TYPE_NOISE_REDUCTION},
        {"SOUND_FIELD", NODE_TYPE_SOUND_FIELD},
        {"AUDIO_SEPARATION", NODE_TYPE_AUDIO_SEPARATION},
        {"VOICE_BEAUTIFIER", NODE_TYPE_VOICE_BEAUTIFIER},
        {"ENVIRONMENT_EFFECT", NODE_TYPE_ENVIRONMENT_EFFECT}};

class AudioSuiteCapabilitiesParser {
public:
    AudioSuiteCapabilitiesParser()
    {
        AUDIO_DEBUG_LOG("AudioSuiteCapabilitiesParser ctor");
    }

    ~AudioSuiteCapabilitiesParser()
    {
        AUDIO_DEBUG_LOG("AudioSuiteCapabilitiesParser dtor");
    }

    bool LoadConfiguration(
        std::unordered_map<AudioNodeType, NodeCapability> &audioSuiteCapabilities);

private:
    bool ParseInternal(std::shared_ptr<AudioXmlNode> audioSuiteCapabilitiesXmlNode,
        std::unordered_map<AudioNodeType, NodeCapability> &audioSuiteCapabilities);
    void ParserNodeType(std::shared_ptr<AudioXmlNode> curNode,
        std::unordered_map<AudioNodeType, NodeCapability> &audioSuiteCapabilities);
};
}  // namespace AudioSuite
}  // namespace AudioStandard
}  // namespace OHOS

#endif // AUDIO_SUITE_CAPABILITIES_PARSER_H
