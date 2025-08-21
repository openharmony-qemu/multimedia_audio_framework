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

#include "stream_filter_router_ext_unit_test.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {
void StreamFilterRouterExtUnitTest::SetUpTestCase(void) {}
void StreamFilterRouterExtUnitTest::TearDownTestCase(void) {}

void StreamFilterRouterExtUnitTest::SetUp(void)
{
    streamFilterRouter_ = std::make_shared<StreamFilterRouter>();
}

void StreamFilterRouterExtUnitTest::TearDown(void)
{
    streamFilterRouter_ = nullptr;
}

/**
 * @tc.name  : Test StreamFilterRouter.
 * @tc.number: GetMediaRenderDevice_001
 * @tc.desc  : Test GetMediaRenderDevice type == CAST_TYPE_NULL.
 */
HWTEST_F(StreamFilterRouterExtUnitTest, GetMediaRenderDevice_001, TestSize.Level4)
{
    auto descriptor = std::make_shared<AudioDeviceDescriptor>();
    DistributedRoutingInfo distributedRoutingInfo = {
        .descriptor = descriptor,
        .type = CAST_TYPE_NULL
    };
    AudioPolicyService::GetAudioPolicyService().distributedRoutingInfo_ = distributedRoutingInfo;
    StreamUsage streamUsage = STREAM_USAGE_INVALID;
    int32_t clientUID = 1;
    EXPECT_NE(streamFilterRouter_->GetMediaRenderDevice(streamUsage, clientUID), nullptr);
}

/**
 * @tc.name  : Test StreamFilterRouter.
 * @tc.number: GetMediaRenderDevice_002
 * @tc.desc  : Test GetMediaRenderDevice type == CAST_TYPE_ALL.
 */
HWTEST_F(StreamFilterRouterExtUnitTest, GetMediaRenderDevice_002, TestSize.Level4)
{
    auto descriptor = std::make_shared<AudioDeviceDescriptor>();
    DistributedRoutingInfo distributedRoutingInfo = {
        .descriptor = descriptor,
        .type = CAST_TYPE_ALL
    };
    AudioPolicyService::GetAudioPolicyService().distributedRoutingInfo_ = distributedRoutingInfo;
    StreamUsage streamUsage = STREAM_USAGE_INVALID;
    int32_t clientUID = 1;
    EXPECT_NE(streamFilterRouter_->GetMediaRenderDevice(streamUsage, clientUID), nullptr);
}

/**
 * @tc.name  : Test StreamFilterRouter.
 * @tc.number: GetMediaRenderDevice_003
 * @tc.desc  : Test GetMediaRenderDevice type == CAST_TYPE_PROJECTION.
 */
HWTEST_F(StreamFilterRouterExtUnitTest, GetMediaRenderDevice_003, TestSize.Level4)
{
    auto descriptor = std::make_shared<AudioDeviceDescriptor>();
    DistributedRoutingInfo distributedRoutingInfo = {
        .descriptor = descriptor,
        .type = CAST_TYPE_PROJECTION
    };
    AudioPolicyService::GetAudioPolicyService().distributedRoutingInfo_ = distributedRoutingInfo;
    StreamUsage streamUsage = STREAM_USAGE_INVALID;
    int32_t clientUID = 1;
    AudioDeviceManager::GetAudioDeviceManager().remoteRenderDevices_.clear();
    vector<shared_ptr<AudioDeviceDescriptor>> descriptors;
    EXPECT_FALSE(streamFilterRouter_->IsIncomingDeviceInRemoteDevice(descriptors, descriptor));
    EXPECT_NE(streamFilterRouter_->GetMediaRenderDevice(streamUsage, clientUID), nullptr);

    AudioDeviceManager::GetAudioDeviceManager().remoteRenderDevices_.push_back(descriptor);
    streamUsage = STREAM_USAGE_MUSIC;
    descriptors.push_back(descriptor);
    EXPECT_TRUE(streamFilterRouter_->IsIncomingDeviceInRemoteDevice(descriptors, descriptor));
    EXPECT_NE(streamFilterRouter_->GetMediaRenderDevice(streamUsage, clientUID), nullptr);
}

/**
 * @tc.name  : Test StreamFilterRouter.
 * @tc.number: GetMediaRenderDevice_004
 * @tc.desc  : Test GetMediaRenderDevice type == CAST_TYPE_COOPERATION.
 */
