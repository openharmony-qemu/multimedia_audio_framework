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

#include "../include/stream_filter_router_unit_test.h"
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

const int32_t SYSTEM_ABILITY_ID = 3009;
const bool RUN_ON_CREATE = false;
bool g_isInit = false;
static AudioPolicyServer g_server(SYSTEM_ABILITY_ID, RUN_ON_CREATE);


void StreamFilterRouterUnitTest::SetUpTestCase(void) {}
void StreamFilterRouterUnitTest::TearDownTestCase(void) {}
void StreamFilterRouterUnitTest::SetUp(void) {}
void StreamFilterRouterUnitTest::TearDown(void)
{
    g_server.OnStop();
    g_isInit = false;
}

AudioPolicyServer *GetServerPtr()
{
    if (!g_isInit) {
        g_server.OnStart();
        g_server.OnAddSystemAbility(DISTRIBUTED_KV_DATA_SERVICE_ABILITY_ID, "");
        g_server.OnAddSystemAbility(DISTRIBUTED_HARDWARE_DEVICEMANAGER_SA_ID, "");
#ifdef FEATURE_MULTIMODALINPUT_INPUT
        g_server.OnAddSystemAbility(MULTIMODAL_INPUT_SERVICE_ID, "");
#endif
        g_server.OnAddSystemAbility(COMMON_EVENT_SERVICE_ID, "");
        g_server.OnAddSystemAbility(AUDIO_DISTRIBUTED_SERVICE_ID, "");
        g_server.OnAddSystemAbility(MULTIMODAL_INPUT_SERVICE_ID, "");
        g_server.OnAddSystemAbility(BLUETOOTH_HOST_SYS_ABILITY_ID, "");
        g_server.OnAddSystemAbility(POWER_MANAGER_SERVICE_ID, "");
        g_server.OnAddSystemAbility(SUBSYS_ACCOUNT_SYS_ABILITY_ID_BEGIN, "");
        g_server.OnAddSystemAbility(ACCESSIBILITY_MANAGER_SERVICE_ID, "");
#ifdef USB_ENABLE
        g_server.OnAddSystemAbility(USB_SYSTEM_ABILITY_ID, "");
#endif
        g_server.audioPolicyService_.SetDefaultDeviceLoadFlag(true);
        g_isInit = true;
    }
    return &g_server;
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
    auto server = GetServerPtr();
    int32_t callerPid = IPCSkeleton::GetCallingPid();
    std::cout<<callerPid<<std::endl;
    server->audioPolicyService_.StoreDistributedRoutingRoleInfo(deviceDescriptor, type);
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
    auto server = GetServerPtr();
    server->audioPolicyService_.StoreDistributedRoutingRoleInfo(deviceDescriptor, type);
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
    auto server = GetServerPtr();
    server->audioPolicyService_.StoreDistributedRoutingRoleInfo(deviceDescriptor, type);
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
    auto server = GetServerPtr();
    server->audioPolicyService_.StoreDistributedRoutingRoleInfo(deviceDescriptor, type);
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
    auto server = GetServerPtr();
    server->audioPolicyService_.StoreDistributedRoutingRoleInfo(deviceDescriptor, type);
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
    auto server = GetServerPtr();
    server->audioPolicyService_.StoreDistributedRoutingRoleInfo(deviceDescriptor, type);
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
    auto server = GetServerPtr();
    server->audioPolicyService_.StoreDistributedRoutingRoleInfo(deviceDescriptor, type);
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
    auto server = GetServerPtr();
    server->audioPolicyService_.StoreDistributedRoutingRoleInfo(deviceDescriptor, type);
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
    auto server = GetServerPtr();
    server->audioPolicyService_.StoreDistributedRoutingRoleInfo(deviceDescriptor, type);
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
    auto server = GetServerPtr();
    server->audioPolicyService_.StoreDistributedRoutingRoleInfo(deviceDescriptor, type);
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
    auto server = GetServerPtr();
    server->audioPolicyService_.StoreDistributedRoutingRoleInfo(deviceDescriptor, type);
    auto result = streamFilterRouter_->GetCallRenderDevice(streamUsage, clientId);
    EXPECT_NE(streamFilterRouter_, nullptr);

    result = streamFilterRouter_->GetMediaRenderDevice(streamUsage, clientId);
    EXPECT_NE(streamFilterRouter_, nullptr);

    result = streamFilterRouter_->GetCallCaptureDevice(sourceType, clientId);
    EXPECT_NE(streamFilterRouter_, nullptr);

    result = streamFilterRouter_->GetRecordCaptureDevice(sourceType, clientId);
    EXPECT_NE(streamFilterRouter_, nullptr);
}

} // namespace AudioStandard
} // namespace OHOS
