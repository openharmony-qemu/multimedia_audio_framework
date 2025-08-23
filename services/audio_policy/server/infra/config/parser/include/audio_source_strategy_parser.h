/*
 * Copyright (c) 2025-2025 Huawei Device Co., Ltd.
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
#ifndef AUDIO_SOURCE_STRATEGY_PARSER_H
#define AUDIO_SOURCE_STRATEGY_PARSER_H

#include <map>
#include <string>
#include "audio_errors.h"
#include "audio_info.h"
#include "audio_policy_log.h"
#include "audio_xml_parser.h"
#include "audio_definition_adapter_info.h"

namespace OHOS {
namespace AudioStandard {

class AudioSourceStrategyParser {
public:
    AudioSourceStrategyParser();
    virtual ~AudioSourceStrategyParser();
    bool LoadConfig();

private:
#ifdef USE_CONFIG_POLICY
    static constexpr char AUDIO_SOURCE_STRATEGY_CONFIG_FILE[] = "etc/audio/audio_source_strategy.xml";
#else
    static constexpr char AUDIO_SOURCE_STRATEGY_CONFIG_FILE[] = "/system/etc/audio/audio_source_strategy.xml";
#endif
    std::shared_ptr<AudioXmlNode> curNode_ = nullptr;
    void ParseSourceStrategyMap(std::shared_ptr<AudioXmlNode> curNode, const std::string &source,
        const std::string &hdiSource, std::shared_ptr<std::map<SourceType, AudioSourceStrategyType>>
        &sourceStrategyMap);
    void ParseConfig(std::shared_ptr<AudioXmlNode> curNode,
        std::shared_ptr<std::map<SourceType, AudioSourceStrategyType>> &sourceStrategyMap);
    void AddSourceStrategyMap(std::shared_ptr<AudioXmlNode> curNode, const std::string &source,
        const std::string &hdiSource, std::shared_ptr<std::map<SourceType, AudioSourceStrategyType>>
        &sourceStrategyMap);
    static const std::unordered_map<std::string, SourceType> sourceTypeMap;
};
} // namespace AudioStandard
} // namespace OHOS
#endif // AUDIO_SOURCE_STRATEGY_PARSER_H

