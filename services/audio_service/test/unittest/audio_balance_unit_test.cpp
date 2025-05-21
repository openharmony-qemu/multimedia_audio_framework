/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

#include <gtest/gtest.h>
#include "audio_system_manager.h"

using namespace testing::ext;
namespace OHOS {
namespace AudioStandard {
class AudioBalanceUnitTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void AudioBalanceUnitTest::SetUpTestCase(void)
{
    // input testsuit setup step，setup invoked before all testcases
}

void AudioBalanceUnitTest::TearDownTestCase(void)
{
    // input testsuit teardown step，teardown invoked after all testcases
}

void AudioBalanceUnitTest::SetUp(void)
{
    // input testcase setup step，setup invoked before each testcases
}

void AudioBalanceUnitTest::TearDown(void)
{
    // input testcase teardown step，teardown invoked after each testcases
}

/**
* @tc.name  : Test SetAudioMonoState API
* @tc.type  : FUNC
* @tc.number: SetAudioMonoState_001
* @tc.desc  : Test SetAudioMonoState interface. Set audio mono state to true
*/
HWTEST(AudioBalanceUnitTest, SetAudioMonoState_001, TestSize.Level1)
{
    bool audioMonoState = true;
    ASSERT_TRUE(AudioSystemManager::GetInstance() != nullptr);
    AudioSystemManager::GetInstance()->SetAudioMonoState(audioMonoState);
}

/**
* @tc.name  : Test SetAudioMonoState API
* @tc.type  : FUNC
* @tc.number: SetAudioMonoState_002
* @tc.desc  : Test SetAudioMonoState interface. Set audio mono state to false
*/
HWTEST(AudioBalanceUnitTest, SetAudioMonoState_002, TestSize.Level1)
{
    bool audioMonoState = false;
    ASSERT_TRUE(AudioSystemManager::GetInstance() != nullptr);
    AudioSystemManager::GetInstance()->SetAudioMonoState(audioMonoState);
}

/**
* @tc.name  : Test SetAudioBalanceValue API
* @tc.type  : FUNC
* @tc.number: SetAudioBalanceValue_001
* @tc.desc  : Test SetAudioBalanceValue interface. Set audio balance value to -1.0f
*/
HWTEST(AudioBalanceUnitTest, SetAudioBalanceValue_001, TestSize.Level1)
{
    float audioBalanceValue = -1.0f;
    ASSERT_TRUE(AudioSystemManager::GetInstance() != nullptr);
    AudioSystemManager::GetInstance()->SetAudioBalanceValue(audioBalanceValue);
}

/**
* @tc.name  : Test SetAudioBalanceValue API
* @tc.type  : FUNC
* @tc.number: SetAudioBalanceValue_002
* @tc.desc  : Test SetAudioBalanceValue interface. Set audio balance value to -0.5f
*/
HWTEST(AudioBalanceUnitTest, SetAudioBalanceValue_002, TestSize.Level1)
{
    float audioBalanceValue = -0.5f;
    ASSERT_TRUE(AudioSystemManager::GetInstance() != nullptr);
    AudioSystemManager::GetInstance()->SetAudioBalanceValue(audioBalanceValue);
}

/**
* @tc.name  : Test SetAudioBalanceValue API
* @tc.type  : FUNC
* @tc.number: SetAudioBalanceValue_003
* @tc.desc  : Test SetAudioBalanceValue interface. Set audio balance value to 0.5f
*/
HWTEST(AudioBalanceUnitTest, SetAudioBalanceValue_003, TestSize.Level1)
{
    float audioBalanceValue = 0.5f;
    ASSERT_TRUE(AudioSystemManager::GetInstance() != nullptr);
    AudioSystemManager::GetInstance()->SetAudioBalanceValue(audioBalanceValue);
}

/**
* @tc.name  : Test SetAudioBalanceValue API
* @tc.type  : FUNC
* @tc.number: SetAudioBalanceValue_004
* @tc.desc  : Test SetAudioBalanceValue interface. Set audio balance value to 1.0f
*/
HWTEST(AudioBalanceUnitTest, SetAudioBalanceValue_004, TestSize.Level1)
{
    float audioBalanceValue = 1.0f;
    ASSERT_TRUE(AudioSystemManager::GetInstance() != nullptr);
    AudioSystemManager::GetInstance()->SetAudioBalanceValue(audioBalanceValue);
}

/**
* @tc.name  : Test SetAudioBalanceValue API
* @tc.type  : FUNC
* @tc.number: SetAudioBalanceValue_005
* @tc.desc  : Test SetAudioBalanceValue interface. Set audio balance value to 0.0f
*/
HWTEST(AudioBalanceUnitTest, SetAudioBalanceValue_005, TestSize.Level1)
{
    float audioBalanceValue = 0.0f;
    ASSERT_TRUE(AudioSystemManager::GetInstance() != nullptr);
    AudioSystemManager::GetInstance()->SetAudioBalanceValue(audioBalanceValue);
}
} // namespace AudioStandard
} // namespace OHOS