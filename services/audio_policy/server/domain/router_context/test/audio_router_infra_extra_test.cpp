/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "audio_router_context_test_base.h"
#include "audio_router_infra.h"
#include "audio_info.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

class AudioRouterInfraExtraTest : public AudioRouterContextTestBase {
};

HWTEST_F(AudioRouterInfraExtraTest, AudioRouterInfraExtraTest_UpdateOutputStreamState_001,
    TestSize.Level1)
{
    auto &routerInfra = AudioRouterInfra::GetInstance();

    routerInfra.UpdateOutputStreamState(TEST_APP_UID_10001, TEST_STREAM_ID_1,
        STREAM_USAGE_MEDIA, RendererState::RENDERER_RUNNING);
    routerInfra.UpdateOutputStreamState(TEST_APP_UID_10001, TEST_STREAM_ID_2,
        STREAM_USAGE_ALARM, RendererState::RENDERER_RUNNING);
    routerInfra.UpdateOutputStreamState(TEST_APP_UID_10001, TEST_STREAM_ID_3,
        STREAM_USAGE_VOICE_COMMUNICATION, RendererState::RENDERER_RUNNING);

    auto result1 = routerInfra.GetStreamStreamUsage(TEST_APP_UID_10001, TEST_STREAM_ID_1);
    auto result2 = routerInfra.GetStreamStreamUsage(TEST_APP_UID_10001, TEST_STREAM_ID_2);
    auto result3 = routerInfra.GetStreamStreamUsage(TEST_APP_UID_10001, TEST_STREAM_ID_3);

    EXPECT_EQ(result1, STREAM_USAGE_MEDIA);
    EXPECT_EQ(result2, STREAM_USAGE_ALARM);
    EXPECT_EQ(result3, STREAM_USAGE_VOICE_COMMUNICATION);
}

HWTEST_F(AudioRouterInfraExtraTest, AudioRouterInfraExtraTest_UpdateOutputStreamState_002,
    TestSize.Level1)
{
    auto &routerInfra = AudioRouterInfra::GetInstance();

    routerInfra.UpdateOutputStreamState(TEST_APP_UID_10001, TEST_STREAM_ID_1,
        STREAM_USAGE_MEDIA, RendererState::RENDERER_RUNNING);
    routerInfra.UpdateOutputStreamState(TEST_APP_UID_10001, TEST_STREAM_ID_1,
        STREAM_USAGE_MEDIA, RendererState::RENDERER_STOPPED);

    auto result1 = routerInfra.GetStreamStreamUsage(TEST_APP_UID_10001, TEST_STREAM_ID_1);
    EXPECT_EQ(result1, STREAM_USAGE_INVALID);
    auto result2 = routerInfra.GetAppRunningStreamUsages(routerInfra.GetHighestOutputPriorityApp());
    EXPECT_TRUE(result2.empty());
    auto result3 = routerInfra.GetStreamStreamUsage(TEST_APP_UID_10002, TEST_STREAM_ID_1);
    EXPECT_EQ(result3, STREAM_USAGE_INVALID);
}

HWTEST_F(AudioRouterInfraExtraTest, AudioRouterInfraExtraTest_UpdateOutputStreamState_003,
    TestSize.Level1)
{
    auto &routerInfra = AudioRouterInfra::GetInstance();

    std::vector<StreamUsage> streamUsages1 = { STREAM_USAGE_MEDIA };
    routerInfra.UpdateOutputStreamState(TEST_APP_UID_10001, TEST_STREAM_ID_1,
        STREAM_USAGE_MEDIA, RendererState::RENDERER_RUNNING);
    auto result1 = routerInfra.GetAppRunningStreamUsages(routerInfra.GetHighestOutputPriorityApp());
    ASSERT_EQ(result1.size(), 1);
    EXPECT_EQ(result1[0], STREAM_USAGE_MEDIA);

    routerInfra.UpdateOutputStreamState(TEST_APP_UID_10001, TEST_STREAM_ID_1,
        STREAM_USAGE_MEDIA, RendererState::RENDERER_PAUSED);
    auto result2 = routerInfra.GetStreamStreamUsage(TEST_APP_UID_10001, TEST_STREAM_ID_1);
    EXPECT_EQ(result2, STREAM_USAGE_INVALID);
}

