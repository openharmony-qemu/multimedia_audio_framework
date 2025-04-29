/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "audio_sasdk.h"
#include <gtest/gtest.h>

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

class AudioSaSdkUnitTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void AudioSaSdkUnitTest::SetUpTestCase(void)
{
    // input testsuit setup step，setup invoked before all testcases
}

void AudioSaSdkUnitTest::TearDownTestCase(void)
{
    // input testsuit teardown step，teardown invoked after all testcases
}

void AudioSaSdkUnitTest::SetUp(void)
{
    // input testcase setup step，setup invoked before each testcases
}

void AudioSaSdkUnitTest::TearDown(void)
{
    // input testcase teardown step，teardown invoked after each testcases
}

/**
 * @tc.name  : Test AudioSaSdk API
 * @tc.type  : FUNC
 * @tc.number: AudioSaSdk_001
 * @tc.desc  : Test IsStreamActive interface.
 */
HWTEST(AudioSaSdkUnitTest, AudioSaSdk_001, TestSize.Level1)
{
    SaSdkAudioVolumeType streamType = SASDK_STREAM_MUSIC;
    auto ret = AudioSaSdk::GetInstance()->IsStreamActive(streamType);
    EXPECT_FALSE(ret);
}
} // namespace AudioStandard
} // namespace OHOS