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

#include "audio_policy_server_handler_unit_test.h"
#include "istandard_audio_routing_manager_listener.h"
#include "audio_errors.h"
#include "audio_info.h"
#include "audio_policy_log.h"
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
    std::shared_ptr<AudioPolicyClientHolder> cb = nullptr;
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
    std::shared_ptr<AudioPolicyClientHolder> cb = nullptr;
    audioPolicyServerHandler_->AddAudioPolicyClientProxyMap(clientPid, cb);
    EXPECT_NE(audioPolicyServerHandler_, nullptr);

    std::shared_ptr<AudioPolicyClientHolder> cb2 = nullptr;
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
    std::shared_ptr<AudioPolicyManagerListenerCallback> cb = nullptr;
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
    std::shared_ptr<AudioPolicyManagerListenerCallback> cb = nullptr;
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

/**
 * @tc.name  : Test SendWakeupCloseEvent API
 * @tc.number: SendVolumeKeyEventCallback_001
 * @tc.desc  : Test SendVolumeKeyEventCallback function when volume type is STREAM_VOICE_CALL_ASSISTANT.
 */
HWTEST(AudioPolicyServerHandlerUnitTest, SendVolumeKeyEventCallback_001, TestSize.Level2)
{
    auto audioPolicyServerHandler_ = std::make_shared<AudioPolicyServerHandler>();
    VolumeEvent volumeEvent;
    volumeEvent.volumeType = AudioStreamType::STREAM_VOICE_CALL_ASSISTANT;
    EXPECT_FALSE(audioPolicyServerHandler_->SendVolumeKeyEventCallback(volumeEvent));
}

/**
 * @tc.name  : Test SendWakeupCloseEvent API
 * @tc.number: SendVolumeKeyEventCallback_002
 * @tc.desc  : Test SendVolumeKeyEventCallback function when volume type is not STREAM_VOICE_CALL_ASSISTANT.
 */
HWTEST(AudioPolicyServerHandlerUnitTest, SendVolumeKeyEventCallback_002, TestSize.Level2)
{
    auto audioPolicyServerHandler_ = std::make_shared<AudioPolicyServerHandler>();
    VolumeEvent volumeEvent;
    volumeEvent.volumeType = AudioStreamType::STREAM_DEFAULT;
    EXPECT_TRUE(audioPolicyServerHandler_->SendVolumeKeyEventCallback(volumeEvent));
}

/**
 * @tc.name  : HandleMicrophoneBlockedCallback_Test_001
 * @tc.number: Audio_HandleMicrophoneBlockedCallback_001
 * @tc.desc  : Test HandleMicrophoneBlockedCallback function when eventContextObj is nullptr.
 */
HWTEST(AudioPolicyServerHandlerUnitTest, HandleMicrophoneBlockedCallback_Test_001, TestSize.Level2)
{
    auto audioPolicyServerHandler_ = std::make_shared<AudioPolicyServerHandler>();
    EXPECT_NE(audioPolicyServerHandler_, nullptr);
    AppExecFwk::InnerEvent::Pointer event =
        AppExecFwk::InnerEvent::Get(AudioPolicyServerHandler::EventAudioServerCmd::NN_STATE_CHANGE, 0);
    audioPolicyServerHandler_->HandleMicrophoneBlockedCallback(event);
    EXPECT_EQ(audioPolicyServerHandler_->audioPolicyClientProxyAPSCbsMap_.size(), 0);
}

/**
 * @tc.name  : HandleAvailableDeviceChange_Test_001
 * @tc.number: HandleAvailableDeviceChange_Test_001
 * @tc.desc  : Test SetClientCallbacksEnable function when CallbackChange is CALLBACK_FOCUS_INFO_CHANGE.
 */
HWTEST(AudioPolicyServerHandlerUnitTest, HandleAvailableDeviceChange_Test_001, TestSize.Level2)
{
    auto audioPolicyServerHandler_ = std::make_shared<AudioPolicyServerHandler>();
    EXPECT_NE(audioPolicyServerHandler_, nullptr);
    AppExecFwk::InnerEvent::Pointer event =
        AppExecFwk::InnerEvent::Get(AudioPolicyServerHandler::EventAudioServerCmd::NN_STATE_CHANGE, 1);
    audioPolicyServerHandler_->HandleAvailableDeviceChange(event);
    int32_t ret =
        audioPolicyServerHandler_->SetClientCallbacksEnable(CallbackChange::CALLBACK_SET_MICROPHONE_BLOCKED, false);
    EXPECT_EQ(ret, AUDIO_OK);
}

/**
 * @tc.name  : HandleVolumeKeyEvent_Test_001
 * @tc.number: HandleVolumeKeyEvent_Test_001
 * @tc.desc  : Test HandleVolumeKeyEvent function when eventContextObj is nullptr.
 */
HWTEST(AudioPolicyServerHandlerUnitTest, HandleVolumeKeyEvent_Test_001, TestSize.Level2)
{
    auto audioPolicyServerHandler_ = std::make_shared<AudioPolicyServerHandler>();
    EXPECT_NE(audioPolicyServerHandler_, nullptr);
    AppExecFwk::InnerEvent::Pointer event =
        AppExecFwk::InnerEvent::Get(AudioPolicyServerHandler::EventAudioServerCmd::NN_STATE_CHANGE, 0);
    audioPolicyServerHandler_->HandleVolumeKeyEvent(event);
    EXPECT_EQ(audioPolicyServerHandler_->audioPolicyClientProxyAPSCbsMap_.size(), 0);
}

/**
 * @tc.name  : HandleVolumeKeyEvent_Test_002
 * @tc.number: HandleVolumeKeyEvent_Test_002
 * @tc.desc  : Test HandleVolumeKeyEvent function when CallbackChange is CALLBACK_SET_MICROPHONE_BLOCKED.
 */
HWTEST(AudioPolicyServerHandlerUnitTest, HandleVolumeKeyEvent_Test_002, TestSize.Level2)
{
    auto audioPolicyServerHandler_ = std::make_shared<AudioPolicyServerHandler>();
    EXPECT_NE(audioPolicyServerHandler_, nullptr);

    int32_t clientPid = 1;
    std::shared_ptr<AudioPolicyClientHolder> cb;
    audioPolicyServerHandler_->AddAudioPolicyClientProxyMap(clientPid, cb);

    int32_t ret =
        audioPolicyServerHandler_->SetClientCallbacksEnable(CallbackChange::CALLBACK_SET_MICROPHONE_BLOCKED, false);
    AppExecFwk::InnerEvent::Pointer event =
        AppExecFwk::InnerEvent::Get(AudioPolicyServerHandler::EventAudioServerCmd::NN_STATE_CHANGE, 1);

    audioPolicyServerHandler_->HandleVolumeKeyEvent(event);
    EXPECT_EQ(ret, AUDIO_OK);
}

/**
 * @tc.name  : HandleAudioSessionDeactiveCallback_001
 * @tc.number: HandleAudioSessionDeactiveCallback_001
 * @tc.desc  : Test HandleAudioSessionDeactiveCallback function when eventContextObj is nullptr.
 */
HWTEST(AudioPolicyServerHandlerUnitTest, HandleAudioSessionDeactiveCallback_001, TestSize.Level2)
{
    auto audioPolicyServerHandler_ = std::make_shared<AudioPolicyServerHandler>();
    EXPECT_NE(audioPolicyServerHandler_, nullptr);
    int32_t clientPid = 1;
    std::shared_ptr<AudioPolicyClientHolder> cb;
    audioPolicyServerHandler_->AddAudioPolicyClientProxyMap(clientPid, cb);
    AppExecFwk::InnerEvent::Pointer event =
        AppExecFwk::InnerEvent::Get(AudioPolicyServerHandler::EventAudioServerCmd::NN_STATE_CHANGE, 0);
    audioPolicyServerHandler_->HandleAudioSessionDeactiveCallback(event);
    EXPECT_EQ(audioPolicyServerHandler_->audioPolicyClientProxyAPSCbsMap_.size(), 1);
}