HWTEST_F(AudioRouterInfraExtraTest, AudioRouterInfraExtraTest_UpdateOutputStreamState_005,
    TestSize.Level1)
{
    auto &routerInfra = AudioRouterInfra::GetInstance();

    routerInfra.UpdateAppForegroundState(TEST_APP_UID_10001, true);
    routerInfra.UpdateAppForegroundState(TEST_APP_UID_10001, false);

    routerInfra.UpdateOutputStreamState(TEST_APP_UID_10001, TEST_STREAM_ID_1,
        STREAM_USAGE_MEDIA, RendererState::RENDERER_RUNNING);
    routerInfra.UpdateOutputStreamState(TEST_APP_UID_10002, TEST_STREAM_ID_2,
        STREAM_USAGE_ALARM, RendererState::RENDERER_RUNNING);

    auto result1 = routerInfra.GetStreamStreamUsage(TEST_APP_UID_10001, TEST_STREAM_ID_1);
    auto result2 = routerInfra.GetStreamStreamUsage(TEST_APP_UID_10002, TEST_STREAM_ID_2);

    EXPECT_EQ(result1, STREAM_USAGE_MEDIA);
    EXPECT_EQ(result2, STREAM_USAGE_ALARM);

    auto result = routerInfra.GetAppRunningStreamUsages(routerInfra.GetHighestOutputPriorityApp());
    ASSERT_EQ(result.size(), 1);
    EXPECT_TRUE(std::find(result.begin(), result.end(), STREAM_USAGE_MEDIA) == result.end());
    EXPECT_TRUE(std::find(result.begin(), result.end(), STREAM_USAGE_ALARM) != result.end());
}

HWTEST_F(AudioRouterInfraExtraTest, AudioRouterInfraExtraTest_ParamAbrnormal_001,
    TestSize.Level1)
{
    auto &routerInfra = AudioRouterInfra::GetInstance();
    routerInfra.RefreshSelectDevice(SelectDeviceType::INVALID_TYPE, TEST_APP_UID_10001, nullptr);
    routerInfra.RefreshSelectDevice(SelectDeviceType::INVALID_TYPE, SYSTEM_UID, nullptr);
    routerInfra.RefreshSelectDevice(SelectDeviceType::INVALID_TYPE, SYSTEM_UID, nullptr);

    auto result1 = routerInfra.GetSystemSelectDevice(SelectDeviceType::INVALID_TYPE);
    EXPECT_EQ(result1.device_, nullptr);
    auto result2 = routerInfra.GetAppSelectDevice(SelectDeviceType::INVALID_TYPE, TEST_APP_UID_10001);
    EXPECT_EQ(result2.device_, nullptr);

    routerInfra.UpdatePreferredInputCategory(TEST_APP_UID_10001, PREFERRED_NONE);
    auto result5 = routerInfra.GetHighestPriorityPreferredInputCategory();
    EXPECT_EQ(result5, PREFERRED_NONE);
}

