/*
* Copyright (c) 2025 Huawei Device Co., Ltd.
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
 
#ifndef AUDIO_SOURCE_STRATEGY_PARSER_UNIT_TEST_H
#define AUDIO_SOURCE_STRATEGY_PARSER_UNIT_TEST_H
#include "../test/unittest/audio_source_strategy_parser_unit_test/include/audio_source_strategy_parser_mock.h"
#include "audio_xml_node_mock.h"
#include "gtest/gtest.h"
#include "audio_source_strategy_parser.h"
 
namespace OHOS {
namespace AudioStandard {
 
class AudioSourceStrategyParserUnitTest : public testing::Test {
public:
    // SetUpTestCase: Called before all test cases
    static void SetUpTestCase(void);
    // TearDownTestCase: Called after all test case
    static void TearDownTestCase(void);
    // SetUp: Called before each test cases
    void SetUp(void);
    // TearDown: Called after each test cases
    void TearDown(void);
public:
    std::shared_ptr<AudioSourceStrategyParser> audioSourceStrategyParser_{nullptr};
    std::shared_ptr<MockAudioSourceStrategyParser> mockaudioSourceStrategyParser_{nullptr};
    std::shared_ptr<MockAudioXmlNode> mockAudioXmlNode_{nullptr};
    std::shared_ptr<MockAudioXmlNode> mockChildNode_{nullptr};
    std::shared_ptr<MockAudioXmlNode> mockItemNode_{nullptr};
    std::shared_ptr<std::map<SourceType, AudioSourceStrategyType>> sourceStrategyMap_;
};
} // namespace AudioStandard
} // namespace OHOS
#endif //AUDIO_SOURCE_STRATEGY_PARSER_UNIT_TEST_H