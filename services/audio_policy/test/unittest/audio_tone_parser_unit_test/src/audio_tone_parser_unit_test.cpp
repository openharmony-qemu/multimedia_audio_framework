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

#include "audio_tone_parser_unit_test.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

void AudioToneParserUnitTest::SetUpTestCase(void) {}
void AudioToneParserUnitTest::TearDownTestCase(void) {}
void AudioToneParserUnitTest::SetUp(void) {}
void AudioToneParserUnitTest::TearDown(void) {}

/**
 * @tc.name  : Test AudioVolumeManager.
 * @tc.number: AudioToneParserUnitTest_001
 * @tc.desc  : Test GetSharedVolume interface.
 */
HWTEST_F(AudioToneParserUnitTest, AudioToneParserUnitTest_001, TestSize.Level1)
{
    AudioToneParser audioToneParser;
    std::unordered_map<int32_t, std::shared_ptr<ToneInfo>> map;
    std::shared_ptr<ToneInfo> toneInfo = std::make_shared<ToneInfo>();
    map.insert(std::make_pair(1, toneInfo));
    int ret = audioToneParser.LoadConfig(map);
    EXPECT_EQ(ret, SUCCESS);
}
/**
 * @tc.name  : Test AudioVolumeManager.
 * @tc.number: AudioToneParserUnitTest_002
 * @tc.desc  : Test GetSharedVolume interface.
 */
HWTEST_F(AudioToneParserUnitTest, AudioToneParserUnitTest_002, TestSize.Level1)
{
    AudioToneParser audioToneParser;
    std::unordered_map<int32_t, std::shared_ptr<ToneInfo>> map;
    std::shared_ptr<ToneInfo> toneInfo = std::make_shared<ToneInfo>();
    map.insert(std::make_pair(1, toneInfo));
    std::string newConfigPath = AudioToneParser::AUDIO_TONE_CONFIG_FILE;
    std::string mapStr = "DTMF";
    std::unordered_map<std::string, ToneInfoMap> toneInfoMap;
    toneInfoMap.insert(std::make_pair(mapStr, map));
    int ret = audioToneParser.LoadNewConfig(newConfigPath, map, toneInfoMap);
    EXPECT_EQ(ret, SUCCESS);
}
/**
 * @tc.name  : Test AudioVolumeManager.
 * @tc.number: AudioToneParserUnitTest_003
 * @tc.desc  : Test GetSharedVolume interface.
 */
HWTEST_F(AudioToneParserUnitTest, AudioToneParserUnitTest_003, TestSize.Level1)
{
    AudioToneParser audioToneParser;
    std::shared_ptr<AudioXmlNode> curNode = AudioXmlNode::Create();
    std::unordered_map<int32_t, std::shared_ptr<ToneInfo>> map;
    std::shared_ptr<ToneInfo> toneInfo = std::make_shared<ToneInfo>();
    map.insert(std::make_pair(1, toneInfo));
    std::string mapStr = "DTMF";
    std::unordered_map<std::string, ToneInfoMap> toneInfoMap;
    toneInfoMap.insert(std::make_pair(mapStr, map));
    int ret = audioToneParser.LoadConfig(map);
    audioToneParser.ParseCustom(curNode, toneInfoMap);
    EXPECT_EQ(ret, SUCCESS);
}
/**
 * @tc.name  : Test AudioVolumeManager.
 * @tc.number: AudioToneParserUnitTest_004
 * @tc.desc  : Test GetSharedVolume interface.
 */
HWTEST_F(AudioToneParserUnitTest, AudioToneParserUnitTest_004, TestSize.Level1)
{
    AudioToneParser audioToneParser;
    std::shared_ptr<AudioXmlNode> curNode = AudioXmlNode::Create();
    std::unordered_map<int32_t, std::shared_ptr<ToneInfo>> map;
    std::shared_ptr<ToneInfo> toneInfo = std::make_shared<ToneInfo>();
    map.insert(std::make_pair(1, toneInfo));
    int ret = audioToneParser.LoadConfig(map);
    audioToneParser.ParseToneInfoAttribute(curNode, toneInfo);
    EXPECT_EQ(ret, SUCCESS);
}
/**
 * @tc.name  : Test AudioVolumeManager.
 * @tc.number: AudioToneParserUnitTest_005
 * @tc.desc  : Test GetSharedVolume interface.
 */
HWTEST_F(AudioToneParserUnitTest, AudioToneParserUnitTest_005, TestSize.Level1)
{
    AudioToneParser audioToneParser;
    std::shared_ptr<AudioXmlNode> curNode = AudioXmlNode::Create();
    std::unordered_map<int32_t, std::shared_ptr<ToneInfo>> map;
    std::shared_ptr<ToneInfo> toneInfo = std::make_shared<ToneInfo>();
    map.insert(std::make_pair(1, toneInfo));
    std::vector<ToneInfoMap*> toneInfoMapArr;
    ToneInfoMap *toneInfoMapPtr = &map;
    toneInfoMapArr.push_back(toneInfoMapPtr);
    int ret = audioToneParser.LoadConfig(map);
    audioToneParser.ParseToneInfo(curNode, toneInfoMapArr);
    EXPECT_EQ(ret, SUCCESS);
}

HWTEST_F(AudioToneParserUnitTest, AudioToneParserUnitTest_006, TestSize.Level1)
{
    AudioToneParser audioToneParser;
    std::shared_ptr<AudioXmlNode> curNode = AudioXmlNode::Create();
    std::unordered_map<int32_t, std::shared_ptr<ToneInfo>> map;
    std::shared_ptr<ToneInfo> toneInfo = std::make_shared<ToneInfo>();
    map.insert(std::make_pair(1, toneInfo));
    int ret = audioToneParser.LoadConfig(map);
    audioToneParser.ParseSegment(curNode, 0, toneInfo);
    EXPECT_EQ(ret, SUCCESS);
}
} // namespace AudioStandard
} // namespace OHOS