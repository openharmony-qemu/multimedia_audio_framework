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

#include "audio_sasdk_unit_test.h"

#include "audio_errors.h"
#include "audio_info.h"

#include <chrono>
#include <thread>
#include <fstream>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

using namespace std;
using namespace testing::ext;
using namespace testing;

namespace OHOS {
namespace AudioStandard {

void AudioSasdkUnitTest::SetUpTestCase(void) {}
void AudioSasdkUnitTest::TearDownTestCase(void) {}
void AudioSasdkUnitTest::SetUp(void) {}
void AudioSasdkUnitTest::TearDown(void) {}

/**
* @tc.name   : Test AudioSasdk API
* @tc.number : IsStreamActive_001
* @tc.desc   : Test IsStreamActive interface - in no call scene.
*/
HWTEST(AudioSasdkUnitTest, IsStreamActive_001, TestSize.Level1)
{
    AudioSaSdk *instance = AudioSaSdk::GetInstance();
    SaSdkAudioVolumeType callType = SASDK_STREAM_VOICE_CALL;
    bool isCallActivate = instance->IsStreamActive(callType);
    EXPECT_EQ(false, isCallActivate);
}
} // namespace AudioStandard
} // namespace OHOS