/**
 * @tc.name  : HandleRequestCateGoryEvent_001
 * @tc.number: HandleRequestCateGoryEvent_001
 * @tc.desc  : Test HandleRequestCateGoryEvent function when eventContextObj is nullptr.
 */
HWTEST(AudioPolicyServerHandlerUnitTest, HandleRequestCateGoryEvent_001, TestSize.Level2)
{
    auto audioPolicyServerHandler_ = std::make_shared<AudioPolicyServerHandler>();
    EXPECT_NE(audioPolicyServerHandler_, nullptr);
    int32_t clientPid = 1;
    std::shared_ptr<AudioPolicyClientHolder> cb;
    audioPolicyServerHandler_->AddAudioPolicyClientProxyMap(clientPid, cb);
    AppExecFwk::InnerEvent::Pointer event =
        AppExecFwk::InnerEvent::Get(AudioPolicyServerHandler::EventAudioServerCmd::NN_STATE_CHANGE, 0);
    audioPolicyServerHandler_->HandleRequestCateGoryEvent(event);
    EXPECT_EQ(audioPolicyServerHandler_->audioPolicyClientProxyAPSCbsMap_.size(), 1);
}

/**
 * @tc.name  : HandleAbandonCateGoryEvent_001
 * @tc.number: HandleAbandonCateGoryEvent_001
 * @tc.desc  : Test HandleAbandonCateGoryEvent function when eventContextObj is nullptr.
 */
HWTEST(AudioPolicyServerHandlerUnitTest, HandleAbandonCateGoryEvent_001, TestSize.Level2)
{
    auto audioPolicyServerHandler_ = std::make_shared<AudioPolicyServerHandler>();
    EXPECT_NE(audioPolicyServerHandler_, nullptr);
    int32_t clientPid = 1;
    std::shared_ptr<AudioPolicyClientHolder> cb;
    audioPolicyServerHandler_->AddAudioPolicyClientProxyMap(clientPid, cb);
    AppExecFwk::InnerEvent::Pointer event =
        AppExecFwk::InnerEvent::Get(AudioPolicyServerHandler::EventAudioServerCmd::NN_STATE_CHANGE, 0);
    audioPolicyServerHandler_->HandleAbandonCateGoryEvent(event);
    EXPECT_EQ(audioPolicyServerHandler_->audioPolicyClientProxyAPSCbsMap_.size(), 1);
}

/**
 * @tc.name  : HandleFocusInfoChangeEvent_001
 * @tc.number: HandleFocusInfoChangeEvent_001
 * @tc.desc  : Test HandleFocusInfoChangeEvent function when eventContextObj is nullptr.
 */
HWTEST(AudioPolicyServerHandlerUnitTest, HandleFocusInfoChangeEvent_001, TestSize.Level2)
{
    auto audioPolicyServerHandler_ = std::make_shared<AudioPolicyServerHandler>();
    EXPECT_NE(audioPolicyServerHandler_, nullptr);
    int32_t clientPid = 1;
    std::shared_ptr<AudioPolicyClientHolder> cb;
    audioPolicyServerHandler_->AddAudioPolicyClientProxyMap(clientPid, cb);
    AppExecFwk::InnerEvent::Pointer event =
        AppExecFwk::InnerEvent::Get(AudioPolicyServerHandler::EventAudioServerCmd::NN_STATE_CHANGE, 0);
    audioPolicyServerHandler_->HandleFocusInfoChangeEvent(event);
    EXPECT_EQ(audioPolicyServerHandler_->audioPolicyClientProxyAPSCbsMap_.size(), 1);
}

/**
 * @tc.name  : HandleRingerModeUpdatedEvent_001
 * @tc.number: HandleRingerModeUpdatedEvent_001
 * @tc.desc  : Test HandleRingerModeUpdatedEvent function when eventContextObj is nullptr.
 */
HWTEST(AudioPolicyServerHandlerUnitTest, HandleRingerModeUpdatedEvent_001, TestSize.Level2)
{
    auto audioPolicyServerHandler_ = std::make_shared<AudioPolicyServerHandler>();
    EXPECT_NE(audioPolicyServerHandler_, nullptr);
    int32_t clientPid = 1;
    std::shared_ptr<AudioPolicyClientHolder> cb;
    audioPolicyServerHandler_->AddAudioPolicyClientProxyMap(clientPid, cb);
    AppExecFwk::InnerEvent::Pointer event =
        AppExecFwk::InnerEvent::Get(AudioPolicyServerHandler::EventAudioServerCmd::NN_STATE_CHANGE, 0);
    audioPolicyServerHandler_->HandleRingerModeUpdatedEvent(event);
    EXPECT_EQ(audioPolicyServerHandler_->audioPolicyClientProxyAPSCbsMap_.size(), 1);
}

/**
 * @tc.name  : HandleMicStateUpdatedEvent_001
 * @tc.number: HandleMicStateUpdatedEvent_001
 * @tc.desc  : Test HandleMicStateUpdatedEvent function when eventContextObj is nullptr.
 */
HWTEST(AudioPolicyServerHandlerUnitTest, HandleMicStateUpdatedEvent_001, TestSize.Level2)
{
    auto audioPolicyServerHandler_ = std::make_shared<AudioPolicyServerHandler>();
    EXPECT_NE(audioPolicyServerHandler_, nullptr);
    int32_t clientPid = 1;
    std::shared_ptr<AudioPolicyClientHolder> cb;
    audioPolicyServerHandler_->AddAudioPolicyClientProxyMap(clientPid, cb);
    AppExecFwk::InnerEvent::Pointer event =
        AppExecFwk::InnerEvent::Get(AudioPolicyServerHandler::EventAudioServerCmd::NN_STATE_CHANGE, 0);
    audioPolicyServerHandler_->HandleMicStateUpdatedEvent(event);
    EXPECT_EQ(audioPolicyServerHandler_->audioPolicyClientProxyAPSCbsMap_.size(), 1);
}

/**
 * @tc.name  : HandleMicStateUpdatedEventWithClientId_001
 * @tc.number: HandleMicStateUpdatedEventWithClientId_001
 * @tc.desc  : Test HandleMicStateUpdatedEventWithClientId function when eventContextObj is nullptr.
 */
HWTEST(AudioPolicyServerHandlerUnitTest, HandleMicStateUpdatedEventWithClientId_001, TestSize.Level2)
{
    auto audioPolicyServerHandler_ = std::make_shared<AudioPolicyServerHandler>();
    EXPECT_NE(audioPolicyServerHandler_, nullptr);
    int32_t clientPid = 1;
    std::shared_ptr<AudioPolicyClientHolder> cb;
    audioPolicyServerHandler_->AddAudioPolicyClientProxyMap(clientPid, cb);
    AppExecFwk::InnerEvent::Pointer event =
        AppExecFwk::InnerEvent::Get(AudioPolicyServerHandler::EventAudioServerCmd::NN_STATE_CHANGE, 0);
    audioPolicyServerHandler_->HandleMicStateUpdatedEventWithClientId(event);
    EXPECT_EQ(audioPolicyServerHandler_->audioPolicyClientProxyAPSCbsMap_.size(), 1);
}

/**
 * @tc.name  : HandleInterruptEventWithSessionId_001
 * @tc.number: HandleInterruptEventWithSessionId_001
 * @tc.desc  : Test HandleInterruptEventWithSessionId function when eventContextObj is nullptr.
 */
