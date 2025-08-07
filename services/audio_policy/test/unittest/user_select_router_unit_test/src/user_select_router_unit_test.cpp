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

#include "user_select_router_unit_test.h"
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

void UserSelectRouterUnitTest::SetUpTestCase(void) {}
void UserSelectRouterUnitTest::TearDownTestCase(void) {}
void UserSelectRouterUnitTest::SetUp(void) {}
void UserSelectRouterUnitTest::TearDown(void) {}

/**
 * @tc.name  : Test GetMediaRenderDevice.
 * @tc.number: GetMediaRenderDevice_001
 * @tc.desc  : GetMediaRenderDevice.
 */
HWTEST(UserSelectRouterUnitTest, GetMediaRenderDevice_001, TestSize.Level3)
{
    UserSelectRouter userSelectRouter;
    int32_t clientUID = 1;
    auto result = userSelectRouter.GetMediaRenderDevice(STREAM_USAGE_RINGTONE, clientUID);
    EXPECT_NE(result, nullptr);
}

/**
 * @tc.name  : Test GetMediaRenderDevice.
 * @tc.number: GetMediaRenderDevice_002
 * @tc.desc  : GetMediaRenderDevice.
 */
HWTEST(UserSelectRouterUnitTest, GetMediaRenderDevice_002, TestSize.Level3)
{
    UserSelectRouter userSelectRouter;
    int32_t clientUID = 1;
    auto result = userSelectRouter.GetMediaRenderDevice(STREAM_USAGE_VOICE_RINGTONE, clientUID);
    EXPECT_NE(result, nullptr);
}

/**
 * @tc.name  : Test GetMediaRenderDevice.
 * @tc.number: GetMediaRenderDevice_003
 * @tc.desc  : GetMediaRenderDevice.
 */
HWTEST(UserSelectRouterUnitTest, GetMediaRenderDevice_003, TestSize.Level3)
{
    UserSelectRouter userSelectRouter;
    int32_t clientUID = 1;
    auto result = userSelectRouter.GetMediaRenderDevice(STREAM_USAGE_MEDIA, clientUID);
    EXPECT_NE(result, nullptr);
}

/**
 * @tc.name  : Test GetMediaRenderDevice.
 * @tc.number: GetCallRenderDevice_001
 * @tc.desc  : GetMediaRenderDevice.
 */
HWTEST(UserSelectRouterUnitTest, GetCallRenderDevice_001, TestSize.Level3)
{
    UserSelectRouter userSelectRouter;
    StreamUsage streamUsage = StreamUsage::STREAM_USAGE_VOICE_MESSAGE;
    int32_t clientUID = 1;
    auto result = userSelectRouter.GetCallRenderDevice(streamUsage, clientUID);
    EXPECT_NE(result, nullptr);
}

/**
 * @tc.name  : Test GetCallRenderDevic.
 * @tc.number: GetCallRenderDevice_002
 * @tc.desc  : GetCallRenderDevic.
 */
HWTEST(UserSelectRouterUnitTest, GetCallRenderDevice_002, TestSize.Level3)
{
    UserSelectRouter userSelectRouter;
    StreamUsage streamUsage = StreamUsage::STREAM_USAGE_VOICE_MESSAGE;
    int32_t clientUID = 1;
    auto preferredDevice = std::make_shared<AudioDeviceDescriptor>();
    preferredDevice->deviceId_ = 0;
    auto result = userSelectRouter.GetCallRenderDevice(streamUsage, clientUID);
    EXPECT_NE(result, nullptr);
}

/**
 * @tc.name  : Test GetMediaRenderDevice.
 * @tc.number: GetCallRenderDevice_003
 * @tc.desc  : GetMediaRenderDevice.
 */
HWTEST(UserSelectRouterUnitTest, GetCallRenderDevice_003, TestSize.Level3)
{
    UserSelectRouter userSelectRouter;
    StreamUsage streamUsage = StreamUsage::STREAM_USAGE_VOICE_MESSAGE;
    int32_t clientUID = 1;
    auto preferredDevice = std::make_shared<AudioDeviceDescriptor>();
    preferredDevice->deviceId_ = 1;
    auto result = userSelectRouter.GetCallRenderDevice(streamUsage, clientUID);
    EXPECT_NE(result, nullptr);
}

/**
 * @tc.name  : Test GetCallCaptureDevice.
 * @tc.number: GetCallCaptureDevice_001
 * @tc.desc  : GetCallCaptureDevice.
 */
