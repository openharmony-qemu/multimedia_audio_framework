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

#include "audio_session_state_monitor.h"
#include "audio_policy_state_monitor.h"
#include "audio_session_service.h"
#include "gtest/gtest.h"
#include <vector>

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

class AudioSessionStateMonitorTest : public testing::Test {};

/**
 * @tc.name  : NormalStartStopMonitorTest.
 * @tc.desc  : Test for normal start and stop monitor.
 */
HWTEST(AudioSessionStateMonitorTest, NormalStartStopMonitorTest, TestSize.Level1)
{
    auto audioSessionService = std::make_shared<AudioSessionService>();
    EXPECT_NE(audioSessionService, nullptr);
    auto audioSessionMonitor = std::static_pointer_cast<AudioSessionStateMonitor>(audioSessionService);
    int32_t callerPid = 1;
    audioSessionMonitor->StartMonitor(callerPid);
    int32_t moniteredPidNum = audioSessionMonitor->pidCbIdMap_.count(callerPid);
    EXPECT_EQ(moniteredPidNum, 1);
    audioSessionMonitor->StopMonitor(callerPid);
    moniteredPidNum = audioSessionMonitor->pidCbIdMap_.count(callerPid);
    EXPECT_EQ(moniteredPidNum, 0);
    EXPECT_EQ(audioSessionMonitor->pidCbIdMap_.size(), 0);
}

/**
 * @tc.name  : StartMonitorWithSamePidFailedTest.
 * @tc.desc  : Test for start monitor with same pid should fail.
 */
HWTEST(AudioSessionStateMonitorTest, StartMonitorWithSamePidFailTest, TestSize.Level1)
{
    auto audioSessionService = std::make_shared<AudioSessionService>();
    EXPECT_NE(audioSessionService, nullptr);
    auto audioSessionMonitor = std::static_pointer_cast<AudioSessionStateMonitor>(audioSessionService);
    int32_t callerPid = 1;
    audioSessionMonitor->StartMonitor(callerPid);
    int32_t moniteredPidNum = audioSessionMonitor->pidCbIdMap_.count(callerPid);
    EXPECT_EQ(moniteredPidNum, 1);
    audioSessionMonitor->StartMonitor(callerPid);
    moniteredPidNum = audioSessionMonitor->pidCbIdMap_.count(callerPid);
    EXPECT_EQ(moniteredPidNum, 1);
    EXPECT_EQ(audioSessionMonitor->pidCbIdMap_.size(), 1);
    audioSessionMonitor->StopMonitor(callerPid);
    moniteredPidNum = audioSessionMonitor->pidCbIdMap_.count(callerPid);
    EXPECT_EQ(moniteredPidNum, 0);
    EXPECT_EQ(audioSessionMonitor->pidCbIdMap_.size(), 0);
}

/**
 * @tc.name  : StopMonitorWithSamePidFailedTest.
 * @tc.desc  : Test for stop monitor with same pid should fail.
 */
HWTEST(AudioSessionStateMonitorTest, StopMonitorWithSamePidFailedTest, TestSize.Level1)
{
    auto audioSessionService = std::make_shared<AudioSessionService>();
    EXPECT_NE(audioSessionService, nullptr);
    auto audioSessionMonitor = std::static_pointer_cast<AudioSessionStateMonitor>(audioSessionService);
    int32_t callerPid = 1;
    audioSessionMonitor->StartMonitor(callerPid);
    int32_t moniteredPidNum = audioSessionMonitor->pidCbIdMap_.count(callerPid);
    EXPECT_EQ(moniteredPidNum, 1);
    int32_t callerPid2 = 2;
    audioSessionMonitor->StartMonitor(callerPid2);
    moniteredPidNum = audioSessionMonitor->pidCbIdMap_.count(callerPid2);
    EXPECT_EQ(moniteredPidNum, 1);
    EXPECT_EQ(audioSessionMonitor->pidCbIdMap_.size(), 2);
    audioSessionMonitor->StopMonitor(callerPid);
    moniteredPidNum = audioSessionMonitor->pidCbIdMap_.count(callerPid);
    EXPECT_EQ(moniteredPidNum, 0);
    EXPECT_EQ(audioSessionMonitor->pidCbIdMap_.size(), 1);
    audioSessionMonitor->StopMonitor(callerPid);
    EXPECT_EQ(audioSessionMonitor->pidCbIdMap_.size(), 1);
    audioSessionMonitor->StopMonitor(callerPid2);
    moniteredPidNum = audioSessionMonitor->pidCbIdMap_.count(callerPid2);
    EXPECT_EQ(moniteredPidNum, 0);
    EXPECT_EQ(audioSessionMonitor->pidCbIdMap_.size(), 0);
}

/**
 * @tc.name  : StartMonitorWithMaxPidNumTest.
 * @tc.desc  : Test for start monitor with max pid num.
 */
HWTEST(AudioSessionStateMonitorTest, StartMonitorWithMaxPidNumTest, TestSize.Level1)
{
    auto audioSessionService = std::make_shared<AudioSessionService>();
    EXPECT_NE(audioSessionService, nullptr);
    auto audioSessionMonitor = std::static_pointer_cast<AudioSessionStateMonitor>(audioSessionService);

    for (int32_t i = 0; i < MAX_CB_ID_NUM; i++) {
        audioSessionMonitor->StartMonitor(i);
        int32_t moniteredPidNum = audioSessionMonitor->pidCbIdMap_.count(i);
        EXPECT_EQ(moniteredPidNum, 1);
        EXPECT_EQ(audioSessionMonitor->pidCbIdMap_.size(), i + 1);
    }

    for (int32_t i = 0; i < MAX_CB_ID_NUM; i++) {
        audioSessionMonitor->StopMonitor(i);
        int32_t moniteredPidNum = audioSessionMonitor->pidCbIdMap_.count(i);
        EXPECT_EQ(moniteredPidNum, 0);
        EXPECT_EQ(audioSessionMonitor->pidCbIdMap_.size(), MAX_CB_ID_NUM - i - 1);
    }
}

/**
 * @tc.name  : StartMonitorOutOfMaxPidNumTest.
 * @tc.desc  : Test for start monitor out of max pid num range.
 */
HWTEST(AudioSessionStateMonitorTest, StartMonitorOutOfMaxPidNumTest, TestSize.Level1)
{
    auto audioSessionService = std::make_shared<AudioSessionService>();
    EXPECT_NE(audioSessionService, nullptr);
    auto audioSessionMonitor = std::static_pointer_cast<AudioSessionStateMonitor>(audioSessionService);

    for (int32_t i = 0; i < MAX_CB_ID_NUM; i++) {
        audioSessionMonitor->StartMonitor(i);
    }

    EXPECT_EQ(audioSessionMonitor->pidCbIdMap_.size(), MAX_CB_ID_NUM);

    for (int32_t i = 0; i < MAX_CB_ID_NUM; i++) {
        audioSessionMonitor->StopMonitor(i);
    }
}

} // AudioStandardnamespace
} // OHOSnamespace