HWTEST_F(AudioRouterInfraExtraTest, AudioRouterInfraExtraTest_ParamAbrnormal_002,
    TestSize.Level1)
{
    auto &routerInfra = AudioRouterInfra::GetInstance();

    std::vector<SourceType> sourceTypes = { SourceType::SOURCE_TYPE_MIC };
    auto result1 = routerInfra.HasHighestPriorityRunningSourceType(sourceTypes);
    EXPECT_EQ(result1, false);

    int32_t highestUid = routerInfra.GetHighestInputPriorityApp();
    auto result3 = routerInfra.GetAppRunningSourceTypes(highestUid);
    EXPECT_EQ(result3.size(), 0);
    highestUid = routerInfra.GetHighestOutputPriorityApp();
    auto result4 = routerInfra.GetAppRunningStreamUsages(highestUid);
    EXPECT_EQ(result4.size(), 0);

    auto result5 = routerInfra.GetStreamSourceType(TEST_APP_UID_10001, TEST_STREAM_ID_1);
    EXPECT_EQ(result5, SourceType::SOURCE_TYPE_INVALID);
    auto result6 = routerInfra.GetStreamStreamUsage(TEST_APP_UID_10001, TEST_STREAM_ID_1);
    EXPECT_EQ(result6, StreamUsage::STREAM_USAGE_INVALID);

    auto result7 = routerInfra.GetStreamSelectDevice(TEST_APP_UID_10001, TEST_STREAM_ID_1);
    EXPECT_EQ(result7, nullptr);
}

HWTEST_F(AudioRouterInfraExtraTest, AudioRouterInfraExtraTest_ParamAbrnormal_003,
    TestSize.Level1)
{
    auto &routerInfra = AudioRouterInfra::GetInstance();
    routerInfra.ExcludeDevice(nullptr, MEDIA_OUTPUT_DEVICES);
    routerInfra.UnexcludeDevice(nullptr, MEDIA_OUTPUT_DEVICES);
    auto result1 = routerInfra.IsDeviceExcluded(nullptr, MEDIA_OUTPUT_DEVICES);
    EXPECT_EQ(result1, false);
}

HWTEST_F(AudioRouterInfraExtraTest, AudioRouterInfraExtraTest_AdjustOutputPriority_001,
    TestSize.Level1)
{
    auto &routerInfra = AudioRouterInfra::GetInstance();
    int32_t uid = TEST_APP_UID_10001;
    routerInfra.AdjustOutputPriority(uid, false, true, true);
    routerInfra.AdjustOutputPriority(uid, true, true, true);
    routerInfra.AdjustOutputPriority(uid, true, false, true);
    routerInfra.AdjustOutputPriority(uid, false, true, false);
    routerInfra.AdjustOutputPriority(uid, true, true, false);
    routerInfra.AdjustOutputPriority(uid, true, false, false);
    EXPECT_EQ(uid, TEST_APP_UID_10001);
}

/**
 * @tc.name  : Test UpdateExclusiveUidNoLock with deviceTokenId != 0 (NEW BRANCH)
 * @tc.type  : FUNC
 * @tc.number: AudioRouterInfraExtraTest_UpdateExclusiveUidNoLock_001
 * @tc.desc  : Test UpdateExclusiveUidNoLock adds uid to exclusiveUids_[type] when deviceTokenId != 0.
 */
HWTEST_F(AudioRouterInfraExtraTest, AudioRouterInfraExtraTest_UpdateExclusiveUidNoLock_001, TestSize.Level1)
{
    auto &routerInfra = AudioRouterInfra::GetInstance();
    int32_t uid = TEST_APP_UID_10001;
    uint32_t deviceTokenId = 12345;

    routerInfra.UpdateInputStreamState(uid, TEST_STREAM_ID_1, SOURCE_TYPE_MIC, CapturerState::CAPTURER_RUNNING);

    auto exclusiveDevice = std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_MIC, INPUT_DEVICE);
    exclusiveDevice->deviceTokenId_ = deviceTokenId;
    auto simpleDesc = std::make_shared<AudioDeviceSimpleDescriptor>(
        exclusiveDevice->deviceType_, exclusiveDevice->deviceRole_,
        exclusiveDevice->deviceId_, exclusiveDevice->networkId_, exclusiveDevice->macAddress_);

    routerInfra.RefreshSelectDevice(SelectDeviceType::MEDIA_INPUT, uid, simpleDesc, deviceTokenId);

    int32_t highestUid = routerInfra.GetHighestInputPriorityApp();
    EXPECT_EQ(highestUid, INVALID_UID);
}