HWTEST(AudioPolicyServerHandlerUnitTest, HandleInterruptEventWithStreamId_001, TestSize.Level2)
{
    auto audioPolicyServerHandler_ = std::make_shared<AudioPolicyServerHandler>();
    EXPECT_NE(audioPolicyServerHandler_, nullptr);
    int32_t clientPid = 1;
    std::shared_ptr<AudioPolicyClientHolder> cb;
    audioPolicyServerHandler_->AddAudioPolicyClientProxyMap(clientPid, cb);
    AppExecFwk::InnerEvent::Pointer event =
        AppExecFwk::InnerEvent::Get(AudioPolicyServerHandler::EventAudioServerCmd::NN_STATE_CHANGE, 0);
    audioPolicyServerHandler_->HandleInterruptEventWithStreamId(event);
    EXPECT_EQ(audioPolicyServerHandler_->audioPolicyClientProxyAPSCbsMap_.size(), 1);
}

/**
 * @tc.name  : HandleInterruptEventWithClientId_001
 * @tc.number: HandleInterruptEventWithClientId_001
 * @tc.desc  : Test HandleInterruptEventWithSessionId function when eventContextObj is nullptr.
 */
HWTEST(AudioPolicyServerHandlerUnitTest, HandleInterruptEventWithClientId_001, TestSize.Level2)
{
    auto audioPolicyServerHandler_ = std::make_shared<AudioPolicyServerHandler>();
    EXPECT_NE(audioPolicyServerHandler_, nullptr);
    int32_t clientPid = 1;
    std::shared_ptr<AudioPolicyClientHolder> cb;
    audioPolicyServerHandler_->AddAudioPolicyClientProxyMap(clientPid, cb);
    AppExecFwk::InnerEvent::Pointer event =
        AppExecFwk::InnerEvent::Get(AudioPolicyServerHandler::EventAudioServerCmd::NN_STATE_CHANGE, 0);
    audioPolicyServerHandler_->HandleInterruptEventWithClientId(event);
    EXPECT_EQ(audioPolicyServerHandler_->audioPolicyClientProxyAPSCbsMap_.size(), 1);
}

/**
 * @tc.name  : HandlePreferredOutputDeviceUpdated_001
 * @tc.number: HandlePreferredOutputDeviceUpdated_001
 * @tc.desc  : Test HandlePreferredOutputDeviceUpdated function when eventContextObj is nullptr.
 */
HWTEST(AudioPolicyServerHandlerUnitTest, HandlePreferredOutputDeviceUpdated_001, TestSize.Level2)
{
    auto audioPolicyServerHandler_ = std::make_shared<AudioPolicyServerHandler>();
    EXPECT_NE(audioPolicyServerHandler_, nullptr);
    int32_t clientPid = 1;
    std::shared_ptr<AudioPolicyClientHolder> cb;
    audioPolicyServerHandler_->AddAudioPolicyClientProxyMap(clientPid, cb);
    audioPolicyServerHandler_->HandlePreferredOutputDeviceUpdated();
    EXPECT_EQ(audioPolicyServerHandler_->audioPolicyClientProxyAPSCbsMap_.size(), 1);
}

/**
 * @tc.name  : HandlePreferredInputDeviceUpdated_001
 * @tc.number: HandlePreferredInputDeviceUpdated_001
 * @tc.desc  : Test HandleInterruptEventWithSessionId function when eventContextObj is nullptr.
 */
HWTEST(AudioPolicyServerHandlerUnitTest, HandlePreferredInputDeviceUpdated, TestSize.Level2)
{
    auto audioPolicyServerHandler_ = std::make_shared<AudioPolicyServerHandler>();
    EXPECT_NE(audioPolicyServerHandler_, nullptr);
    int32_t clientPid = 1;
    std::shared_ptr<AudioPolicyClientHolder> cb;
    audioPolicyServerHandler_->AddAudioPolicyClientProxyMap(clientPid, cb);
    audioPolicyServerHandler_->HandlePreferredInputDeviceUpdated();
    EXPECT_EQ(audioPolicyServerHandler_->audioPolicyClientProxyAPSCbsMap_.size(), 1);
}

/**
 * @tc.name  : HandleRendererInfoEvent_001
 * @tc.number: HandleRendererInfoEvent_001
 * @tc.desc  : Test HandleInterruptEventWithSessionId function when eventContextObj is nullptr.
 */
HWTEST(AudioPolicyServerHandlerUnitTest, HandleRendererInfoEvent_001, TestSize.Level2)
{
    auto audioPolicyServerHandler_ = std::make_shared<AudioPolicyServerHandler>();
    EXPECT_NE(audioPolicyServerHandler_, nullptr);
    int32_t clientPid = 1;
    std::shared_ptr<AudioPolicyClientHolder> cb;
    audioPolicyServerHandler_->AddAudioPolicyClientProxyMap(clientPid, cb);
    AppExecFwk::InnerEvent::Pointer event =
        AppExecFwk::InnerEvent::Get(AudioPolicyServerHandler::EventAudioServerCmd::NN_STATE_CHANGE, 0);
    int32_t ret =
        audioPolicyServerHandler_->SetClientCallbacksEnable(CallbackChange::CALLBACK_SET_MICROPHONE_BLOCKED, false);
    audioPolicyServerHandler_->HandleRendererInfoEvent(event);
    audioPolicyServerHandler_->SetClientCallbacksEnable(CallbackChange::CALLBACK_RENDERER_STATE_CHANGE, true);
    audioPolicyServerHandler_->HandleRendererInfoEvent(event);
    EXPECT_EQ(audioPolicyServerHandler_->audioPolicyClientProxyAPSCbsMap_.size(), 1);
}

/**
 * @tc.name  : HandleCapturerInfoEvent_001
 * @tc.number: HandleCapturerInfoEvent_001
 * @tc.desc  : Test HandleInterruptEventWithSessionId function when eventContextObj is nullptr.
 */
HWTEST(AudioPolicyServerHandlerUnitTest, HandleCapturerInfoEvent_001, TestSize.Level2)
{
    auto audioPolicyServerHandler_ = std::make_shared<AudioPolicyServerHandler>();
    EXPECT_NE(audioPolicyServerHandler_, nullptr);
    int32_t clientPid = 1;
    std::shared_ptr<AudioPolicyClientHolder> cb;
    audioPolicyServerHandler_->AddAudioPolicyClientProxyMap(clientPid, cb);
    AppExecFwk::InnerEvent::Pointer event =
        AppExecFwk::InnerEvent::Get(AudioPolicyServerHandler::EventAudioServerCmd::NN_STATE_CHANGE, 0);
    int32_t ret =
        audioPolicyServerHandler_->SetClientCallbacksEnable(CallbackChange::CALLBACK_SET_MICROPHONE_BLOCKED, false);
    audioPolicyServerHandler_->HandleCapturerInfoEvent(event);
    audioPolicyServerHandler_->SetClientCallbacksEnable(CallbackChange::CALLBACK_CAPTURER_STATE_CHANGE, true);
    audioPolicyServerHandler_->HandleCapturerInfoEvent(event);
    EXPECT_EQ(audioPolicyServerHandler_->audioPolicyClientProxyAPSCbsMap_.size(), 1);
}

/**
 * @tc.name  : HandleRendererDeviceChangeEvent_001
 * @tc.number: HandleRendererDeviceChangeEvent_001
 * @tc.desc  : Test HandleInterruptEventWithSessionId function when eventContextObj is nullptr.
 */
HWTEST(AudioPolicyServerHandlerUnitTest, HandleRendererDeviceChangeEvent_001, TestSize.Level2)
{
    auto audioPolicyServerHandler_ = std::make_shared<AudioPolicyServerHandler>();
    EXPECT_NE(audioPolicyServerHandler_, nullptr);
    int32_t clientPid = 1;
    std::shared_ptr<AudioPolicyClientHolder> cb;
    audioPolicyServerHandler_->AddAudioPolicyClientProxyMap(clientPid, cb);
    AppExecFwk::InnerEvent::Pointer event =
        AppExecFwk::InnerEvent::Get(AudioPolicyServerHandler::EventAudioServerCmd::NN_STATE_CHANGE, 0);
    int32_t ret =
        audioPolicyServerHandler_->SetClientCallbacksEnable(CallbackChange::CALLBACK_SET_MICROPHONE_BLOCKED, false);
    audioPolicyServerHandler_->HandleRendererDeviceChangeEvent(event);
    audioPolicyServerHandler_->SetClientCallbacksEnable(CallbackChange::CALLBACK_CAPTURER_STATE_CHANGE, true);
    audioPolicyServerHandler_->HandleRendererDeviceChangeEvent(event);
    EXPECT_EQ(audioPolicyServerHandler_->audioPolicyClientProxyAPSCbsMap_.size(), 1);
}

