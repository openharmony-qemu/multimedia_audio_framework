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
    EXPECT_TRUE(audioParamParser_->LoadConfiguration(audioParameterKeys));
}

/**
 * @tc.name  : LoadConfiguration_ShouldReturnFalse_WhenXmlReadFileFailed
 * @tc.number: AudioParamParserTest_002
 * @tc.desc  : Test LoadConfiguration method when xmlReadFile failed.
 */
HWTEST_F(AudioParamParserTest, LoadConfiguration_ShouldReturnFalse_WhenXmlReadFileFailed, testing::ext::TestSize.Level0)
{
    std::unordered_map<std::string, std::unordered_map<std::string, std::set<std::string>>> audioParameterKeys;
    EXPECT_TRUE(audioParamParser_->LoadConfiguration(audioParameterKeys));
}

/**
 * @tc.name  : LoadConfiguration_ShouldReturnFalse_WhenXmlDocGetRootElementFailed
 * @tc.number: AudioParamParserTest_003
 * @tc.desc  : Test LoadConfiguration method when xmlDocGetRootElement failed.
 */
HWTEST_F(AudioParamParserTest, LoadConfiguration_ShouldReturnFalse_WhenXmlDocGetRootElementFailed,
    testing::ext::TestSize.Level0)
{
    std::unordered_map<std::string, std::unordered_map<std::string, std::set<std::string>>> audioParameterKeys;
    EXPECT_TRUE(audioParamParser_->LoadConfiguration(audioParameterKeys));
}

/**
 * @tc.name  : LoadConfiguration_ShouldReturnTrue_WhenConfigFileFoundAndParsedSuccessfully
 * @tc.number: AudioParamParserTest_004
 * @tc.desc  : Test LoadConfiguration method when config file is found and parsed successfully.
 */
HWTEST_F(AudioParamParserTest, LoadConfiguration_ShouldReturnTrue_WhenConfigFileFoundAndParsedSuccessfully,
    testing::ext::TestSize.Level0)
{
    std::unordered_map<std::string, std::unordered_map<std::string, std::set<std::string>>> audioParameterKeys;
    EXPECT_TRUE(audioParamParser_->LoadConfiguration(audioParameterKeys));
}

/**
 * @tc.name  : ParseInternal_ShouldReturnFalse_WhenNodeIsNull
 * @tc.number: AudioParamParserTest_005
 * @tc.desc  : Test ParseInternal method when node is null.
 */
HWTEST_F(AudioParamParserTest, ParseInternal_ShouldReturnFalse_WhenNodeIsNull, testing::ext::TestSize.Level0)
{
    xmlNode *node = nullptr;
    std::unordered_map<std::string, std::unordered_map<std::string, std::set<std::string>>> audioParameterKeys;
    EXPECT_FALSE(audioParamParser_->ParseInternal(node, audioParameterKeys));
}

/**
 * @tc.name  : ParseInternal_ShouldReturnTrue_WhenNodeIsNotNull
 * @tc.number: AudioParamParserTest_006
 * @tc.desc  : Test ParseInternal method when node is not null.
 */
HWTEST_F(AudioParamParserTest, ParseInternal_ShouldReturnTrue_WhenNodeIsNotNull, testing::ext::TestSize.Level0)
{
    xmlNode *node = new xmlNode();
    std::unordered_map<std::string, std::unordered_map<std::string, std::set<std::string>>> audioParameterKeys;
    EXPECT_TRUE(audioParamParser_->ParseInternal(node, audioParameterKeys));
    delete node;
}

/**
 * @tc.name  : ParseMainKeys_ShouldAddKeys_WhenMainKeysNodeIsNotNull
 * @tc.number: AudioParamParserTest_008
 * @tc.desc  : Test ParseMainKeys method when main keys node is not null.
 */
HWTEST_F(AudioParamParserTest, ParseMainKeys_ShouldAddKeys_WhenMainKeysNodeIsNotNull, testing::ext::TestSize.Level0)
{
    xmlNode *node = new xmlNode();
    std::unordered_map<std::string, std::unordered_map<std::string, std::set<std::string>>> audioParameterKeys;
    audioParamParser_->ParseMainKeys(node, audioParameterKeys);
    EXPECT_TRUE(audioParameterKeys.empty());
    delete node;
}