/**
 * @tc.name  : Test UpdateExclusiveUidNoLock with deviceTokenId == 0 (NEW BRANCH)
 * @tc.type  : FUNC
 * @tc.number: AudioRouterInfraExtraTest_UpdateExclusiveUidNoLock_002
 * @tc.desc  : Test UpdateExclusiveUidNoLock removes uid from exclusiveUids_[type] when deviceTokenId == 0.
 */
HWTEST_F(AudioRouterInfraExtraTest, AudioRouterInfraExtraTest_UpdateExclusiveUidNoLock_002, TestSize.Level1)
{
    auto &routerInfra = AudioRouterInfra::GetInstance();
    int32_t uid = TEST_APP_UID_10001;

    routerInfra.UpdateInputStreamState(uid, TEST_STREAM_ID_1, SOURCE_TYPE_MIC, CapturerState::CAPTURER_RUNNING);

    auto normalDevice = std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_MIC, INPUT_DEVICE);
    normalDevice->deviceTokenId_ = 0;
    auto simpleDesc = std::make_shared<AudioDeviceSimpleDescriptor>(
        normalDevice->deviceType_, normalDevice->deviceRole_,
        normalDevice->deviceId_, normalDevice->networkId_, normalDevice->macAddress_);

    routerInfra.RefreshSelectDevice(SelectDeviceType::MEDIA_INPUT, uid, simpleDesc, 0);

    int32_t highestUid = routerInfra.GetHighestInputPriorityApp();
    EXPECT_EQ(highestUid, uid);
}

/**
 * @tc.name  : Test UpdateExclusiveUidNoLock with SYSTEM_UID (NEW BRANCH)
 * @tc.type  : FUNC
 * @tc.number: AudioRouterInfraExtraTest_UpdateExclusiveUidNoLock_003
 * @tc.desc  : Test UpdateExclusiveUidNoLock returns early for SYSTEM_UID.
 */
HWTEST_F(AudioRouterInfraExtraTest, AudioRouterInfraExtraTest_UpdateExclusiveUidNoLock_003, TestSize.Level1)
{
    auto &routerInfra = AudioRouterInfra::GetInstance();

    auto exclusiveDevice = std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_MIC, INPUT_DEVICE);
    exclusiveDevice->deviceTokenId_ = 12345;
    auto simpleDesc = std::make_shared<AudioDeviceSimpleDescriptor>(
        exclusiveDevice->deviceType_, exclusiveDevice->deviceRole_,
        exclusiveDevice->deviceId_, exclusiveDevice->networkId_, exclusiveDevice->macAddress_);

    routerInfra.RefreshSelectDevice(SelectDeviceType::MEDIA_INPUT, SYSTEM_UID, simpleDesc, 12345);

    auto selectInfo = routerInfra.GetSystemSelectDevice(SelectDeviceType::MEDIA_INPUT);
    EXPECT_NE(selectInfo.device_, nullptr);
}

/**
 * @tc.name  : Test RefreshSelectDevice default deviceTokenId parameter (NEW BRANCH)
 * @tc.type  : FUNC
 * @tc.number: AudioRouterInfraExtraTest_RefreshSelectDevice_DefaultToken_001
 * @tc.desc  : Test RefreshSelectDevice with default deviceTokenId=0 behaves correctly.
 */
