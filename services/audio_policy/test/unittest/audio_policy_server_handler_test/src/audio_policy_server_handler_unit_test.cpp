/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "../include/audio_policy_server_handler_unit_test.h"
#include "audio_errors.h"
#include "audio_info.h"
#include "audio_policy_log.h"
#include "audio_utils.h"
#include "audio_policy_service.h"
#include "inner_event.h"
#include "event_handler.h"

#include <thread>
#include <memory>
#include <vector>

using namespace testing::ext;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace AudioStandard {
const int32_t CLIENT_ID = 10;

void AudioPolicyServerHandlerUnitTest::SetUpTestCase(void) {}
void AudioPolicyServerHandlerUnitTest::TearDownTestCase(void) {}
void AudioPolicyServerHandlerUnitTest::SetUp(void) {}
void AudioPolicyServerHandlerUnitTest::TearDown(void) {}

/**
 * @tc.name  : Test AddAudioPolicyClientProxyMap API
 * @tc.type  : FUNC
 * @tc.number: AddAudioPolicyClientProxyMap_001
 * @tc.desc  : Test AddAudioPolicyClientProxyMap interface.
 */
HWTEST(AudioPolicyServerHandlerUnitTest, AddAudioPolicyClientProxyMap_001, TestSize.Level1)
{
    auto audioPolicyServerHandler_ = std::make_shared<AudioPolicyServerHandler>();
    int32_t clientPid = 1;
    sptr<IAudioPolicyClient> cb = nullptr;
    audioPolicyServerHandler_->AddAudioPolicyClientProxyMap(clientPid, cb);
    EXPECT_NE(audioPolicyServerHandler_, nullptr);

    audioPolicyServerHandler_->AddAudioPolicyClientProxyMap(clientPid, cb);
    EXPECT_NE(audioPolicyServerHandler_, nullptr);
}

/**
 * @tc.name  : Test AddAudioPolicyClientProxyMap API
 * @tc.type  : FUNC
 * @tc.number: AddAudioPolicyClientProxyMap_002
 * @tc.desc  : Test AddAudioPolicyClientProxyMap interface.
 */
HWTEST(AudioPolicyServerHandlerUnitTest, AddAudioPolicyClientProxyMap_002, TestSize.Level1)
{
    auto audioPolicyServerHandler_ = std::make_shared<AudioPolicyServerHandler>();
    int32_t clientPid = 1;
    sptr<IAudioPolicyClient> cb = nullptr;
    audioPolicyServerHandler_->AddAudioPolicyClientProxyMap(clientPid, cb);
    EXPECT_NE(audioPolicyServerHandler_, nullptr);

    sptr<IAudioPolicyClient> cb2 = nullptr;
    audioPolicyServerHandler_->AddAudioPolicyClientProxyMap(clientPid, cb2);
    EXPECT_NE(audioPolicyServerHandler_, nullptr);
}


/**
 * @tc.name  : Test RemoveAvailableDeviceChangeMap API
 * @tc.type  : FUNC
 * @tc.number: RemoveAvailableDeviceChangeMap_001
 * @tc.desc  : Test RemoveAvailableDeviceChangeMap interface.
 */
HWTEST(AudioPolicyServerHandlerUnitTest, RemoveAvailableDeviceChangeMap_001, TestSize.Level1)
{
    auto audioPolicyServerHandler_ = std::make_shared<AudioPolicyServerHandler>();
    int32_t clientPid = 1;
    AudioDeviceUsage usage = AudioDeviceUsage::ALL_CALL_DEVICES;
    audioPolicyServerHandler_->RemoveAvailableDeviceChangeMap(clientPid, usage);
    EXPECT_NE(audioPolicyServerHandler_, nullptr);
}

/**
 * @tc.name  : Test RemoveAvailableDeviceChangeMap API
 * @tc.type  : FUNC
 * @tc.number: RemoveAvailableDeviceChangeMap_002
 * @tc.desc  : Test RemoveAvailableDeviceChangeMap interface.
 */
HWTEST(AudioPolicyServerHandlerUnitTest, RemoveAvailableDeviceChangeMap_002, TestSize.Level1)
{
    auto audioPolicyServerHandler_ = std::make_shared<AudioPolicyServerHandler>();
    int32_t clientPid = 1;
    AudioDeviceUsage usage = AudioDeviceUsage::D_ALL_DEVICES;
    sptr<IStandardAudioPolicyManagerListener> cb = nullptr;
    audioPolicyServerHandler_->AddAvailableDeviceChangeMap(clientPid, AudioDeviceUsage::ALL_CALL_DEVICES, cb);
    audioPolicyServerHandler_->AddAvailableDeviceChangeMap(clientPid, AudioDeviceUsage::ALL_MEDIA_DEVICES, cb);
    audioPolicyServerHandler_->RemoveAvailableDeviceChangeMap(clientPid, usage);
    EXPECT_NE(audioPolicyServerHandler_, nullptr);
}

/**
 * @tc.name  : Test RemoveAvailableDeviceChangeMap API
 * @tc.type  : FUNC
 * @tc.number: RemoveAvailableDeviceChangeMap_003
 * @tc.desc  : Test RemoveAvailableDeviceChangeMap interface.
 */
HWTEST(AudioPolicyServerHandlerUnitTest, RemoveAvailableDeviceChangeMap_003, TestSize.Level1)
{
    auto audioPolicyServerHandler_ = std::make_shared<AudioPolicyServerHandler>();
    int32_t clientPid = CLIENT_ID;
    AudioDeviceUsage usage = AudioDeviceUsage::ALL_CALL_DEVICES;
    sptr<IStandardAudioPolicyManagerListener> cb = nullptr;
    audioPolicyServerHandler_->AddAvailableDeviceChangeMap(1, AudioDeviceUsage::ALL_CALL_DEVICES, cb);
    audioPolicyServerHandler_->AddAvailableDeviceChangeMap(1, AudioDeviceUsage::ALL_MEDIA_DEVICES, cb);
    audioPolicyServerHandler_->AddAvailableDeviceChangeMap(clientPid, AudioDeviceUsage::CALL_INPUT_DEVICES, cb);
    audioPolicyServerHandler_->RemoveAvailableDeviceChangeMap(clientPid, usage);
    EXPECT_NE(audioPolicyServerHandler_, nullptr);
}

/**
 * @tc.name  : Test AddDistributedRoutingRoleChangeCbsMap API
 * @tc.type  : FUNC
 * @tc.number: AddDistributedRoutingRoleChangeCbsMap_001
 * @tc.desc  : Test AddDistributedRoutingRoleChangeCbsMap interface.
 */
HWTEST(AudioPolicyServerHandlerUnitTest, AddDistributedRoutingRoleChangeCbsMap_001, TestSize.Level1)
{
    auto audioPolicyServerHandler_ = std::make_shared<AudioPolicyServerHandler>();
    int32_t clientPid = 1;
    sptr<IStandardAudioRoutingManagerListener> cb = nullptr;
    audioPolicyServerHandler_->AddDistributedRoutingRoleChangeCbsMap(clientPid, cb);
    EXPECT_NE(audioPolicyServerHandler_, nullptr);
}

/**
 * @tc.name  : Test RemoveDistributedRoutingRoleChangeCbsMap API
 * @tc.type  : FUNC
 * @tc.number: RemoveDistributedRoutingRoleChangeCbsMap_001
 * @tc.desc  : Test RemoveDistributedRoutingRoleChangeCbsMap interface.
 */
HWTEST(AudioPolicyServerHandlerUnitTest, RemoveDistributedRoutingRoleChangeCbsMap_001, TestSize.Level1)
{
    auto audioPolicyServerHandler_ = std::make_shared<AudioPolicyServerHandler>();
    int32_t clientPid = CLIENT_ID;
    int32_t ret = audioPolicyServerHandler_->RemoveDistributedRoutingRoleChangeCbsMap(clientPid);
    EXPECT_EQ(ret, ERR_INVALID_OPERATION);
}

/**
 * @tc.name  : Test SendCapturerCreateEvent API
 * @tc.type  : FUNC
 * @tc.number: SendCapturerCreateEvent_001
 * @tc.desc  : Test SendCapturerCreateEvent interface.
 */
HWTEST(AudioPolicyServerHandlerUnitTest, SendCapturerCreateEvent_001, TestSize.Level1)
{
    auto audioPolicyServerHandler_ = std::make_shared<AudioPolicyServerHandler>();
    AudioCapturerInfo capturerInfo;
    AudioStreamInfo streamInfo;
    uint64_t sessionId = 0;
    bool isSync = false;
    int32_t error = 0;
    int32_t ret =
        audioPolicyServerHandler_->SendCapturerCreateEvent(capturerInfo, streamInfo, sessionId, isSync, error);
    EXPECT_NE(ret, ERR_INVALID_OPERATION);
}

/**
 * @tc.name  : Test SendCapturerCreateEvent API
 * @tc.type  : FUNC
 * @tc.number: SendCapturerCreateEvent_002
 * @tc.desc  : Test SendCapturerCreateEvent interface.
 */
HWTEST(AudioPolicyServerHandlerUnitTest, SendCapturerCreateEvent_002, TestSize.Level1)
{
    auto audioPolicyServerHandler_ = std::make_shared<AudioPolicyServerHandler>();
    AudioCapturerInfo capturerInfo;
    AudioStreamInfo streamInfo;
    uint64_t sessionId = 0;
    bool isSync = true;
    int32_t error = 0;
    int32_t ret =
        audioPolicyServerHandler_->SendCapturerCreateEvent(capturerInfo, streamInfo, sessionId, isSync, error);
    EXPECT_NE(ret, ERR_INVALID_OPERATION);
}

/**
 * @tc.name  : Test SendCapturerRemovedEvent API
 * @tc.type  : FUNC
 * @tc.number: SendCapturerRemovedEvent_001
 * @tc.desc  : Test SendCapturerRemovedEvent interface.
 */
HWTEST(AudioPolicyServerHandlerUnitTest, SendCapturerRemovedEvent_001, TestSize.Level1)
{
    auto audioPolicyServerHandler_ = std::make_shared<AudioPolicyServerHandler>();
    uint64_t sessionId = 0;
    bool isSync = true;
    bool ret = audioPolicyServerHandler_->SendCapturerRemovedEvent(sessionId, isSync);
    EXPECT_NE(ret, ERR_INVALID_OPERATION);
}

/**
 * @tc.name  : Test SendCapturerRemovedEvent API
 * @tc.type  : FUNC
 * @tc.number: SendCapturerRemovedEvent_002
 * @tc.desc  : Test SendCapturerRemovedEvent interface.
 */
HWTEST(AudioPolicyServerHandlerUnitTest, SendCapturerRemovedEvent_002, TestSize.Level1)
{
    auto audioPolicyServerHandler_ = std::make_shared<AudioPolicyServerHandler>();
    uint64_t sessionId = 0;
    bool isSync = false;
    bool ret = audioPolicyServerHandler_->SendCapturerRemovedEvent(sessionId, isSync);
    EXPECT_NE(ret, ERR_INVALID_OPERATION);
}

/**
 * @tc.name  : Test SendWakeupCloseEvent API
 * @tc.type  : FUNC
 * @tc.number: SendWakeupCloseEvent_001
 * @tc.desc  : Test SendWakeupCloseEvent interface.
 */
HWTEST(AudioPolicyServerHandlerUnitTest, SendWakeupCloseEvent_001, TestSize.Level1)
{
    auto audioPolicyServerHandler_ = std::make_shared<AudioPolicyServerHandler>();
    bool isSync = true;
    bool ret = audioPolicyServerHandler_->SendWakeupCloseEvent(isSync);
    EXPECT_NE(ret, ERR_INVALID_OPERATION);
}

/**
 * @tc.name  : Test SendWakeupCloseEvent API
 * @tc.type  : FUNC
 * @tc.number: SendWakeupCloseEvent_002
 * @tc.desc  : Test SendWakeupCloseEvent interface.
 */
HWTEST(AudioPolicyServerHandlerUnitTest, SendWakeupCloseEvent_002, TestSize.Level1)
{
    auto audioPolicyServerHandler_ = std::make_shared<AudioPolicyServerHandler>();
    bool isSync = false;
    bool ret = audioPolicyServerHandler_->SendWakeupCloseEvent(isSync);
    EXPECT_NE(ret, ERR_INVALID_OPERATION);
}

} // namespace AudioStandard
} // namespace OHOS
