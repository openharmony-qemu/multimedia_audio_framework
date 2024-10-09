/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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
#include <libxml/parser.h>
#include <libxml/tree.h>
#include "audio_errors.h"
#include "audio_info.h"
#include "audio_policy_log.h"

namespace OHOS {
namespace AudioStandard {
#ifdef USE_CONFIG_POLICY
static constexpr char AUDIO_FOCUS_CONFIG_FILE[] = "etc/audio/audio_interrupt_policy_config.xml";
#else
static constexpr char AUDIO_FOCUS_CONFIG_FILE[] = "system/etc/audio/audio_interrupt_policy_config.xml";
#endif

class AudioFocusParser {
public:
    AudioFocusParser();
    virtual ~AudioFocusParser();
    int32_t LoadConfig(std::map<std::pair<AudioFocusType, AudioFocusType>, AudioFocusEntry> &focusMap);

private:
    static std::map<std::string, AudioFocusType> audioFocusMap;
    static std::map<std::string, InterruptHint> actionMap;
    static std::map<std::string, ActionTarget> targetMap;
    static std::map<std::string, InterruptForceType> forceMap;

    void LoadDefaultConfig(std::map<std::pair<AudioFocusType, AudioFocusType>, AudioFocusEntry> &focusMap);
    void ParseFocusChildrenMap(xmlNode *node, const std::string &curStream,
        std::map<std::pair<AudioFocusType, AudioFocusType>, AudioFocusEntry> &focusMap);
    void ParseFocusMap(xmlNode *node, const std::string &curStream, std::map<std::pair<AudioFocusType, AudioFocusType>,
        AudioFocusEntry> &focusMap);
    void ParseStreams(xmlNode *node, std::map<std::pair<AudioFocusType, AudioFocusType>,
        AudioFocusEntry> &focusMap);
    void AddAllowedFocusEntry(xmlNode *currNode, const std::string &curStream,
        std::map<std::pair<AudioFocusType, AudioFocusType>, AudioFocusEntry> &focusMap);
    void ParseAllowedStreams(xmlNode *node, const std::string &curStream,
        std::map<std::pair<AudioFocusType, AudioFocusType>, AudioFocusEntry> &focusMap);
    void AddRejectedFocusEntry(xmlNode *currNode, const std::string &curStream,
        std::map<std::pair<AudioFocusType, AudioFocusType>, AudioFocusEntry> &focusMap);
    void ParseRejectedStreams(xmlNode *node, const std::string &curStream,
        std::map<std::pair<AudioFocusType, AudioFocusType>, AudioFocusEntry> &focusMap);
    void WriteConfigErrorEvent();
};
} // namespace AudioStandard
} // namespace OHOS
#endif // AUDIO_FOCUS_PARSER_H
