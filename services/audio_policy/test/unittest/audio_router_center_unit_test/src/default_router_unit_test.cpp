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

#include "audio_router_center_unit_test.h"
#include "audio_errors.h"
#include "audio_policy_log.h"
#include "audio_zone_service.h"
#include "default_router.h"
#include "app_select_router.h"
#include "pair_device_router.h"
#include "cockpit_phone_router.h"

#include <thread>
#include <memory>
#include <vector>
using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

/**
 * @tc.name  : Test GetMediaRenderDevice.
 * @tc.number: GetMediaRenderDevice_001
 * @tc.desc  : Test GetMediaRenderDevice interface.
 */
HWTEST(AudioRouterCenterUnitTest, GetMediaRenderDevice_001, TestSize.Level4)
{
    StreamUsage streamUsage = STREAM_USAGE_MEDIA;
    int32_t clientUID = 1000;
    DefaultRouter defaultRouter;
    std::shared_ptr<AudioDeviceDescriptor> actualDevice =
        defaultRouter.GetMediaRenderDevice(streamUsage, clientUID);
    EXPECT_EQ(actualDevice->deviceType_, DeviceType::DEVICE_TYPE_NONE);
}

/**
 * @tc.name  : Test GetCallRenderDevice.
 * @tc.number: GetCallRenderDevice_001
 * @tc.desc  : Test GetCallRenderDevice interface.
 */
HWTEST(AudioRouterCenterUnitTest, GetCallRenderDevice_001, TestSize.Level4)
{
    StreamUsage streamUsage = STREAM_USAGE_VOICE_MESSAGE;
    int32_t clientUID = 1000;
    DefaultRouter defaultRouter;
    auto result = defaultRouter.GetCallRenderDevice(streamUsage, clientUID);
    EXPECT_EQ(result->deviceType_, DeviceType::DEVICE_TYPE_NONE);
}

/**
 * @tc.name  : Test GetCallCaptureDevice.
 * @tc.number: GetCallCaptureDevice_001
 * @tc.desc  : Test GetCallCaptureDevice interface.
 */
HWTEST(AudioRouterCenterUnitTest, GetCallCaptureDevice_001, TestSize.Level4)
{
    SourceType sourceType = SOURCE_TYPE_MIC;
    int32_t clientUID = 1000;
    uint32_t sessionID = 123;
    AppSelectRouter appSelectRouter;
    auto result = appSelectRouter.GetCallCaptureDevice(sourceType, clientUID, sessionID);
    EXPECT_NE(result, nullptr);
}

/**
 * @tc.name  : Test GetCallCaptureDevice.
 * @tc.number: GetCallCaptureDevice_002
 * @tc.desc  : Test GetCallCaptureDevice interface.
 */
HWTEST(AudioRouterCenterUnitTest, GetCallCaptureDevice_002, TestSize.Level4)
{
    SourceType sourceType = SOURCE_TYPE_MIC;
    int32_t clientUID = 1000;
    uint32_t sessionID = 0;
    AppSelectRouter appSelectRouter;
    auto result = appSelectRouter.GetCallCaptureDevice(sourceType, clientUID, sessionID);
    EXPECT_NE(result, nullptr);
}

/**
 * @tc.name  : Test GetRecordCaptureDevice.
 * @tc.number: GetRecordCaptureDevice_001
 * @tc.desc  : Test GetRecordCaptureDevice interface.
 */
HWTEST(AudioRouterCenterUnitTest, GetRecordCaptureDevice_001, TestSize.Level4)
{
    SourceType sourceType = SOURCE_TYPE_MIC;
    int32_t clientUID = 1000;
    uint32_t sessionID = 123;
    AppSelectRouter appSelectRouter;
    shared_ptr<AudioDeviceDescriptor> actualDevice =
        appSelectRouter.GetRecordCaptureDevice(sourceType, clientUID, sessionID);
    EXPECT_NE(actualDevice, nullptr);

    sessionID = 0;
    actualDevice = appSelectRouter.GetRecordCaptureDevice(sourceType, clientUID, sessionID);
    EXPECT_NE(actualDevice, nullptr);
}

/**
 * @tc.name  : Test PairDeviceRouter_GetCallCaptureDevice.
 * @tc.number: PairDeviceRouter_GetCallCaptureDevice_001
 * @tc.desc  : Test PairDeviceRouter_GetCallCaptureDevice interface.
 */
HWTEST(AudioRouterCenterUnitTest, PairDeviceRouter_GetCallCaptureDevice_001, TestSize.Level4)
{
    SourceType sourceType = SOURCE_TYPE_MIC;
    int32_t clientUID = 1000;
    uint32_t sessionID = 123;
    PairDeviceRouter pairDeviceRouter;
    std::shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>();
    desc->pairDeviceDescriptor_ = std::make_shared<AudioDeviceDescriptor>();
    desc->pairDeviceDescriptor_->connectState_ = CONNECTED;
    desc->pairDeviceDescriptor_->exceptionFlag_ = false;
    desc->pairDeviceDescriptor_->isEnable_ = true;
    std::shared_ptr<AudioDeviceDescriptor> result =
        pairDeviceRouter.GetCallCaptureDevice(sourceType, clientUID, sessionID);
    EXPECT_NE(result, nullptr);
    EXPECT_EQ(result->deviceType_, desc->pairDeviceDescriptor_->deviceType_);
}

