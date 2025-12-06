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

#include "privacy_priority_router_unit_test.h"
#include "audio_errors.h"
#include "audio_policy_log.h"

#include <thread>
#include <memory>
#include <vector>
using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

const static int32_t TEST_CLIENT_UID = 1;
const static uint32_t TEST_SESSION_ID = 1;

void PrivacyPriorityRouterUnitTest::SetUpTestCase(void) {}
void PrivacyPriorityRouterUnitTest::TearDownTestCase(void) {}
void PrivacyPriorityRouterUnitTest::SetUp(void) {}
void PrivacyPriorityRouterUnitTest::TearDown(void) {}

/**
 * @tc.name  : Test PrivacyPriorityRouter.
 * @tc.number: GetRecordCaptureDevice_002
 * @tc.desc  : Test GetRecordCaptureDevice interface.
 */
HWTEST(PrivacyPriorityRouterUnitTest, GetRecordCaptureDevice_002, TestSize.Level1)
{
    PrivacyPriorityRouter privacyPriorityRouter;
    SourceType sourceType = SOURCE_TYPE_VOICE_RECOGNITION;
    int32_t clientUID = TEST_CLIENT_UID;
    uint32_t sessionID = TEST_SESSION_ID;
    auto result = privacyPriorityRouter.GetRecordCaptureDevice(sourceType, clientUID, sessionID);
    EXPECT_NE(result, nullptr);
}

/**
 * @tc.name  : Test PrivacyPriorityRouter.
 * @tc.number: GetRecordCaptureDevice_003
 * @tc.desc  : Test GetRecordCaptureDevice interface.
 */
HWTEST(PrivacyPriorityRouterUnitTest, GetRecordCaptureDevice_003, TestSize.Level1)
{
    PrivacyPriorityRouter privacyPriorityRouter;
    SourceType sourceType = SOURCE_TYPE_INVALID;
    int32_t clientUID = TEST_CLIENT_UID;
    uint32_t sessionID = TEST_SESSION_ID;
    auto result = privacyPriorityRouter.GetRecordCaptureDevice(sourceType, clientUID, sessionID);
    EXPECT_NE(result, nullptr);
}

/**
 * @tc.name  : Test PrivacyPriorityRouter.
 * @tc.number: GetRecordCaptureDevice_004
 * @tc.desc  : Test GetRecordCaptureDevice interface.
 */
HWTEST(PrivacyPriorityRouterUnitTest, GetRecordCaptureDevice_004, TestSize.Level1)
{
    PrivacyPriorityRouter privacyPriorityRouter;
    SourceType sourceType = SOURCE_TYPE_INVALID;
    int32_t clientUID = TEST_CLIENT_UID;
    uint32_t sessionID = TEST_SESSION_ID;
    shared_ptr<AudioDeviceDescriptor> desc = make_shared<AudioDeviceDescriptor>();
    desc->deviceType_ = DEVICE_TYPE_BLUETOOTH_SCO;
    auto result = privacyPriorityRouter.GetRecordCaptureDevice(sourceType, clientUID, sessionID);
    EXPECT_NE(result->deviceType_, desc->deviceType_);
}

/**
 * Building SCO
 */
shared_ptr<AudioDeviceDescriptor> CreateScoDevice(int32_t timestamp, bool isSuspended = false)
{
    auto scoDevice = make_shared<AudioDeviceDescriptor>();
    scoDevice->deviceType_ = DEVICE_TYPE_BLUETOOTH_SCO;
    scoDevice->exceptionFlag_ = false;
    scoDevice->isEnable_ = true;
    scoDevice->connectState_ = isSuspended ? SUSPEND_CONNECTED : CONNECTED;
    scoDevice->connectTimeStamp_ = timestamp;
    return scoDevice;
}

/**
 * Building a2dp_in
 */
shared_ptr<AudioDeviceDescriptor> CreateA2dpInDevice(int32_t timestamp)
{
    auto a2dpInDevice = make_shared<AudioDeviceDescriptor>();
    a2dpInDevice->deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP_IN;
    a2dpInDevice->exceptionFlag_ = false;
    a2dpInDevice->isEnable_ = true;
    a2dpInDevice->connectState_ = CONNECTED;
    a2dpInDevice->connectTimeStamp_ = timestamp;
    return a2dpInDevice;
}

