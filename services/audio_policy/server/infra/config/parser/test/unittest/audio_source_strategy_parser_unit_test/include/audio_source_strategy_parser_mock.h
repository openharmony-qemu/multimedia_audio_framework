/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
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
 
#ifndef AUDIO_SOURCE_STRATEGY_PARSER_MOCK_H
#define AUDIO_SOURCE_STRATEGY_PARSER_MOCK_H
 
#include <gmock/gmock.h>
#include "audio_source_strategy_parser.h"
 
namespace OHOS {
namespace AudioStandard {
class MockAudioSourceStrategyParser : public AudioSourceStrategyParser {
public:
    MockAudioSourceStrategyParser() = default;
    ~MockAudioSourceStrategyParser() override = default;
    
    MOCK_METHOD(void, AddSourceStrategyMap,
        ((std::shared_ptr<AudioXmlNode>),
        (const std::string&),
        (const std::string&),
        (std::shared_ptr<std::map<SourceType, AudioSourceStrategyType>>&)));
};
}  // namespace AudioStandard
}  // namespace OHOS
 
#endif // AUDIO_SOURCE_STRATEGY_PARSER_MOCK_H