/**
 * @tc.name  : HandleSendRecreateRendererStreamEvent_001
 * @tc.number: HandleSendRecreateRendererStreamEvent_001
 * @tc.desc  : Test HandleInterruptEventWithSessionId function when eventContextObj is nullptr.
 */
HWTEST(AudioPolicyServerHandlerUnitTest, HandleSendRecreateRendererStreamEvent_001, TestSize.Level2)
{
    auto audioPolicyServerHandler_ = std::make_shared<AudioPolicyServerHandler>();
    EXPECT_NE(audioPolicyServerHandler_, nullptr);
    int32_t clientPid = 1;
    std::shared_ptr<AudioPolicyClientHolder> cb;
    audioPolicyServerHandler_->AddAudioPolicyClientProxyMap(clientPid, cb);
    AppExecFwk::InnerEvent::Pointer event =
        AppExecFwk::InnerEvent::Get(AudioPolicyServerHandler::EventAudioServerCmd::NN_STATE_CHANGE, 0);
    int32_t ret =
        audioPolicyServerHandler_->SetClientCallbacksEnable(CallbackChange::CALLBACK_SET_MICROPHONE_BLOCKED, false);
    audioPolicyServerHandler_->HandleSendRecreateRendererStreamEvent(event);
    audioPolicyServerHandler_->SetClientCallbacksEnable(CallbackChange::CALLBACK_DEVICE_CHANGE_WITH_INFO, true);
    audioPolicyServerHandler_->HandleSendRecreateRendererStreamEvent(event);
    EXPECT_EQ(audioPolicyServerHandler_->audioPolicyClientProxyAPSCbsMap_.size(), 1);
}

/**
 * @tc.name  : HandleSendRecreateCapturerStreamEvent_001
 * @tc.number: HandleSendRecreateCapturerStreamEvent_001
 * @tc.desc  : Test HandleInterruptEventWithSessionId function when eventContextObj is nullptr.
 */
HWTEST(AudioPolicyServerHandlerUnitTest, HandleSendRecreateCapturerStreamEvent_001, TestSize.Level2)
{
    auto audioPolicyServerHandler_ = std::make_shared<AudioPolicyServerHandler>();
    EXPECT_NE(audioPolicyServerHandler_, nullptr);
    int32_t clientPid = 1;
    std::shared_ptr<AudioPolicyClientHolder> cb;
    audioPolicyServerHandler_->AddAudioPolicyClientProxyMap(clientPid, cb);
    AppExecFwk::InnerEvent::Pointer event =
        AppExecFwk::InnerEvent::Get(AudioPolicyServerHandler::EventAudioServerCmd::NN_STATE_CHANGE, 0);
    int32_t ret =
        audioPolicyServerHandler_->SetClientCallbacksEnable(CallbackChange::CALLBACK_SET_MICROPHONE_BLOCKED, false);
    audioPolicyServerHandler_->HandleSendRecreateCapturerStreamEvent(event);
    audioPolicyServerHandler_->SetClientCallbacksEnable(CallbackChange::CALLBACK_DEVICE_CHANGE_WITH_INFO, true);
    audioPolicyServerHandler_->HandleSendRecreateCapturerStreamEvent(event);
    EXPECT_EQ(audioPolicyServerHandler_->audioPolicyClientProxyAPSCbsMap_.size(), 1);
}

/**
 * @tc.name  : HandleNnStateChangeEvent_001
 * @tc.number: HandleNnStateChangeEvent_001
 * @tc.desc  : Test HandleInterruptEventWithSessionId function when eventContextObj is nullptr.
 */
HWTEST(AudioPolicyServerHandlerUnitTest, HandleNnStateChangeEvent_001, TestSize.Level2)
{
    auto audioPolicyServerHandler_ = std::make_shared<AudioPolicyServerHandler>();
    EXPECT_NE(audioPolicyServerHandler_, nullptr);
    int32_t clientPid = 1;
    std::shared_ptr<AudioPolicyClientHolder> cb;
    audioPolicyServerHandler_->AddAudioPolicyClientProxyMap(clientPid, cb);
    AppExecFwk::InnerEvent::Pointer event =
        AppExecFwk::InnerEvent::Get(AudioPolicyServerHandler::EventAudioServerCmd::NN_STATE_CHANGE, 0);
    int32_t ret =
        audioPolicyServerHandler_->SetClientCallbacksEnable(CallbackChange::CALLBACK_SET_MICROPHONE_BLOCKED, false);
    audioPolicyServerHandler_->HandleNnStateChangeEvent(event);
    audioPolicyServerHandler_->SetClientCallbacksEnable(CallbackChange::CALLBACK_NN_STATE_CHANGE, true);
    audioPolicyServerHandler_->HandleNnStateChangeEvent(event);
    EXPECT_EQ(audioPolicyServerHandler_->audioPolicyClientProxyAPSCbsMap_.size(), 1);
}

/**
 * @tc.name  : HandleHeadTrackingDeviceChangeEvent_001
 * @tc.number: HandleHeadTrackingDeviceChangeEvent_001
 * @tc.desc  : Test HandleInterruptEventWithSessionId function when eventContextObj is nullptr.
 */
HWTEST(AudioPolicyServerHandlerUnitTest, HandleHeadTrackingDeviceChangeEvent_001, TestSize.Level2)
{
    auto audioPolicyServerHandler_ = std::make_shared<AudioPolicyServerHandler>();
    EXPECT_NE(audioPolicyServerHandler_, nullptr);
    int32_t clientPid = 1;
    std::shared_ptr<AudioPolicyClientHolder> cb;
    audioPolicyServerHandler_->AddAudioPolicyClientProxyMap(clientPid, cb);
    AppExecFwk::InnerEvent::Pointer event =
        AppExecFwk::InnerEvent::Get(AudioPolicyServerHandler::EventAudioServerCmd::NN_STATE_CHANGE, 0);
    int32_t ret =
        audioPolicyServerHandler_->SetClientCallbacksEnable(CallbackChange::CALLBACK_SET_MICROPHONE_BLOCKED, false);
    audioPolicyServerHandler_->HandleHeadTrackingDeviceChangeEvent(event);
    audioPolicyServerHandler_->SetClientCallbacksEnable(
        CallbackChange::CALLBACK_HEAD_TRACKING_DATA_REQUESTED_CHANGE, true);
    audioPolicyServerHandler_->HandleHeadTrackingDeviceChangeEvent(event);
    EXPECT_EQ(audioPolicyServerHandler_->audioPolicyClientProxyAPSCbsMap_.size(), 1);
}

/**
 * @tc.name  : HandleSpatializatonEnabledChangeEvent_001
 * @tc.number: HandleSpatializatonEnabledChangeEvent_001
 * @tc.desc  : Test HandleInterruptEventWithSessionId function when eventContextObj is nullptr.
 */