/**
 * Building WiredHeadset
 */
shared_ptr<AudioDeviceDescriptor> CreateWiredHeadsetDevice(int32_t timestamp)
{
    auto wiredDevice = make_shared<AudioDeviceDescriptor>();
    wiredDevice->deviceType_ = DEVICE_TYPE_WIRED_HEADSET;
    wiredDevice->exceptionFlag_ = false;
    wiredDevice->isEnable_ = true;
    wiredDevice->connectState_ = CONNECTED;
    wiredDevice->connectTimeStamp_ = timestamp;
    return wiredDevice;
}

/**
 * @tc.name  : Test PrivacyPriorityRouter.
 * @tc.number: GetRecordCaptureDevice_005
 * @tc.desc  : If SCO and A2DP_IN are available, SCO should be selected.
 */
HWTEST(PrivacyPriorityRouterUnitTest, GetRecordCaptureDevice_005, TestSize.Level1)
{
    PrivacyPriorityRouter privacyPriorityRouter;
    int32_t clientUID = TEST_CLIENT_UID;
    uint32_t sessionID = TEST_SESSION_ID;

    auto &manager = AudioDeviceManager::GetAudioDeviceManager();
    auto sco = CreateScoDevice(100);
    auto a2dpin = CreateA2dpInDevice(200);
    manager.reconCapturePrivacyDevices_.clear();
    manager.mediaCapturePrivacyDevices_.clear();
    manager.reconCapturePrivacyDevices_.push_back(sco);
    manager.mediaCapturePrivacyDevices_.push_back(a2dpin);
    auto result = privacyPriorityRouter.GetRecordCaptureDevice(SOURCE_TYPE_VOICE_RECOGNITION, clientUID, sessionID);
    EXPECT_NE(result, nullptr);
    EXPECT_EQ(result->deviceType_, DEVICE_TYPE_BLUETOOTH_SCO);
    manager.reconCapturePrivacyDevices_.clear();
    manager.mediaCapturePrivacyDevices_.clear();
}

/**
 * @tc.name  : Test PrivacyPriorityRouter.
 * @tc.number: GetRecordCaptureDevice_006
 * @tc.desc  : Only A2DP_IN, recognition source should return NONE (because SCO is suspend).
 */
HWTEST(PrivacyPriorityRouterUnitTest, GetRecordCaptureDevice_006, TestSize.Level1)
{
    PrivacyPriorityRouter privacyPriorityRouter;
    int32_t clientUID = TEST_CLIENT_UID;
    uint32_t sessionID = TEST_SESSION_ID;

    auto &manager = AudioDeviceManager::GetAudioDeviceManager();
    auto a2dpin = CreateA2dpInDevice(200);
    manager.reconCapturePrivacyDevices_.clear();
    manager.mediaCapturePrivacyDevices_.clear();
    manager.mediaCapturePrivacyDevices_.push_back(a2dpin);
    auto result = privacyPriorityRouter.GetRecordCaptureDevice(SOURCE_TYPE_VOICE_RECOGNITION, clientUID, sessionID);
    EXPECT_NE(result, nullptr);
    EXPECT_EQ(result->deviceType_, DEVICE_TYPE_NONE);
    manager.reconCapturePrivacyDevices_.clear();
    manager.mediaCapturePrivacyDevices_.clear();
}

/**
 * @tc.name  : Test PrivacyPriorityRouter.
 * @tc.number: GetRecordCaptureDevice_007
 * @tc.desc  : Normal recording source microphone, A2DP_IN is available, and A2DP_IN should be selected.
 */
HWTEST(PrivacyPriorityRouterUnitTest, GetRecordCaptureDevice_007, TestSize.Level1)
{
    PrivacyPriorityRouter privacyPriorityRouter;
    int32_t clientUID = TEST_CLIENT_UID;
    uint32_t sessionID = TEST_SESSION_ID;

    auto &manager = AudioDeviceManager::GetAudioDeviceManager();
    auto a2dpin = CreateA2dpInDevice(1000);
    manager.reconCapturePrivacyDevices_.clear();
    manager.mediaCapturePrivacyDevices_.clear();
    manager.mediaCapturePrivacyDevices_.push_back(a2dpin);
    auto result = privacyPriorityRouter.GetRecordCaptureDevice(SOURCE_TYPE_MIC, clientUID, sessionID);
    EXPECT_NE(result, nullptr);
    EXPECT_EQ(result->deviceType_, DEVICE_TYPE_BLUETOOTH_A2DP_IN);
    manager.reconCapturePrivacyDevices_.clear();
    manager.mediaCapturePrivacyDevices_.clear();
}