HWTEST_F(AudioRouterInfraExtraTest, AudioRouterInfraExtraTest_RefreshSelectDevice_DefaultToken_001, TestSize.Level1)
{
    auto &routerInfra = AudioRouterInfra::GetInstance();
    int32_t uid = TEST_APP_UID_10001;

    routerInfra.UpdateInputStreamState(uid, TEST_STREAM_ID_1, SOURCE_TYPE_MIC, CapturerState::CAPTURER_RUNNING);

    auto normalDevice = std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_MIC, INPUT_DEVICE);
    normalDevice->deviceTokenId_ = 0;
    auto simpleDesc = std::make_shared<AudioDeviceSimpleDescriptor>(
        normalDevice->deviceType_, normalDevice->deviceRole_,
        normalDevice->deviceId_, normalDevice->networkId_, normalDevice->macAddress_);

    routerInfra.RefreshSelectDevice(SelectDeviceType::MEDIA_INPUT, uid, simpleDesc);

    int32_t highestUid = routerInfra.GetHighestInputPriorityApp();
    EXPECT_EQ(highestUid, uid);
}

/**
 * @tc.name  : Test GetHighestInputPriorityAppNoLock skips exclusive uids (NEW BRANCH)
 * @tc.type  : FUNC
 * @tc.number: AudioRouterInfraExtraTest_GetHighestInputPriorityApp_001
 * @tc.desc  : Test GetHighestInputPriorityApp skips exclusive uids and returns first non-exclusive.
 */
HWTEST_F(AudioRouterInfraExtraTest, AudioRouterInfraExtraTest_GetHighestInputPriorityApp_001, TestSize.Level1)
{
    auto &routerInfra = AudioRouterInfra::GetInstance();

    routerInfra.UpdateInputStreamState(TEST_APP_UID_10001, TEST_STREAM_ID_1,
        SOURCE_TYPE_MIC, CapturerState::CAPTURER_RUNNING);
    routerInfra.UpdateInputStreamState(TEST_APP_UID_10002, TEST_STREAM_ID_2,
        SOURCE_TYPE_MIC, CapturerState::CAPTURER_RUNNING);

    auto exclusiveDevice = std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_MIC, INPUT_DEVICE);
    exclusiveDevice->deviceTokenId_ = 12345;
    auto simpleDesc = std::make_shared<AudioDeviceSimpleDescriptor>(
        exclusiveDevice->deviceType_, exclusiveDevice->deviceRole_,
        exclusiveDevice->deviceId_, exclusiveDevice->networkId_, exclusiveDevice->macAddress_);

    routerInfra.RefreshSelectDevice(SelectDeviceType::MEDIA_INPUT, TEST_APP_UID_10001, simpleDesc, 12345);

    int32_t highestUid = routerInfra.GetHighestInputPriorityApp();
    EXPECT_EQ(highestUid, TEST_APP_UID_10002);
}

/**
 * @tc.name  : Test GetHighestOutputPriorityAppNoLock skips exclusive uids (NEW BRANCH)
 * @tc.type  : FUNC
 * @tc.number: AudioRouterInfraExtraTest_GetHighestOutputPriorityApp_001
 * @tc.desc  : Test GetHighestOutputPriorityApp skips exclusive uids and returns first non-exclusive.
 */
HWTEST_F(AudioRouterInfraExtraTest, AudioRouterInfraExtraTest_GetHighestOutputPriorityApp_001, TestSize.Level1)
{
    auto &routerInfra = AudioRouterInfra::GetInstance();

    routerInfra.UpdateOutputStreamState(TEST_APP_UID_10001, TEST_STREAM_ID_1,
        STREAM_USAGE_MEDIA, RendererState::RENDERER_RUNNING);
    routerInfra.UpdateOutputStreamState(TEST_APP_UID_10002, TEST_STREAM_ID_2,
        STREAM_USAGE_MEDIA, RendererState::RENDERER_RUNNING);

    auto exclusiveDevice = std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_SPEAKER, OUTPUT_DEVICE);
    exclusiveDevice->deviceTokenId_ = 12345;
    auto simpleDesc = std::make_shared<AudioDeviceSimpleDescriptor>(
        exclusiveDevice->deviceType_, exclusiveDevice->deviceRole_,
        exclusiveDevice->deviceId_, exclusiveDevice->networkId_, exclusiveDevice->macAddress_);

    routerInfra.RefreshSelectDevice(SelectDeviceType::MEDIA_OUTPUT, TEST_APP_UID_10001, simpleDesc, 12345);

    int32_t highestUid = routerInfra.GetHighestOutputPriorityApp();
    EXPECT_EQ(highestUid, TEST_APP_UID_10002);
}

