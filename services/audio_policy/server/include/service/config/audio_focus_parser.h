/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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
#ifndef AUDIO_FOCUS_PARSER_H
#define AUDIO_FOCUS_PARSER_H

#include <map>
#include <string>
#include "audio_errors.h"
#include "audio_info.h"
#include "audio_policy_log.h"
#include "audio_xml_parser.h"

namespace OHOS {
namespace AudioStandard {

class AudioFocusParser {
public:
    AudioFocusParser();
    virtual ~AudioFocusParser();
    int32_t LoadConfig(std::map<std::pair<AudioFocusType, AudioFocusType>, AudioFocusEntry> &focusMap);

private:
    #ifdef USE_CONFIG_POLICY
    static constexpr char AUDIO_FOCUS_CONFIG_FILE[] = "etc/audio/audio_interrupt_policy_config.xml";
    #else
    static constexpr char AUDIO_FOCUS_CONFIG_FILE[] = "/system/etc/audio/audio_interrupt_policy_config.xml";
    #endif
    static std::map<std::string, AudioFocusType> audioFocusMap;
    static std::map<std::string, InterruptHint> actionMap;
    static std::map<std::string, ActionTarget> targetMap;
    static std::map<std::string, InterruptForceType> forceMap;
    std::shared_ptr<AudioXmlNode> curNode_ = nullptr;

    void LoadDefaultConfig(std::map<std::pair<AudioFocusType, AudioFocusType>, AudioFocusEntry> &focusMap);
    void ParseFocusChildrenMap(std::shared_ptr<AudioXmlNode> curNode, const std::string &curStream,
        std::map<std::pair<AudioFocusType, AudioFocusType>, AudioFocusEntry> &focusMap);
    void ParseFocusMap(std::shared_ptr<AudioXmlNode> curNode, const std::string &curStream,
        std::map<std::pair<AudioFocusType, AudioFocusType>, AudioFocusEntry> &focusMap);
    void ParseStreams(std::shared_ptr<AudioXmlNode> curNode, std::map<std::pair<AudioFocusType, AudioFocusType>,
        AudioFocusEntry> &focusMap);
    void AddAllowedFocusEntry(std::shared_ptr<AudioXmlNode> curNode, const std::string &curStream,
        std::map<std::pair<AudioFocusType, AudioFocusType>, AudioFocusEntry> &focusMap);
    void ParseAllowedStreams(std::shared_ptr<AudioXmlNode> curNode, const std::string &curStream,
        std::map<std::pair<AudioFocusType, AudioFocusType>, AudioFocusEntry> &focusMap);
    void AddRejectedFocusEntry(std::shared_ptr<AudioXmlNode> curNode, const std::string &curStream,
        std::map<std::pair<AudioFocusType, AudioFocusType>, AudioFocusEntry> &focusMap);
    void ParseRejectedStreams(std::shared_ptr<AudioXmlNode> curNode, const std::string &curStream,
        std::map<std::pair<AudioFocusType, AudioFocusType>, AudioFocusEntry> &focusMap);
    void WriteConfigErrorEvent();
};
} // namespace AudioStandard
} // namespace OHOS
#endif // AUDIO_FOCUS_PARSER_H