/**
 * @tc.name  : Test PrivacyPriorityRouter.
 * @tc.number: GetRecordCaptureDevice_008
 * @tc.desc  : No equipment, None should be returned.
 */
HWTEST(PrivacyPriorityRouterUnitTest, GetRecordCaptureDevice_008, TestSize.Level1)
{
    PrivacyPriorityRouter privacyPriorityRouter;
    int32_t clientUID = TEST_CLIENT_UID;
    uint32_t sessionID = TEST_SESSION_ID;

    auto &manager = AudioDeviceManager::GetAudioDeviceManager();
    manager.reconCapturePrivacyDevices_.clear();
    manager.mediaCapturePrivacyDevices_.clear();
    auto result = privacyPriorityRouter.GetRecordCaptureDevice(SOURCE_TYPE_MIC, clientUID, sessionID);
    EXPECT_NE(result, nullptr);
    EXPECT_EQ(result->deviceType_, DEVICE_TYPE_NONE);
    manager.reconCapturePrivacyDevices_.clear();
    manager.mediaCapturePrivacyDevices_.clear();
}

/**
 * @tc.name  : Test PrivacyPriorityRouter.
 * @tc.number: GetRecordCaptureDevice_009
 * @tc.desc  : Enable Bluetooth SCO, Connect Bluetooth and then WiredHeadset, Select WiredHeadset.
 */
HWTEST(PrivacyPriorityRouterUnitTest, GetRecordCaptureDevice_009, TestSize.Level1)
{
    PrivacyPriorityRouter privacyPriorityRouter;
    int32_t clientUID = TEST_CLIENT_UID;
    uint32_t sessionID = TEST_SESSION_ID;

    auto &manager = AudioDeviceManager::GetAudioDeviceManager();
    auto sco = CreateScoDevice(1000);
    auto wired = CreateWiredHeadsetDevice(2000);
    manager.reconCapturePrivacyDevices_.clear();
    manager.mediaCapturePrivacyDevices_.clear();
    manager.reconCapturePrivacyDevices_.push_back(sco);
    manager.reconCapturePrivacyDevices_.push_back(wired);
    AudioPolicyUtils::GetInstance().SetScoExcluded(false);
    auto result = privacyPriorityRouter.GetRecordCaptureDevice(SOURCE_TYPE_VOICE_RECOGNITION, clientUID, sessionID);
    EXPECT_NE(result, nullptr);
    EXPECT_EQ(result->deviceType_, DEVICE_TYPE_WIRED_HEADSET);
    manager.reconCapturePrivacyDevices_.clear();
    manager.mediaCapturePrivacyDevices_.clear();
}

/**
 * @tc.name  : Test PrivacyPriorityRouter.
 * @tc.number: GetRecordCaptureDevice_010
 * @tc.desc  : If Bluetooth SCO is enabled, connect WiredHeadset and then connect SCO, SELECT SCO.
 */
HWTEST(PrivacyPriorityRouterUnitTest, GetRecordCaptureDevice_010, TestSize.Level1)
{
    PrivacyPriorityRouter privacyPriorityRouter;
    int32_t clientUID = TEST_CLIENT_UID;
    uint32_t sessionID = TEST_SESSION_ID;

    auto &manager = AudioDeviceManager::GetAudioDeviceManager();
    auto wired = CreateWiredHeadsetDevice(1000);
    auto sco = CreateScoDevice(2000);
    manager.reconCapturePrivacyDevices_.clear();
    manager.reconCapturePrivacyDevices_.push_back(wired);
    manager.reconCapturePrivacyDevices_.push_back(sco);
    AudioPolicyUtils::GetInstance().SetScoExcluded(false);
    auto result = privacyPriorityRouter.GetRecordCaptureDevice(SOURCE_TYPE_VOICE_RECOGNITION, clientUID, sessionID);
    EXPECT_NE(result, nullptr);
    EXPECT_EQ(result->deviceType_, DEVICE_TYPE_BLUETOOTH_SCO);
    manager.reconCapturePrivacyDevices_.clear();
}

