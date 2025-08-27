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

#include "stream_filter_router_unit_test.h"
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

const uint32_t TEST_GROUPID = 10;
const uint32_t TEST_RESULTZERO = 0;

void StreamFilterRouterUnitTest::SetUpTestCase(void) {}
void StreamFilterRouterUnitTest::TearDownTestCase(void) {}
void StreamFilterRouterUnitTest::SetUp(void) {}
void StreamFilterRouterUnitTest::TearDown(void) {}

/**
 * @tc.name  : Test StreamFilterRouter.
 * @tc.number: SelectRemoteCaptureDevice_001
 * @tc.desc  : Test SelectRemoteCaptureDevice.
 */
HWTEST(StreamFilterRouterUnitTest, SelectRemoteCaptureDevice_001, TestSize.Level1)
{
    std::shared_ptr<AudioDeviceDescriptor> incomingDevice = std::make_shared<AudioDeviceDescriptor>();
    incomingDevice->networkId_ = "networkId123";
    incomingDevice->deviceRole_ = INPUT_DEVICE;
    incomingDevice->deviceType_ = DEVICE_TYPE_MIC;

    std::shared_ptr<AudioDeviceDescriptor> matchingDevice = std::make_shared<AudioDeviceDescriptor>();
    matchingDevice->networkId_ = "networkId123";
    matchingDevice->deviceRole_ = INPUT_DEVICE;
    matchingDevice->deviceType_ = DEVICE_TYPE_MIC;

    std::vector<std::shared_ptr<AudioDeviceDescriptor>> descriptors = {matchingDevice};

    bool hasDescriptor = false;
    StreamFilterRouter rot;
    std::shared_ptr<AudioDeviceDescriptor> result =
    rot.SelectRemoteCaptureDevice(descriptors, incomingDevice, hasDescriptor);
    EXPECT_TRUE(hasDescriptor);
    EXPECT_EQ(result->networkId_, "networkId123");
    EXPECT_EQ(result->deviceRole_, INPUT_DEVICE);
    EXPECT_EQ(result->deviceType_, DEVICE_TYPE_MIC);
}

/**
 * @tc.name  : Test StreamFilterRouter.
 * @tc.number: SelectRemoteCaptureDevice_002
 * @tc.desc  : Test SelectRemoteCaptureDevice.
 */
HWTEST(StreamFilterRouterUnitTest, SelectRemoteCaptureDevice_002, TestSize.Level1)
{
    std::shared_ptr<AudioDeviceDescriptor> incomingDevice = std::make_shared<AudioDeviceDescriptor>();
    incomingDevice->networkId_ = "networkId123";
    incomingDevice->deviceRole_ = INPUT_DEVICE;
    incomingDevice->deviceType_ = DEVICE_TYPE_MIC;

    std::shared_ptr<AudioDeviceDescriptor> nonMatchingDevice = std::make_shared<AudioDeviceDescriptor>();
    nonMatchingDevice->networkId_ = "otherNetworkId";
    nonMatchingDevice->deviceRole_ = OUTPUT_DEVICE;
    nonMatchingDevice->deviceType_ = DEVICE_TYPE_SPEAKER;

    std::vector<std::shared_ptr<AudioDeviceDescriptor>> descriptors = {nonMatchingDevice};

    bool hasDescriptor = false;
    StreamFilterRouter rot;
    std::shared_ptr<AudioDeviceDescriptor> result =
    rot.SelectRemoteCaptureDevice(descriptors, incomingDevice, hasDescriptor);

    EXPECT_FALSE(hasDescriptor);
}

/**
 * @tc.name  : Test StreamFilterRouter.
 * @tc.number: SelectRemoteCaptureDevice_003
 * @tc.desc  : Test SelectRemoteCaptureDevice.
 */
HWTEST(StreamFilterRouterUnitTest, SelectRemoteCaptureDevice_003, TestSize.Level1)
{
    std::shared_ptr<AudioDeviceDescriptor> incomingDevice = std::make_shared<AudioDeviceDescriptor>();
    incomingDevice->networkId_ = "networkId123";
    incomingDevice->deviceRole_ = INPUT_DEVICE;
    incomingDevice->deviceType_ = DEVICE_TYPE_MIC;

    std::vector<std::shared_ptr<AudioDeviceDescriptor>> descriptors = {};

    bool hasDescriptor = false;
    StreamFilterRouter rot;
    std::shared_ptr<AudioDeviceDescriptor> result =
    rot.SelectRemoteCaptureDevice(descriptors, incomingDevice, hasDescriptor);
    
    EXPECT_FALSE(hasDescriptor);
}