HWTEST(UserSelectRouterUnitTest, GetCallCaptureDevice_001, TestSize.Level3)
{
    UserSelectRouter userSelectRouter;
    SourceType sourceType = SourceType::SOURCE_TYPE_VOICE_RECOGNITION;
    StreamUsage streamUsage = StreamUsage::STREAM_USAGE_VOICE_MESSAGE;
    int32_t clientUID = 1;
    uint32_t sessionID = 678;
    auto preferredDevice = std::make_shared<AudioDeviceDescriptor>();
    preferredDevice->deviceId_ = 1;
    auto result = userSelectRouter.GetCallCaptureDevice(sourceType, clientUID, sessionID);
    EXPECT_NE(result, nullptr);
}

/**
 * @tc.name  : Test GetCallCaptureDevice.
 * @tc.number: GetCallCaptureDevice_001
 * @tc.desc  : GetCallCaptureDevice.
 */
HWTEST(UserSelectRouterUnitTest, GetCallCaptureDevice_002, TestSize.Level4)
{
    UserSelectRouter userSelectRouter;
    SourceType sourceType = SourceType::SOURCE_TYPE_VOICE_RECOGNITION;
    StreamUsage streamUsage = StreamUsage::STREAM_USAGE_VOICE_MESSAGE;
    int32_t clientUID = 1;
    uint32_t sessionID = 678;
    auto preferredDevice = std::make_shared<AudioDeviceDescriptor>();
    preferredDevice->deviceId_ = 0;
    auto result = userSelectRouter.GetCallCaptureDevice(sourceType, clientUID, sessionID);
    EXPECT_NE(result, nullptr);
}

/**
 * @tc.name  : Test GetRingRenderDevices.
 * @tc.number: GetRingRenderDevices_001
 * @tc.desc  : GetRingRenderDevices.
 */
HWTEST(UserSelectRouterUnitTest, GetRingRenderDevices_001, TestSize.Level3)
{
    UserSelectRouter userSelectRouter;
    int32_t clientUID = 1;
    vector<shared_ptr<AudioDeviceDescriptor>> descs = userSelectRouter.GetRingRenderDevices(STREAM_USAGE_RINGTONE,
        clientUID);
    EXPECT_EQ(descs.size(), 1);
}

/**
 * @tc.name  : Test GetRingRenderDevices.
 * @tc.number: GetRingRenderDevices_002
 * @tc.desc  : GetRingRenderDevices.
 */
HWTEST(UserSelectRouterUnitTest, GetRingRenderDevices_002, TestSize.Level3)
{
    UserSelectRouter userSelectRouter;
    int32_t clientUID = 1;
    vector<shared_ptr<AudioDeviceDescriptor>> descs = userSelectRouter.GetRingRenderDevices(STREAM_USAGE_VOICE_RINGTONE,
        clientUID);
    EXPECT_EQ(descs.size(), 1);
}

/**
 * @tc.name  : Test GetRingRenderDevices.
 * @tc.number: GetRingRenderDevices_003
 * @tc.desc  : GetRingRenderDevices.
 */
HWTEST(UserSelectRouterUnitTest, GetRingRenderDevices_003, TestSize.Level3)
{
    UserSelectRouter userSelectRouter;
    int32_t clientUID = 1;
    vector<shared_ptr<AudioDeviceDescriptor>> descs = userSelectRouter.GetRingRenderDevices(STREAM_USAGE_ALARM,
        clientUID);
    EXPECT_EQ(descs.size(), 1);
}

/**
 * @tc.name  : Test GetRecordCaptureDevice.
 * @tc.number: GetRecordCaptureDevice_001
 * @tc.desc  : GetRecordCaptureDevice.
 */
HWTEST(UserSelectRouterUnitTest, GetRecordCaptureDevice_001, TestSize.Level4)
{
    UserSelectRouter userSelectRouter;
    SourceType sourceType = SourceType::SOURCE_TYPE_VOICE_RECOGNITION;
    int32_t clientUID = 1;
    uint32_t sessionID = 678;
    auto result = userSelectRouter.GetRecordCaptureDevice(sourceType, clientUID, sessionID);
    EXPECT_NE(result, nullptr);
}

/**
 * @tc.name  : Test GetRecordCaptureDevice.
 * @tc.number: GetRecordCaptureDevice_002
 * @tc.desc  : GetRecordCaptureDevice.
 */
HWTEST(UserSelectRouterUnitTest, GetRecordCaptureDevice_002, TestSize.Level3)
{
    UserSelectRouter userSelectRouter;
    SourceType sourceType = SourceType::SOURCE_TYPE_VOICE_TRANSCRIPTION;
    int32_t clientUID = 1;
    uint32_t sessionID = 678;
    auto result = userSelectRouter.GetRecordCaptureDevice(sourceType, clientUID, sessionID);
    EXPECT_NE(result, nullptr);
}
} // namespace AudioStandard
} // namespace OHOS