HWTEST(AudioPolicyServerHandlerUnitTest, HandleSpatializatonEnabledChangeEvent_001, TestSize.Level2)
{
    auto audioPolicyServerHandler_ = std::make_shared<AudioPolicyServerHandler>();
    EXPECT_NE(audioPolicyServerHandler_, nullptr);
    int32_t clientPid = 1;
    std::shared_ptr<AudioPolicyClientHolder> cb;
    audioPolicyServerHandler_->AddAudioPolicyClientProxyMap(clientPid, cb);
    AppExecFwk::InnerEvent::Pointer event =
        AppExecFwk::InnerEvent::Get(AudioPolicyServerHandler::EventAudioServerCmd::NN_STATE_CHANGE, 0);
    int32_t ret =
        audioPolicyServerHandler_->SetClientCallbacksEnable(CallbackChange::CALLBACK_SET_MICROPHONE_BLOCKED, false);
    audioPolicyServerHandler_->HandleSpatializatonEnabledChangeEvent(event);
    audioPolicyServerHandler_->SetClientCallbacksEnable(
        CallbackChange::CALLBACK_SPATIALIZATION_ENABLED_CHANGE, true);
    audioPolicyServerHandler_->HandleSpatializatonEnabledChangeEvent(event);
    EXPECT_EQ(audioPolicyServerHandler_->audioPolicyClientProxyAPSCbsMap_.size(), 1);
}

/**
 * @tc.name  : HandleSpatializatonEnabledChangeForAnyDeviceEvent_001
 * @tc.number: HandleSpatializatonEnabledChangeForAnyDeviceEvent_001
 * @tc.desc  : Test HandleInterruptEventWithSessionId function when eventContextObj is nullptr.
 */
HWTEST(AudioPolicyServerHandlerUnitTest, HandleSpatializatonEnabledChangeForAnyDeviceEvent_001, TestSize.Level2)
{
    auto audioPolicyServerHandler_ = std::make_shared<AudioPolicyServerHandler>();
    EXPECT_NE(audioPolicyServerHandler_, nullptr);
    int32_t clientPid = 1;
    std::shared_ptr<AudioPolicyClientHolder> cb;
    audioPolicyServerHandler_->AddAudioPolicyClientProxyMap(clientPid, cb);
    AppExecFwk::InnerEvent::Pointer event =
        AppExecFwk::InnerEvent::Get(AudioPolicyServerHandler::EventAudioServerCmd::NN_STATE_CHANGE, 0);
    int32_t ret =
        audioPolicyServerHandler_->SetClientCallbacksEnable(CallbackChange::CALLBACK_SET_MICROPHONE_BLOCKED, false);
    audioPolicyServerHandler_->HandleSpatializatonEnabledChangeForAnyDeviceEvent(event);
    audioPolicyServerHandler_->SetClientCallbacksEnable(
        CallbackChange::CALLBACK_SPATIALIZATION_ENABLED_CHANGE, true);
    audioPolicyServerHandler_->HandleSpatializatonEnabledChangeForAnyDeviceEvent(event);
    EXPECT_EQ(audioPolicyServerHandler_->audioPolicyClientProxyAPSCbsMap_.size(), 1);
}

/**
 * @tc.name  : HandleHeadTrackingEnabledChangeEvent_001
 * @tc.number: HandleHeadTrackingEnabledChangeEvent_001
 * @tc.desc  : Test HandleInterruptEventWithSessionId function when eventContextObj is nullptr.
 */
HWTEST(AudioPolicyServerHandlerUnitTest, HandleHeadTrackingEnabledChangeEvent_001, TestSize.Level2)
{
    auto audioPolicyServerHandler_ = std::make_shared<AudioPolicyServerHandler>();
    EXPECT_NE(audioPolicyServerHandler_, nullptr);
    int32_t clientPid = 1;
    std::shared_ptr<AudioPolicyClientHolder> cb;
    audioPolicyServerHandler_->AddAudioPolicyClientProxyMap(clientPid, cb);
    AppExecFwk::InnerEvent::Pointer event =
        AppExecFwk::InnerEvent::Get(AudioPolicyServerHandler::EventAudioServerCmd::NN_STATE_CHANGE, 0);
    int32_t ret =
        audioPolicyServerHandler_->SetClientCallbacksEnable(CallbackChange::CALLBACK_SET_MICROPHONE_BLOCKED, false);
    audioPolicyServerHandler_->HandleHeadTrackingEnabledChangeEvent(event);
    audioPolicyServerHandler_->SetClientCallbacksEnable(
        CallbackChange::CALLBACK_HEAD_TRACKING_ENABLED_CHANGE, true);
    audioPolicyServerHandler_->HandleHeadTrackingEnabledChangeEvent(event);
    EXPECT_EQ(audioPolicyServerHandler_->audioPolicyClientProxyAPSCbsMap_.size(), 1);
}

/**
 * @tc.name  : HandleHeadTrackingEnabledChangeForAnyDeviceEvent_001
 * @tc.number: HandleHeadTrackingEnabledChangeForAnyDeviceEvent_001
 * @tc.desc  : Test HandleInterruptEventWithSessionId function when eventContextObj is nullptr.
 */
HWTEST(AudioPolicyServerHandlerUnitTest, HandleHeadTrackingEnabledChangeForAnyDeviceEvent_001, TestSize.Level2)
{
    auto audioPolicyServerHandler_ = std::make_shared<AudioPolicyServerHandler>();
    EXPECT_NE(audioPolicyServerHandler_, nullptr);
    int32_t clientPid = 1;
    std::shared_ptr<AudioPolicyClientHolder> cb;
    audioPolicyServerHandler_->AddAudioPolicyClientProxyMap(clientPid, cb);
    AppExecFwk::InnerEvent::Pointer event =
        AppExecFwk::InnerEvent::Get(AudioPolicyServerHandler::EventAudioServerCmd::NN_STATE_CHANGE, 0);
    int32_t ret =
        audioPolicyServerHandler_->SetClientCallbacksEnable(CallbackChange::CALLBACK_SET_MICROPHONE_BLOCKED, false);
    audioPolicyServerHandler_->HandleHeadTrackingEnabledChangeForAnyDeviceEvent(event);
    audioPolicyServerHandler_->SetClientCallbacksEnable(
        CallbackChange::CALLBACK_HEAD_TRACKING_ENABLED_CHANGE, true);
    audioPolicyServerHandler_->HandleHeadTrackingEnabledChangeForAnyDeviceEvent(event);
    EXPECT_EQ(audioPolicyServerHandler_->audioPolicyClientProxyAPSCbsMap_.size(), 1);
}

/**
 * @tc.name  : HandleOtherServiceEvent_001
 * @tc.number: HandleOtherServiceEvent_001
 * @tc.desc  : Test HandleInterruptEventWithSessionId function when eventContextObj is nullptr.
 */
HWTEST(AudioPolicyServerHandlerUnitTest, HandleOtherServiceEvent_001, TestSize.Level2)
{
    auto audioPolicyServerHandler_ = std::make_shared<AudioPolicyServerHandler>();
    EXPECT_NE(audioPolicyServerHandler_, nullptr);
    int32_t clientPid = 1;
    std::shared_ptr<AudioPolicyClientHolder> cb;
    audioPolicyServerHandler_->AddAudioPolicyClientProxyMap(clientPid, cb);
    AppExecFwk::InnerEvent::Pointer event =
        AppExecFwk::InnerEvent::Get(AudioPolicyServerHandler::EventAudioServerCmd::NN_STATE_CHANGE, 0);
    int32_t ret =
        audioPolicyServerHandler_->SetClientCallbacksEnable(CallbackChange::CALLBACK_SET_MICROPHONE_BLOCKED, false);
    audioPolicyServerHandler_->SetClientCallbacksEnable(
        CallbackChange::CALLBACK_HEAD_TRACKING_ENABLED_CHANGE, true);
    uint32_t eventId = AudioPolicyServerHandler::EventAudioServerCmd::PREFERRED_OUTPUT_DEVICE_UPDATED;
    audioPolicyServerHandler_->HandleOtherServiceEvent(eventId, event);
    eventId = AudioPolicyServerHandler::EventAudioServerCmd::NN_STATE_CHANGE;
    audioPolicyServerHandler_->HandleOtherServiceEvent(eventId, event);
    EXPECT_EQ(audioPolicyServerHandler_->audioPolicyClientProxyAPSCbsMap_.size(), 1);
}