/**
 * @tc.name  : Test StreamFilterRouter.
 * @tc.number: StreamFilterRouter_001
 * @tc.desc  : Test GetCallRenderDevice And GetMediaRenderDevice.
 */
HWTEST(StreamFilterRouterUnitTest, StreamFilterRouter_001, TestSize.Level1)
{
    auto streamFilterRouter_ = std::make_shared<StreamFilterRouter>();
    std::shared_ptr<AudioDeviceDescriptor> deviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    StreamUsage streamUsage = STREAM_USAGE_MEDIA;
    int32_t clientId = 1;
    CastType type = CAST_TYPE_NULL;
    int32_t callerPid = IPCSkeleton::GetCallingPid();
    std::cout<<callerPid<<std::endl;
    auto result = streamFilterRouter_->GetCallRenderDevice(streamUsage, clientId);
    EXPECT_NE(streamFilterRouter_, nullptr);

    result = streamFilterRouter_->GetMediaRenderDevice(streamUsage, clientId);
    EXPECT_NE(streamFilterRouter_, nullptr);
}

/**
 * @tc.name  : Test StreamFilterRouter.
 * @tc.number: StreamFilterRouter_002
 * @tc.desc  : Test GetCallRenderDevice And GetMediaRenderDevice.
 */
HWTEST(StreamFilterRouterUnitTest, StreamFilterRouter_002, TestSize.Level1)
{
    auto streamFilterRouter_ = std::make_shared<StreamFilterRouter>();
    std::shared_ptr<AudioDeviceDescriptor> deviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    StreamUsage streamUsage = STREAM_USAGE_MEDIA;
    int32_t clientId = 1;
    CastType type = CAST_TYPE_ALL;
    auto result = streamFilterRouter_->GetCallRenderDevice(streamUsage, clientId);
    EXPECT_NE(streamFilterRouter_, nullptr);

    result = streamFilterRouter_->GetMediaRenderDevice(streamUsage, clientId);
    EXPECT_NE(streamFilterRouter_, nullptr);
}

/**
 * @tc.name  : Test StreamFilterRouter.
 * @tc.number: StreamFilterRouter_003
 * @tc.desc  : Test GetCallRenderDevice And GetMediaRenderDevice.
 */
HWTEST(StreamFilterRouterUnitTest, StreamFilterRouter_003, TestSize.Level1)
{
    auto streamFilterRouter_ = std::make_shared<StreamFilterRouter>();
    std::shared_ptr<AudioDeviceDescriptor> deviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    StreamUsage streamUsage = STREAM_USAGE_MEDIA;
    int32_t clientId = 1;
    CastType type = CAST_TYPE_PROJECTION;
    auto result = streamFilterRouter_->GetCallRenderDevice(streamUsage, clientId);
    EXPECT_NE(streamFilterRouter_, nullptr);

    result = streamFilterRouter_->GetMediaRenderDevice(streamUsage, clientId);
    EXPECT_NE(streamFilterRouter_, nullptr);

    streamUsage = STREAM_USAGE_MUSIC;
    result = streamFilterRouter_->GetMediaRenderDevice(streamUsage, clientId);
    EXPECT_NE(streamFilterRouter_, nullptr);
}

/**
 * @tc.name  : Test StreamFilterRouter.
 * @tc.number: StreamFilterRouter_004
 * @tc.desc  : Test GetCallRenderDevice And GetMediaRenderDevice.
 */
HWTEST(StreamFilterRouterUnitTest, StreamFilterRouter_004, TestSize.Level1)
{
    auto streamFilterRouter_ = std::make_shared<StreamFilterRouter>();
    std::shared_ptr<AudioDeviceDescriptor> deviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    StreamUsage streamUsage = STREAM_USAGE_MEDIA;
    int32_t clientId = 1;
    CastType type = CAST_TYPE_COOPERATION;
    auto result = streamFilterRouter_->GetCallRenderDevice(streamUsage, clientId);
    EXPECT_NE(streamFilterRouter_, nullptr);

    result = streamFilterRouter_->GetMediaRenderDevice(streamUsage, clientId);
    EXPECT_NE(streamFilterRouter_, nullptr);
}