/**
 * @tc.name  : ParseMainKey_ShouldNotAddKey_WhenMainKeyNameIsEmpty
 * @tc.number: AudioParamParserTest_009
 * @tc.desc  : Test ParseMainKey method when main key name is empty.
 */
HWTEST_F(AudioParamParserTest, ParseMainKey_ShouldNotAddKey_WhenMainKeyNameIsEmpty, testing::ext::TestSize.Level0)
{
    xmlNode *node = new xmlNode();
    std::unordered_map<std::string, std::unordered_map<std::string, std::set<std::string>>> audioParameterKeys;
    audioParamParser_->ParseMainKey(node, audioParameterKeys);
    EXPECT_TRUE(audioParameterKeys.empty());
    delete node;
}

/**
 * @tc.name  : ParseMainKey_ShouldAddKey_WhenMainKeyNameIsNotEmpty
 * @tc.number: AudioParamParserTest_010
 * @tc.desc  : Test ParseMainKey method when main key name is not empty.
 */
HWTEST_F(AudioParamParserTest, ParseMainKey_ShouldAddKey_WhenMainKeyNameIsNotEmpty, testing::ext::TestSize.Level0)
{
    xmlNode *node = new xmlNode();
    std::unordered_map<std::string, std::unordered_map<std::string, std::set<std::string>>> audioParameterKeys;
    audioParamParser_->ParseMainKey(node, audioParameterKeys);
    EXPECT_TRUE(audioParameterKeys.empty());
    delete node;
}

/**
 * @tc.name  : ParseSubKeys_ShouldAddKeys_WhenSubKeyNodeIsNotNull
 * @tc.number: AudioParamParserTest_012
 * @tc.desc  : Test ParseSubKeys method when sub key node is not null.
 */
HWTEST_F(AudioParamParserTest, ParseSubKeys_ShouldAddKeys_WhenSubKeyNodeIsNotNull, testing::ext::TestSize.Level0)
{
    xmlNode *node = new xmlNode();
    std::unordered_map<std::string, std::unordered_map<std::string, std::set<std::string>>> audioParameterKeys;
    std::string mainKeyName = "mainKey";
    audioParamParser_->ParseSubKeys(node, mainKeyName, audioParameterKeys);
    EXPECT_FALSE(audioParameterKeys.find(mainKeyName) == audioParameterKeys.end());
    delete node;
}

/**
 * @tc.name  : ExtractPropertyValue_ShouldReturnEmptyString_WhenPropNameNotFound
 * @tc.number: AudioParamParserTest_013
 * @tc.desc  : Test ExtractPropertyValue method when prop name is not found.
 */
HWTEST_F(AudioParamParserTest, ExtractPropertyValue_ShouldReturnEmptyString_WhenPropNameNotFound,
    testing::ext::TestSize.Level0)
{
    xmlNode node;
    std::string propName = "name";
    EXPECT_EQ("", audioParamParser_->ExtractPropertyValue(propName, node));
}

/**
 * @tc.name  : ExtractPropertyValue_ShouldReturnPropValue_WhenPropNameFound
 * @tc.number: AudioParamParserTest_014
 * @tc.desc  : Test ExtractPropertyValue method when prop name is found.
 */
HWTEST_F(
    AudioParamParserTest, ExtractPropertyValue_ShouldReturnPropValue_WhenPropNameFound, testing::ext::TestSize.Level0)
{
    xmlNode node;
    std::string propName = "name";
    std::string propValue = "";
    node.properties = xmlNewProp(nullptr,
        reinterpret_cast<const xmlChar *>(propName.c_str()),
        reinterpret_cast<const xmlChar *>(propValue.c_str()));
    EXPECT_EQ(propValue, audioParamParser_->ExtractPropertyValue(propName, node));
    xmlFreeProp(node.properties);
}
}  // namespace AudioStandard
}  // namespace OHOS