HWTEST_F(StreamFilterRouterExtUnitTest, GetMediaRenderDevice_004, TestSize.Level4)
{
    auto descriptor = std::make_shared<AudioDeviceDescriptor>();
    DistributedRoutingInfo distributedRoutingInfo = {
        .descriptor = descriptor,
        .type = CAST_TYPE_COOPERATION
    };
    AudioPolicyService::GetAudioPolicyService().distributedRoutingInfo_ = distributedRoutingInfo;
    StreamUsage streamUsage = STREAM_USAGE_INVALID;
    int32_t clientUID = 1;
    EXPECT_NE(streamFilterRouter_->GetMediaRenderDevice(streamUsage, clientUID), nullptr);
}

/**
 * @tc.name  : Test StreamFilterRouter.
 * @tc.number: GetCallRenderDevice_001
 * @tc.desc  : Test GetCallRenderDevice type == CAST_TYPE_NULL.
 */
HWTEST_F(StreamFilterRouterExtUnitTest, GetCallRenderDevice_001, TestSize.Level4)
{
    auto descriptor = std::make_shared<AudioDeviceDescriptor>();
    DistributedRoutingInfo distributedRoutingInfo = {
        .descriptor = descriptor,
        .type = CAST_TYPE_NULL
    };
    AudioPolicyService::GetAudioPolicyService().distributedRoutingInfo_ = distributedRoutingInfo;
    StreamUsage streamUsage = STREAM_USAGE_INVALID;
    int32_t clientUID = 1;
    EXPECT_NE(streamFilterRouter_->GetCallRenderDevice(streamUsage, clientUID), nullptr);
}

/**
 * @tc.name  : Test StreamFilterRouter.
 * @tc.number: GetCallRenderDevice_002
 * @tc.desc  : Test GetCallRenderDevice type == CAST_TYPE_ALL.
 */
HWTEST_F(StreamFilterRouterExtUnitTest, GetCallRenderDevice_002, TestSize.Level4)
{
    auto descriptor = std::make_shared<AudioDeviceDescriptor>();
    DistributedRoutingInfo distributedRoutingInfo = {
        .descriptor = descriptor,
        .type = CAST_TYPE_ALL
    };
    AudioPolicyService::GetAudioPolicyService().distributedRoutingInfo_ = distributedRoutingInfo;
    AudioDeviceManager::GetAudioDeviceManager().remoteRenderDevices_.push_back(descriptor);
    int32_t clientUID = 1;
    StreamUsage streamUsage = STREAM_USAGE_INVALID;
    EXPECT_NE(streamFilterRouter_->GetCallRenderDevice(streamUsage, clientUID), nullptr);
}

/**
 * @tc.name  : Test StreamFilterRouter.
 * @tc.number: GetCallRenderDevice_003
 * @tc.desc  : Test GetCallRenderDevice type == CAST_TYPE_PROJECTION.
 */
HWTEST_F(StreamFilterRouterExtUnitTest, GetCallRenderDevice_003, TestSize.Level4)
{
    auto descriptor = std::make_shared<AudioDeviceDescriptor>();
    DistributedRoutingInfo distributedRoutingInfo = {
        .descriptor = descriptor,
        .type = CAST_TYPE_PROJECTION
    };
    AudioPolicyService::GetAudioPolicyService().distributedRoutingInfo_ = distributedRoutingInfo;
    StreamUsage streamUsage = STREAM_USAGE_INVALID;
    int32_t clientUID = 1;
    EXPECT_NE(streamFilterRouter_->GetCallRenderDevice(streamUsage, clientUID), nullptr);
}

/**
 * @tc.name  : Test StreamFilterRouter.
 * @tc.number: GetCallRenderDevice_004
 * @tc.desc  : Test GetCallRenderDevice type == CAST_TYPE_COOPERATION.
 */
HWTEST_F(StreamFilterRouterExtUnitTest, GetCallRenderDevice_004, TestSize.Level4)
{
    auto descriptor = std::make_shared<AudioDeviceDescriptor>();
    DistributedRoutingInfo distributedRoutingInfo = {
        .descriptor = descriptor,
        .type = CAST_TYPE_COOPERATION
    };
    AudioPolicyService::GetAudioPolicyService().distributedRoutingInfo_ = distributedRoutingInfo;
    StreamUsage streamUsage = STREAM_USAGE_INVALID;
    int32_t clientUID = 1;
    EXPECT_NE(streamFilterRouter_->GetCallRenderDevice(streamUsage, clientUID), nullptr);
}

