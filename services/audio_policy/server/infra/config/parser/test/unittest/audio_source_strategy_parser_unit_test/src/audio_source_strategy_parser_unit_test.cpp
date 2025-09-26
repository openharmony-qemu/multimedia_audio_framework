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
#define GET_ONE_CFG_FILE GetOneCfgFileMock
char *GetOneCfgFileMock(const char *pathSuffix, char *buf, unsigned int bufLength);
#include "audio_source_strategy_parser_unit_test.h"
#undef GET_ONE_CFG_FILE
 
char *GetOneCfgFileMock(const char *pathSuffix, char *buf, unsigned int bufLength)
{
    const char *mockPath = "path";
    strncpy_s(buf, bufLength, mockPath, bufLength);
    buf[bufLength - 1] = '\0';
    return buf;
}
using namespace testing;
using namespace testing::ext;
 
namespace OHOS {
namespace AudioStandard {
void AudioSourceStrategyParserUnitTest::SetUpTestCase(void) {}
void AudioSourceStrategyParserUnitTest::TearDownTestCase(void) {}
void AudioSourceStrategyParserUnitTest::SetUp(void)
{
    audioSourceStrategyParser_ = std::make_shared<AudioSourceStrategyParser>();
    ASSERT_TRUE(audioSourceStrategyParser_ != nullptr);
 
    mockaudioSourceStrategyParser_ = std::make_shared<MockAudioSourceStrategyParser>();
    ASSERT_TRUE(mockaudioSourceStrategyParser_ != nullptr);
 
    mockAudioXmlNode_ = std::make_shared<MockAudioXmlNode>();
    ASSERT_TRUE(mockAudioXmlNode_ != nullptr);

    mockChildNode_ = std::make_shared<MockAudioXmlNode>();
    ASSERT_TRUE(mockChildNode_ != nullptr);

    mockItemNode_ = std::make_shared<MockAudioXmlNode>();
    ASSERT_TRUE(mockItemNode_ != nullptr);
 
    sourceStrategyMap_ = std::make_shared<std::map<SourceType, AudioSourceStrategyType>>();
}
void AudioSourceStrategyParserUnitTest::TearDown(void)
{
    audioSourceStrategyParser_ = nullptr;
    mockAudioXmlNode_ = nullptr;
    sourceStrategyMap_ = nullptr;
    mockaudioSourceStrategyParser_ = nullptr;
    mockChildNode_ = nullptr;
    mockItemNode_ = nullptr;
}
 
/**
 * @tc.name  : Test ParseConfig.
 * @tc.number: AudioSourceStrategyParserUnitTest_001
 * @tc.desc  : Test ParseConfig interface.
 */
HWTEST_F(AudioSourceStrategyParserUnitTest, AudioSourceStrategyParserUnitTest_001, TestSize.Level1)
{
    std::shared_ptr<std::map<SourceType, AudioSourceStrategyType>> sourceStrategyMaptemp =
        std::make_shared<std::map<SourceType, AudioSourceStrategyType>>();
    sourceStrategyMaptemp = nullptr;
    audioSourceStrategyParser_->ParseConfig(mockAudioXmlNode_, sourceStrategyMaptemp);
    EXPECT_EQ(sourceStrategyMap_->size(), 0);
}
 
/**
 * @tc.name  : Test ParseConfig.
 * @tc.number: AudioSourceStrategyParserUnitTest_002
 * @tc.desc  : Test ParseConfig interface.
 */
HWTEST_F(AudioSourceStrategyParserUnitTest, AudioSourceStrategyParserUnitTest_002, TestSize.Level4)
{
    int32_t ret = audioSourceStrategyParser_->LoadConfig();
    EXPECT_EQ(ret, true);
 
    std::string typeStr_source = "validSource";
    std::string typeStr_hdiSource = "validHdiSource";
    std::shared_ptr<AudioXmlNode> mockChildNode = std::make_shared<MockAudioXmlNode>();
    EXPECT_CALL(*mockAudioXmlNode_, IsNodeValid())
        .WillOnce(Return(true))      // 第一次调用返回 true
        .WillRepeatedly(Return(false)); // 第二次及以后所有调用返回 false
    EXPECT_CALL(*mockAudioXmlNode_, IsElementNode())
        .WillOnce(Return(true));
    EXPECT_CALL(*mockAudioXmlNode_, GetProp(StrEq("source"), _))
        .WillOnce(DoAll(SetArgReferee<1>(typeStr_source), Return(0)));
    EXPECT_CALL(*mockAudioXmlNode_, GetProp(StrEq("hdiSource"), _))
        .WillOnce(DoAll(SetArgReferee<1>(typeStr_hdiSource), Return(0)));
    EXPECT_CALL(*mockAudioXmlNode_, GetChildrenNode())
        .WillOnce(Return(mockChildNode));
    EXPECT_CALL(*mockAudioXmlNode_, MoveToNext())
        .Times(1);
 
    audioSourceStrategyParser_->ParseConfig(mockAudioXmlNode_, sourceStrategyMap_);
    EXPECT_EQ(sourceStrategyMap_->size(), 0);
}
 
/**
 * @tc.name  : Test ParseConfig.
 * @tc.number: AudioSourceStrategyParserUnitTest_003
 * @tc.desc  : Test ParseConfig interface.
 */
HWTEST_F(AudioSourceStrategyParserUnitTest, AudioSourceStrategyParserUnitTest_003, TestSize.Level4)
{
    std::string typeStr_source = "SOURCE_TYPE_VOICE_TRANSCRIPTION";
    std::string typeStr_hdiSource = "primary";
    
    // 创建另一个mock节点来表示GetChildrenNode()返回的节点
    auto mockItemNode = std::make_shared<MockAudioXmlNode>();
    
    // 设置 AddSourceStrategyMap 中需要的属性值
    std::string adapterStr = "default_adapter";
    std::string pipeStr = "default_pipe";
    std::string audioFlagStr = "AUDIO_INPUT_FLAG_AI";
    std::string priorityStr = "2";

    // 设置主要的属性期望（在ParseConfig中调用）
    EXPECT_CALL(*mockAudioXmlNode_, GetProp(StrEq("source"), _))
        .WillOnce(DoAll(SetArgReferee<1>(typeStr_source), Return(0)));
    EXPECT_CALL(*mockAudioXmlNode_, GetProp(StrEq("hdiSource"), _))
        .WillOnce(DoAll(SetArgReferee<1>(typeStr_hdiSource), Return(0)));

    // 设置节点验证期望
    EXPECT_CALL(*mockAudioXmlNode_, IsNodeValid())
        .WillOnce(Return(true))
        .WillRepeatedly(Return(false));

    EXPECT_CALL(*mockAudioXmlNode_, IsElementNode())
        .WillOnce(Return(true));

    EXPECT_CALL(*mockAudioXmlNode_, GetChildrenNode())
        .WillOnce(Return(mockItemNode));

    EXPECT_CALL(*mockAudioXmlNode_, MoveToNext())
        .Times(1);

    // 设置 mockItemNode 的期望（在ParseSourceStrategyMap中调用）
    EXPECT_CALL(*mockItemNode, IsNodeValid())
        .WillOnce(Return(true))
        .WillRepeatedly(Return(false));

    EXPECT_CALL(*mockItemNode, CompareName(StrEq("item")))
        .WillOnce(Return(true));

    EXPECT_CALL(*mockItemNode, GetName())
        .WillOnce(Return("item"));

    EXPECT_CALL(*mockItemNode, GetProp(StrEq("adapter"), _))
        .WillOnce(DoAll(SetArgReferee<1>(adapterStr), Return(0)));
    EXPECT_CALL(*mockItemNode, GetProp(StrEq("pipe"), _))
        .WillOnce(DoAll(SetArgReferee<1>(pipeStr), Return(0)));
    EXPECT_CALL(*mockItemNode, GetProp(StrEq("audioFlag"), _))
        .WillOnce(DoAll(SetArgReferee<1>(audioFlagStr), Return(0)));
    EXPECT_CALL(*mockItemNode, GetProp(StrEq("priority"), _))
        .WillOnce(DoAll(SetArgReferee<1>(priorityStr), Return(0)));

    EXPECT_CALL(*mockItemNode, MoveToNext())
        .Times(1);

    // 执行测试
    audioSourceStrategyParser_->ParseConfig(mockAudioXmlNode_, sourceStrategyMap_);
    
    // 验证结果,添加成功
    EXPECT_EQ(sourceStrategyMap_->size(), 1);
}
 
/**
 * @tc.name  : Test ParseConfig.
 * @tc.number: AudioSourceStrategyParserUnitTest_004
 * @tc.desc  : Test ParseConfig interface.
 */
HWTEST_F(AudioSourceStrategyParserUnitTest, AudioSourceStrategyParserUnitTest_004, TestSize.Level4)
{
    std::shared_ptr<MockAudioXmlNode> mockNode = std::make_shared<MockAudioXmlNode>();
    auto sourceStrategyMap = std::make_shared<std::map<SourceType, AudioSourceStrategyType>>();
    
    // 节点直接无效
    EXPECT_CALL(*mockNode, IsNodeValid())
        .WillOnce(Return(false));
        
    // 不应调用其他方法
    EXPECT_CALL(*mockNode, CompareName(_)).Times(0);
    EXPECT_CALL(*mockNode, GetName()).Times(0);
    EXPECT_CALL(*mockNode, MoveToNext()).Times(0);
    EXPECT_CALL(*mockaudioSourceStrategyParser_, AddSourceStrategyMap(_, _, _, _)).Times(0);
        
    // 执行测试
    audioSourceStrategyParser_->ParseSourceStrategyMap(mockNode, "MIC", "AUDIO_HDF_MIC", sourceStrategyMap);
    
    // 验证结果map应为空
    EXPECT_TRUE(sourceStrategyMap->empty());
}
 
/**
 * @tc.name  : Test ParseConfig.
 * @tc.number: AudioSourceStrategyParserUnitTest_005
 * @tc.desc  : Test ParseConfig interface.
 */
HWTEST_F(AudioSourceStrategyParserUnitTest, AudioSourceStrategyParserUnitTest_005, TestSize.Level4)
{
    // 测试非item节点被跳过
    std::shared_ptr<MockAudioXmlNode> mockNode = std::make_shared<MockAudioXmlNode>();
    auto strategyMap = std::make_shared<std::map<SourceType, AudioSourceStrategyType>>();
    
    // 设置一个有效节点，但不是item类型
    EXPECT_CALL(*mockNode, IsNodeValid())
        .WillOnce(Return(true))   // 第一次：进入循环
        .WillRepeatedly(Return(false)); // 后续：退出循环
        
    EXPECT_CALL(*mockNode, CompareName("item"))
        .WillOnce(Return(false));  // 不是item类型
        
    EXPECT_CALL(*mockNode, MoveToNext())
        .Times(1);  // 应该移动一次

    EXPECT_CALL(*mockaudioSourceStrategyParser_, AddSourceStrategyMap(_, _, _, _))
        .Times(0);
    
    // 执行测试
    audioSourceStrategyParser_->ParseSourceStrategyMap(mockNode, "MIC", "AUDIO_HDF_MIC", strategyMap);
    EXPECT_TRUE(strategyMap->empty());
}
 
/**
 * @tc.name  : Test AddSourceStrategyMap.
 * @tc.number: AudioSourceStrategyParserUnitTest_006
 * @tc.desc  : Test AddSourceStrategyMap interface.
 */
HWTEST_F(AudioSourceStrategyParserUnitTest, AudioSourceStrategyParserUnitTest_006, TestSize.Level4)
{
    std::shared_ptr<std::map<SourceType, AudioSourceStrategyType>> sourceStrategyMaptemp =
        std::make_shared<std::map<SourceType, AudioSourceStrategyType>>();
    audioSourceStrategyParser_->curNode_ = mockAudioXmlNode_;
    std::string source = "SOURCE_TYPE_MIC";
    std::string hdiSource = "validHdiSource";
    std::string adapterStr = "adapter";
    std::string pipeStr = "pipe";
    std::string priorityStr = "10";
    std::string audioFlagStr = "AUDIO_INPUT_FLAG_AI";
 
    EXPECT_CALL(*mockAudioXmlNode_, GetProp(StrEq("adapter"), _))
        .WillOnce(DoAll(SetArgReferee<1>(adapterStr), Return(0)));
    EXPECT_CALL(*mockAudioXmlNode_, GetProp(StrEq("pipe"), _))
        .WillOnce(DoAll(SetArgReferee<1>(pipeStr), Return(0)));
    EXPECT_CALL(*mockAudioXmlNode_, GetProp(StrEq("audioFlag"), _))
        .WillOnce(DoAll(SetArgReferee<1>(audioFlagStr), Return(0)));
    EXPECT_CALL(*mockAudioXmlNode_, GetProp(StrEq("priority"), _))
        .WillOnce(DoAll(SetArgReferee<1>(priorityStr), Return(0)));
 
    // 设置 AudioDefinitionPolicyUtils::flagStrToEnum 包含 audioFlagStr
    AudioDefinitionPolicyUtils::flagStrToEnum[audioFlagStr] = AudioFlag::AUDIO_INPUT_FLAG_AI;
 
    audioSourceStrategyParser_->AddSourceStrategyMap(mockAudioXmlNode_, source, hdiSource, sourceStrategyMaptemp);
    EXPECT_EQ(sourceStrategyMaptemp->size(), 1);
    EXPECT_NE(sourceStrategyMaptemp->find(SOURCE_TYPE_MIC), sourceStrategyMaptemp->end());
}
 
/**
 * @tc.name  : Test AddSourceStrategyMap.
 * @tc.number: AudioSourceStrategyParserUnitTest_007
 * @tc.desc  : Test AddSourceStrategyMap interface.
 */
HWTEST_F(AudioSourceStrategyParserUnitTest, AudioSourceStrategyParserUnitTest_007, TestSize.Level4)
{
    std::shared_ptr<std::map<SourceType, AudioSourceStrategyType>> sourceStrategyMaptemp =
        std::make_shared<std::map<SourceType, AudioSourceStrategyType>>();
    std::string source = "";
    std::string hdiSource = "validHdiSource";
    audioSourceStrategyParser_->AddSourceStrategyMap(mockAudioXmlNode_, source, hdiSource, sourceStrategyMaptemp);
 
    EXPECT_TRUE(sourceStrategyMaptemp->empty());
}
 
/**
 * @tc.name  : Test AddSourceStrategyMap.
 * @tc.number: AudioSourceStrategyParserUnitTest_008
 * @tc.desc  : Test AddSourceStrategyMap interface.
 */
 HWTEST_F(AudioSourceStrategyParserUnitTest, AudioSourceStrategyParserUnitTest_008, TestSize.Level4)
 {
    std::shared_ptr<std::map<SourceType, AudioSourceStrategyType>> sourceStrategyMaptemp =
        std::make_shared<std::map<SourceType, AudioSourceStrategyType>>();
    std::string source = "invalidSource";
    std::string hdiSource = "validHdiSource";
    audioSourceStrategyParser_->AddSourceStrategyMap(mockAudioXmlNode_, source, hdiSource, sourceStrategyMaptemp);
 
    EXPECT_TRUE(sourceStrategyMaptemp->empty());
}

/**
 * @tc.name  : Test AddSourceStrategyMap.
 * @tc.number: AudioSourceStrategyParserUnitTest_009
 * @tc.desc  : Test AddSourceStrategyMap interface invalid audioflag.
 */
HWTEST_F(AudioSourceStrategyParserUnitTest, AudioSourceStrategyParserUnitTest_009, TestSize.Level4)
{
    std::shared_ptr<std::map<SourceType, AudioSourceStrategyType>> sourceStrategyMaptemp =
        std::make_shared<std::map<SourceType, AudioSourceStrategyType>>();
    audioSourceStrategyParser_->curNode_ = mockAudioXmlNode_;
    std::string source = "SOURCE_TYPE_MIC";
    std::string hdiSource = "validHdiSource";
    std::string adapterStr = "adapter";
    std::string pipeStr = "pipe";
    std::string priorityStr = "10";
    std::string audioFlagStr = "AUDIO_INPUT_FLAG_AI_INVALID";
 
    EXPECT_CALL(*mockAudioXmlNode_, GetProp(StrEq("adapter"), _))
        .WillOnce(DoAll(SetArgReferee<1>(adapterStr), Return(0)));
    EXPECT_CALL(*mockAudioXmlNode_, GetProp(StrEq("pipe"), _))
        .WillOnce(DoAll(SetArgReferee<1>(pipeStr), Return(0)));
    EXPECT_CALL(*mockAudioXmlNode_, GetProp(StrEq("audioFlag"), _))
        .WillOnce(DoAll(SetArgReferee<1>(audioFlagStr), Return(0)));
    EXPECT_CALL(*mockAudioXmlNode_, GetProp(StrEq("priority"), _))
        .WillOnce(DoAll(SetArgReferee<1>(priorityStr), Return(0)));
 
    audioSourceStrategyParser_->AddSourceStrategyMap(mockAudioXmlNode_, source, hdiSource, sourceStrategyMaptemp);
    EXPECT_EQ(sourceStrategyMaptemp->size(), 0);
    EXPECT_EQ(sourceStrategyMaptemp->find(SOURCE_TYPE_MIC), sourceStrategyMaptemp->end());
}
} // namespace AudioStandard
} // namespace OHOS