/**
 * @tc.name  : Test StreamFilterRouter.
 * @tc.number: StreamFilterRouter_005
 * @tc.desc  : Test GetCallRenderDevice And GetMediaRenderDevice.
 */
HWTEST(StreamFilterRouterUnitTest, StreamFilterRouter_005, TestSize.Level1)
{
    auto streamFilterRouter_ = std::make_shared<StreamFilterRouter>();
    std::shared_ptr<AudioDeviceDescriptor> deviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    StreamUsage streamUsage = STREAM_USAGE_MEDIA;
    int32_t clientId = 1;
    CastType type = static_cast<CastType>(99);
    auto result = streamFilterRouter_->GetCallRenderDevice(streamUsage, clientId);
    EXPECT_NE(streamFilterRouter_, nullptr);

    result = streamFilterRouter_->GetMediaRenderDevice(streamUsage, clientId);
    EXPECT_NE(streamFilterRouter_, nullptr);
}

/**
 * @tc.name  : Test StreamFilterRouter.
 * @tc.number: StreamFilterRouter_006
 * @tc.desc  : Test GetCallCaptureDevice And GetRecordCaptureDevice.
 */
HWTEST(StreamFilterRouterUnitTest, StreamFilterRouter_006, TestSize.Level1)
{
    auto streamFilterRouter_ = std::make_shared<StreamFilterRouter>();
    std::shared_ptr<AudioDeviceDescriptor> deviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    SourceType sourceType = SOURCE_TYPE_VOICE_CALL;
    int32_t clientId = 1;
    CastType type = CAST_TYPE_NULL;
    auto result = streamFilterRouter_->GetCallCaptureDevice(sourceType, clientId);
    EXPECT_NE(streamFilterRouter_, nullptr);

    result = streamFilterRouter_->GetRecordCaptureDevice(sourceType, clientId);
    EXPECT_NE(streamFilterRouter_, nullptr);
}

/**
 * @tc.name  : Test StreamFilterRouter.
 * @tc.number: StreamFilterRouter_007
 * @tc.desc  : Test GetCallCaptureDevice And GetRecordCaptureDevice.
 */
HWTEST(StreamFilterRouterUnitTest, StreamFilterRouter_007, TestSize.Level1)
{
    auto streamFilterRouter_ = std::make_shared<StreamFilterRouter>();
    std::shared_ptr<AudioDeviceDescriptor> deviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    SourceType sourceType = SOURCE_TYPE_VOICE_CALL;
    int32_t clientId = 1;
    CastType type = CAST_TYPE_ALL;
    auto result = streamFilterRouter_->GetCallCaptureDevice(sourceType, clientId);
    EXPECT_NE(streamFilterRouter_, nullptr);

    result = streamFilterRouter_->GetRecordCaptureDevice(sourceType, clientId);
    EXPECT_NE(streamFilterRouter_, nullptr);
}

/**
 * @tc.name  : Test StreamFilterRouter.
 * @tc.number: StreamFilterRouter_008
 * @tc.desc  : Test GetCallCaptureDevice And GetRecordCaptureDevice.
 */
HWTEST(StreamFilterRouterUnitTest, StreamFilterRouter_008, TestSize.Level1)
{
    auto streamFilterRouter_ = std::make_shared<StreamFilterRouter>();
    std::shared_ptr<AudioDeviceDescriptor> deviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    SourceType sourceType = SOURCE_TYPE_VOICE_CALL;
    int32_t clientId = 1;
    CastType type = CAST_TYPE_PROJECTION;
    auto result = streamFilterRouter_->GetCallCaptureDevice(sourceType, clientId);
    EXPECT_NE(streamFilterRouter_, nullptr);

    result = streamFilterRouter_->GetRecordCaptureDevice(sourceType, clientId);
    EXPECT_NE(streamFilterRouter_, nullptr);

    sourceType = SOURCE_TYPE_MIC;
    result = streamFilterRouter_->GetRecordCaptureDevice(sourceType, clientId);
    EXPECT_NE(streamFilterRouter_, nullptr);
}