/**
 * @tc.name  : Test StreamFilterRouter.
 * @tc.number: GetCallCaptureDevice_001
 * @tc.desc  : Test GetCallCaptureDevice type == CAST_TYPE_NULL.
 */
HWTEST_F(StreamFilterRouterExtUnitTest, GetCallCaptureDevice_001, TestSize.Level4)
{
    auto descriptor = std::make_shared<AudioDeviceDescriptor>();
    DistributedRoutingInfo distributedRoutingInfo = {
        .descriptor = descriptor,
        .type = CAST_TYPE_NULL
    };
    AudioPolicyService::GetAudioPolicyService().distributedRoutingInfo_ = distributedRoutingInfo;
    SourceType sourceType = SOURCE_TYPE_INVALID;
    int32_t clientUID = 1;
    EXPECT_NE(streamFilterRouter_->GetCallCaptureDevice(sourceType, clientUID), nullptr);
}

/**
 * @tc.name  : Test StreamFilterRouter.
 * @tc.number: GetCallCaptureDevice_002
 * @tc.desc  : Test GetCallCaptureDevice type == CAST_TYPE_ALL.
 */
HWTEST_F(StreamFilterRouterExtUnitTest, GetCallCaptureDevice_002, TestSize.Level4)
{
    auto descriptor = std::make_shared<AudioDeviceDescriptor>();
    DistributedRoutingInfo distributedRoutingInfo = {
        .descriptor = descriptor,
        .type = CAST_TYPE_ALL
    };
    AudioPolicyService::GetAudioPolicyService().distributedRoutingInfo_ = distributedRoutingInfo;
    AudioDeviceManager::GetAudioDeviceManager().remoteRenderDevices_.push_back(descriptor);
    SourceType sourceType = SOURCE_TYPE_INVALID;
    int32_t clientUID = 1;
    EXPECT_NE(streamFilterRouter_->GetCallCaptureDevice(sourceType, clientUID), nullptr);
}

/**
 * @tc.name  : Test StreamFilterRouter.
 * @tc.number: GetCallCaptureDevice_003
 * @tc.desc  : Test GetCallCaptureDevice type == CAST_TYPE_PROJECTION.
 */
HWTEST_F(StreamFilterRouterExtUnitTest, GetCallCaptureDevice_003, TestSize.Level4)
{
    auto descriptor = std::make_shared<AudioDeviceDescriptor>();
    DistributedRoutingInfo distributedRoutingInfo = {
        .descriptor = descriptor,
        .type = CAST_TYPE_PROJECTION
    };
    AudioPolicyService::GetAudioPolicyService().distributedRoutingInfo_ = distributedRoutingInfo;
    SourceType sourceType = SOURCE_TYPE_INVALID;
    int32_t clientUID = 1;
    EXPECT_NE(streamFilterRouter_->GetCallCaptureDevice(sourceType, clientUID), nullptr);
}

/**
 * @tc.name  : Test StreamFilterRouter.
 * @tc.number: GetCallCaptureDevice_004
 * @tc.desc  : Test GetCallCaptureDevice type == CAST_TYPE_COOPERATION.
 */
HWTEST_F(StreamFilterRouterExtUnitTest, GetCallCaptureDevice_004, TestSize.Level4)
{
    auto descriptor = std::make_shared<AudioDeviceDescriptor>();
    DistributedRoutingInfo distributedRoutingInfo = {
        .descriptor = descriptor,
        .type = CAST_TYPE_COOPERATION
    };
    AudioPolicyService::GetAudioPolicyService().distributedRoutingInfo_ = distributedRoutingInfo;
    SourceType sourceType = SOURCE_TYPE_INVALID;
    int32_t clientUID = 1;
    EXPECT_NE(streamFilterRouter_->GetCallCaptureDevice(sourceType, clientUID), nullptr);
}

/**
 * @tc.name  : Test StreamFilterRouter.
 * @tc.number: GetRecordCaptureDevice_001
 * @tc.desc  : Test GetRecordCaptureDevice type == CAST_TYPE_NULL.
 */
