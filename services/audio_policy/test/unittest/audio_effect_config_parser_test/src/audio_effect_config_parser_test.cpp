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

#include "audio_effect_config_parser_test.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

void AudioEffectConfigParserTest::SetUpTestCase(void) {}
void AudioEffectConfigParserTest::TearDownTestCase(void) {}
void AudioEffectConfigParserTest::SetUp(void) {}

void AudioEffectConfigParserTest::TearDown(void) {}

/**
* @tc.name  : Test AudioEffectConfigParser.
* @tc.number: AudioEffectConfigParser_001
* @tc.desc  : Test AudioEffectConfigParser interfaces.
*/
HWTEST(AudioEffectConfigParserTest, AudioEffectConfigParser_001, TestSize.Level1)
{
    OriginalEffectConfig result;
    AudioEffectConfigParser effectConfigParserTest;

    int32_t ret = effectConfigParserTest.LoadEffectConfig(result);
    EXPECT_NE(ret, 0);
}
} // namespace AudioStandard
} // namespace OHOS
