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

#include "negative_priority_router_unit_test.h"
#include "audio_errors.h"
#include "audio_policy_log.h"
#include "audio_router_infra.h"

#include <thread>
#include <memory>
#include <vector>
using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

const static int32_t TEST_CLIENT_UID = 1;
const static uint32_t TEST_SESSION_ID = 1;

void NegativePriorityRouterUnitTest::SetUpTestCase(void) {}
void NegativePriorityRouterUnitTest::TearDownTestCase(void) {}
void NegativePriorityRouterUnitTest::SetUp(void) {}
void NegativePriorityRouterUnitTest::TearDown(void) {}

/**
 * Building watch device, which is a negative device and should NOT be filtered.
 */
shared_ptr<AudioDeviceDescriptor> CreateWatchDevice(int32_t timestamp, DeviceUsage usage = MEDIA)
{
    auto watchDevice = make_shared<AudioDeviceDescriptor>();
    watchDevice->deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    watchDevice->deviceCategory_ = BT_WATCH;
    watchDevice->deviceUsage_ = usage;
    watchDevice->exceptionFlag_ = false;
    watchDevice->isEnable_ = true;
    watchDevice->connectState_ = CONNECTED;
    watchDevice->connectTimeStamp_ = timestamp;
    return watchDevice;
}

/**
 * Building unwear BT headphone, which is a negative device and should be filtered.
 */
shared_ptr<AudioDeviceDescriptor> CreateUnwearBtHeadphoneDevice(int32_t timestamp, DeviceUsage usage = MEDIA)
{
    auto unwearDevice = make_shared<AudioDeviceDescriptor>();
    unwearDevice->deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    unwearDevice->deviceCategory_ = BT_UNWEAR_HEADPHONE;
    unwearDevice->deviceUsage_ = usage;
    unwearDevice->exceptionFlag_ = false;
    unwearDevice->isEnable_ = true;
    unwearDevice->connectState_ = CONNECTED;
    unwearDevice->connectTimeStamp_ = timestamp;
    return unwearDevice;
}

/**
 * Building unwear Nearlink headphone, which is a negative device and should be filtered.
 */
shared_ptr<AudioDeviceDescriptor> CreateUnwearNearlinkDevice(int32_t timestamp, DeviceUsage usage = MEDIA)
{
    auto unwearDevice = make_shared<AudioDeviceDescriptor>();
    unwearDevice->deviceType_ = DEVICE_TYPE_NEARLINK;
    unwearDevice->deviceCategory_ = BT_UNWEAR_HEADPHONE;
    unwearDevice->deviceUsage_ = usage;
    unwearDevice->exceptionFlag_ = false;
    unwearDevice->isEnable_ = true;
    unwearDevice->connectState_ = CONNECTED;
    unwearDevice->connectTimeStamp_ = timestamp;
    return unwearDevice;
}

/**
 * Building HDMI device, which is a negative device and should NOT be filtered.
 */
shared_ptr<AudioDeviceDescriptor> CreateHdmiDevice(int32_t timestamp, DeviceUsage usage = MEDIA)
{
    auto hdmiDevice = make_shared<AudioDeviceDescriptor>();
    hdmiDevice->deviceType_ = DEVICE_TYPE_HDMI;
    hdmiDevice->deviceCategory_ = CATEGORY_DEFAULT;
    hdmiDevice->deviceUsage_ = usage;
    hdmiDevice->exceptionFlag_ = false;
    hdmiDevice->isEnable_ = true;
    hdmiDevice->connectState_ = CONNECTED;
    hdmiDevice->connectTimeStamp_ = timestamp;
    return hdmiDevice;
}

/**
 * @tc.name  : Test NegativePriorityRouter.
 * @tc.number: GetMediaRenderDevice_001
 * @tc.desc  : No negative device, DEVICE_TYPE_NONE should be returned.
 */
HWTEST(NegativePriorityRouterUnitTest, GetMediaRenderDevice_001, TestSize.Level1)
{
    NegativePriorityRouter negativePriorityRouter;
    StreamUsage streamUsage = STREAM_USAGE_MEDIA;
    int32_t clientUID = TEST_CLIENT_UID;
    uint32_t sessionID = TEST_SESSION_ID;
    auto &manager = AudioDeviceManager::GetAudioDeviceManager();
    manager.mediaRenderNegativeDevices_.clear();
    auto result = negativePriorityRouter.GetMediaRenderDevice(streamUsage, clientUID, sessionID);
    EXPECT_NE(result, nullptr);
    EXPECT_EQ(result->deviceType_, DEVICE_TYPE_NONE);
    manager.mediaRenderNegativeDevices_.clear();
}

