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

#include <iostream>
#include "audio_global_config_manager_test.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {
static AudioGlobalConfigManager audioGlobalConfigManager_ = AudioGlobalConfigManager::GetAudioGlobalConfigManager();

void AudioGlobalConfigManagerTest::SetUpTestCase(void) {}
void AudioGlobalConfigManagerTest::TearDownTestCase(void) {}
void AudioGlobalConfigManagerTest::SetUp(void) {}
void AudioGlobalConfigManagerTest::TearDown(void) {}

/**
* @tc.name  : Test ParseGlobalConfigXml.
* @tc.number: ParseGlobalConfigXml_001
* @tc.desc  : Test ParseGlobalConfigXml.
*/
HWTEST(AudioGlobalConfigManagerTest, ParseGlobalConfigXml_001, TestSize.Level1)
{
    audioGlobalConfigManager_.ParseGlobalConfigXml();
    EXPECT_NE(&audioGlobalConfigManager_, nullptr);
}

} // namespace AudioStandard
} // namespace OHOS