/**
 * @tc.name  : Test StreamFilterRouter.
 * @tc.number: StreamFilterRouter_009
 * @tc.desc  : Test GetCallCaptureDevice And GetRecordCaptureDevice.
 */
HWTEST(StreamFilterRouterUnitTest, StreamFilterRouter_009, TestSize.Level1)
{
    auto streamFilterRouter_ = std::make_shared<StreamFilterRouter>();
    std::shared_ptr<AudioDeviceDescriptor> deviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    SourceType sourceType = SOURCE_TYPE_VOICE_CALL;
    int32_t clientId = 1;
    CastType type = CAST_TYPE_COOPERATION;
    auto result = streamFilterRouter_->GetCallCaptureDevice(sourceType, clientId);
    EXPECT_NE(streamFilterRouter_, nullptr);

    result = streamFilterRouter_->GetRecordCaptureDevice(sourceType, clientId);
    EXPECT_NE(streamFilterRouter_, nullptr);
}

/**
 * @tc.name  : Test StreamFilterRouter.
 * @tc.number: StreamFilterRouter_010
 * @tc.desc  : Test GetCallCaptureDevice And GetRecordCaptureDevice.
 */
HWTEST(StreamFilterRouterUnitTest, StreamFilterRouter_010, TestSize.Level1)
{
    auto streamFilterRouter_ = std::make_shared<StreamFilterRouter>();
    std::shared_ptr<AudioDeviceDescriptor> deviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    SourceType sourceType = SOURCE_TYPE_VOICE_CALL;
    int32_t clientId = 1;
    CastType type = static_cast<CastType>(99);
    auto result = streamFilterRouter_->GetCallCaptureDevice(sourceType, clientId);
    EXPECT_NE(streamFilterRouter_, nullptr);

    result = streamFilterRouter_->GetRecordCaptureDevice(sourceType, clientId);
    EXPECT_NE(streamFilterRouter_, nullptr);
}

/**
 * @tc.name  : Test StreamFilterRouter.
 * @tc.number: StreamFilterRouter_011
 * @tc.desc  : Test routingInfo.descriptor equals nullptr.
 */
HWTEST(StreamFilterRouterUnitTest, StreamFilterRouter_011, TestSize.Level1)
{
    auto streamFilterRouter_ = std::make_shared<StreamFilterRouter>();
    std::shared_ptr<AudioDeviceDescriptor> deviceDescriptor = nullptr;
    StreamUsage streamUsage = STREAM_USAGE_MEDIA;
    SourceType sourceType = SOURCE_TYPE_VOICE_CALL;
    int32_t clientId = 1;
    CastType type = static_cast<CastType>(99);
    auto result = streamFilterRouter_->GetCallRenderDevice(streamUsage, clientId);
    EXPECT_NE(streamFilterRouter_, nullptr);

    result = streamFilterRouter_->GetMediaRenderDevice(streamUsage, clientId);
    EXPECT_NE(streamFilterRouter_, nullptr);

    result = streamFilterRouter_->GetCallCaptureDevice(sourceType, clientId);
    EXPECT_NE(streamFilterRouter_, nullptr);

    result = streamFilterRouter_->GetRecordCaptureDevice(sourceType, clientId);
    EXPECT_NE(streamFilterRouter_, nullptr);
}

/**
 * @tc.name  : Test IsIncomingDeviceInRemoteDevice.
 * @tc.number: StreamFilterRouter_012
 * @tc.desc  : Test IsIncomingDeviceInRemoteDevice.
 */