/**
 * @tc.name  : Test NegativePriorityRouter.
 * @tc.number: GetMediaRenderDevice_002
 * @tc.desc  : Only one negative device exists, it should be selected as the default device.
 */
HWTEST(NegativePriorityRouterUnitTest, GetMediaRenderDevice_002, TestSize.Level1)
{
    NegativePriorityRouter negativePriorityRouter;
    StreamUsage streamUsage = STREAM_USAGE_MEDIA;
    int32_t clientUID = TEST_CLIENT_UID;
    uint32_t sessionID = TEST_SESSION_ID;
    auto &manager = AudioDeviceManager::GetAudioDeviceManager();
    auto watch = CreateWatchDevice(1000);
    manager.mediaRenderNegativeDevices_.clear();
    manager.mediaRenderNegativeDevices_.push_back(watch);
    auto result = negativePriorityRouter.GetMediaRenderDevice(streamUsage, clientUID, sessionID);
    EXPECT_NE(result, nullptr);
    EXPECT_EQ(result->deviceType_, DEVICE_TYPE_BLUETOOTH_A2DP);
    EXPECT_EQ(result->deviceCategory_, BT_WATCH);
    manager.mediaRenderNegativeDevices_.clear();
}

/**
 * @tc.name  : Test NegativePriorityRouter.
 * @tc.number: GetMediaRenderDevice_003
 * @tc.desc  : Multiple negative devices exist, the latest connected device should be selected.
 */
HWTEST(NegativePriorityRouterUnitTest, GetMediaRenderDevice_003, TestSize.Level1)
{
    NegativePriorityRouter negativePriorityRouter;
    StreamUsage streamUsage = STREAM_USAGE_MEDIA;
    int32_t clientUID = TEST_CLIENT_UID;
    uint32_t sessionID = TEST_SESSION_ID;
    auto &manager = AudioDeviceManager::GetAudioDeviceManager();
    auto watch1 = CreateWatchDevice(1000);
    auto watch2 = CreateWatchDevice(2000);
    auto hdmi = CreateHdmiDevice(3000);
    manager.mediaRenderNegativeDevices_.clear();
    manager.mediaRenderNegativeDevices_.push_back(watch1);
    manager.mediaRenderNegativeDevices_.push_back(watch2);
    manager.mediaRenderNegativeDevices_.push_back(hdmi);
    auto result = negativePriorityRouter.GetMediaRenderDevice(streamUsage, clientUID, sessionID);
    EXPECT_NE(result, nullptr);
    EXPECT_EQ(result->deviceType_, DEVICE_TYPE_HDMI);
    manager.mediaRenderNegativeDevices_.clear();
}

/**
 * @tc.name  : Test NegativePriorityRouter.
 * @tc.number: GetMediaRenderDevice_004
 * @tc.desc  : Only unwear BT headphone exists, it should be filtered and DEVICE_TYPE_NONE returned.
 */
HWTEST(NegativePriorityRouterUnitTest, GetMediaRenderDevice_004, TestSize.Level1)
{
    NegativePriorityRouter negativePriorityRouter;
    StreamUsage streamUsage = STREAM_USAGE_MEDIA;
    int32_t clientUID = TEST_CLIENT_UID;
    uint32_t sessionID = TEST_SESSION_ID;
    auto &manager = AudioDeviceManager::GetAudioDeviceManager();
    auto unwear = CreateUnwearBtHeadphoneDevice(1000);
    manager.mediaRenderNegativeDevices_.clear();
    manager.mediaRenderNegativeDevices_.push_back(unwear);
    auto result = negativePriorityRouter.GetMediaRenderDevice(streamUsage, clientUID, sessionID);
    EXPECT_NE(result, nullptr);
    EXPECT_EQ(result->deviceType_, DEVICE_TYPE_NONE);
    manager.mediaRenderNegativeDevices_.clear();
}

/**
 * @tc.name  : Test NegativePriorityRouter.
 * @tc.number: GetMediaRenderDevice_005
 * @tc.desc  : Unwear BT headphone and unwear Nearlink are filtered, watch is selected.
 */
