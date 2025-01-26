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

#include <iostream>
#include "audio_affinity_parser_test.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

void AudioAffinityParserTest::SetUpTestCase(void) {}
void AudioAffinityParserTest::TearDownTestCase(void) {}
void AudioAffinityParserTest::SetUp(void) {}
void AudioAffinityParserTest::TearDown(void) {}

/**
* @tc.name  : Test audioAffinityParser.
* @tc.number: audioAffinityParser_001
* @tc.desc  : Test LoadConfiguration.
*/
HWTEST(AudioAffinityParserTest, audioAffinityParser_001, TestSize.Level1)
{
    auto affinityManager = std::make_shared<AudioAffinityManager>();
    auto audioAffinity_ = std::make_shared<AudioAffinityParser>(affinityManager.get());
    audioAffinity_->LoadConfiguration();
    EXPECT_NE(audioAffinity_, nullptr);
}

/**
* @tc.name  : Test audioAffinityParser.
* @tc.number: audioAffinityParser_003
* @tc.desc  : Test Destroy.
*/
HWTEST(AudioAffinityParserTest, audioAffinityParser_003, TestSize.Level1)
{
    auto affinityManager = std::make_shared<AudioAffinityManager>();
    auto audioAffinity_ = std::make_shared<AudioAffinityParser>(affinityManager.get());
    audioAffinity_->Destroy();
    EXPECT_NE(audioAffinity_, nullptr);

    audioAffinity_->LoadConfiguration();
}

/**
* @tc.name  : Test audioAffinityParser.
* @tc.number: audioAffinityParser_004
* @tc.desc  : Test Destory.
*/
HWTEST(AudioAffinityParserTest, audioAffinityParser_004, TestSize.Level1)
{
    auto affinityManager = std::make_shared<AudioAffinityManager>();
    auto audioAffinity_ = std::make_shared<AudioAffinityParser>(affinityManager.get());
    std::shared_ptr<AudioXmlNode> node = AudioXmlNode::Create();
    audioAffinity_->ParseInternal(node);
    EXPECT_NE(audioAffinity_, nullptr);
}

} // namespace AudioStandard
} // namespace OHOS