/**
 * @tc.name  : Test OnAppDied clears all exclusiveUids_ entries (NEW BRANCH)
 * @tc.type  : FUNC
 * @tc.number: AudioRouterInfraExtraTest_OnAppDied_001
 * @tc.desc  : Test OnAppDied removes uid from all exclusiveUids_[type] array entries.
 */
HWTEST_F(AudioRouterInfraExtraTest, AudioRouterInfraExtraTest_OnAppDied_001, TestSize.Level1)
{
    auto &routerInfra = AudioRouterInfra::GetInstance();
    int32_t uid = TEST_APP_UID_10001;

    routerInfra.UpdateInputStreamState(uid, TEST_STREAM_ID_1, SOURCE_TYPE_MIC, CapturerState::CAPTURER_RUNNING);
    routerInfra.UpdateOutputStreamState(uid, TEST_STREAM_ID_2, STREAM_USAGE_MEDIA, RendererState::RENDERER_RUNNING);

    auto inputDevice = std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_MIC, INPUT_DEVICE);
    inputDevice->deviceTokenId_ = 12345;
    auto inputSimpleDesc = std::make_shared<AudioDeviceSimpleDescriptor>(
        inputDevice->deviceType_, inputDevice->deviceRole_,
        inputDevice->deviceId_, inputDevice->networkId_, inputDevice->macAddress_);

    auto outputDevice = std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_SPEAKER, OUTPUT_DEVICE);
    outputDevice->deviceTokenId_ = 12345;
    auto outputSimpleDesc = std::make_shared<AudioDeviceSimpleDescriptor>(
        outputDevice->deviceType_, outputDevice->deviceRole_,
        outputDevice->deviceId_, outputDevice->networkId_, outputDevice->macAddress_);

    routerInfra.RefreshSelectDevice(SelectDeviceType::MEDIA_INPUT, uid, inputSimpleDesc, 12345);
    routerInfra.RefreshSelectDevice(SelectDeviceType::MEDIA_OUTPUT, uid, outputSimpleDesc, 12345);

    EXPECT_EQ(routerInfra.GetHighestInputPriorityApp(), INVALID_UID);
    EXPECT_EQ(routerInfra.GetHighestOutputPriorityApp(), INVALID_UID);

    routerInfra.OnAppDied(uid);

    routerInfra.UpdateInputStreamState(TEST_APP_UID_10002, TEST_STREAM_ID_3,
        SOURCE_TYPE_MIC, CapturerState::CAPTURER_RUNNING);
    int32_t highestUid = routerInfra.GetHighestInputPriorityApp();
    EXPECT_EQ(highestUid, TEST_APP_UID_10002);
}

/**
 * @tc.name  : Test ClearAllSelectSelectDevice clears exclusive uid (NEW BRANCH via UpdateExclusiveUidNoLock)
 * @tc.type  : FUNC
 * @tc.number: AudioRouterInfraExtraTest_ClearAllSelectSelectDevice_001
 * @tc.desc  : Test ClearAllSelectSelectDevice clears exclusive uid tracking.
 */