HWTEST(NegativePriorityRouterUnitTest, GetMediaRenderDevice_005, TestSize.Level1)
{
    NegativePriorityRouter negativePriorityRouter;
    StreamUsage streamUsage = STREAM_USAGE_MEDIA;
    int32_t clientUID = TEST_CLIENT_UID;
    uint32_t sessionID = TEST_SESSION_ID;
    auto &manager = AudioDeviceManager::GetAudioDeviceManager();
    auto unwearBt = CreateUnwearBtHeadphoneDevice(1000);
    auto unwearNearlink = CreateUnwearNearlinkDevice(2000);
    auto watch = CreateWatchDevice(3000);
    manager.mediaRenderNegativeDevices_.clear();
    manager.mediaRenderNegativeDevices_.push_back(unwearBt);
    manager.mediaRenderNegativeDevices_.push_back(unwearNearlink);
    manager.mediaRenderNegativeDevices_.push_back(watch);
    auto result = negativePriorityRouter.GetMediaRenderDevice(streamUsage, clientUID, sessionID);
    EXPECT_NE(result, nullptr);
    EXPECT_EQ(result->deviceCategory_, BT_WATCH);
    manager.mediaRenderNegativeDevices_.clear();
}

/**
 * @tc.name  : Test NegativePriorityRouter.
 * @tc.number: GetCallRenderDevice_001
 * @tc.desc  : No negative device, DEVICE_TYPE_NONE should be returned.
 */
HWTEST(NegativePriorityRouterUnitTest, GetCallRenderDevice_001, TestSize.Level1)
{
    NegativePriorityRouter negativePriorityRouter;
    StreamUsage streamUsage = STREAM_USAGE_VOICE_MODEM_COMMUNICATION;
    int32_t clientUID = TEST_CLIENT_UID;
    auto &manager = AudioDeviceManager::GetAudioDeviceManager();
    manager.commRenderNegativeDevices_.clear();
    auto result = negativePriorityRouter.GetCallRenderDevice(streamUsage, clientUID);
    EXPECT_NE(result, nullptr);
    EXPECT_EQ(result->deviceType_, DEVICE_TYPE_NONE);
    manager.commRenderNegativeDevices_.clear();
}

/**
 * @tc.name  : Test NegativePriorityRouter.
 * @tc.number: GetCallRenderDevice_002
 * @tc.desc  : Only one negative device exists, it should be selected.
 */
HWTEST(NegativePriorityRouterUnitTest, GetCallRenderDevice_002, TestSize.Level1)
{
    NegativePriorityRouter negativePriorityRouter;
    StreamUsage streamUsage = STREAM_USAGE_VOICE_MODEM_COMMUNICATION;
    int32_t clientUID = TEST_CLIENT_UID;
    auto &manager = AudioDeviceManager::GetAudioDeviceManager();
    auto watch = CreateWatchDevice(1000, VOICE);
    manager.commRenderNegativeDevices_.clear();
    manager.commRenderNegativeDevices_.push_back(watch);
    auto result = negativePriorityRouter.GetCallRenderDevice(streamUsage, clientUID);
    EXPECT_NE(result, nullptr);
    EXPECT_EQ(result->deviceType_, DEVICE_TYPE_BLUETOOTH_A2DP);
    manager.commRenderNegativeDevices_.clear();
}

/**
 * @tc.name  : Test NegativePriorityRouter.
 * @tc.number: GetCallRenderDevice_003
 * @tc.desc  : Multiple negative devices exist, the latest connected device should be selected.
 */
HWTEST(NegativePriorityRouterUnitTest, GetCallRenderDevice_003, TestSize.Level1)
{
    NegativePriorityRouter negativePriorityRouter;
    StreamUsage streamUsage = STREAM_USAGE_VOICE_MODEM_COMMUNICATION;
    int32_t clientUID = TEST_CLIENT_UID;
    auto &manager = AudioDeviceManager::GetAudioDeviceManager();
    auto watch1 = CreateWatchDevice(1000, VOICE);
    auto watch2 = CreateWatchDevice(2000, VOICE);
    manager.commRenderNegativeDevices_.clear();
    manager.commRenderNegativeDevices_.push_back(watch1);
    manager.commRenderNegativeDevices_.push_back(watch2);
    auto result = negativePriorityRouter.GetCallRenderDevice(streamUsage, clientUID);
    EXPECT_NE(result, nullptr);
    EXPECT_EQ(result->connectTimeStamp_, 2000);
    manager.commRenderNegativeDevices_.clear();
}