/**
 * @tc.name  : ProcessEvent_001
 * @tc.number: ProcessEvent_001
 * @tc.desc  : Test HandleInterruptEventWithSessionId function when eventContextObj is nullptr.
 */
HWTEST(AudioPolicyServerHandlerUnitTest, ProcessEvent_001, TestSize.Level2)
{
    auto audioPolicyServerHandler_ = std::make_shared<AudioPolicyServerHandler>();
    EXPECT_NE(audioPolicyServerHandler_, nullptr);
    int32_t clientPid = 1;
    std::shared_ptr<AudioPolicyClientHolder> cb;
    audioPolicyServerHandler_->AddAudioPolicyClientProxyMap(clientPid, cb);
    AppExecFwk::InnerEvent::Pointer event =
        AppExecFwk::InnerEvent::Get(AudioPolicyServerHandler::EventAudioServerCmd::NN_STATE_CHANGE, 0);
    int32_t ret =
        audioPolicyServerHandler_->SetClientCallbacksEnable(CallbackChange::CALLBACK_SET_MICROPHONE_BLOCKED, false);
    audioPolicyServerHandler_->SetClientCallbacksEnable(CallbackChange::CALLBACK_HEAD_TRACKING_ENABLED_CHANGE, true);
    event->innerEventId_ = AudioPolicyServerHandler::EventAudioServerCmd::VOLUME_KEY_EVENT;
    audioPolicyServerHandler_->ProcessEvent(event);
    event->innerEventId_ = AudioPolicyServerHandler::EventAudioServerCmd::REQUEST_CATEGORY_EVENT;
    audioPolicyServerHandler_->ProcessEvent(event);
    event->innerEventId_ = AudioPolicyServerHandler::EventAudioServerCmd::ABANDON_CATEGORY_EVENT;
    audioPolicyServerHandler_->ProcessEvent(event);
    event->innerEventId_ = AudioPolicyServerHandler::EventAudioServerCmd::RINGER_MODEUPDATE_EVENT;
    audioPolicyServerHandler_->ProcessEvent(event);
    event->innerEventId_ = AudioPolicyServerHandler::EventAudioServerCmd::MIC_STATE_CHANGE_EVENT;
    audioPolicyServerHandler_->ProcessEvent(event);
    event->innerEventId_ = AudioPolicyServerHandler::EventAudioServerCmd::MIC_STATE_CHANGE_EVENT_WITH_CLIENTID;
    audioPolicyServerHandler_->ProcessEvent(event);
    event->innerEventId_ = AudioPolicyServerHandler::EventAudioServerCmd::INTERRUPT_EVENT;
    audioPolicyServerHandler_->ProcessEvent(event);
    event->innerEventId_ = AudioPolicyServerHandler::EventAudioServerCmd::INTERRUPT_EVENT_WITH_CLIENTID;
    audioPolicyServerHandler_->ProcessEvent(event);
    event->innerEventId_ = AudioPolicyServerHandler::EventAudioServerCmd::DISTRIBUTED_ROUTING_ROLE_CHANGE;
    audioPolicyServerHandler_->ProcessEvent(event);
    event->innerEventId_ = AudioPolicyServerHandler::EventAudioServerCmd::HEAD_TRACKING_DEVICE_CHANGE;
    audioPolicyServerHandler_->ProcessEvent(event);
    EXPECT_EQ(audioPolicyServerHandler_->audioPolicyClientProxyAPSCbsMap_.size(), 1);
}

/**
 * @tc.name  : SetClientCallbacksEnable_Test_001
 * @tc.number: SetClientCallbacksEnable_Test_001
 * @tc.desc  : Test SetClientCallbacksEnable function.
 */
HWTEST(AudioPolicyServerHandlerUnitTest, SetClientCallbacksEnable_Test_001, TestSize.Level2)
{
    auto audioPolicyServerHandler_ = std::make_shared<AudioPolicyServerHandler>();
    EXPECT_NE(audioPolicyServerHandler_, nullptr);
    bool enable = true;
    int32_t ret =
        audioPolicyServerHandler_->SetClientCallbacksEnable(CallbackChange::CALLBACK_FOCUS_INFO_CHANGE, enable);
    EXPECT_EQ(ret, AUDIO_OK);

    CallbackChange callbackChange = static_cast<CallbackChange>(CallbackChange::CALLBACK_MAX + 1);
    ret = audioPolicyServerHandler_->SetClientCallbacksEnable(callbackChange, enable);
    EXPECT_EQ(ret, AUDIO_ERR);

    callbackChange = static_cast<CallbackChange>(CallbackChange::CALLBACK_UNKNOWN - 1);
    ret = audioPolicyServerHandler_->SetClientCallbacksEnable(callbackChange, enable);
    EXPECT_EQ(ret, AUDIO_ERR);

    callbackChange = CallbackChange::CALLBACK_SET_RINGER_MODE;
    enable = false;
    ret = audioPolicyServerHandler_->SetClientCallbacksEnable(callbackChange, enable);
    EXPECT_EQ(ret, AUDIO_OK);
}

/**
 * @tc.name  : GetCallbackRendererInfoList_001
 * @tc.number: GetCallbackRendererInfoList_001
 * @tc.desc  : Test GetCallbackRendererInfoList method when clientPid is not found in the map.
 */
HWTEST(AudioPolicyServerHandlerUnitTest, GetCallbackRendererInfoList_001, TestSize.Level1)
{
    auto audioPolicyServerHandler_ = std::make_shared<AudioPolicyServerHandler>();
    EXPECT_NE(audioPolicyServerHandler_, nullptr);
    int32_t clientPid = 123;
    audioPolicyServerHandler_->GetCallbackRendererInfoList(clientPid);
    EXPECT_EQ(audioPolicyServerHandler_->audioPolicyClientProxyAPSCbsMap_.size(), 0);
}

/**
 * @tc.name  : GetCallbackRendererInfoList_002
 * @tc.number: GetCallbackRendererInfoList_002
 * @tc.desc  : Test GetCallbackRendererInfoList method when clientPid is found in the map.
 */
HWTEST(AudioPolicyServerHandlerUnitTest, GetCallbackRendererInfoList_002, TestSize.Level1)
{
    auto audioPolicyServerHandler_ = std::make_shared<AudioPolicyServerHandler>();
    EXPECT_NE(audioPolicyServerHandler_, nullptr);
    int32_t clientPid = 123;
    std::vector<AudioRendererInfo> infoList = {AudioRendererInfo()};
    audioPolicyServerHandler_->clientCbRendererInfoMap_[clientPid] = infoList;
    audioPolicyServerHandler_->GetCallbackRendererInfoList(clientPid);
    EXPECT_EQ(audioPolicyServerHandler_->audioPolicyClientProxyAPSCbsMap_.size(), 0);
}

/**
 * @tc.name  : GetCallbackCapturerInfoList_001
 * @tc.number: GetCallbackCapturerInfoList_001
 * @tc.desc  : Test GetCallbackCapturerInfoList method when clientPid is not found in the map.
 */
HWTEST(AudioPolicyServerHandlerUnitTest, GetCallbackCapturerInfoList_001, TestSize.Level1)
{
    auto audioPolicyServerHandler_ = std::make_shared<AudioPolicyServerHandler>();
    EXPECT_NE(audioPolicyServerHandler_, nullptr);
    int32_t clientPid = 123;
    audioPolicyServerHandler_->GetCallbackCapturerInfoList(clientPid);
    EXPECT_EQ(audioPolicyServerHandler_->audioPolicyClientProxyAPSCbsMap_.size(), 0);
}

/**
 * @tc.name  : GetCallbackCapturerInfoList_002
 * @tc.number: GetCallbackCapturerInfoList_002
 * @tc.desc  : Test GetCallbackCapturerInfoList method when clientPid is found in the map.
 */