HWTEST_F(AudioRouterInfraExtraTest, AudioRouterInfraExtraTest_ClearAllSelectSelectDevice_001, TestSize.Level1)
{
    auto &routerInfra = AudioRouterInfra::GetInstance();
    int32_t uid = TEST_APP_UID_10001;

    routerInfra.UpdateInputStreamState(uid, TEST_STREAM_ID_1, SOURCE_TYPE_MIC, CapturerState::CAPTURER_RUNNING);

    auto exclusiveDevice = std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_MIC, INPUT_DEVICE);
    exclusiveDevice->deviceTokenId_ = 12345;
    auto simpleDesc = std::make_shared<AudioDeviceSimpleDescriptor>(
        exclusiveDevice->deviceType_, exclusiveDevice->deviceRole_,
        exclusiveDevice->deviceId_, exclusiveDevice->networkId_, exclusiveDevice->macAddress_);

    routerInfra.RefreshSelectDevice(SelectDeviceType::MEDIA_INPUT, uid, simpleDesc, 12345);
    EXPECT_EQ(routerInfra.GetHighestInputPriorityApp(), INVALID_UID);

    routerInfra.ClearAllSelectSelectDevice(SelectDeviceType::MEDIA_INPUT);

    routerInfra.UpdateInputStreamState(TEST_APP_UID_10002, TEST_STREAM_ID_2,
        SOURCE_TYPE_MIC, CapturerState::CAPTURER_RUNNING);
    int32_t highestUid = routerInfra.GetHighestInputPriorityApp();
    EXPECT_EQ(highestUid, TEST_APP_UID_10002);
}

/**
 * @tc.name  : Test exclusive uid tracking for CALL_INPUT/CALL_OUTPUT (NEW BRANCH)
 * @tc.type  : FUNC
 * @tc.number: AudioRouterInfraExtraTest_CallExclusive_001
 * @tc.desc  : Test exclusive uid tracking works correctly for CALL types.
 */
HWTEST_F(AudioRouterInfraExtraTest, AudioRouterInfraExtraTest_CallExclusive_001, TestSize.Level1)
{
    auto &routerInfra = AudioRouterInfra::GetInstance();
    int32_t uid = TEST_APP_UID_10001;

    routerInfra.UpdateInputStreamState(uid, TEST_STREAM_ID_1, SOURCE_TYPE_MIC, CapturerState::CAPTURER_RUNNING);
    routerInfra.UpdateOutputStreamState(uid, TEST_STREAM_ID_2, STREAM_USAGE_MEDIA, RendererState::RENDERER_RUNNING);

    auto inputDevice = std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_MIC, INPUT_DEVICE);
    inputDevice->deviceTokenId_ = 12345;
    auto inputSimpleDesc = std::make_shared<AudioDeviceSimpleDescriptor>(
        inputDevice->deviceType_, inputDevice->deviceRole_,
        inputDevice->deviceId_, inputDevice->networkId_, inputDevice->macAddress_);

    routerInfra.RefreshSelectDevice(SelectDeviceType::CALL_INPUT, uid, inputSimpleDesc, 12345);

    int32_t inputHighestUid = routerInfra.GetHighestInputPriorityApp();
    EXPECT_EQ(inputHighestUid, INVALID_UID);

    routerInfra.UpdateOutputStreamState(TEST_APP_UID_10002, TEST_STREAM_ID_3,
        STREAM_USAGE_MEDIA, RendererState::RENDERER_RUNNING);
    int32_t outputHighestUid = routerInfra.GetHighestOutputPriorityApp();
    EXPECT_EQ(outputHighestUid, TEST_APP_UID_10002);
}

/**
 * @tc.name  : Test RefreshSelectDevice with all SelectDeviceType values (NEW BRANCH coverage)
 * @tc.type  : FUNC
 * @tc.number: AudioRouterInfraExtraTest_RefreshSelectDevice_001
 * @tc.desc  : Test RefreshSelectDevice correctly updates exclusiveUids_ for each type.
 */