/**
 * @tc.name  : Test NegativePriorityRouter.
 * @tc.number: GetCallRenderDevice_004
 * @tc.desc  : Unwear BT headphone is filtered, DEVICE_TYPE_NONE should be returned.
 */
HWTEST(NegativePriorityRouterUnitTest, GetCallRenderDevice_004, TestSize.Level1)
{
    NegativePriorityRouter negativePriorityRouter;
    StreamUsage streamUsage = STREAM_USAGE_VOICE_MODEM_COMMUNICATION;
    int32_t clientUID = TEST_CLIENT_UID;
    auto &manager = AudioDeviceManager::GetAudioDeviceManager();
    auto unwear = CreateUnwearBtHeadphoneDevice(1000, VOICE);
    manager.commRenderNegativeDevices_.clear();
    manager.commRenderNegativeDevices_.push_back(unwear);
    auto result = negativePriorityRouter.GetCallRenderDevice(streamUsage, clientUID);
    EXPECT_NE(result, nullptr);
    EXPECT_EQ(result->deviceType_, DEVICE_TYPE_NONE);
    manager.commRenderNegativeDevices_.clear();
}

/**
 * @tc.name  : Test NegativePriorityRouter.
 * @tc.number: GetCallCaptureDevice_001
 * @tc.desc  : No negative device, DEVICE_TYPE_NONE should be returned.
 */
HWTEST(NegativePriorityRouterUnitTest, GetCallCaptureDevice_001, TestSize.Level1)
{
    NegativePriorityRouter negativePriorityRouter;
    SourceType sourceType = SOURCE_TYPE_MIC;
    int32_t clientUID = TEST_CLIENT_UID;
    uint32_t sessionID = TEST_SESSION_ID;
    auto &manager = AudioDeviceManager::GetAudioDeviceManager();
    manager.commCaptureNegativeDevices_.clear();
    auto result = negativePriorityRouter.GetCallCaptureDevice(sourceType, clientUID, sessionID);
    EXPECT_NE(result, nullptr);
    EXPECT_EQ(result->deviceType_, DEVICE_TYPE_NONE);
    manager.commCaptureNegativeDevices_.clear();
}

/**
 * @tc.name  : Test NegativePriorityRouter.
 * @tc.number: GetCallCaptureDevice_002
 * @tc.desc  : Only one negative device exists, it should be selected.
 */
HWTEST(NegativePriorityRouterUnitTest, GetCallCaptureDevice_002, TestSize.Level1)
{
    NegativePriorityRouter negativePriorityRouter;
    SourceType sourceType = SOURCE_TYPE_MIC;
    int32_t clientUID = TEST_CLIENT_UID;
    uint32_t sessionID = TEST_SESSION_ID;
    auto &manager = AudioDeviceManager::GetAudioDeviceManager();
    auto watch = CreateWatchDevice(1000, VOICE);
    manager.commCaptureNegativeDevices_.clear();
    manager.commCaptureNegativeDevices_.push_back(watch);
    auto result = negativePriorityRouter.GetCallCaptureDevice(sourceType, clientUID, sessionID);
    EXPECT_NE(result, nullptr);
    EXPECT_EQ(result->deviceType_, DEVICE_TYPE_BLUETOOTH_A2DP);
    manager.commCaptureNegativeDevices_.clear();
}

/**
 * @tc.name  : Test NegativePriorityRouter.
 * @tc.number: GetRecordCaptureDevice_001
 * @tc.desc  : No negative device, DEVICE_TYPE_NONE should be returned.
 */
HWTEST(NegativePriorityRouterUnitTest, GetRecordCaptureDevice_001, TestSize.Level1)
{
    NegativePriorityRouter negativePriorityRouter;
    SourceType sourceType = SOURCE_TYPE_MIC;
    int32_t clientUID = TEST_CLIENT_UID;
    uint32_t sessionID = TEST_SESSION_ID;
    auto &manager = AudioDeviceManager::GetAudioDeviceManager();
    manager.mediaCaptureNegativeDevices_.clear();
    auto result = negativePriorityRouter.GetRecordCaptureDevice(sourceType, clientUID, sessionID);
    EXPECT_NE(result, nullptr);
    EXPECT_EQ(result->deviceType_, DEVICE_TYPE_NONE);
    manager.mediaCaptureNegativeDevices_.clear();
}

/**
 * @tc.name  : Test NegativePriorityRouter.
 * @tc.number: GetRecordCaptureDevice_002
 * @tc.desc  : Only one negative device exists, it should be selected.
 */
