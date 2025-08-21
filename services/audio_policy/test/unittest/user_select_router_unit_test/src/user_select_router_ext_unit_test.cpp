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

#include "user_select_router_ext_unit_test.h"
#include "audio_errors.h"
#include "audio_policy_log.h"
#include "audio_policy_server.h"
#include "audio_policy_service.h"

#include <thread>
#include <memory>
#include <vector>
using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

void UserSelectRouterExtUnitTest::SetUpTestCase(void) {}
void UserSelectRouterExtUnitTest::TearDownTestCase(void) {}
void UserSelectRouterExtUnitTest::SetUp(void) {}
void UserSelectRouterExtUnitTest::TearDown(void) {}

/**
 * @tc.name  : Test GetMediaRenderDevice.
 * @tc.number: GetMediaRenderDevice_004
 * @tc.desc  : GetMediaRenderDevice.
 */
HWTEST(UserSelectRouterExtUnitTest, GetMediaRenderDevice_004, TestSize.Level3)
{
    UserSelectRouter userSelectRouter;
    int32_t clientUID = 1;
    auto& audioStateManager = AudioStateManager::GetAudioStateManager();
    audioStateManager.preferredMediaRenderDevice_->deviceId_ = 1;
    audioStateManager.preferredMediaRenderDevice_->deviceUsage_ = MEDIA;
    auto result = userSelectRouter.GetMediaRenderDevice(STREAM_USAGE_MEDIA, clientUID);
    EXPECT_NE(result, nullptr);
}

/**
 * @tc.name  : Test GetMediaRenderDevice.
 * @tc.number: GetCallRenderDevice_004
 * @tc.desc  : GetCallRenderDevice
 */
HWTEST(UserSelectRouterExtUnitTest, GetCallRenderDevice_004, TestSize.Level3)
{
    UserSelectRouter userSelectRouter;
    int32_t clientUID = 1;
    auto& audioStateManager = AudioStateManager::GetAudioStateManager();
    audioStateManager.preferredMediaRenderDevice_->deviceId_ = 1;
    audioStateManager.preferredMediaRenderDevice_->deviceUsage_ = VOICE;
    auto result = userSelectRouter.GetCallRenderDevice(STREAM_USAGE_VOICE_MESSAGE, clientUID);
    EXPECT_NE(result, nullptr);
}

/**
 * @tc.name  : Test GetCallCaptureDevice.
 * @tc.number: GetCallCaptureDevice_003
 * @tc.desc  : GetCallCaptureDevice.
 */
HWTEST(UserSelectRouterExtUnitTest, GetCallCaptureDevice_003, TestSize.Level3)
{
    UserSelectRouter userSelectRouter;
    SourceType sourceType = SourceType::SOURCE_TYPE_VOICE_RECOGNITION;
    StreamUsage streamUsage = StreamUsage::STREAM_USAGE_VOICE_MESSAGE;
    int32_t clientUID = 1;
    uint32_t sessionID = 678;
    auto preferredDevice = std::make_shared<AudioDeviceDescriptor>();
    preferredDevice->deviceId_ = 1;
    preferredDevice->deviceUsage_ = VOICE;
    AudioStateManager::GetAudioStateManager().SetPreferredCallRenderDevice(preferredDevice);
    auto result = userSelectRouter.GetCallCaptureDevice(sourceType, clientUID, sessionID);
    EXPECT_NE(result, nullptr);
}

/**
 * @tc.name  : Test GetCallCaptureDevice.
 * @tc.number: GetCallCaptureDevice_004
 * @tc.desc  : GetCallCaptureDevice.
 */
HWTEST(UserSelectRouterExtUnitTest, GetCallCaptureDevice_004, TestSize.Level4)
{
    UserSelectRouter userSelectRouter;
    SourceType sourceType = SourceType::SOURCE_TYPE_VOICE_RECOGNITION;
    StreamUsage streamUsage = StreamUsage::STREAM_USAGE_VOICE_MESSAGE;
    int32_t clientUID = 1;
    uint32_t sessionID = 678;
    auto preferredDevice = std::make_shared<AudioDeviceDescriptor>();
    preferredDevice->deviceId_ = 0;
    preferredDevice->deviceUsage_ = VOICE;
    AudioStateManager::GetAudioStateManager().SetPreferredCallRenderDevice(preferredDevice);

    auto result = userSelectRouter.GetCallCaptureDevice(sourceType, clientUID, sessionID);
    EXPECT_NE(result, nullptr);
}

/**
 * @tc.name  : Test GetRingRenderDevices.
 * @tc.number: GetRingRenderDevices_004
 * @tc.desc  : GetRingRenderDevices.
 */
HWTEST(UserSelectRouterExtUnitTest, GetRingRenderDevices_004, TestSize.Level3)
{
    UserSelectRouter userSelectRouter;
    SourceType sourceType = SourceType::SOURCE_TYPE_VOICE_RECOGNITION;
    StreamUsage streamUsage = StreamUsage::STREAM_USAGE_VOICE_MESSAGE;
    int32_t clientUID = 1;
    uint32_t sessionID = 678;
    auto preferredDevice = std::make_shared<AudioDeviceDescriptor>();
    preferredDevice->deviceId_ = 0;
    preferredDevice->deviceUsage_ = VOICE;
    preferredDevice->deviceType_ = DEVICE_TYPE_WIRED_HEADSET;
    vector<shared_ptr<AudioDeviceDescriptor>> descs =
        userSelectRouter.GetRingRenderDevices(STREAM_USAGE_RINGTONE, clientUID);
    EXPECT_EQ(descs.size(), 1);
}