HWTEST_F(StreamFilterRouterExtUnitTest, GetRecordCaptureDevice_001, TestSize.Level4)
{
    auto descriptor = std::make_shared<AudioDeviceDescriptor>();
    DistributedRoutingInfo distributedRoutingInfo = {
        .descriptor = descriptor,
        .type = CAST_TYPE_NULL
    };
    AudioPolicyService::GetAudioPolicyService().distributedRoutingInfo_ = distributedRoutingInfo;
    SourceType sourceType = SOURCE_TYPE_INVALID;
    int32_t clientUID = 1;
    uint32_t sessionID = 1;
    EXPECT_NE(streamFilterRouter_->GetRecordCaptureDevice(sourceType, clientUID, sessionID), nullptr);
}

/**
 * @tc.name  : Test StreamFilterRouter.
 * @tc.number: GetRecordCaptureDevice_002
 * @tc.desc  : Test GetRecordCaptureDevice type == CAST_TYPE_ALL.
 */
HWTEST_F(StreamFilterRouterExtUnitTest, GetRecordCaptureDevice_002, TestSize.Level4)
{
    auto descriptor = std::make_shared<AudioDeviceDescriptor>();
    DistributedRoutingInfo distributedRoutingInfo = {
        .descriptor = descriptor,
        .type = CAST_TYPE_ALL
    };
    AudioPolicyService::GetAudioPolicyService().distributedRoutingInfo_ = distributedRoutingInfo;
    SourceType sourceType = SOURCE_TYPE_INVALID;
    int32_t clientUID = 1;
    uint32_t sessionID = 1;
    EXPECT_NE(streamFilterRouter_->GetRecordCaptureDevice(sourceType, clientUID, sessionID), nullptr);
}

/**
 * @tc.name  : Test StreamFilterRouter.
 * @tc.number: GetRecordCaptureDevice_003
 * @tc.desc  : Test GetRecordCaptureDevice type == CAST_TYPE_PROJECTION.
 */
HWTEST_F(StreamFilterRouterExtUnitTest, GetRecordCaptureDevice_003, TestSize.Level4)
{
    auto descriptor = std::make_shared<AudioDeviceDescriptor>();
    descriptor->deviceRole_ = INPUT_DEVICE;
    descriptor->deviceType_ = DEVICE_TYPE_MIC;
    DistributedRoutingInfo distributedRoutingInfo = {
        .descriptor = descriptor,
        .type = CAST_TYPE_PROJECTION
    };
    AudioPolicyService::GetAudioPolicyService().distributedRoutingInfo_ = distributedRoutingInfo;
    SourceType sourceType = SOURCE_TYPE_INVALID;
    int32_t clientUID = 1;
    uint32_t sessionID = 1;
    AudioDeviceManager::GetAudioDeviceManager().remoteRenderDevices_.clear();
    EXPECT_NE(streamFilterRouter_->GetRecordCaptureDevice(sourceType, clientUID, sessionID), nullptr);

    AudioDeviceManager::GetAudioDeviceManager().remoteRenderDevices_.push_back(descriptor);
    sourceType = SOURCE_TYPE_MIC;
    EXPECT_NE(streamFilterRouter_->GetRecordCaptureDevice(sourceType, clientUID, sessionID), nullptr);
}

/**
 * @tc.name  : Test StreamFilterRouter.
 * @tc.number: GetRecordCaptureDevice_004
 * @tc.desc  : Test GetRecordCaptureDevice type == CAST_TYPE_COOPERATION.
 */
HWTEST_F(StreamFilterRouterExtUnitTest, GetRecordCaptureDevice_004, TestSize.Level4)
{
    auto descriptor = std::make_shared<AudioDeviceDescriptor>();
    DistributedRoutingInfo distributedRoutingInfo = {
        .descriptor = descriptor,
        .type = CAST_TYPE_COOPERATION
    };
    AudioPolicyService::GetAudioPolicyService().distributedRoutingInfo_ = distributedRoutingInfo;
    SourceType sourceType = SOURCE_TYPE_INVALID;
    int32_t clientUID = 1;
    uint32_t sessionID = 1;
    EXPECT_NE(streamFilterRouter_->GetRecordCaptureDevice(sourceType, clientUID, sessionID), nullptr);
}

/**
 * @tc.name  : Test StreamFilterRouter.
 * @tc.number: IsIncomingDeviceInRemoteDevice_001
 * @tc.desc  : Test IsIncomingDeviceInRemoteDevice desc == nullptr.
 */
