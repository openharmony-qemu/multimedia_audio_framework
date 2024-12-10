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

#ifndef AUDIO_PARAM_PARSER_H
#define AUDIO_PARAM_PARSER_H

#include <libxml/parser.h>
#include <libxml/tree.h>
#include <list>
#include <unordered_map>
#include <string>
#include <regex>
#include <set>

namespace OHOS {
namespace AudioStandard {
#ifdef USE_CONFIG_POLICY
static constexpr char CONFIG_FILE[] = "etc/audio/audio_param_config.xml";
#endif

class AudioParamParser {
public:
    AudioParamParser();
    ~AudioParamParser();

    bool LoadConfiguration(
        std::unordered_map<std::string, std::unordered_map<std::string, std::set<std::string>>> &audioParameterKeys);

private:
    bool ParseInternal(xmlNode *node,
        std::unordered_map<std::string, std::unordered_map<std::string, std::set<std::string>>> &audioParameterKeys);
    void ParseMainKeys(xmlNode *node,
        std::unordered_map<std::string, std::unordered_map<std::string, std::set<std::string>>> &audioParameterKeys);
    void ParseMainKey(xmlNode *node,
        std::unordered_map<std::string, std::unordered_map<std::string, std::set<std::string>>> &audioParameterKeys);
    void ParseSubKeys(xmlNode *node, std::string &className,
        std::unordered_map<std::string, std::unordered_map<std::string, std::set<std::string>>> &audioParameterKeys);

    std::string ExtractPropertyValue(const std::string &propName, xmlNode &node);
};
}  // namespace AudioStandard
}  // namespace OHOS

#endif // AUDIO_PARAM_PARSER_H