HWTEST(AudioPolicyServerHandlerUnitTest, GetCallbackCapturerInfoList_002, TestSize.Level1)
{
    auto audioPolicyServerHandler_ = std::make_shared<AudioPolicyServerHandler>();
    EXPECT_NE(audioPolicyServerHandler_, nullptr);
    int32_t clientPid = 123;
    std::vector<AudioCapturerInfo> infoList = {AudioCapturerInfo()};
    audioPolicyServerHandler_->clientCbCapturerInfoMap_[clientPid] = infoList;
    audioPolicyServerHandler_->GetCallbackCapturerInfoList(clientPid);
    EXPECT_EQ(audioPolicyServerHandler_->audioPolicyClientProxyAPSCbsMap_.size(), 0);
}

/**
 * @tc.name  : SetCallbackCapturerInfo_001
 * @tc.number: SetCallbackCapturerInfo_001
 * @tc.desc  : Test SetCallbackCapturerInfo method when set audioCapturerInfo into clientCbCapturerInfoMap_.
 */
HWTEST(AudioPolicyServerHandlerUnitTest, SetCallbackCapturerInfo_001, TestSize.Level1)
{
    auto audioPolicyServerHandler_ = std::make_shared<AudioPolicyServerHandler>();
    EXPECT_NE(audioPolicyServerHandler_, nullptr);
    AudioCapturerInfo audioCapturerInfo;
    int32_t ret = audioPolicyServerHandler_->SetCallbackCapturerInfo(audioCapturerInfo);
    EXPECT_EQ(ret, AUDIO_OK);
}

/**
 * @tc.name  : SetCallbackRendererInfo_001
 * @tc.number: SetCallbackRendererInfo_001
 * @tc.desc  : Test SetCallbackRendererInfo method when set SetCallbackRendererInfo into clientCbRendererInfoMap_.
 */
HWTEST(AudioPolicyServerHandlerUnitTest, SetCallbackRendererInfo_001, TestSize.Level1)
{
    auto audioPolicyServerHandler_ = std::make_shared<AudioPolicyServerHandler>();
    EXPECT_NE(audioPolicyServerHandler_, nullptr);
    AudioRendererInfo audioRendererInfo;
    int32_t ret = audioPolicyServerHandler_->SetCallbackRendererInfo(audioRendererInfo);
    EXPECT_EQ(ret, AUDIO_OK);
}

/**
 * @tc.name  : AddExternInterruptCbsMap_001
 * @tc.number: AddExternInterruptCbsMap_001
 * @tc.desc  : Test AddExternInterruptCbsMap method when add audioInterruptCallback into amInterruptCbsMap_.
 */
HWTEST(AudioPolicyServerHandlerUnitTest, AddExternInterruptCbsMap_001, TestSize.Level1)
{
    auto audioPolicyServerHandler_ = std::make_shared<AudioPolicyServerHandler>();
    EXPECT_NE(audioPolicyServerHandler_, nullptr);
    int32_t clientPid = 123;
    std::shared_ptr<AudioInterruptCallback> audioInterruptCallback = nullptr;
    audioPolicyServerHandler_->AddExternInterruptCbsMap(clientPid, audioInterruptCallback);
    EXPECT_EQ(audioPolicyServerHandler_->amInterruptCbsMap_[clientPid], audioInterruptCallback);
}

/**
 * @tc.name  : SendMicrophoneBlockedCallback_001
 * @tc.number: SendMicrophoneBlockedCallback_001
 * @tc.desc  : Test SendMicrophoneBlockedCallback method when send MicrophoneBlockedCallback.
 */