HWTEST(NegativePriorityRouterUnitTest, GetRecordCaptureDevice_002, TestSize.Level1)
{
    NegativePriorityRouter negativePriorityRouter;
    SourceType sourceType = SOURCE_TYPE_MIC;
    int32_t clientUID = TEST_CLIENT_UID;
    uint32_t sessionID = TEST_SESSION_ID;
    auto &manager = AudioDeviceManager::GetAudioDeviceManager();
    auto watch = CreateWatchDevice(1000, MEDIA);
    manager.mediaCaptureNegativeDevices_.clear();
    manager.mediaCaptureNegativeDevices_.push_back(watch);
    auto result = negativePriorityRouter.GetRecordCaptureDevice(sourceType, clientUID, sessionID);
    EXPECT_NE(result, nullptr);
    EXPECT_EQ(result->deviceType_, DEVICE_TYPE_BLUETOOTH_A2DP);
    manager.mediaCaptureNegativeDevices_.clear();
}

/**
 * @tc.name  : Test NegativePriorityRouter.
 * @tc.number: GetRecordCaptureDevice_003
 * @tc.desc  : Unwear BT headphone is filtered, DEVICE_TYPE_NONE should be returned.
 */
HWTEST(NegativePriorityRouterUnitTest, GetRecordCaptureDevice_003, TestSize.Level1)
{
    NegativePriorityRouter negativePriorityRouter;
    SourceType sourceType = SOURCE_TYPE_MIC;
    int32_t clientUID = TEST_CLIENT_UID;
    uint32_t sessionID = TEST_SESSION_ID;
    auto &manager = AudioDeviceManager::GetAudioDeviceManager();
    auto unwear = CreateUnwearBtHeadphoneDevice(1000, MEDIA);
    manager.mediaCaptureNegativeDevices_.clear();
    manager.mediaCaptureNegativeDevices_.push_back(unwear);
    auto result = negativePriorityRouter.GetRecordCaptureDevice(sourceType, clientUID, sessionID);
    EXPECT_NE(result, nullptr);
    EXPECT_EQ(result->deviceType_, DEVICE_TYPE_NONE);
    manager.mediaCaptureNegativeDevices_.clear();
}

/**
 * @tc.name  : Test NegativePriorityRouter.
 * @tc.number: GetRingRenderDevices_001
 * @tc.desc  : No negative device, one empty device should be returned.
 */
HWTEST(NegativePriorityRouterUnitTest, GetRingRenderDevices_001, TestSize.Level1)
{
    NegativePriorityRouter negativePriorityRouter;
    auto &manager = AudioDeviceManager::GetAudioDeviceManager();
    manager.commRenderNegativeDevices_.clear();
    manager.mediaRenderNegativeDevices_.clear();
    EXPECT_EQ(1, negativePriorityRouter.GetRingRenderDevices(STREAM_USAGE_VOICE_RINGTONE, 1).size());
    EXPECT_EQ(1, negativePriorityRouter.GetRingRenderDevices(STREAM_USAGE_RINGTONE, 1).size());
    EXPECT_EQ(1, negativePriorityRouter.GetRingRenderDevices(STREAM_USAGE_ALARM, 1).size());
    manager.commRenderNegativeDevices_.clear();
    manager.mediaRenderNegativeDevices_.clear();
}

/**
 * @tc.name  : Test NegativePriorityRouter.
 * @tc.number: GetRingRenderDevices_002
 * @tc.desc  : Voice ringtone, comm negative watch available, should select the watch.
 */
HWTEST(NegativePriorityRouterUnitTest, GetRingRenderDevices_002, TestSize.Level1)
{
    NegativePriorityRouter negativePriorityRouter;
    negativePriorityRouter.audioPolicyManager_.SetRingerMode(RINGER_MODE_NORMAL);
    auto &manager = AudioDeviceManager::GetAudioDeviceManager();
    auto watch = CreateWatchDevice(1000, VOICE);
    manager.commRenderNegativeDevices_.clear();
    manager.commRenderNegativeDevices_.push_back(watch);
    auto descs = negativePriorityRouter.GetRingRenderDevices(STREAM_USAGE_VOICE_RINGTONE, TEST_CLIENT_UID);
    EXPECT_EQ(descs.size(), 2);
    EXPECT_EQ(descs[0]->deviceType_, DEVICE_TYPE_BLUETOOTH_A2DP);
    manager.commRenderNegativeDevices_.clear();
}