/**
 * @tc.name  : Test PrivacyPriorityRouter.
 * @tc.number: GetRecordCaptureDevice_011
 * @tc.desc  : Bluetooth SCO is disabled (suspended), connect to SCO first, select WiredHeadset.
 */
HWTEST(PrivacyPriorityRouterUnitTest, GetRecordCaptureDevice_011, TestSize.Level1)
{
    PrivacyPriorityRouter privacyPriorityRouter;
    int32_t clientUID = TEST_CLIENT_UID;
    uint32_t sessionID = TEST_SESSION_ID;

    auto &manager = AudioDeviceManager::GetAudioDeviceManager();
    auto sco = CreateScoDevice(1000, true);
    auto a2dpin = CreateA2dpInDevice(1000);
    auto wired = CreateWiredHeadsetDevice(2000);
    manager.reconCapturePrivacyDevices_.clear();
    manager.mediaCapturePrivacyDevices_.clear();
    manager.mediaCapturePrivacyDevices_.push_back(a2dpin);
    manager.reconCapturePrivacyDevices_.push_back(wired);
    AudioPolicyUtils::GetInstance().SetScoExcluded(true);
    auto result = privacyPriorityRouter.GetRecordCaptureDevice(SOURCE_TYPE_VOICE_RECOGNITION, clientUID, sessionID);
    EXPECT_NE(result, nullptr);
    EXPECT_EQ(result->deviceType_, DEVICE_TYPE_WIRED_HEADSET);
    manager.reconCapturePrivacyDevices_.clear();
    manager.mediaCapturePrivacyDevices_.clear();
}

/**
 * @tc.name  : Test PrivacyPriorityRouter.
 * @tc.number: GetRecordCaptureDevice_012
 * @tc.desc  : Bluetooth SCO is disabled. Connect the WiredHeadset and then connect the SCO, select WiredHeadset.
 */
HWTEST(PrivacyPriorityRouterUnitTest, GetRecordCaptureDevice_012, TestSize.Level1)
{
    PrivacyPriorityRouter privacyPriorityRouter;
    int32_t clientUID = TEST_CLIENT_UID;
    uint32_t sessionID = TEST_SESSION_ID;

    auto &manager = AudioDeviceManager::GetAudioDeviceManager();
    auto wired = CreateWiredHeadsetDevice(1000);
    auto a2dpin = CreateA2dpInDevice(1000);
    auto sco = CreateScoDevice(2000, true);
    manager.reconCapturePrivacyDevices_.clear();
    manager.mediaCapturePrivacyDevices_.clear();
    manager.reconCapturePrivacyDevices_.push_back(wired);
    manager.mediaCapturePrivacyDevices_.push_back(a2dpin);
    AudioPolicyUtils::GetInstance().SetScoExcluded(true);
    auto result = privacyPriorityRouter.GetRecordCaptureDevice(SOURCE_TYPE_VOICE_RECOGNITION, clientUID, sessionID);
    EXPECT_NE(result, nullptr);
    EXPECT_EQ(result->deviceType_, DEVICE_TYPE_WIRED_HEADSET);
    manager.reconCapturePrivacyDevices_.clear();
    manager.mediaCapturePrivacyDevices_.clear();
}

/**
 * @tc.name  : Test PrivacyPriorityRouter.
 * @tc.number: GetRecordCaptureDevice_013
 * @tc.desc  : If SCO and A2DP_IN are available, SCO should be selected.
 */
