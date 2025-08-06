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

#include "audio_concurrency_parser_unit_test.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

void AudioConcurrencyParserUnitTest::SetUpTestCase(void) {}
void AudioConcurrencyParserUnitTest::TearDownTestCase(void) {}
void AudioConcurrencyParserUnitTest::SetUp(void)
{
    audioConcurrencyParser_ = std::make_shared<AudioConcurrencyParser>();
    ASSERT_TRUE(audioConcurrencyParser_ != nullptr);

    mockAudioXmlNode_ = std::make_shared<MockAudioXmlNode>();
    ASSERT_TRUE(mockAudioXmlNode_ != nullptr);
}
void AudioConcurrencyParserUnitTest::TearDown(void)
{
    audioConcurrencyParser_ = nullptr;
    mockAudioXmlNode_ = nullptr;
}

/**
 * @tc.name  : Test ParseToneInfo.
 * @tc.number: AudioConcurrencyParserUnitTest_001
 * @tc.desc  : Test ParseToneInfo interface. !curNode->CompareName("ToneInfo")
 */
HWTEST_F(AudioConcurrencyParserUnitTest, AudioConcurrencyParserUnitTest_001, TestSize.Level4)
{
    audioConcurrencyParser_->curNode_ = mockAudioXmlNode_;
    EXPECT_CALL(*(mockAudioXmlNode_), Config(testing::_, testing::_, testing::_)).WillOnce(Return(SUCCESS));

    EXPECT_CALL(*(mockAudioXmlNode_), CompareName(testing::_)).WillOnce(Return(false));
    EXPECT_CALL(*(mockAudioXmlNode_), FreeDoc()).Times(1);

    std::map<std::pair<AudioPipeType, AudioPipeType>, ConcurrencyAction> map;
    int32_t ret = audioConcurrencyParser_->LoadConfig(map);
    EXPECT_EQ(ret, ERR_OPERATION_FAILED);
}

} // namespace AudioStandard
} // namespace OHOSu