HWTEST_F(StreamFilterRouterExtUnitTest, IsIncomingDeviceInRemoteDevice_001, TestSize.Level4)
{
    vector<shared_ptr<AudioDeviceDescriptor>> descriptors;
    std::shared_ptr<AudioDeviceDescriptor> incomingDevice = nullptr;
    descriptors.push_back(incomingDevice);
    bool ret = streamFilterRouter_->IsIncomingDeviceInRemoteDevice(descriptors, incomingDevice);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name  : Test StreamFilterRouter.
 * @tc.number: IsIncomingDeviceInRemoteDevice_002
 * @tc.desc  : Test IsIncomingDeviceInRemoteDevice desc != nullptr.
 */
HWTEST_F(StreamFilterRouterExtUnitTest, IsIncomingDeviceInRemoteDevice_002, TestSize.Level4)
{
    vector<shared_ptr<AudioDeviceDescriptor>> descriptors;
    auto incomingDevice = std::make_shared<AudioDeviceDescriptor>();
    auto descriptor1 = std::make_shared<AudioDeviceDescriptor>(incomingDevice);
    descriptor1->deviceRole_ = INPUT_DEVICE;
    auto descriptor2 = std::make_shared<AudioDeviceDescriptor>(incomingDevice);
    descriptor2->deviceType_ = DEVICE_TYPE_INVALID;
    auto descriptor3 = std::make_shared<AudioDeviceDescriptor>(incomingDevice);
    descriptor3->interruptGroupId_ = 1;
    auto descriptor4 = std::make_shared<AudioDeviceDescriptor>(incomingDevice);
    descriptor4->volumeGroupId_ = 1;
    auto descriptor5 = std::make_shared<AudioDeviceDescriptor>(incomingDevice);
    descriptor5->networkId_ = "testNetworkId";
    auto descriptor6 = std::make_shared<AudioDeviceDescriptor>(incomingDevice);
    descriptor6->macAddress_ = "00:11:22:33:44:55";
    descriptors = { descriptor1, descriptor2, descriptor3, descriptor4, descriptor5, descriptor6 };
    bool ret = streamFilterRouter_->IsIncomingDeviceInRemoteDevice(descriptors, incomingDevice);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name  : Test StreamFilterRouter.
 * @tc.number: SelectRemoteCaptureDevice_001
 * @tc.desc  : Test SelectRemoteCaptureDevice descriptor == nullptr.
 */
HWTEST_F(StreamFilterRouterExtUnitTest, SelectRemoteCaptureDevice_001, TestSize.Level4)
{
    vector<shared_ptr<AudioDeviceDescriptor>> descriptors;
    std::shared_ptr<AudioDeviceDescriptor> incomingDevice = nullptr;
    descriptors.push_back(incomingDevice);
    bool hasDescriptor = false;
    auto descriptorPtr = streamFilterRouter_->SelectRemoteCaptureDevice(descriptors, incomingDevice, hasDescriptor);
    EXPECT_NE(descriptorPtr, nullptr);
}

/**
 * @tc.name  : Test StreamFilterRouter.
 * @tc.number: SelectRemoteCaptureDevice_002
 * @tc.desc  : Test SelectRemoteCaptureDevice descriptor != nullptr.
 */
HWTEST_F(StreamFilterRouterExtUnitTest, SelectRemoteCaptureDevice_002, TestSize.Level4)
{
    vector<shared_ptr<AudioDeviceDescriptor>> descriptors;
    auto incomingDevice = std::make_shared<AudioDeviceDescriptor>();
    auto descriptor1 = std::make_shared<AudioDeviceDescriptor>(incomingDevice);
    descriptor1->networkId_ = "testNetworkId";
    auto descriptor2 = std::make_shared<AudioDeviceDescriptor>(incomingDevice);
    descriptor2->deviceRole_ = INPUT_DEVICE;
    auto descriptor3 = std::make_shared<AudioDeviceDescriptor>(incomingDevice);
    descriptor3->deviceType_ = DEVICE_TYPE_MIC;
    descriptors = { descriptor1, descriptor2, descriptor3 };
    bool hasDescriptor = false;
    auto descriptorPtr = streamFilterRouter_->SelectRemoteCaptureDevice(descriptors, incomingDevice, hasDescriptor);
    EXPECT_NE(descriptorPtr, nullptr);
}
} // namespace AudioStandard
} // namespace OHOS
