/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

#include "audio_offload_stream_unit_test.h"

#include <iostream>
#include "gtest/gtest.h"
#include "audio_errors.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

static const uint32_t TEST_OFFLOAD_IN_PRIMARY_1_SESSION_ID = 100000;
static const uint32_t TEST_OFFLOAD_IN_PRIMARY_2_SESSION_ID = 100001;
static const uint32_t TEST_OFFLOAD_IN_REMOTE_1_SESSION_ID = 100002;
static const uint32_t TEST_OFFLOAD_IN_REMOTE_2_SESSION_ID = 100003;

void AudioOffloadStreamTest::SetUpTestCase(void) {}

void AudioOffloadStreamTest::TearDownTestCase(void) {}

void AudioOffloadStreamTest::SetUp(void)
{
    testOffloadModule_ = std::make_shared<AudioOffloadStream>();
}

void AudioOffloadStreamTest::TearDown(void)
{
    testOffloadModule_ = nullptr;
}

/**
 * @tc.name  : HandlePowerStateChanged_NoChange_Test
 * @tc.number: AudioOffloadStreamTest_001
 * @tc.desc  : Test scenario where the power state does not change.
 */
HWTEST_F(AudioOffloadStreamTest, HandlePowerStateChanged_NoChange_Test, TestSize.Level2)
{
    testOffloadModule_->currentPowerState_ = PowerMgr::PowerState::FREEZE;
    PowerMgr::PowerState state = PowerMgr::PowerState::FREEZE;
    testOffloadModule_->HandlePowerStateChanged(state);
    EXPECT_EQ(testOffloadModule_->currentPowerState_, PowerMgr::PowerState::FREEZE);
}

/**
 * @tc.name  : HandlePowerStateChanged_ActiveToInactive_Test
 * @tc.number: AudioOffloadStreamTest_002
 * @tc.desc  : Test scenario where the power state changes from ACTIVE to INACTIVE.
 */
HWTEST_F(AudioOffloadStreamTest, HandlePowerStateChanged_ActiveToInactive_Test, TestSize.Level2)
{
    testOffloadModule_->currentPowerState_ = PowerMgr::PowerState::FREEZE;
    PowerMgr::PowerState state = PowerMgr::PowerState::INACTIVE;
    testOffloadModule_->HandlePowerStateChanged(state);
    EXPECT_EQ(testOffloadModule_->currentPowerState_, PowerMgr::PowerState::INACTIVE);
}

/**
 * @tc.name  : HandlePowerStateChanged_NoOffloadSupport_Test
 * @tc.number: AudioOffloadStreamTest_003
 * @tc.desc  : Test scenario where the device does not support offload.
 */
HWTEST_F(AudioOffloadStreamTest, HandlePowerStateChanged_NoOffloadSupport_Test, TestSize.Level3)
{
    testOffloadModule_->currentPowerState_ = PowerMgr::PowerState::FREEZE;
    PowerMgr::PowerState state = PowerMgr::PowerState::FREEZE;
    testOffloadModule_->HandlePowerStateChanged(state);
    EXPECT_EQ(testOffloadModule_->currentPowerState_, PowerMgr::PowerState::FREEZE);
}

/**
 * @tc.name  : HandlePowerStateChanged_OffloadSessionIDNotSet_Test
 * @tc.number: audioOffloadStreamTest_004
 * @tc.desc  : Test scenario where the offload session ID is not set.
 */
HWTEST_F(AudioOffloadStreamTest, HandlePowerStateChanged_OffloadSessionIDNotSet_Test, TestSize.Level3)
{
    testOffloadModule_->currentPowerState_ = PowerMgr::PowerState::FREEZE;
    PowerMgr::PowerState state = PowerMgr::PowerState::FREEZE;
    testOffloadModule_->HandlePowerStateChanged(state);
    EXPECT_EQ(testOffloadModule_->currentPowerState_, PowerMgr::PowerState::FREEZE);
}

/**
 * @tc.name  : AudioOffloadStreamTest_013
 * @tc.number: AudioOffloadStreamTest_013
 * @tc.desc  : Test FilterSinkInputs function
 */
HWTEST_F(AudioOffloadStreamTest, FilterSinkInputs_ShouldReturnEmpty_WhenSinkInputsIsEmpty, TestSize.Level4)
{
    AudioOffloadStream testModule;
    std::vector<SinkInput> sinkInputs;
    std::vector<SinkInput> result = testModule.FilterSinkInputs(1, sinkInputs);
    EXPECT_TRUE(result.empty());
    SinkInput sinkInput;
    sinkInput.uid = 123;
    sinkInput.streamType = AudioStreamType::STREAM_DEFAULT;
    sinkInputs.push_back(sinkInput);

    result = testModule.FilterSinkInputs(1, sinkInputs);
    EXPECT_TRUE(result.empty());
    sinkInput.streamId = 1;
    sinkInputs.push_back(sinkInput);

    result = testModule.FilterSinkInputs(1, sinkInputs);
    EXPECT_EQ(result.size(), 0);
}

/**
 * @tc.name   : AudioOffloadStreamTest_GetInstance_001
 * @tc.number : GetInstance_001
 * @tc.desc   : Test GetInstance()
 */
HWTEST_F(AudioOffloadStreamTest, GetInstance_001, TestSize.Level3)
{
    AudioOffloadStream &testModule = AudioOffloadStream::GetInstance();
    EXPECT_EQ(OFFLOAD_IN_ADAPTER_SIZE, testModule.offloadSessionIdMap_.size());
}

/**
 * @tc.name   : AudioOffloadStreamTest_GetOffloadSessionId_001
 * @tc.number : GetOffloadSessionId_001
 * @tc.desc   : Test GetOffloadSessionId() abnormal cases
 */