HWTEST(AudioPolicyServerHandlerUnitTest, SendMicrophoneBlockedCallback_001, TestSize.Level1)
{
    auto audioPolicyServerHandler_ = std::make_shared<AudioPolicyServerHandler>();
    EXPECT_NE(audioPolicyServerHandler_, nullptr);
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> descForCb = {};
    DeviceBlockStatus status = DEVICE_UNBLOCKED;
    bool ret = audioPolicyServerHandler_->SendMicrophoneBlockedCallback(descForCb, status);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name  : SendMicStateUpdatedCallback_001
 * @tc.number: SendMicStateUpdatedCallback_001
 * @tc.desc  : Test SendMicStateUpdatedCallback method when send MicStateUpdatedCallback.
 */
HWTEST(AudioPolicyServerHandlerUnitTest, SendMicStateUpdatedCallback_001, TestSize.Level1)
{
    auto audioPolicyServerHandler_ = std::make_shared<AudioPolicyServerHandler>();
    EXPECT_NE(audioPolicyServerHandler_, nullptr);
    MicStateChangeEvent micStateChangeEvent;
    micStateChangeEvent.mute = false;
    bool ret = audioPolicyServerHandler_->SendMicStateUpdatedCallback(micStateChangeEvent);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name  : SendMicStateWithClientIdCallback_001
 * @tc.number: SendMicStateWithClientIdCallback_001
 * @tc.desc  : Test SendMicStateWithClientIdCallback method when send MicStateWithClientIdCallback.
 */
HWTEST(AudioPolicyServerHandlerUnitTest, SendMicStateWithClientIdCallback_001, TestSize.Level1)
{
    auto audioPolicyServerHandler_ = std::make_shared<AudioPolicyServerHandler>();
    EXPECT_NE(audioPolicyServerHandler_, nullptr);
    int32_t clientPid = 123;
    MicStateChangeEvent micStateChangeEvent;
    micStateChangeEvent.mute = false;
    bool ret = audioPolicyServerHandler_->SendMicStateWithClientIdCallback(micStateChangeEvent, clientPid);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name  : SendInterruptEventInternalCallback_001
 * @tc.number: SendInterruptEventInternalCallback_001
 * @tc.desc  : Test SendInterruptEventInternalCallback method when send InterruptEventInternalCallback.
 */
HWTEST(AudioPolicyServerHandlerUnitTest, SendInterruptEventInternalCallback_001, TestSize.Level1)
{
    auto audioPolicyServerHandler_ = std::make_shared<AudioPolicyServerHandler>();
    EXPECT_NE(audioPolicyServerHandler_, nullptr);
    InterruptEventInternal interruptEventInternal;
    bool ret = audioPolicyServerHandler_->SendInterruptEventInternalCallback(interruptEventInternal);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name  : SendInterruptEventWithClientIdCallback_001
 * @tc.number: SendInterruptEventWithClientIdCallback_001
 * @tc.desc  : Test SendInterruptEventWithClientIdCallback method when send InterruptEventWithClientIdCallback.
 */
HWTEST(AudioPolicyServerHandlerUnitTest, SendInterruptEventWithClientIdCallback_001, TestSize.Level1)
{
    auto audioPolicyServerHandler_ = std::make_shared<AudioPolicyServerHandler>();
    EXPECT_NE(audioPolicyServerHandler_, nullptr);
    InterruptEventInternal interruptEvent = {};
    interruptEvent.eventType = INTERRUPT_TYPE_END;
    interruptEvent.forceType = INTERRUPT_SHARE;
    interruptEvent.hintType = INTERRUPT_HINT_NONE;
    interruptEvent.duckVolume = 0;
    int32_t clientPid = 123;
    bool ret = audioPolicyServerHandler_->SendInterruptEventWithClientIdCallback(interruptEvent, clientPid);
    EXPECT_EQ(ret, true);
}
/**
 * @tc.name  : AudioPolicyServerHandlerUnitTest_001
 * @tc.number: AudioPolicyServerHandlerUnitTest_001
 * @tc.desc  : Test HandleInterruptEventWithSessionId function when eventContextObj is nullptr.
 */
HWTEST(AudioPolicyServerHandlerUnitTest, AudioPolicyServerHandlerUnitTest_001, TestSize.Level2)
{
    auto audioPolicyServerHandler_ = std::make_shared<AudioPolicyServerHandler>();
    EXPECT_NE(audioPolicyServerHandler_, nullptr);
    int32_t clientPid = 1;
    audioPolicyServerHandler_->RemoveAudioPolicyClientProxyMap(clientPid);
    EXPECT_NE(audioPolicyServerHandler_, nullptr);
}
/**
 * @tc.name  : AudioPolicyServerHandlerUnitTest_002
 * @tc.number: AudioPolicyServerHandlerUnitTest_002
 * @tc.desc  : Test HandleInterruptEventWithSessionId function when eventContextObj is nullptr.
 */
HWTEST(AudioPolicyServerHandlerUnitTest, AudioPolicyServerHandlerUnitTest_002, TestSize.Level2)
{
    auto audioPolicyServerHandler_ = std::make_shared<AudioPolicyServerHandler>();
    EXPECT_NE(audioPolicyServerHandler_, nullptr);
    CastType type = CAST_TYPE_NULL;
    std::shared_ptr<AudioDeviceDescriptor> descriptor = std::make_shared<AudioDeviceDescriptor>();
    audioPolicyServerHandler_->SendDistributedRoutingRoleChange(descriptor, CAST_TYPE_NULL);
    EXPECT_NE(audioPolicyServerHandler_, nullptr);
}
/**
 * @tc.name  : AudioPolicyServerHandlerUnitTest_003
 * @tc.number: AudioPolicyServerHandlerUnitTest_003
 * @tc.desc  : Test HandleInterruptEventWithSessionId function when eventContextObj is nullptr.
 */
HWTEST(AudioPolicyServerHandlerUnitTest, AudioPolicyServerHandlerUnitTest_003, TestSize.Level2)
{
    auto audioPolicyServerHandler_ = std::make_shared<AudioPolicyServerHandler>();
    EXPECT_NE(audioPolicyServerHandler_, nullptr);
    int32_t clientPid = 1;
    uint64_t sessionId = 0;
    AudioDeviceDescriptor outputDeviceInfo;
    AudioStreamDeviceChangeReasonExt reason = AudioStreamDeviceChangeReasonExt::ExtEnum::UNKNOWN;
    bool ret = audioPolicyServerHandler_->SendRendererDeviceChangeEvent(clientPid,
        sessionId, outputDeviceInfo, reason);
    EXPECT_NE(ret, false);
}
/**
 * @tc.name  : AudioPolicyServerHandlerUnitTest_004
 * @tc.number: AudioPolicyServerHandlerUnitTest_004
 * @tc.desc  : Test HandleInterruptEventWithSessionId function when eventContextObj is nullptr.
 */
HWTEST(AudioPolicyServerHandlerUnitTest, AudioPolicyServerHandlerUnitTest_004, TestSize.Level2)
{
    auto audioPolicyServerHandler_ = std::make_shared<AudioPolicyServerHandler>();
    EXPECT_NE(audioPolicyServerHandler_, nullptr);
    int32_t clientPid = 1;
    uint64_t sessionId = 0;
    int32_t streamFlag = 0;
    AudioStreamDeviceChangeReasonExt reason = AudioStreamDeviceChangeReasonExt::ExtEnum::UNKNOWN;
    bool ret = audioPolicyServerHandler_->SendRecreateRendererStreamEvent(clientPid,
        sessionId, streamFlag, reason);
    EXPECT_NE(ret, false);
}
/**
 * @tc.name  : AudioPolicyServerHandlerUnitTest_005
 * @tc.number: AudioPolicyServerHandlerUnitTest_005
 * @tc.desc  : Test HandleInterruptEventWithSessionId function when eventContextObj is nullptr.
 */
HWTEST(AudioPolicyServerHandlerUnitTest, AudioPolicyServerHandlerUnitTest_005, TestSize.Level2)
{
    auto audioPolicyServerHandler_ = std::make_shared<AudioPolicyServerHandler>();
    EXPECT_NE(audioPolicyServerHandler_, nullptr);
    int32_t clientPid = 1;
    uint64_t sessionId = 0;
    int32_t streamFlag = 0;
    AudioStreamDeviceChangeReasonExt reason = AudioStreamDeviceChangeReasonExt::ExtEnum::UNKNOWN;
    bool ret = audioPolicyServerHandler_->SendRecreateCapturerStreamEvent(clientPid,
        sessionId, streamFlag, reason);
    EXPECT_NE(ret, false);
}
/**
 * @tc.name  : AudioPolicyServerHandlerUnitTest_006
 * @tc.number: AudioPolicyServerHandlerUnitTest_006
 * @tc.desc  : Test HandleInterruptEventWithSessionId function when eventContextObj is nullptr.
 */
HWTEST(AudioPolicyServerHandlerUnitTest, AudioPolicyServerHandlerUnitTest_006, TestSize.Level2)
{
    auto audioPolicyServerHandler_ = std::make_shared<AudioPolicyServerHandler>();
    EXPECT_NE(audioPolicyServerHandler_, nullptr);
    std::unordered_map<std::string, bool> changeInfo;
    bool ret = audioPolicyServerHandler_->SendHeadTrackingDeviceChangeEvent(changeInfo);
    EXPECT_NE(ret, false);
}
/**
 * @tc.name  : AudioPolicyServerHandlerUnitTest_007
 * @tc.number: AudioPolicyServerHandlerUnitTest_007
 * @tc.desc  : Test HandleInterruptEventWithSessionId function when eventContextObj is nullptr.
 */
HWTEST(AudioPolicyServerHandlerUnitTest, AudioPolicyServerHandlerUnitTest_007, TestSize.Level2)
{
    auto audioPolicyServerHandler_ = std::make_shared<AudioPolicyServerHandler>();
    EXPECT_NE(audioPolicyServerHandler_, nullptr);
    AudioPipeType pipeType = PIPE_TYPE_UNKNOWN;
    bool ret = audioPolicyServerHandler_->SendPipeStreamCleanEvent(PIPE_TYPE_UNKNOWN);
    EXPECT_NE(ret, false);
}

/**
 * @tc.name  : SendFormatUnsupportedErrorEvent_001
 * @tc.number: SendFormatUnsupportedErrorEvent_001
 * @tc.desc  : Test SendFormatUnsupportedErrorEvent method when send FormatUnsupportedError.
 */
HWTEST(AudioPolicyServerHandlerUnitTest, SendFormatUnsupportedErrorEvent_001, TestSize.Level1)
{
    auto audioPolicyServerHandler_ = std::make_shared<AudioPolicyServerHandler>();
    EXPECT_NE(audioPolicyServerHandler_, nullptr);
    AudioErrors errorCode = ERROR_UNSUPPORTED_FORMAT;
    bool ret = audioPolicyServerHandler_->SendFormatUnsupportedErrorEvent(errorCode);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name  : HandleFormatUnsupportedErrorEvent_001
 * @tc.number: HandleFormatUnsupportedErrorEvent_001
 * @tc.desc  : Test HandleFormatUnsupportedErrorEvent function when eventContextObj is nullptr.
 */
HWTEST(AudioPolicyServerHandlerUnitTest, HandleFormatUnsupportedErrorEvent_001, TestSize.Level2)
{
    auto audioPolicyServerHandler_ = std::make_shared<AudioPolicyServerHandler>();
    EXPECT_NE(audioPolicyServerHandler_, nullptr);
    int32_t clientPid = 1;
    std::shared_ptr<AudioPolicyClientHolder> cb;
    audioPolicyServerHandler_->AddAudioPolicyClientProxyMap(clientPid, cb);
    AppExecFwk::InnerEvent::Pointer event =
        AppExecFwk::InnerEvent::Get(AudioPolicyServerHandler::EventAudioServerCmd::FORMAT_UNSUPPORTED_ERROR, 0);
    audioPolicyServerHandler_->HandleFormatUnsupportedErrorEvent(event);
    EXPECT_EQ(audioPolicyServerHandler_->audioPolicyClientProxyAPSCbsMap_.size(), 1);
}
} // namespace AudioStandard
} // namespace OHOS