HWTEST(PrivacyPriorityRouterUnitTest, GetRecordCaptureDevice_013, TestSize.Level1)
{
    PrivacyPriorityRouter privacyPriorityRouter;
    int32_t clientUID = TEST_CLIENT_UID;
    uint32_t sessionID = TEST_SESSION_ID;

    auto &manager = AudioDeviceManager::GetAudioDeviceManager();
    auto sco = CreateScoDevice(100);
    auto a2dpin = CreateA2dpInDevice(100);
    manager.reconCapturePrivacyDevices_.clear();
    manager.mediaCapturePrivacyDevices_.clear();
    manager.reconCapturePrivacyDevices_.push_back(sco);
    manager.mediaCapturePrivacyDevices_.push_back(a2dpin);
    AudioPolicyUtils::GetInstance().SetScoExcluded(false);
    auto result = privacyPriorityRouter.GetRecordCaptureDevice(SOURCE_TYPE_VOICE_TRANSCRIPTION, clientUID, sessionID);
    EXPECT_NE(result, nullptr);
    EXPECT_EQ(result->deviceType_, DEVICE_TYPE_BLUETOOTH_SCO);
    manager.reconCapturePrivacyDevices_.clear();
    manager.mediaCapturePrivacyDevices_.clear();
}

/**
 * @tc.name  : Test PrivacyPriorityRouter.
 * @tc.number: GetRecordCaptureDevice_014
 * @tc.desc  : Only A2DP_IN, device select should return NONE (because SCO is suspend).
 */
HWTEST(PrivacyPriorityRouterUnitTest, GetRecordCaptureDevice_014, TestSize.Level1)
{
    PrivacyPriorityRouter privacyPriorityRouter;
    int32_t clientUID = TEST_CLIENT_UID;
    uint32_t sessionID = TEST_SESSION_ID;

    auto &manager = AudioDeviceManager::GetAudioDeviceManager();
    auto a2dpin = CreateA2dpInDevice(200);
    manager.reconCapturePrivacyDevices_.clear();
    manager.mediaCapturePrivacyDevices_.clear();
    manager.mediaCapturePrivacyDevices_.push_back(a2dpin);
    auto result = privacyPriorityRouter.GetRecordCaptureDevice(SOURCE_TYPE_VOICE_TRANSCRIPTION, clientUID, sessionID);
    EXPECT_NE(result, nullptr);
    EXPECT_EQ(result->deviceType_, DEVICE_TYPE_NONE);
    manager.reconCapturePrivacyDevices_.clear();
    manager.mediaCapturePrivacyDevices_.clear();
}

/**
 * @tc.name  : Test PrivacyPriorityRouter.
 * @tc.number: GetRecordCaptureDevice_015
 * @tc.desc  : Enable Bluetooth SCO, Connect Bluetooth and then WiredHeadset, Select WiredHeadset.
 */
HWTEST(PrivacyPriorityRouterUnitTest, GetRecordCaptureDevice_015, TestSize.Level1)
{
    PrivacyPriorityRouter privacyPriorityRouter;
    int32_t clientUID = TEST_CLIENT_UID;
    uint32_t sessionID = TEST_SESSION_ID;

    auto &manager = AudioDeviceManager::GetAudioDeviceManager();
    auto sco = CreateScoDevice(1000);
    auto wired = CreateWiredHeadsetDevice(2000);
    manager.reconCapturePrivacyDevices_.clear();
    manager.mediaCapturePrivacyDevices_.clear();
    manager.reconCapturePrivacyDevices_.push_back(sco);
    manager.reconCapturePrivacyDevices_.push_back(wired);
    AudioPolicyUtils::GetInstance().SetScoExcluded(false);
    auto result = privacyPriorityRouter.GetRecordCaptureDevice(SOURCE_TYPE_VOICE_TRANSCRIPTION, clientUID, sessionID);
    EXPECT_NE(result, nullptr);
    EXPECT_EQ(result->deviceType_, DEVICE_TYPE_WIRED_HEADSET);
    manager.reconCapturePrivacyDevices_.clear();
    manager.mediaCapturePrivacyDevices_.clear();
}

/**
 * @tc.name  : Test PrivacyPriorityRouter.
 * @tc.number: GetRecordCaptureDevice_016
 * @tc.desc  : If Bluetooth SCO is enabled, connect WiredHeadset and then connect SCO, should be selected SCO.
 */