HWTEST(StreamFilterRouterUnitTest, StreamFilterRouter_012, TestSize.Level1)
{
    std::shared_ptr<AudioDeviceDescriptor> incomingDevice = std::make_shared<AudioDeviceDescriptor>();
    incomingDevice->networkId_ = "networkId123";
    incomingDevice->deviceRole_ = INPUT_DEVICE;
    incomingDevice->deviceType_ = DEVICE_TYPE_MIC;
    incomingDevice->interruptGroupId_ = TEST_GROUPID;
    incomingDevice->volumeGroupId_ = TEST_GROUPID;
    incomingDevice->macAddress_ = "macAddress_123";

    std::shared_ptr<AudioDeviceDescriptor> matchingDevice = std::make_shared<AudioDeviceDescriptor>();
    matchingDevice->networkId_ = "networkId123";
    matchingDevice->deviceRole_ = INPUT_DEVICE;
    matchingDevice->deviceType_ = DEVICE_TYPE_MIC;
    matchingDevice->interruptGroupId_ = TEST_GROUPID;
    matchingDevice->volumeGroupId_ = TEST_GROUPID;
    matchingDevice->macAddress_ = "macAddress_123";

    auto streamFilterRouter_ = std::make_shared<StreamFilterRouter>();
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> descriptors = {};
    bool result = streamFilterRouter_->IsIncomingDeviceInRemoteDevice(descriptors, incomingDevice);
    EXPECT_FALSE(result);

    matchingDevice->macAddress_ = "macAddress";
    descriptors = {matchingDevice};
    result = streamFilterRouter_->IsIncomingDeviceInRemoteDevice(descriptors, incomingDevice);
    EXPECT_TRUE(incomingDevice->isVrSupported_);

    matchingDevice->volumeGroupId_ = TEST_GROUPID - 1;
    descriptors = {matchingDevice};
    result = streamFilterRouter_->IsIncomingDeviceInRemoteDevice(descriptors, incomingDevice);
    EXPECT_EQ(NO_A2DP_DEVICE, incomingDevice->a2dpOffloadFlag_);

    matchingDevice->interruptGroupId_ = TEST_GROUPID - 1;
    descriptors = {matchingDevice};
    result = streamFilterRouter_->IsIncomingDeviceInRemoteDevice(descriptors, incomingDevice);
    EXPECT_EQ(TEST_RESULTZERO, incomingDevice->deviceId_);

    matchingDevice->deviceType_ = DEVICE_TYPE_SPEAKER;
    descriptors = {matchingDevice};
    result = streamFilterRouter_->IsIncomingDeviceInRemoteDevice(descriptors, incomingDevice);
    EXPECT_FALSE(matchingDevice->isScoRealConnected_);
    
    incomingDevice->deviceRole_ = OUTPUT_DEVICE;
    descriptors = {matchingDevice};
    result = streamFilterRouter_->IsIncomingDeviceInRemoteDevice(descriptors, incomingDevice);
    EXPECT_EQ(ALL_USAGE, matchingDevice->deviceUsage_);

    matchingDevice->networkId_ = "otherNetworkId";
    result = streamFilterRouter_->IsIncomingDeviceInRemoteDevice(descriptors, incomingDevice);
    EXPECT_FALSE(matchingDevice->spatializationSupported_);
}

/**
 * @tc.name  : Test SelectRemoteCaptureDevice.
 * @tc.number: StreamFilterRouter_013
 * @tc.desc  : Test SelectRemoteCaptureDevice.
 */
HWTEST(StreamFilterRouterUnitTest, StreamFilterRouter_013, TestSize.Level1)
{
    std::shared_ptr<AudioDeviceDescriptor> incomingDevice = std::make_shared<AudioDeviceDescriptor>();
    incomingDevice->networkId_ = "networkId123";
    incomingDevice->deviceRole_ = INPUT_DEVICE;
    incomingDevice->deviceType_ = DEVICE_TYPE_MIC;

    std::shared_ptr<AudioDeviceDescriptor> matchingDevice = std::make_shared<AudioDeviceDescriptor>();
    matchingDevice->networkId_ = "networkId123";
    matchingDevice->deviceRole_ = INPUT_DEVICE;
    matchingDevice->deviceType_ = DEVICE_TYPE_SPEAKER;

    StreamFilterRouter rot;
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> descriptors = {matchingDevice};
    bool hasDescriptor = false;
    std::shared_ptr<AudioDeviceDescriptor> ret =
    rot.SelectRemoteCaptureDevice(descriptors, incomingDevice, hasDescriptor);
    EXPECT_EQ(incomingDevice->a2dpOffloadFlag_, matchingDevice->a2dpOffloadFlag_);

    incomingDevice->deviceRole_ = OUTPUT_DEVICE;
    descriptors = {matchingDevice};
    ret = rot.SelectRemoteCaptureDevice(descriptors, incomingDevice, hasDescriptor);
    EXPECT_EQ(incomingDevice->routerType_, matchingDevice->routerType_);
}
} // namespace AudioStandard
} // namespace OHOS