HWTEST_F(AudioOffloadStreamTest, GetOffloadSessionId_001, TestSize.Level3)
{
    EXPECT_EQ(NO_OFFLOAD_STREAM_SESSIONID, testOffloadModule_->GetOffloadSessionId(OFFLOAD_IN_PRIMARY));
    EXPECT_EQ(NO_OFFLOAD_STREAM_SESSIONID, testOffloadModule_->GetOffloadSessionId(OFFLOAD_IN_REMOTE));
}

/**
 * @tc.name   : AudioOffloadStreamTest_SetOffloadStatus_001
 * @tc.number : SetOffloadStatus_001
 * @tc.desc   : Test SetOffloadStatus() in normal situation
 */
HWTEST_F(AudioOffloadStreamTest, SetOffloadStatus_001, TestSize.Level2)
{
    testOffloadModule_->SetOffloadStatus(OFFLOAD_IN_PRIMARY, TEST_OFFLOAD_IN_PRIMARY_1_SESSION_ID);
    EXPECT_EQ(TEST_OFFLOAD_IN_PRIMARY_1_SESSION_ID, testOffloadModule_->GetOffloadSessionId(OFFLOAD_IN_PRIMARY));
    testOffloadModule_->UnsetOffloadStatus(TEST_OFFLOAD_IN_PRIMARY_1_SESSION_ID);
    EXPECT_EQ(NO_OFFLOAD_STREAM_SESSIONID, testOffloadModule_->GetOffloadSessionId(OFFLOAD_IN_PRIMARY));
}

/**
 * @tc.name   : AudioOffloadStreamTest_SetOffloadStatus_002
 * @tc.number : SetOffloadStatus_002
 * @tc.desc   : Test SetOffloadStatus() in abnormal situation
 */
HWTEST_F(AudioOffloadStreamTest, SetOffloadStatus_002, TestSize.Level3)
{
    testOffloadModule_->SetOffloadStatus(OFFLOAD_IN_PRIMARY, TEST_OFFLOAD_IN_PRIMARY_1_SESSION_ID);
    EXPECT_EQ(TEST_OFFLOAD_IN_PRIMARY_1_SESSION_ID, testOffloadModule_->GetOffloadSessionId(OFFLOAD_IN_PRIMARY));
    testOffloadModule_->SetOffloadStatus(OFFLOAD_IN_REMOTE, TEST_OFFLOAD_IN_PRIMARY_1_SESSION_ID);
    EXPECT_EQ(NO_OFFLOAD_STREAM_SESSIONID, testOffloadModule_->GetOffloadSessionId(OFFLOAD_IN_PRIMARY));
    EXPECT_EQ(TEST_OFFLOAD_IN_PRIMARY_1_SESSION_ID, testOffloadModule_->GetOffloadSessionId(OFFLOAD_IN_REMOTE));
}

/**
 * @tc.name   : AudioOffloadStreamTest_UpdateOffloadStatusFromUpdateTracker_001
 * @tc.number : UpdateOffloadStatusFromUpdateTracker_001
 * @tc.desc   : Test UpdateOffloadStatusFromUpdateTracker() in normal situation
 */
HWTEST_F(AudioOffloadStreamTest, UpdateOffloadStatusFromUpdateTracker_001, TestSize.Level2)
{
    testOffloadModule_->SetOffloadStatus(OFFLOAD_IN_PRIMARY, TEST_OFFLOAD_IN_PRIMARY_1_SESSION_ID);

    testOffloadModule_->UpdateOffloadStatusFromUpdateTracker(TEST_OFFLOAD_IN_PRIMARY_1_SESSION_ID, RENDERER_STOPPED);
    EXPECT_EQ(TEST_OFFLOAD_IN_PRIMARY_1_SESSION_ID, testOffloadModule_->GetOffloadSessionId(OFFLOAD_IN_PRIMARY));

    testOffloadModule_->UpdateOffloadStatusFromUpdateTracker(TEST_OFFLOAD_IN_PRIMARY_1_SESSION_ID, RENDERER_RUNNING);
    EXPECT_EQ(TEST_OFFLOAD_IN_PRIMARY_1_SESSION_ID, testOffloadModule_->GetOffloadSessionId(OFFLOAD_IN_PRIMARY));
}

/**
 * @tc.name   : AudioOffloadStreamTest_UpdateOffloadStatusFromUpdateTracker_002
 * @tc.number : UpdateOffloadStatusFromUpdateTracker_002
 * @tc.desc   : Test UpdateOffloadStatusFromUpdateTracker() in abnormal stream situation
 */
HWTEST_F(AudioOffloadStreamTest, UpdateOffloadStatusFromUpdateTracker_002, TestSize.Level4)
{
    testOffloadModule_->SetOffloadStatus(OFFLOAD_IN_PRIMARY, TEST_OFFLOAD_IN_PRIMARY_1_SESSION_ID);
    testOffloadModule_->UpdateOffloadStatusFromUpdateTracker(TEST_OFFLOAD_IN_PRIMARY_2_SESSION_ID, RENDERER_RUNNING);
    EXPECT_EQ(TEST_OFFLOAD_IN_PRIMARY_1_SESSION_ID, testOffloadModule_->GetOffloadSessionId(OFFLOAD_IN_PRIMARY));
}

/**
 * @tc.name   : AudioOffloadStreamTest_Dump_001
 * @tc.number : Dump_001
 * @tc.desc   : Test Dump() in abnormal situation
 */
HWTEST_F(AudioOffloadStreamTest, Dump_001, TestSize.Level3)
{
    std::string dumpStr;
    testOffloadModule_->Dump(dumpStr);
    EXPECT_NE("", dumpStr);
}

} // namespace AudioStandard
} // namespace OHOS