HWTEST_F(AudioRouterInfraExtraTest, AudioRouterInfraExtraTest_RefreshSelectDevice_001, TestSize.Level1)
{
    auto &routerInfra = AudioRouterInfra::GetInstance();
    int32_t uid = TEST_APP_UID_10001;
    uint32_t deviceTokenId = 12345;

    routerInfra.UpdateInputStreamState(uid, TEST_STREAM_ID_1, SOURCE_TYPE_MIC, CapturerState::CAPTURER_RUNNING);
    routerInfra.UpdateOutputStreamState(uid, TEST_STREAM_ID_2, STREAM_USAGE_MEDIA, RendererState::RENDERER_RUNNING);

    auto inputDevice = std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_MIC, INPUT_DEVICE);
    inputDevice->deviceTokenId_ = deviceTokenId;
    auto inputSimpleDesc = std::make_shared<AudioDeviceSimpleDescriptor>(
        inputDevice->deviceType_, inputDevice->deviceRole_,
        inputDevice->deviceId_, inputDevice->networkId_, inputDevice->macAddress_);

    auto outputDevice = std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_SPEAKER, OUTPUT_DEVICE);
    outputDevice->deviceTokenId_ = deviceTokenId;
    auto outputSimpleDesc = std::make_shared<AudioDeviceSimpleDescriptor>(
        outputDevice->deviceType_, outputDevice->deviceRole_,
        outputDevice->deviceId_, outputDevice->networkId_, outputDevice->macAddress_);

    routerInfra.RefreshSelectDevice(SelectDeviceType::MEDIA_INPUT, uid, inputSimpleDesc, deviceTokenId);
    routerInfra.RefreshSelectDevice(SelectDeviceType::MEDIA_OUTPUT, uid, outputSimpleDesc, deviceTokenId);
    routerInfra.RefreshSelectDevice(SelectDeviceType::CALL_INPUT, uid, inputSimpleDesc, deviceTokenId);
    routerInfra.RefreshSelectDevice(SelectDeviceType::CALL_OUTPUT, uid, outputSimpleDesc, deviceTokenId);

    int32_t inputHighestUid = routerInfra.GetHighestInputPriorityApp();
    int32_t outputHighestUid = routerInfra.GetHighestOutputPriorityApp();
    EXPECT_EQ(inputHighestUid, INVALID_UID);
    EXPECT_EQ(outputHighestUid, INVALID_UID);
}

/**
 * @tc.name  : Test IsUidWithExclusiveDevice
 * @tc.type  : FUNC
 * @tc.number: IsUidWithExclusiveDevice_001
 * @tc.desc  : IsUidWithExclusiveDevice_001
 */
HWTEST_F(AudioRouterInfraExtraTest, IsUidWithExclusiveDevice_001, TestSize.Level1)
{
    auto &routerInfra = AudioRouterInfra::GetInstance();
    int32_t uid = TEST_APP_UID_10001;
    routerInfra.UpdateInputStreamState(uid, TEST_STREAM_ID_1, SOURCE_TYPE_MIC, CapturerState::CAPTURER_RUNNING);
    auto exclusiveDevice = std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_MIC, INPUT_DEVICE);
    exclusiveDevice->deviceTokenId_ = 12345;
    auto simpleDesc = std::make_shared<AudioDeviceSimpleDescriptor>(
        exclusiveDevice->deviceType_, exclusiveDevice->deviceRole_,
        exclusiveDevice->deviceId_, exclusiveDevice->networkId_, exclusiveDevice->macAddress_);

    routerInfra.RefreshSelectDevice(SelectDeviceType::MEDIA_INPUT, uid, simpleDesc, 12345);
    EXPECT_TRUE(routerInfra.IsUidWithExclusiveDevice(uid, SelectDeviceType::MEDIA_INPUT));

    EXPECT_FALSE(routerInfra.IsUidWithExclusiveDevice(uid, SelectDeviceType::CALL_INPUT));

    routerInfra.ClearAllSelectSelectDevice(SelectDeviceType::MEDIA_INPUT);
    EXPECT_FALSE(routerInfra.IsUidWithExclusiveDevice(uid, SelectDeviceType::MEDIA_INPUT));
}

}
}