HWTEST(PrivacyPriorityRouterUnitTest, GetRecordCaptureDevice_016, TestSize.Level1)
{
    PrivacyPriorityRouter privacyPriorityRouter;
    int32_t clientUID = TEST_CLIENT_UID;
    uint32_t sessionID = TEST_SESSION_ID;

    auto &manager = AudioDeviceManager::GetAudioDeviceManager();
    auto wired = CreateWiredHeadsetDevice(1000);
    auto sco = CreateScoDevice(2000);
    manager.reconCapturePrivacyDevices_.clear();
    manager.mediaCapturePrivacyDevices_.clear();
    manager.reconCapturePrivacyDevices_.push_back(wired);
    manager.reconCapturePrivacyDevices_.push_back(sco);
    AudioPolicyUtils::GetInstance().SetScoExcluded(false);
    auto result = privacyPriorityRouter.GetRecordCaptureDevice(SOURCE_TYPE_VOICE_TRANSCRIPTION, clientUID, sessionID);
    EXPECT_NE(result, nullptr);
    EXPECT_EQ(result->deviceType_, DEVICE_TYPE_BLUETOOTH_SCO);
    manager.reconCapturePrivacyDevices_.clear();
    manager.mediaCapturePrivacyDevices_.clear();
}

/**
 * @tc.name  : Test PrivacyPriorityRouter.
 * @tc.number: GetRecordCaptureDevice_017
 * @tc.desc  : Bluetooth SCO is disabled, connect SCO first, then connect WiredHeadset, selecte WiredHeadset.
 */
HWTEST(PrivacyPriorityRouterUnitTest, GetRecordCaptureDevice_017, TestSize.Level1)
{
    PrivacyPriorityRouter privacyPriorityRouter;
    int32_t clientUID = TEST_CLIENT_UID;
    uint32_t sessionID = TEST_SESSION_ID;

    auto &manager = AudioDeviceManager::GetAudioDeviceManager();
    auto sco = CreateScoDevice(1000, true);
    auto a2dpin = CreateA2dpInDevice(1000);
    auto wired = CreateWiredHeadsetDevice(2000);
    manager.reconCapturePrivacyDevices_.clear();
    manager.mediaCapturePrivacyDevices_.clear();
    manager.mediaCapturePrivacyDevices_.push_back(a2dpin);
    manager.reconCapturePrivacyDevices_.push_back(wired);
    AudioPolicyUtils::GetInstance().SetScoExcluded(true);
    auto result = privacyPriorityRouter.GetRecordCaptureDevice(SOURCE_TYPE_VOICE_TRANSCRIPTION, clientUID, sessionID);
    EXPECT_NE(result, nullptr);
    EXPECT_EQ(result->deviceType_, DEVICE_TYPE_WIRED_HEADSET);
    manager.reconCapturePrivacyDevices_.clear();
    manager.mediaCapturePrivacyDevices_.clear();
}

/**
 * @tc.name  : Test PrivacyPriorityRouter.
 * @tc.number: GetRecordCaptureDevice_018
 * @tc.desc  : Bluetooth SCO is disabled. Connect the WiredHeadset and then connect the SCO, selecte WiredHeadset.
 */
HWTEST(PrivacyPriorityRouterUnitTest, GetRecordCaptureDevice_018, TestSize.Level1)
{
    PrivacyPriorityRouter privacyPriorityRouter;
    int32_t clientUID = TEST_CLIENT_UID;
    uint32_t sessionID = TEST_SESSION_ID;

    auto &manager = AudioDeviceManager::GetAudioDeviceManager();
    auto wired = CreateWiredHeadsetDevice(1000);
    auto a2dpin = CreateA2dpInDevice(1000);
    auto sco = CreateScoDevice(2000, true);
    manager.reconCapturePrivacyDevices_.clear();
    manager.mediaCapturePrivacyDevices_.clear();
    manager.reconCapturePrivacyDevices_.push_back(wired);
    manager.mediaCapturePrivacyDevices_.push_back(a2dpin);
    AudioPolicyUtils::GetInstance().SetScoExcluded(true);
    auto result = privacyPriorityRouter.GetRecordCaptureDevice(SOURCE_TYPE_VOICE_TRANSCRIPTION, clientUID, sessionID);
    EXPECT_NE(result, nullptr);
    EXPECT_EQ(result->deviceType_, DEVICE_TYPE_WIRED_HEADSET);
    manager.reconCapturePrivacyDevices_.clear();
    manager.mediaCapturePrivacyDevices_.clear();
}

/**
 * @tc.name  : Test PrivacyPriorityRouter.
 * @tc.number: GetCallRenderDevice_001
 * @tc.desc  : Test GetCallRenderDevice interface.
 */