/**
 * @tc.name  : Test GetRingRenderDevices.
 * @tc.number: GetRingRenderDevices_005
 * @tc.desc  : GetRingRenderDevices.
 */
HWTEST(UserSelectRouterExtUnitTest, GetRingRenderDevices_005, TestSize.Level3)
{
    UserSelectRouter userSelectRouter;
    SourceType sourceType = SourceType::SOURCE_TYPE_VOICE_RECOGNITION;
    StreamUsage streamUsage = StreamUsage::STREAM_USAGE_VOICE_MESSAGE;
    int32_t clientUID = 1;
    uint32_t sessionID = 678;
    auto preferredDevice = std::make_shared<AudioDeviceDescriptor>();
    preferredDevice->deviceId_ = 0;
    preferredDevice->deviceUsage_ = VOICE;
    preferredDevice->deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    vector<shared_ptr<AudioDeviceDescriptor>> descs =
        userSelectRouter.GetRingRenderDevices(STREAM_USAGE_VOICE_RINGTONE, clientUID);
    EXPECT_EQ(descs.size(), 1);
}

/**
 * @tc.name  : Test GetRingRenderDevices.
 * @tc.number: GetRingRenderDevices_006
 * @tc.desc  : GetRingRenderDevices.
 */
HWTEST(UserSelectRouterExtUnitTest, GetRingRenderDevices_006, TestSize.Level3)
{
    UserSelectRouter userSelectRouter;
    SourceType sourceType = SourceType::SOURCE_TYPE_VOICE_RECOGNITION;
    StreamUsage streamUsage = StreamUsage::STREAM_USAGE_VOICE_MESSAGE;
    int32_t clientUID = 1;
    uint32_t sessionID = 678;
    auto preferredDevice = std::make_shared<AudioDeviceDescriptor>();
    preferredDevice->deviceId_ = 0;
    preferredDevice->deviceUsage_ = VOICE;
    preferredDevice->deviceType_ = DEVICE_TYPE_NONE;
    vector<shared_ptr<AudioDeviceDescriptor>> descs =
        userSelectRouter.GetRingRenderDevices(STREAM_USAGE_ALARM, clientUID);
    EXPECT_EQ(descs.size(), 1);
}
/**
 * @tc.name  : Test GetRingRenderDevices.
 * @tc.number: GetRingRenderDevices_007
 * @tc.desc  : GetRingRenderDevices.
 */
HWTEST(UserSelectRouterExtUnitTest, GetRingRenderDevices_007, TestSize.Level3)
{
    UserSelectRouter userSelectRouter;
    SourceType sourceType = SourceType::SOURCE_TYPE_VOICE_RECOGNITION;
    StreamUsage streamUsage = StreamUsage::STREAM_USAGE_VOICE_MESSAGE;
    int32_t clientUID = 1;
    uint32_t sessionID = 678;
    auto preferredDevice = std::make_shared<AudioDeviceDescriptor>();
    preferredDevice->deviceId_ = 0;
    preferredDevice->deviceUsage_ = VOICE;
    preferredDevice->deviceType_ = DEVICE_TYPE_WIRED_HEADSET;
    vector<shared_ptr<AudioDeviceDescriptor>> descs =
        userSelectRouter.GetRingRenderDevices(STREAM_USAGE_ALARM, clientUID);
    EXPECT_EQ(descs.size(), 1);
}

/**
 * @tc.name  : Test GetRecordCaptureDevice.
 * @tc.number: GetRecordCaptureDevice_003
 * @tc.desc  : GetRecordCaptureDevice.
 */
HWTEST(UserSelectRouterExtUnitTest, GetRecordCaptureDevice_003, TestSize.Level4)
{
    UserSelectRouter userSelectRouter;
    SourceType sourceType = SourceType::SOURCE_TYPE_VOICE_RECOGNITION;
    int32_t clientUID = 1;
    uint32_t sessionID = 678;
    auto& audioStateManager = AudioStateManager::GetAudioStateManager();
    audioStateManager.preferredMediaRenderDevice_->deviceId_ = 1;
    audioStateManager.preferredMediaRenderDevice_->deviceUsage_ = MEDIA;
    auto result = userSelectRouter.GetRecordCaptureDevice(sourceType, clientUID, sessionID);
    EXPECT_NE(result, nullptr);
}

/**
 * @tc.name  : Test GetRecordCaptureDevice.
 * @tc.number: GetRecordCaptureDevice_004
 * @tc.desc  : GetRecordCaptureDevice.
 */
HWTEST(UserSelectRouterExtUnitTest, GetRecordCaptureDevice_004, TestSize.Level3)
{
    UserSelectRouter userSelectRouter;
    SourceType sourceType = SourceType::SOURCE_TYPE_VOICE_TRANSCRIPTION;
    int32_t clientUID = 1;
    uint32_t sessionID = 678;
    auto& audioStateManager = AudioStateManager::GetAudioStateManager();
    audioStateManager.preferredMediaRenderDevice_->deviceId_ = 1;
    audioStateManager.preferredMediaRenderDevice_->deviceUsage_ = MEDIA;
    auto result = userSelectRouter.GetRecordCaptureDevice(sourceType, clientUID, sessionID);
    EXPECT_NE(result, nullptr);
}
} // namespace AudioStandard
} // namespace OHOS
