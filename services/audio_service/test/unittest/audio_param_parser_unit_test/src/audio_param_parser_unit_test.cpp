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

#include "audio_param_parser_unit_test.h"
#include "gtest/gtest.h"

using namespace testing;
namespace OHOS {
namespace AudioStandard {

void AudioParamParserTest::SetUpTestCase(void)
{}

void AudioParamParserTest::TearDownTestCase(void)
{}

void AudioParamParserTest::SetUp(void)
{
    audioParamParser_ = std::make_shared<AudioParamParser>();
}

void AudioParamParserTest::TearDown(void)
{
    audioParamParser_ = nullptr;
}

/**
 * @tc.name  : LoadConfiguration_ShouldReturnFalse_WhenConfigFileNotFound
 * @tc.number: AudioParamParserTest_001
 * @tc.desc  : Test LoadConfiguration method when config file is not found.
 */
HWTEST_F(
    AudioParamParserTest, LoadConfiguration_ShouldReturnFalse_WhenConfigFileNotFound, testing::ext::TestSize.Level0)
{
    std::unordered_map<std::string, std::unordered_map<std::string, std::set<std::string>>> audioParameterKeys;
    EXPECT_FALSE(audioParamParser_->LoadConfiguration(audioParameterKeys));
}

/**
 * @tc.name  : ParseInternal_ShouldReturnFalse_WhenNodeIsNull
 * @tc.number: AudioParamParserTest_005
 * @tc.desc  : Test ParseInternal method when node is null.
 */
HWTEST_F(AudioParamParserTest, ParseInternal_ShouldReturnFalse_WhenNodeIsNull, testing::ext::TestSize.Level0)
{
    std::shared_ptr<AudioXmlNode> node = AudioXmlNode::Create();
    std::unordered_map<std::string, std::unordered_map<std::string, std::set<std::string>>> audioParameterKeys;
    EXPECT_FALSE(audioParamParser_->ParseInternal(node, audioParameterKeys));
}

/**
 * @tc.name  : ExtractPropertyValue_ShouldReturnEmptyString_WhenPropNameNotFound
 * @tc.number: AudioParamParserTest_013
 * @tc.desc  : Test ExtractPropertyValue method when prop name is not found.
 */
HWTEST_F(AudioParamParserTest, ExtractPropertyValue_ShouldReturnEmptyString_WhenPropNameNotFound,
    testing::ext::TestSize.Level0)
{
    std::shared_ptr<AudioXmlNode> node = AudioXmlNode::Create();
    std::string result;
    node->GetProp("name", result);
    EXPECT_EQ("", result);
}
}  // namespace AudioStandard
}  // namespace OHOS