HWTEST(PrivacyPriorityRouterUnitTest, GetCallRenderDevice_001, TestSize.Level1)
{
    PrivacyPriorityRouter privacyPriorityRouter;
    StreamUsage streamUsage = STREAM_USAGE_VOICE_MODEM_COMMUNICATION;
    int32_t clientUID = TEST_CLIENT_UID;
    auto result = privacyPriorityRouter.GetCallRenderDevice(streamUsage, clientUID);
    EXPECT_NE(result, nullptr);
}

/**
 * @tc.name  : Test PrivacyPriorityRouter.
 * @tc.number: GetCallRenderDevice_002
 * @tc.desc  : Test GetCallRenderDevice interface.
 */
HWTEST(PrivacyPriorityRouterUnitTest, GetCallRenderDevice_002, TestSize.Level1)
{
    PrivacyPriorityRouter privacyPriorityRouter;
    StreamUsage streamUsage = STREAM_USAGE_MEDIA;
    int32_t clientUID = TEST_CLIENT_UID;
    auto result = privacyPriorityRouter.GetCallRenderDevice(streamUsage, clientUID);
    EXPECT_NE(result, nullptr);
}

/**
 * @tc.name  : Test PrivacyPriorityRouter.
 * @tc.number: PrivacyPriorityRouter_001
 * @tc.desc  : Test NeedLatestConnectWithDefaultDevices interface.
 */
HWTEST(PrivacyPriorityRouterUnitTest, PrivacyPriorityRouter_001, TestSize.Level1)
{
    PrivacyPriorityRouter router;

    EXPECT_TRUE(router.NeedLatestConnectWithDefaultDevices(DEVICE_TYPE_WIRED_HEADSET));
    EXPECT_TRUE(router.NeedLatestConnectWithDefaultDevices(DEVICE_TYPE_WIRED_HEADPHONES));
    EXPECT_TRUE(router.NeedLatestConnectWithDefaultDevices(DEVICE_TYPE_BLUETOOTH_SCO));
    EXPECT_TRUE(router.NeedLatestConnectWithDefaultDevices(DEVICE_TYPE_USB_HEADSET));
    EXPECT_TRUE(router.NeedLatestConnectWithDefaultDevices(DEVICE_TYPE_BLUETOOTH_A2DP));
    EXPECT_TRUE(router.NeedLatestConnectWithDefaultDevices(DEVICE_TYPE_USB_ARM_HEADSET));
    EXPECT_FALSE(router.NeedLatestConnectWithDefaultDevices(DEVICE_TYPE_NONE));
}

/**
 * @tc.name  : Test PrivacyPriorityRouter.
 * @tc.number: PrivacyPriorityRouter_002
 * @tc.desc  : Test GetRingRenderDevices interface.
 */
HWTEST(PrivacyPriorityRouterUnitTest, PrivacyPriorityRouter_002, TestSize.Level1)
{
    PrivacyPriorityRouter router;

    EXPECT_EQ(1, router.GetRingRenderDevices(STREAM_USAGE_VOICE_RINGTONE, 1).size());
    EXPECT_EQ(1, router.GetRingRenderDevices(STREAM_USAGE_RINGTONE, 1).size());
    EXPECT_EQ(1, router.GetRingRenderDevices(STREAM_USAGE_ALARM, 1).size());
}

/**
 * @tc.name  : Test PrivacyPriorityRouter.
 * @tc.number: GetMediaRenderDevice_001
 * @tc.desc  : Test GetMediaRenderDevice interface.
 */
HWTEST(PrivacyPriorityRouterUnitTest, GetMediaRenderDevice_001, TestSize.Level4)
{
    PrivacyPriorityRouter router;
    StreamUsage streamUsage = STREAM_USAGE_VOICE_MODEM_COMMUNICATION;
    int32_t clientUID = TEST_CLIENT_UID;
    auto ret = router.GetMediaRenderDevice(streamUsage, clientUID);
    EXPECT_NE(ret, nullptr);
}

/**
 * @tc.name  : Test PrivacyPriorityRouter.
 * @tc.number: GetCallCaptureDevice_001
 * @tc.desc  : Test GetCallCaptureDevice interface.
 */
