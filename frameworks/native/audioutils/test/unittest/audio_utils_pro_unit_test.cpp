/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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

#include <thread>
#include <gtest/gtest.h>
#include "audio_utils.h"
#include "parameter.h"
#include "audio_channel_blend.h"
#include "volume_ramp.h"
#include "audio_speed.h"
#include "audio_errors.h"

using namespace testing::ext;
using namespace std;
namespace OHOS {
namespace AudioStandard {

class AudioUtilsProUnitTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void AudioUtilsProUnitTest::SetUpTestCase(void)
{
    // input testsuit setup step，setup invoked before all testcases
}

void AudioUtilsProUnitTest::TearDownTestCase(void)
{
    // input testsuit teardown step，teardown invoked after all testcases
}

void AudioUtilsProUnitTest::SetUp(void)
{
    // input testcase setup step，setup invoked before each testcases
}

void AudioUtilsProUnitTest::TearDown(void)
{
    // input testcase teardown step，teardown invoked after each testcases
}

/**
* @tc.name  : Test SwitchStreamUtil API
* @tc.type  : FUNC
* @tc.number: SwitchStreamUtil_001
* @tc.desc  : Test SwitchStreamUtil::InsertSwitchStreamRecord().
*/
HWTEST(AudioUtilsProUnitTest, SwitchStreamUtil_001, TestSize.Level1)
{
    SwitchStreamInfo info = {0, 1, 0, 0, 0, CAPTURER_PREPARED};

    SwitchState targetState = SwitchState::SWITCH_STATE_WAITING;

    auto ret = SwitchStreamUtil::InsertSwitchStreamRecord(info, targetState);
    EXPECT_EQ(ret, true);

    ret = SwitchStreamUtil::RemoveAllRecordBySessionId(0);
    EXPECT_EQ(ret, true);
}

/**
* @tc.name  : Test SwitchStreamUtil API
* @tc.type  : FUNC
* @tc.number: SwitchStreamUtil_002
* @tc.desc  : Test SwitchStreamUtil::InsertSwitchStreamRecord().
*/
HWTEST(AudioUtilsProUnitTest, SwitchStreamUtil_002, TestSize.Level1)
{
    SwitchStreamInfo info = {0, 0, 0, 0, 0, CAPTURER_PREPARED};

    SwitchState targetState = SwitchState::SWITCH_STATE_WAITING;

    auto ret = SwitchStreamUtil::InsertSwitchStreamRecord(info, targetState);
    EXPECT_EQ(ret, true);
}

/**
* @tc.name  : Test SwitchStreamUtil API
* @tc.type  : FUNC
* @tc.number: SwitchStreamUtil_003
* @tc.desc  : Test SwitchStreamUtil::IsSwitchStreamSwitching().
*/
HWTEST(AudioUtilsProUnitTest, SwitchStreamUtil_003, TestSize.Level1)
{
    SwitchStreamInfo info = {0, 1, 0, 0, 0, CAPTURER_PREPARED};
    SwitchState targetState = SwitchState::SWITCH_STATE_WAITING;

    SwitchStreamUtil::InsertSwitchStreamRecord(info, targetState);

    SwitchStreamInfo info2 = {0, 1, 0, 0, 0, CAPTURER_PREPARED};
    SwitchState targetState2 = SwitchState::SWITCH_STATE_CREATED;

    auto ret = SwitchStreamUtil::IsSwitchStreamSwitching(info2, targetState2);
    EXPECT_EQ(ret, true);

    ret = SwitchStreamUtil::RemoveAllRecordBySessionId(0);
    EXPECT_EQ(ret, true);
}

/**
* @tc.name  : Test SwitchStreamUtil API
* @tc.type  : FUNC
* @tc.number: SwitchStreamUtil_004
* @tc.desc  : Test SwitchStreamUtil::IsSwitchStreamSwitching().
*/
HWTEST(AudioUtilsProUnitTest, SwitchStreamUtil_004, TestSize.Level1)
{
    SwitchStreamInfo info = {0, 1, 0, 0, 0, CAPTURER_RUNNING};
    SwitchState targetState = SwitchState::SWITCH_STATE_CREATED;

    SwitchStreamUtil::InsertSwitchStreamRecord(info, targetState);

    SwitchStreamInfo info2 = {0, 1, 0, 0, 0, CAPTURER_RUNNING};
    SwitchState targetState2 = SwitchState::SWITCH_STATE_STARTED;

    auto ret = SwitchStreamUtil::IsSwitchStreamSwitching(info2, targetState2);
    EXPECT_EQ(ret, true);

    ret = SwitchStreamUtil::RemoveAllRecordBySessionId(0);
    EXPECT_EQ(ret, true);
}

/**
* @tc.name  : Test SwitchStreamUtil API
* @tc.type  : FUNC
* @tc.number: SwitchStreamUtil_005
* @tc.desc  : Test SwitchStreamUtil::IsSwitchStreamSwitching().
*/
HWTEST(AudioUtilsProUnitTest, SwitchStreamUtil_005, TestSize.Level1)
{
    SwitchStreamInfo info = {0, 2, 0, 0, 0, CAPTURER_RUNNING};
    SwitchState targetState = SwitchState::SWITCH_STATE_CREATED;

    SwitchStreamUtil::InsertSwitchStreamRecord(info, targetState);

    SwitchStreamInfo info2 = {0, 1, 0, 0, 0, CAPTURER_RUNNING};
    SwitchState targetState2 = SwitchState::SWITCH_STATE_STARTED;

    auto ret = SwitchStreamUtil::IsSwitchStreamSwitching(info2, targetState2);
    EXPECT_EQ(ret, false);

    ret = SwitchStreamUtil::RemoveAllRecordBySessionId(0);
    EXPECT_EQ(ret, true);
}

/**
* @tc.name  : Test SwitchStreamUtil API
* @tc.type  : FUNC
* @tc.number: SwitchStreamUtil_006
* @tc.desc  : Test SwitchStreamUtil::IsSwitchStreamSwitching().
*/
HWTEST(AudioUtilsProUnitTest, SwitchStreamUtil_006, TestSize.Level1)
{
    SwitchStreamInfo info = {0, 1, 0, 0, 0, CAPTURER_RUNNING};
    SwitchState targetState = SwitchState::SWITCH_STATE_CREATED;

    SwitchStreamUtil::InsertSwitchStreamRecord(info, targetState);

    SwitchStreamInfo info2 = {0, 1, 0, 0, 0, CAPTURER_RUNNING};
    SwitchState targetState2 = SwitchState::SWITCH_STATE_CREATED;

    auto ret = SwitchStreamUtil::IsSwitchStreamSwitching(info2, targetState2);
    EXPECT_EQ(ret, false);

    ret = SwitchStreamUtil::RemoveAllRecordBySessionId(0);
    EXPECT_EQ(ret, true);
}

/**
* @tc.name  : Test SwitchStreamUtil API
* @tc.type  : FUNC
* @tc.number: SwitchStreamUtil_007
* @tc.desc  : Test SwitchStreamUtil::IsSwitchStreamSwitching().
*/
HWTEST(AudioUtilsProUnitTest, SwitchStreamUtil_007, TestSize.Level1)
{
    SwitchStreamInfo info = {0, 1, 0, 0, 0, CAPTURER_RUNNING};
    SwitchState targetState = SwitchState::SWITCH_STATE_WAITING;

    SwitchStreamUtil::InsertSwitchStreamRecord(info, targetState);

    SwitchStreamInfo info2 = {0, 1, 0, 0, 0, CAPTURER_RUNNING};
    SwitchState targetState2 = SwitchState::SWITCH_STATE_CREATED;

    auto ret = SwitchStreamUtil::IsSwitchStreamSwitching(info2, targetState2);
    EXPECT_EQ(ret, false);

    ret = SwitchStreamUtil::RemoveAllRecordBySessionId(0);
    EXPECT_EQ(ret, true);
}

/**
* @tc.name  : Test SwitchStreamUtil API
* @tc.type  : FUNC
* @tc.number: SwitchStreamUtil_008
* @tc.desc  : Test SwitchStreamUtil::IsSwitchStreamSwitching().
*/
HWTEST(AudioUtilsProUnitTest, SwitchStreamUtil_008, TestSize.Level1)
{
    SwitchStreamInfo info = {0, 1, 0, 0, 0, CAPTURER_RUNNING};
    SwitchState targetState = SwitchState::SWITCH_STATE_WAITING;

    SwitchStreamUtil::InsertSwitchStreamRecord(info, targetState);

    SwitchStreamInfo info2 = {0, 1, 0, 0, 0, CAPTURER_RUNNING};
    SwitchState targetState2 = SwitchState::SWITCH_STATE_STARTED;

    auto ret = SwitchStreamUtil::IsSwitchStreamSwitching(info2, targetState2);
    EXPECT_EQ(ret, false);

    ret = SwitchStreamUtil::RemoveAllRecordBySessionId(0);
    EXPECT_EQ(ret, true);
}

/**
* @tc.name  : Test SwitchStreamUtil API
* @tc.type  : FUNC
* @tc.number: SwitchStreamUtil_009
* @tc.desc  : Test SwitchStreamUtil::IsSwitchStreamSwitching().
*/
HWTEST(AudioUtilsProUnitTest, SwitchStreamUtil_009, TestSize.Level1)
{
    SwitchStreamInfo info = {0, 1, 0, 0, 0, CAPTURER_PREPARED};
    SwitchState targetState = SwitchState::SWITCH_STATE_CREATED;

    SwitchStreamUtil::InsertSwitchStreamRecord(info, targetState);

    SwitchStreamInfo info2 = {0, 1, 0, 0, 0, CAPTURER_PREPARED};
    SwitchState targetState2 = SwitchState::SWITCH_STATE_STARTED;

    auto ret = SwitchStreamUtil::IsSwitchStreamSwitching(info2, targetState2);
    EXPECT_EQ(ret, false);

    ret = SwitchStreamUtil::RemoveAllRecordBySessionId(0);
    EXPECT_EQ(ret, true);
}

/**
* @tc.name  : Test SwitchStreamUtil API
* @tc.type  : FUNC
* @tc.number: SwitchStreamUtil_010
* @tc.desc  : Test SwitchStreamUtil::TimeoutThreadHandleTimeoutRecord().
*/
HWTEST(AudioUtilsProUnitTest, SwitchStreamUtil_010, TestSize.Level1)
{
    SwitchStreamInfo info = {0, 1, 0, 0, 0, CAPTURER_PREPARED};
    SwitchState targetState = SwitchState::SWITCH_STATE_CREATED;

    SwitchStreamUtil::InsertSwitchStreamRecord(info, targetState);

    SwitchStreamInfo info2 = {0, 1, 0, 0, 0, CAPTURER_PREPARED};
    SwitchState targetState2 = SwitchState::SWITCH_STATE_STARTED;

    SwitchStreamUtil::TimeoutThreadHandleTimeoutRecord(info2, targetState2);

    auto ret = SwitchStreamUtil::RemoveAllRecordBySessionId(0);
    EXPECT_EQ(ret, true);
}

/**
* @tc.name  : Test SwitchStreamUtil API
* @tc.type  : FUNC
* @tc.number: SwitchStreamUtil_011
* @tc.desc  : Test SwitchStreamUtil::TimeoutThreadHandleTimeoutRecord().
*/
HWTEST(AudioUtilsProUnitTest, SwitchStreamUtil_011, TestSize.Level1)
{
    SwitchStreamInfo info = {0, 1, 0, 0, 0, CAPTURER_RUNNING};
    SwitchState targetState = SwitchState::SWITCH_STATE_CREATED;

    SwitchStreamUtil::InsertSwitchStreamRecord(info, targetState);

    SwitchStreamInfo info2 = {0, 1, 0, 0, 0, CAPTURER_PREPARED};
    SwitchState targetState2 = SwitchState::SWITCH_STATE_STARTED;

    SwitchStreamUtil::TimeoutThreadHandleTimeoutRecord(info2, targetState2);

    auto ret = SwitchStreamUtil::RemoveAllRecordBySessionId(0);
    EXPECT_EQ(ret, true);
}

/**
* @tc.name  : Test SwitchStreamUtil API
* @tc.type  : FUNC
* @tc.number: SwitchStreamUtil_012
* @tc.desc  : Test SwitchStreamUtil::RemoveSwitchStreamRecord().
*/
HWTEST(AudioUtilsProUnitTest, SwitchStreamUtil_012, TestSize.Level1)
{
    SwitchStreamInfo info = {0, 1, 0, 0, 0, CAPTURER_RUNNING};
    SwitchState targetState = SwitchState::SWITCH_STATE_CREATED;

    SwitchStreamUtil::InsertSwitchStreamRecord(info, targetState);

    SwitchStreamInfo info2 = {0, 1, 0, 0, 0, CAPTURER_PREPARED};
    SwitchState targetState2 = SwitchState::SWITCH_STATE_STARTED;

    auto ret = SwitchStreamUtil::RemoveSwitchStreamRecord(info2, targetState2);
    EXPECT_EQ(ret, true);

    ret = SwitchStreamUtil::RemoveAllRecordBySessionId(0);
    EXPECT_EQ(ret, true);
}

/**
* @tc.name  : Test SwitchStreamUtil API
* @tc.type  : FUNC
* @tc.number: SwitchStreamUtil_013
* @tc.desc  : Test SwitchStreamUtil::RemoveSwitchStreamRecord().
*/
HWTEST(AudioUtilsProUnitTest, SwitchStreamUtil_013, TestSize.Level1)
{
    SwitchStreamInfo info = {0, 1, 0, 0, 0, CAPTURER_RUNNING};
    SwitchState targetState = SwitchState::SWITCH_STATE_CREATED;

    SwitchStreamUtil::InsertSwitchStreamRecord(info, targetState);

    auto ret = SwitchStreamUtil::RemoveSwitchStreamRecord(info, targetState);
    EXPECT_EQ(ret, true);
}

/**
* @tc.name  : Test SwitchStreamUtil API
* @tc.type  : FUNC
* @tc.number: SwitchStreamUtil_014
* @tc.desc  : Test SwitchStreamUtil::RemoveAllRecordBySessionId().
*/
HWTEST(AudioUtilsProUnitTest, SwitchStreamUtil_014, TestSize.Level1)
{
    SwitchStreamInfo info = {0, 1, 0, 0, 0, CAPTURER_RUNNING};
    SwitchState targetState = SwitchState::SWITCH_STATE_CREATED;

    SwitchStreamUtil::InsertSwitchStreamRecord(info, targetState);

    SwitchStreamInfo info2 = {1, 1, 0, 0, 0, CAPTURER_PREPARED};
    SwitchState targetState2 = SwitchState::SWITCH_STATE_STARTED;

    SwitchStreamUtil::InsertSwitchStreamRecord(info2, targetState2);

    auto ret = SwitchStreamUtil::RemoveAllRecordBySessionId(0);
    EXPECT_EQ(ret, true);

    ret = SwitchStreamUtil::RemoveAllRecordBySessionId(1);
    EXPECT_EQ(ret, true);
}

/**
* @tc.name  : Test SwitchStreamUtil API
* @tc.type  : FUNC
* @tc.number: SwitchStreamUtil_015
* @tc.desc  : Test SwitchStreamUtil::HandleStartedSwitchInfoInRecord().
*/
HWTEST(AudioUtilsProUnitTest, SwitchStreamUtil_015, TestSize.Level1)
{
    SwitchStreamInfo info = {0, 1, 0, 0, 0, CAPTURER_RUNNING};
    SwitchState targetState = SwitchState::SWITCH_STATE_CREATED;

    SwitchStreamUtil::InsertSwitchStreamRecord(info, targetState);

    SwitchStreamInfo info2 = {0, 1, 0, 0, 0, CAPTURER_RUNNING};
    SwitchState targetState2 = SwitchState::SWITCH_STATE_STARTED;

    auto ret = SwitchStreamUtil::HandleStartedSwitchInfoInRecord(info2, targetState2);
    EXPECT_EQ(ret, true);

    ret = SwitchStreamUtil::RemoveAllRecordBySessionId(0);
    EXPECT_EQ(ret, true);
}

/**
* @tc.name  : Test SwitchStreamUtil API
* @tc.type  : FUNC
* @tc.number: SwitchStreamUtil_016
* @tc.desc  : Test SwitchStreamUtil::HandleStartedSwitchInfoInRecord().
*/
HWTEST(AudioUtilsProUnitTest, SwitchStreamUtil_016, TestSize.Level1)
{
    SwitchStreamInfo info = {0, 1, 0, 0, 0, CAPTURER_PREPARED};
    SwitchState targetState = SwitchState::SWITCH_STATE_CREATED;

    SwitchStreamUtil::InsertSwitchStreamRecord(info, targetState);

    SwitchStreamInfo info2 = {0, 1, 0, 0, 0, CAPTURER_PREPARED};
    SwitchState targetState2 = SwitchState::SWITCH_STATE_STARTED;

    auto ret = SwitchStreamUtil::HandleStartedSwitchInfoInRecord(info2, targetState2);
    EXPECT_EQ(ret, true);

    ret = SwitchStreamUtil::RemoveAllRecordBySessionId(0);
    EXPECT_EQ(ret, true);
}

/**
* @tc.name  : Test SwitchStreamUtil API
* @tc.type  : FUNC
* @tc.number: SwitchStreamUtil_017
* @tc.desc  : Test SwitchStreamUtil::HandleStartedSwitchInfoInRecord().
*/
HWTEST(AudioUtilsProUnitTest, SwitchStreamUtil_017, TestSize.Level1)
{
    SwitchStreamInfo info = {0, 1, 0, 0, 0, CAPTURER_PREPARED};
    SwitchState targetState = SwitchState::SWITCH_STATE_STARTED;

    SwitchStreamUtil::InsertSwitchStreamRecord(info, targetState);

    SwitchStreamInfo info2 = {0, 1, 0, 0, 0, CAPTURER_PREPARED};
    SwitchState targetState2 = SwitchState::SWITCH_STATE_STARTED;

    auto ret = SwitchStreamUtil::HandleStartedSwitchInfoInRecord(info2, targetState2);
    EXPECT_EQ(ret, true);

    ret = SwitchStreamUtil::RemoveAllRecordBySessionId(0);
    EXPECT_EQ(ret, true);
}

/**
* @tc.name  : Test SwitchStreamUtil API
* @tc.type  : FUNC
* @tc.number: SwitchStreamUtil_018
* @tc.desc  : Test SwitchStreamUtil::HandleSwitchInfoInRecord().
*/
HWTEST(AudioUtilsProUnitTest, SwitchStreamUtil_018, TestSize.Level1)
{
    SwitchStreamInfo info = {0, 1, 0, 0, 0, CAPTURER_INVALID};
    SwitchState targetState = SwitchState::SWITCH_STATE_STARTED;

    SwitchStreamUtil::InsertSwitchStreamRecord(info, targetState);

    SwitchStreamInfo info2 = {0, 1, 0, 0, 0, CAPTURER_INVALID};
    SwitchState targetState2 = SwitchState::SWITCH_STATE_STARTED;

    auto ret = SwitchStreamUtil::HandleSwitchInfoInRecord(info2, targetState2);
    EXPECT_EQ(ret, true);

    ret = SwitchStreamUtil::RemoveAllRecordBySessionId(0);
    EXPECT_EQ(ret, true);
}

/**
* @tc.name  : Test SwitchStreamUtil API
* @tc.type  : FUNC
* @tc.number: SwitchStreamUtil_019
* @tc.desc  : Test SwitchStreamUtil::HandleSwitchInfoInRecord().
*/
HWTEST(AudioUtilsProUnitTest, SwitchStreamUtil_019, TestSize.Level1)
{
    SwitchStreamInfo info = {0, 1, 0, 0, 0, CAPTURER_NEW};
    SwitchState targetState = SwitchState::SWITCH_STATE_CREATED;

    SwitchStreamUtil::InsertSwitchStreamRecord(info, targetState);

    SwitchStreamInfo info2 = {0, 1, 0, 0, 0, CAPTURER_NEW};
    SwitchState targetState2 = SwitchState::SWITCH_STATE_CREATED;

    auto ret = SwitchStreamUtil::HandleSwitchInfoInRecord(info2, targetState2);
    EXPECT_EQ(ret, true);

    ret = SwitchStreamUtil::RemoveAllRecordBySessionId(0);
    EXPECT_EQ(ret, true);
}

/**
* @tc.name  : Test SwitchStreamUtil API
* @tc.type  : FUNC
* @tc.number: SwitchStreamUtil_020
* @tc.desc  : Test SwitchStreamUtil::HandleSwitchInfoInRecord().
*/
HWTEST(AudioUtilsProUnitTest, SwitchStreamUtil_020, TestSize.Level1)
{
    SwitchStreamInfo info = {0, 1, 0, 0, 0, CAPTURER_NEW};
    SwitchState targetState = SwitchState::SWITCH_STATE_FINISHED;

    SwitchStreamUtil::InsertSwitchStreamRecord(info, targetState);

    SwitchStreamInfo info2 = {0, 1, 0, 0, 0, CAPTURER_NEW};
    SwitchState targetState2 = SwitchState::SWITCH_STATE_FINISHED;

    auto ret = SwitchStreamUtil::HandleSwitchInfoInRecord(info2, targetState2);
    EXPECT_EQ(ret, true);

    ret = SwitchStreamUtil::RemoveAllRecordBySessionId(0);
    EXPECT_EQ(ret, true);
}

/**
* @tc.name  : Test SwitchStreamUtil API
* @tc.type  : FUNC
* @tc.number: SwitchStreamUtil_021
* @tc.desc  : Test SwitchStreamUtil::HandleCreatedSwitchInfoInRecord().
*/
HWTEST(AudioUtilsProUnitTest, SwitchStreamUtil_021, TestSize.Level1)
{
    SwitchStreamInfo info = {0, 1, 0, 0, 0, CAPTURER_PREPARED};
    SwitchState targetState = SwitchState::SWITCH_STATE_WAITING;

    SwitchStreamUtil::InsertSwitchStreamRecord(info, targetState);

    SwitchStreamInfo info2 = {0, 1, 0, 0, 0, CAPTURER_PREPARED};
    SwitchState targetState2 = SwitchState::SWITCH_STATE_WAITING;

    auto ret = SwitchStreamUtil::HandleCreatedSwitchInfoInRecord(info2, targetState2);
    EXPECT_EQ(ret, true);

    ret = SwitchStreamUtil::RemoveAllRecordBySessionId(0);
    EXPECT_EQ(ret, true);
}

/**
* @tc.name  : Test SwitchStreamUtil API
* @tc.type  : FUNC
* @tc.number: SwitchStreamUtil_022
* @tc.desc  : Test SwitchStreamUtil::HandleCreatedSwitchInfoInRecord().
*/
HWTEST(AudioUtilsProUnitTest, SwitchStreamUtil_022, TestSize.Level1)
{
    SwitchStreamInfo info = {0, 1, 0, 0, 0, CAPTURER_NEW};
    SwitchState targetState = SwitchState::SWITCH_STATE_WAITING;

    SwitchStreamUtil::InsertSwitchStreamRecord(info, targetState);

    SwitchStreamInfo info2 = {0, 1, 0, 0, 0, CAPTURER_NEW};
    SwitchState targetState2 = SwitchState::SWITCH_STATE_WAITING;

    auto ret = SwitchStreamUtil::HandleCreatedSwitchInfoInRecord(info2, targetState2);
    EXPECT_EQ(ret, true);

    ret = SwitchStreamUtil::RemoveAllRecordBySessionId(0);
    EXPECT_EQ(ret, true);
}

/**
* @tc.name  : Test SwitchStreamUtil API
* @tc.type  : FUNC
* @tc.number: SwitchStreamUtil_023
* @tc.desc  : Test SwitchStreamUtil::HandleCreatedSwitchInfoInRecord().
*/
HWTEST(AudioUtilsProUnitTest, SwitchStreamUtil_023, TestSize.Level1)
{
    SwitchStreamInfo info = {0, 1, 0, 0, 0, CAPTURER_NEW};
    SwitchState targetState = SwitchState::SWITCH_STATE_TIMEOUT;

    SwitchStreamUtil::InsertSwitchStreamRecord(info, targetState);

    SwitchStreamInfo info2 = {0, 1, 0, 0, 0, CAPTURER_NEW};
    SwitchState targetState2 = SwitchState::SWITCH_STATE_TIMEOUT;

    auto ret = SwitchStreamUtil::HandleCreatedSwitchInfoInRecord(info2, targetState2);
    EXPECT_EQ(ret, true);

    ret = SwitchStreamUtil::RemoveAllRecordBySessionId(0);
    EXPECT_EQ(ret, true);
}

/**
* @tc.name  : Test SwitchStreamUtil API
* @tc.type  : FUNC
* @tc.number: SwitchStreamUtil_024
* @tc.desc  : Test SwitchStreamUtil::UpdateSwitchStreamRecord().
*/
HWTEST(AudioUtilsProUnitTest, SwitchStreamUtil_024, TestSize.Level1)
{
    SwitchStreamInfo info = {0, 1, 0, 0, 0, CAPTURER_NEW};
    SwitchState targetState = SwitchState::SWITCH_STATE_TIMEOUT;

    SwitchStreamUtil::InsertSwitchStreamRecord(info, targetState);

    SwitchStreamInfo info2 = {0, 2, 0, 0, 0, CAPTURER_NEW};
    SwitchState targetState2 = SwitchState::SWITCH_STATE_TIMEOUT;

    auto ret = SwitchStreamUtil::UpdateSwitchStreamRecord(info2, targetState2);
    EXPECT_EQ(ret, true);

    ret = SwitchStreamUtil::RemoveAllRecordBySessionId(0);
    EXPECT_EQ(ret, true);
}

/**
* @tc.name  : Test SwitchStreamUtil API
* @tc.type  : FUNC
* @tc.number: SwitchStreamUtil_025
* @tc.desc  : Test SwitchStreamUtil::UpdateSwitchStreamRecord().
*/
HWTEST(AudioUtilsProUnitTest, SwitchStreamUtil_025, TestSize.Level1)
{
    SwitchStreamInfo info = {0, 1, 0, 0, 0, CAPTURER_NEW};
    SwitchState targetState = SwitchState::SWITCH_STATE_TIMEOUT;

    SwitchStreamUtil::InsertSwitchStreamRecord(info, targetState);

    SwitchStreamInfo info2 = {0, 2, 0, 0, 0, CAPTURER_NEW};
    SwitchState targetState2 = SwitchState::SWITCH_STATE_WAITING;

    auto ret = SwitchStreamUtil::UpdateSwitchStreamRecord(info2, targetState2);
    EXPECT_EQ(ret, true);

    ret = SwitchStreamUtil::RemoveAllRecordBySessionId(0);
    EXPECT_EQ(ret, true);
}

/**
* @tc.name  : Test SwitchStreamUtil API
* @tc.type  : FUNC
* @tc.number: SwitchStreamUtil_026
* @tc.desc  : Test SwitchStreamUtil::UpdateSwitchStreamRecord().
*/
HWTEST(AudioUtilsProUnitTest, SwitchStreamUtil_026, TestSize.Level1)
{
    SwitchStreamInfo info = {0, 1, 0, 0, 0, CAPTURER_NEW};
    SwitchState targetState = SwitchState::SWITCH_STATE_TIMEOUT;

    SwitchStreamUtil::InsertSwitchStreamRecord(info, targetState);

    SwitchStreamInfo info2 = {0, 1, 0, 0, 0, CAPTURER_NEW};
    SwitchState targetState2 = SwitchState::SWITCH_STATE_WAITING;

    auto ret = SwitchStreamUtil::UpdateSwitchStreamRecord(info2, targetState2);
    EXPECT_EQ(ret, false);

    ret = SwitchStreamUtil::RemoveAllRecordBySessionId(0);
    EXPECT_EQ(ret, true);
}

/**
* @tc.name  : Test SwitchStreamUtil API
* @tc.type  : FUNC
* @tc.number: SwitchStreamUtil_027
* @tc.desc  : Test SwitchStreamUtil::UpdateSwitchStreamRecord().
*/
HWTEST(AudioUtilsProUnitTest, SwitchStreamUtil_027, TestSize.Level1)
{
    SwitchStreamInfo info = {0, 1, 0, 0, 0, CAPTURER_NEW};
    SwitchState targetState = SwitchState::SWITCH_STATE_FINISHED;

    SwitchStreamUtil::InsertSwitchStreamRecord(info, targetState);

    SwitchStreamInfo info2 = {0, 1, 0, 0, 0, CAPTURER_NEW};
    SwitchState targetState2 = SwitchState::SWITCH_STATE_CREATED;

    auto ret = SwitchStreamUtil::UpdateSwitchStreamRecord(info2, targetState2);
    EXPECT_EQ(ret, false);

    ret = SwitchStreamUtil::RemoveAllRecordBySessionId(0);
    EXPECT_EQ(ret, true);
}

/**
* @tc.name  : Test SwitchStreamUtil API
* @tc.type  : FUNC
* @tc.number: SwitchStreamUtil_028
* @tc.desc  : Test SwitchStreamUtil::UpdateSwitchStreamRecord().
*/
HWTEST(AudioUtilsProUnitTest, SwitchStreamUtil_028, TestSize.Level1)
{
    SwitchStreamInfo info = {0, 1, 0, 0, 0, CAPTURER_NEW};
    SwitchState targetState = SwitchState::SWITCH_STATE_WAITING;

    SwitchStreamUtil::InsertSwitchStreamRecord(info, targetState);

    SwitchStreamInfo info2 = {0, 1, 0, 0, 0, CAPTURER_NEW};
    SwitchState targetState2 = SwitchState::SWITCH_STATE_STARTED;

    auto ret = SwitchStreamUtil::UpdateSwitchStreamRecord(info2, targetState2);
    EXPECT_EQ(ret, true);

    ret = SwitchStreamUtil::RemoveAllRecordBySessionId(0);
    EXPECT_EQ(ret, true);
}

/**
* @tc.name  : Test SwitchStreamUtil API
* @tc.type  : FUNC
* @tc.number: SwitchStreamUtil_029
* @tc.desc  : Test SwitchStreamUtil::UpdateSwitchStreamRecord().
*/
HWTEST(AudioUtilsProUnitTest, SwitchStreamUtil_029, TestSize.Level1)
{
    SwitchStreamInfo info = {0, 1, 0, 0, 0, CAPTURER_NEW};
    SwitchState targetState = SwitchState::SWITCH_STATE_WAITING;

    SwitchStreamUtil::InsertSwitchStreamRecord(info, targetState);

    SwitchStreamInfo info2 = {0, 1, 0, 0, 0, CAPTURER_NEW};
    SwitchState targetState2 = SwitchState::SWITCH_STATE_TIMEOUT;

    auto ret = SwitchStreamUtil::UpdateSwitchStreamRecord(info2, targetState2);
    EXPECT_EQ(ret, true);

    ret = SwitchStreamUtil::RemoveAllRecordBySessionId(0);
    EXPECT_EQ(ret, true);
}
} // namespace AudioStandard
} // namespace OHOS