/**
 * @tc.name  : Test PairDeviceRouter_GetCallCaptureDevice.
 * @tc.number: PairDeviceRouter_GetCallCaptureDevice_002
 * @tc.desc  : Test PairDeviceRouter_GetCallCaptureDevice interface.
 */
HWTEST(AudioRouterCenterUnitTest, PairDeviceRouter_GetCallCaptureDevice_002, TestSize.Level4)
{
    SourceType sourceType = SOURCE_TYPE_MIC;
    int32_t clientUID = 1000;
    uint32_t sessionID = 123;
    PairDeviceRouter pairDeviceRouter;
    std::shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>();
    desc->pairDeviceDescriptor_ = nullptr;
    std::shared_ptr<AudioDeviceDescriptor> result =
        pairDeviceRouter.GetCallCaptureDevice(sourceType, clientUID, sessionID);
    EXPECT_NE(result, nullptr);
}

/**
 * @tc.name  : Test PairDeviceRouter_GetCallCaptureDevice.
 * @tc.number: PairDeviceRouter_GetCallCaptureDevice_003
 * @tc.desc  : Test PairDeviceRouter_GetCallCaptureDevice interface.
 */
HWTEST(AudioRouterCenterUnitTest, PairDeviceRouter_GetCallCaptureDevice_003, TestSize.Level4)
{
    SourceType sourceType = SOURCE_TYPE_MIC;
    int32_t clientUID = 1000;
    uint32_t sessionID = 123;
    PairDeviceRouter pairDeviceRouter;
    std::shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>();
    desc->pairDeviceDescriptor_ = std::make_shared<AudioDeviceDescriptor>();
    desc->pairDeviceDescriptor_->connectState_ = SUSPEND_CONNECTED;
    std::shared_ptr<AudioDeviceDescriptor> result =
        pairDeviceRouter.GetCallCaptureDevice(sourceType, clientUID, sessionID);
    EXPECT_NE(result, nullptr);
}

/**
 * @tc.name  : Test PairDeviceRouter_GetCallCaptureDevice.
 * @tc.number: PairDeviceRouter_GetCallCaptureDevice_004
 * @tc.desc  : Test PairDeviceRouter_GetCallCaptureDevice interface.
 */
HWTEST(AudioRouterCenterUnitTest, PairDeviceRouter_GetCallCaptureDevice_004, TestSize.Level4)
{
    SourceType sourceType = SOURCE_TYPE_MIC;
    int32_t clientUID = 1000;
    uint32_t sessionID = 123;
    PairDeviceRouter pairDeviceRouter;
    std::shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>();
    desc->pairDeviceDescriptor_ = std::make_shared<AudioDeviceDescriptor>();
    desc->pairDeviceDescriptor_->exceptionFlag_ = true;
    std::shared_ptr<AudioDeviceDescriptor> result =
        pairDeviceRouter.GetCallCaptureDevice(sourceType, clientUID, sessionID);
    EXPECT_NE(result, nullptr);
}

/**
 * @tc.name  : Test PairDeviceRouter_GetCallCaptureDevice.
 * @tc.number: PairDeviceRouter_GetCallCaptureDevice_005
 * @tc.desc  : Test PairDeviceRouter_GetCallCaptureDevice interface.
 */
HWTEST(AudioRouterCenterUnitTest, PairDeviceRouter_GetCallCaptureDevice_005, TestSize.Level4)
{
    SourceType sourceType = SOURCE_TYPE_MIC;
    int32_t clientUID = 1000;
    uint32_t sessionID = 123;
    PairDeviceRouter pairDeviceRouter;
    std::shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>();
    desc->pairDeviceDescriptor_ = std::make_shared<AudioDeviceDescriptor>();
    desc->pairDeviceDescriptor_->isEnable_ = false;
    std::shared_ptr<AudioDeviceDescriptor> result =
        pairDeviceRouter.GetCallCaptureDevice(sourceType, clientUID, sessionID);
    EXPECT_NE(result, nullptr);
}

/**
 * @tc.name  : Test CockpitPhoneRouter_GetRingRenderDevices.
 * @tc.number: CockpitPhoneRouter_GetRingRenderDevices_001
 * @tc.desc  : Test CockpitPhoneRouter_GetRingRenderDevices interface.
 */
HWTEST(AudioRouterCenterUnitTest, CockpitPhoneRouter_GetRingRenderDevices_001, TestSize.Level4)
{
    StreamUsage streamUsage = STREAM_USAGE_ALARM;
    int32_t clientUID = 1000;
    CockpitPhoneRouter cockpitPhoneRouter;
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> descs =
        cockpitPhoneRouter.GetRingRenderDevices(streamUsage, clientUID);
    EXPECT_TRUE(descs.empty());
}
} // namespace AudioStandard
} // namespace OHOS
 