HWTEST(PrivacyPriorityRouterUnitTest, GetCallCaptureDevice_001, TestSize.Level4)
{
    PrivacyPriorityRouter router;
    SourceType sourceType = SOURCE_TYPE_VOICE_RECOGNITION;
    int32_t clientUID = TEST_CLIENT_UID;
    uint32_t sessionID = TEST_SESSION_ID;
    auto ret = router.GetCallCaptureDevice(sourceType, clientUID, sessionID);
    EXPECT_NE(ret, nullptr);
}

/**
 * @tc.name  : Test PrivacyPriorityRouter.
 * @tc.number: GetToneRenderDevice_001
 * @tc.desc  : Test GetToneRenderDevice interface.
 */
HWTEST(PrivacyPriorityRouterUnitTest, GetToneRenderDevice_001, TestSize.Level4)
{
    PrivacyPriorityRouter router;
    StreamUsage streamUsage = STREAM_USAGE_VOICE_MODEM_COMMUNICATION;
    int32_t clientUID = TEST_CLIENT_UID;
    auto ret = router.GetToneRenderDevice(streamUsage, clientUID);
    EXPECT_NE(ret, nullptr);
}

/**
 * @tc.name  : Test PrivacyPriorityRouter.
 * @tc.number: RemoveArmUsb_001
 * @tc.desc  : Test RemoveArmUsb interface.
 */
HWTEST(PrivacyPriorityRouterUnitTest, RemoveArmUsb_001, TestSize.Level4)
{
    PrivacyPriorityRouter router;
    vector<shared_ptr<AudioDeviceDescriptor>> descs;
    shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>();
    desc->deviceType_ = DEVICE_TYPE_USB_ARM_HEADSET;
    descs.emplace_back(desc);
    router.RemoveArmUsb(descs);
    EXPECT_EQ(descs.size(), 0);
}

/**
 * @tc.name  : Test IsA2dpDisable.
 * @tc.number: IsA2dpDisable_001
 * @tc.desc  : Test RemoveArmUsb interface.
 */
HWTEST(PrivacyPriorityRouterUnitTest, IsA2dpDisable_001, TestSize.Level4)
{
    PrivacyPriorityRouter router;
    auto &audioDeviceManager = AudioDeviceManager::GetAudioDeviceManager();
    audioDeviceManager.connectedDevices_.clear();
    shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>();
    desc->deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    desc->deviceRole_ = OUTPUT_DEVICE;
    desc->networkId_ = "";
    desc->macAddress_ = "00:11:22:33:44:55";
    desc->connectState_ = CONNECTED;
    desc->isEnable_ = false;
    audioDeviceManager.connectedDevices_.push_back(desc);
    bool isA2dpDisable = router.IsA2dpDisable(desc);
    EXPECT_EQ(isA2dpDisable, true);
}
 
/**
 * @tc.name  : Test PrivacyPriorityRouter.
 * @tc.number: GetRecordCaptureDevice_019
 * @tc.desc  : Test GetRecordCaptureDevice interface.
 */
HWTEST(PrivacyPriorityRouterUnitTest, GetRecordCaptureDevice_019, TestSize.Level1)
{
    PrivacyPriorityRouter privacyPriorityRouter;
    auto &audioDeviceManager = AudioDeviceManager::GetAudioDeviceManager();
    audioDeviceManager.reconCapturePrivacyDevices_.clear();
    SourceType sourceType = SOURCE_TYPE_VOICE_RECOGNITION;
    int32_t clientUID = TEST_CLIENT_UID;
    uint32_t sessionID = TEST_SESSION_ID;
    shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>();
    desc->deviceType_ = DEVICE_TYPE_BLUETOOTH_SCO;
    desc->deviceRole_ = OUTPUT_DEVICE;
    desc->networkId_ = "";
    desc->macAddress_ = "00:11:22:33:44:55";
    desc->connectState_ = SUSPEND_CONNECTED;
    desc->isEnable_ = false;
    desc->deviceType_ = DEVICE_TYPE_BLUETOOTH_SCO;
    audioDeviceManager.reconCapturePrivacyDevices_.push_back(desc);
    auto result = privacyPriorityRouter.GetRecordCaptureDevice(sourceType, clientUID, sessionID);
    EXPECT_NE(result->deviceType_, desc->deviceType_);
}
} // namespace AudioStandard
} // namespace OHOS
 