/**
 * @tc.name  : Test NegativePriorityRouter.
 * @tc.number: GetRingRenderDevices_003
 * @tc.desc  : Voice ringtone, unwear BT headphone is filtered, one empty device returned.
 */
HWTEST(NegativePriorityRouterUnitTest, GetRingRenderDevices_003, TestSize.Level1)
{
    NegativePriorityRouter negativePriorityRouter;
    negativePriorityRouter.audioPolicyManager_.SetRingerMode(RINGER_MODE_NORMAL);
    auto &manager = AudioDeviceManager::GetAudioDeviceManager();
    auto unwear = CreateUnwearBtHeadphoneDevice(1000, VOICE);
    manager.commRenderNegativeDevices_.clear();
    manager.commRenderNegativeDevices_.push_back(unwear);
    auto descs = negativePriorityRouter.GetRingRenderDevices(STREAM_USAGE_VOICE_RINGTONE, TEST_CLIENT_UID);
    EXPECT_EQ(descs.size(), 1);
    EXPECT_EQ(descs[0]->deviceType_, DEVICE_TYPE_NONE);
    manager.commRenderNegativeDevices_.clear();
}

/**
 * @tc.name  : Test NegativePriorityRouter.
 * @tc.number: GetToneRenderDevice_001
 * @tc.desc  : Test GetToneRenderDevice interface.
 */
HWTEST(NegativePriorityRouterUnitTest, GetToneRenderDevice_001, TestSize.Level4)
{
    NegativePriorityRouter negativePriorityRouter;
    StreamUsage streamUsage = STREAM_USAGE_MEDIA;
    int32_t clientUID = TEST_CLIENT_UID;
    auto result = negativePriorityRouter.GetToneRenderDevice(streamUsage, clientUID);
    EXPECT_NE(result, nullptr);
    EXPECT_EQ(result->deviceType_, DEVICE_TYPE_NONE);
}

/**
 * @tc.name  : Test FilterUnwearHeadphones.
 * @tc.number: FilterUnwearHeadphones_001
 * @tc.desc  : Unwear BT headphone and unwear Nearlink are removed, watch is kept.
 */
HWTEST(NegativePriorityRouterUnitTest, FilterUnwearHeadphones_001, TestSize.Level1)
{
    NegativePriorityRouter negativePriorityRouter;
    vector<shared_ptr<AudioDeviceDescriptor>> descs;
    auto unwearBt = CreateUnwearBtHeadphoneDevice(1000);
    auto unwearNearlink = CreateUnwearNearlinkDevice(2000);
    auto watch = CreateWatchDevice(3000);
    descs.push_back(unwearBt);
    descs.push_back(unwearNearlink);
    descs.push_back(watch);
    negativePriorityRouter.FilterUnwearHeadphones(descs);
    EXPECT_EQ(descs.size(), 1);
    EXPECT_EQ(descs[0]->deviceCategory_, BT_WATCH);
}

/**
 * @tc.name  : Test FilterUnwearHeadphones.
 * @tc.number: FilterUnwearHeadphones_002
 * @tc.desc  : Only unwear headphones exist, all should be removed.
 */
HWTEST(NegativePriorityRouterUnitTest, FilterUnwearHeadphones_002, TestSize.Level1)
{
    NegativePriorityRouter negativePriorityRouter;
    vector<shared_ptr<AudioDeviceDescriptor>> descs;
    auto unwearBt = CreateUnwearBtHeadphoneDevice(1000);
    auto unwearNearlink = CreateUnwearNearlinkDevice(2000);
    descs.push_back(unwearBt);
    descs.push_back(unwearNearlink);
    negativePriorityRouter.FilterUnwearHeadphones(descs);
    EXPECT_EQ(descs.size(), 0);
}

/**
 * @tc.name  : Test NegativePriorityRouter GetClassName.
 * @tc.number: NegativePriorityRouter_001
 * @tc.desc  : Test GetClassName and GetRouterType interface.
 */
HWTEST(NegativePriorityRouterUnitTest, NegativePriorityRouter_001, TestSize.Level1)
{
    NegativePriorityRouter negativePriorityRouter;
    EXPECT_EQ(negativePriorityRouter.GetClassName(), "negative_priority_router");
    EXPECT_EQ(negativePriorityRouter.GetRouterType(), ROUTER_TYPE_NEGATIVE_PRIORITY);
}
} // namespace AudioStandard
} // namespace OHOS
