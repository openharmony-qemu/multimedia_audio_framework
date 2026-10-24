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
#include "audio_policy_server_handler.h"
#include "audio_policy_client_holder.h"
#include "mock_audio_policy_client.h"

#include <thread>
#include <memory>
#include <vector>

using namespace testing::ext;
using testing::_;
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
 * @tc.name  : Test SendWakeupCloseEvent API
 * @tc.number: SendVolumeDegreeEventCallback_001
 * @tc.desc  : Test SendVolumeDegreeEventCallback function when volume type is STREAM_VOICE_CALL_ASSISTANT.
 */
HWTEST(AudioPolicyServerHandlerUnitTest, SendVolumeDegreeEventCallback_001, TestSize.Level2)
{
    auto audioPolicyServerHandler_ = std::make_shared<AudioPolicyServerHandler>();
    VolumeEvent volumeEvent;
    volumeEvent.volumeType = AudioStreamType::STREAM_VOICE_CALL_ASSISTANT;
    EXPECT_FALSE(audioPolicyServerHandler_->SendVolumeDegreeEventCallback(volumeEvent));
}

/**
 * @tc.name  : HandleVolumeDegreeEvent_Test_001
 * @tc.number: HandleVolumeDegreeEvent_Test_001
 * @tc.desc  : Test HandleVolumeKeyEvent function
 */
HWTEST(AudioPolicyServerHandlerUnitTest, HandleVolumeDegreeEvent_Test_001, TestSize.Level2)
{
    auto audioPolicyServerHandler_ = std::make_shared<AudioPolicyServerHandler>();
    ASSERT_NE(audioPolicyServerHandler_, nullptr);

    auto eventContextObj = std::make_shared<AudioPolicyServerHandler::EventContextObj>();
    ASSERT_NE(eventContextObj, nullptr);
    eventContextObj->volumeEvent.volumeType = STREAM_SYSTEM;
    AppExecFwk::InnerEvent::Pointer event = AppExecFwk::InnerEvent::Get(
        AudioPolicyServerHandler::EventAudioServerCmd::VOLUME_DEGREE_EVENT, eventContextObj);

    VolumeUtils::SetPCVolumeEnable(true);
    auto volumeChangeCb = std::make_shared<AudioPolicyClientHolder>(nullptr);
    ASSERT_NE(volumeChangeCb, nullptr);
    volumeChangeCb->hasSystemPermission_ = false;
    int32_t clientPid = IPCSkeleton::GetCallingPid();
    audioPolicyServerHandler_->SetClientCallbacksEnable(
        CallbackChange::CALLBACK_SET_VOLUME_DEGREE_CHANGE, true);
    audioPolicyServerHandler_->AddAudioPolicyClientProxyMap(clientPid, nullptr);
    EXPECT_NO_THROW(audioPolicyServerHandler_->HandleVolumeDegreeEvent(event));
    audioPolicyServerHandler_->RemoveAudioPolicyClientProxyMap(clientPid);

    audioPolicyServerHandler_->SetClientCallbacksEnable(
        CallbackChange::CALLBACK_SET_VOLUME_DEGREE_CHANGE, true);
    audioPolicyServerHandler_->AddAudioPolicyClientProxyMap(clientPid, volumeChangeCb);
    EXPECT_NO_THROW(audioPolicyServerHandler_->HandleVolumeDegreeEvent(event));

    volumeChangeCb->hasSystemPermission_ = true;
    EXPECT_NO_THROW(audioPolicyServerHandler_->HandleVolumeDegreeEvent(event));
    VolumeUtils::SetPCVolumeEnable(false);
}

/**
 * @tc.name  : HandleCollaborationEnabledChangeForCurrentDeviceEvent_Test_001
 * @tc.number: HandleCollaborationEnabledChangeForCurrentDeviceEvent_Test_001
 * @tc.desc  : Test HandleCollaborationEnabledChangeForCurrentDeviceEvent function
 */
HWTEST(AudioPolicyServerHandlerUnitTest, HandleCollaborationEnabledChangeForCurrentDeviceEvent_Test_001,
    TestSize.Level2)
{
    auto audioPolicyServerHandler_ = std::make_shared<AudioPolicyServerHandler>();
    ASSERT_NE(audioPolicyServerHandler_, nullptr);

    auto eventContextObj = std::make_shared<AudioPolicyServerHandler::EventContextObj>();
    ASSERT_NE(eventContextObj, nullptr);
    eventContextObj->collaborationEnabled = true;
    AppExecFwk::InnerEvent::Pointer event = AppExecFwk::InnerEvent::Get(
        AudioPolicyServerHandler::EventAudioServerCmd::COLLABORATION_ENABLED_CHANGE_FOR_CURRENT_DEVICE,
        eventContextObj);

    bool testEnabled = true;
    audioPolicyServerHandler_->SendCollaborationEnabledChangeForCurrentDeviceEvent(testEnabled);
    audioPolicyServerHandler_->HandleOtherServiceSecondEvent(AudioPolicyServerHandler::EventAudioServerCmd::
        COLLABORATION_ENABLED_CHANGE_FOR_CURRENT_DEVICE, event);
    audioPolicyServerHandler_->HandleCollaborationEnabledChangeForCurrentDeviceEvent(event);
    std::shared_ptr<AudioPolicyClientHolder> testHolder = nullptr;
    audioPolicyServerHandler_->audioPolicyClientProxyAPSCbsMap_[1] = testHolder;
    EXPECT_EQ(audioPolicyServerHandler_->audioPolicyClientProxyAPSCbsMap_.size() > 0, true);

    std::unordered_map<CallbackChange, bool> testCallBackChangeMap;
    testCallBackChangeMap[CALLBACK_COLLABORATION_ENABLED_CHANGE_FOR_CURRENT_DEVICE] = true;
    audioPolicyServerHandler_->clientCallbacksMap_[1] = testCallBackChangeMap;
    audioPolicyServerHandler_->HandleCollaborationEnabledChangeForCurrentDeviceEvent(event);

    audioPolicyServerHandler_->clientCallbacksMap_.clear();
    audioPolicyServerHandler_->HandleCollaborationEnabledChangeForCurrentDeviceEvent(event);
    testCallBackChangeMap.clear();
    audioPolicyServerHandler_->clientCallbacksMap_[1] = testCallBackChangeMap;
    audioPolicyServerHandler_->HandleCollaborationEnabledChangeForCurrentDeviceEvent(event);
    audioPolicyServerHandler_->clientCallbacksMap_.clear();
    testCallBackChangeMap[CALLBACK_COLLABORATION_ENABLED_CHANGE_FOR_CURRENT_DEVICE] = false;
    audioPolicyServerHandler_->clientCallbacksMap_[1] = testCallBackChangeMap;
    audioPolicyServerHandler_->HandleCollaborationEnabledChangeForCurrentDeviceEvent(event);
    audioPolicyServerHandler_->audioPolicyClientProxyAPSCbsMap_[1] = nullptr;
    audioPolicyServerHandler_->HandleCollaborationEnabledChangeForCurrentDeviceEvent(event);

    audioPolicyServerHandler_->audioPolicyClientProxyAPSCbsMap_.clear();
    audioPolicyServerHandler_->clientCallbacksMap_.clear();
    EXPECT_EQ(audioPolicyServerHandler_->audioPolicyClientProxyAPSCbsMap_.size() > 0, false);
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
 * @tc.name  : HandleVolumeKeyEvent_Test_003
 * @tc.number: HandleVolumeKeyEvent_Test_003
 * @tc.desc  : Test HandleVolumeKeyEvent function when volume type is STREAM_ULTRASONIC
 */
HWTEST(AudioPolicyServerHandlerUnitTest, HandleVolumeKeyEvent_Test_003, TestSize.Level2)
{
    auto audioPolicyServerHandler_ = std::make_shared<AudioPolicyServerHandler>();
    EXPECT_NE(audioPolicyServerHandler_, nullptr);

    int32_t clientPid = 1;
    std::shared_ptr<AudioPolicyClientHolder> cb;
    audioPolicyServerHandler_->AddAudioPolicyClientProxyMap(clientPid, cb);

    VolumeEvent volumeEvent;
    volumeEvent.volumeType = STREAM_ULTRASONIC;
    volumeEvent.volume = 1;
    volumeEvent.updateUi = true;
    volumeEvent.volumeGroupId = 0;
    volumeEvent.networkId = LOCAL_NETWORK_ID;
    int32_t ret = audioPolicyServerHandler_->SendVolumeKeyEventCallback(volumeEvent);

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
    AppExecFwk::InnerEvent::Pointer event = AppExecFwk::InnerEvent::Get(
    AudioPolicyServerHandler::EventAudioServerCmd::PREFERRED_OUTPUT_DEVICE_UPDATED);
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
 * @tc.name  : HandlePreferredDeviceSet_001
 * @tc.number: HandlePreferredDeviceSet_001
 * @tc.desc  : Test HandlePreferredDeviceSet function when eventContextObj is nullptr.
 */
HWTEST(AudioPolicyServerHandlerUnitTest, HandlePreferredDeviceSet, TestSize.Level2)
{
    auto audioPolicyServerHandler_ = std::make_shared<AudioPolicyServerHandler>();
    EXPECT_NE(audioPolicyServerHandler_, nullptr);
    int32_t clientPid = 1;
    std::shared_ptr<AudioPolicyClientHolder> cb = nullptr;
    audioPolicyServerHandler_->AddAudioPolicyClientProxyMap(clientPid, cb);
    AppExecFwk::InnerEvent::Pointer event = AppExecFwk::InnerEvent::Get(
        AudioPolicyServerHandler::EventAudioServerCmd::PREFERRED_DEVICE_SET, 0);
    audioPolicyServerHandler_->HandlePreferredDeviceSetEvent(event);
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
 * @tc.name  : HandleRendererInfoEvent_NoBTPermission
 * @tc.number: HandleRendererInfoEvent_002
 * @tc.desc  : Test HandleRendererInfoEvent does not modify original data when hasBTPermission_ is false
 */
HWTEST(AudioPolicyServerHandlerUnitTest, HandleRendererInfoEvent_NoBTPermission, TestSize.Level2)
{
    auto audioPolicyServerHandler_ = std::make_shared<AudioPolicyServerHandler>();
    ASSERT_NE(audioPolicyServerHandler_, nullptr);

    int32_t clientPid = 1;
    auto mockClientHolder = std::make_shared<AudioPolicyClientHolder>(nullptr);
    ASSERT_NE(mockClientHolder, nullptr);
    mockClientHolder->hasBTPermission_ = false;
    audioPolicyServerHandler_->AddAudioPolicyClientProxyMap(clientPid, mockClientHolder);
    audioPolicyServerHandler_->SetClientCallbacksEnable(
        CallbackChange::CALLBACK_RENDERER_STATE_CHANGE, true);

    auto eventContextObj = std::make_shared<AudioPolicyServerHandler::EventContextObj>();
    ASSERT_NE(eventContextObj, nullptr);
    auto rendererInfo = std::make_shared<AudioRendererChangeInfo>();
    rendererInfo->outputDeviceInfo.deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    rendererInfo->outputDeviceInfo.deviceName_ = "BT_Device";
    rendererInfo->outputDeviceInfo.macAddress_ = "AA:BB:CC:DD:EE:FF";
    eventContextObj->audioRendererChangeInfos.push_back(rendererInfo);

    AppExecFwk::InnerEvent::Pointer event = AppExecFwk::InnerEvent::Get(
        AudioPolicyServerHandler::EventAudioServerCmd::RENDERER_INFO_EVENT, eventContextObj);
    EXPECT_NO_THROW(audioPolicyServerHandler_->HandleRendererInfoEvent(event));
    EXPECT_EQ(rendererInfo->outputDeviceInfo.deviceName_, "BT_Device");
    EXPECT_EQ(rendererInfo->outputDeviceInfo.macAddress_, "AA:BB:CC:DD:EE:FF");

    audioPolicyServerHandler_->RemoveAudioPolicyClientProxyMap(clientPid);
}

/**
 * @tc.name  : HandleCapturerInfoEvent_NoBTPermission
 * @tc.number: HandleCapturerInfoEvent_002
 * @tc.desc  : Test HandleCapturerInfoEvent does not modify original data when hasBTPermission_ is false
 */
HWTEST(AudioPolicyServerHandlerUnitTest, HandleCapturerInfoEvent_NoBTPermission, TestSize.Level2)
{
    auto audioPolicyServerHandler_ = std::make_shared<AudioPolicyServerHandler>();
    ASSERT_NE(audioPolicyServerHandler_, nullptr);

    int32_t clientPid = 1;
    auto mockClientHolder = std::make_shared<AudioPolicyClientHolder>(nullptr);
    ASSERT_NE(mockClientHolder, nullptr);
    mockClientHolder->hasBTPermission_ = false;
    audioPolicyServerHandler_->AddAudioPolicyClientProxyMap(clientPid, mockClientHolder);
    audioPolicyServerHandler_->SetClientCallbacksEnable(
        CallbackChange::CALLBACK_CAPTURER_STATE_CHANGE, true);

    auto eventContextObj = std::make_shared<AudioPolicyServerHandler::EventContextObj>();
    ASSERT_NE(eventContextObj, nullptr);
    auto capturerInfo = std::make_shared<AudioCapturerChangeInfo>();
    capturerInfo->inputDeviceInfo.deviceType_ = DEVICE_TYPE_BLUETOOTH_SCO;
    capturerInfo->inputDeviceInfo.deviceName_ = "BT_SCO";
    capturerInfo->inputDeviceInfo.macAddress_ = "11:22:33:44:55:66";
    eventContextObj->audioCapturerChangeInfos.push_back(capturerInfo);

    AppExecFwk::InnerEvent::Pointer event = AppExecFwk::InnerEvent::Get(
        AudioPolicyServerHandler::EventAudioServerCmd::CAPTURER_INFO_EVENT, eventContextObj);
    EXPECT_NO_THROW(audioPolicyServerHandler_->HandleCapturerInfoEvent(event));
    EXPECT_EQ(capturerInfo->inputDeviceInfo.deviceName_, "BT_SCO");
    EXPECT_EQ(capturerInfo->inputDeviceInfo.macAddress_, "11:22:33:44:55:66");

    audioPolicyServerHandler_->RemoveAudioPolicyClientProxyMap(clientPid);
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
 * @tc.name  : HandleServiceEvent_001
 * @tc.number: HandleServiceEvent_001
 * @tc.desc  : Test HandleInterruptEventWithSessionId function when eventContextObj is nullptr.
 */
HWTEST(AudioPolicyServerHandlerUnitTest, HandleServiceEvent_001, TestSize.Level2)
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
    uint32_t eventId = AudioPolicyServerHandler::EventAudioServerCmd::AUDIO_DEVICE_CHANGE;
    audioPolicyServerHandler_->HandleServiceEvent(eventId, event);
    eventId = AudioPolicyServerHandler::EventAudioServerCmd::PREFERRED_OUTPUT_DEVICE_UPDATED;
    audioPolicyServerHandler_->HandleServiceEvent(eventId, event);
    eventId = AudioPolicyServerHandler::EventAudioServerCmd::PREFERRED_INPUT_DEVICE_UPDATED;
    audioPolicyServerHandler_->HandleServiceEvent(eventId, event);
    eventId = AudioPolicyServerHandler::EventAudioServerCmd::PREFERRED_INPUT_DEVICE_UPDATED;
    audioPolicyServerHandler_->HandleServiceEvent(eventId, event);
    eventId = AudioPolicyServerHandler::EventAudioServerCmd::AVAILABLE_AUDIO_DEVICE_CHANGE;
    audioPolicyServerHandler_->HandleServiceEvent(eventId, event);
    eventId = AudioPolicyServerHandler::EventAudioServerCmd::RENDERER_INFO_EVENT;
    audioPolicyServerHandler_->HandleServiceEvent(eventId, event);
    eventId = AudioPolicyServerHandler::EventAudioServerCmd::CAPTURER_INFO_EVENT;
    audioPolicyServerHandler_->HandleServiceEvent(eventId, event);
    eventId = AudioPolicyServerHandler::EventAudioServerCmd::RENDERER_DEVICE_CHANGE_EVENT;
    audioPolicyServerHandler_->HandleServiceEvent(eventId, event);
    eventId = AudioPolicyServerHandler::EventAudioServerCmd::ON_CAPTURER_CREATE;
    audioPolicyServerHandler_->HandleServiceEvent(eventId, event);
    eventId = AudioPolicyServerHandler::EventAudioServerCmd::ON_CAPTURER_REMOVED;
    audioPolicyServerHandler_->HandleServiceEvent(eventId, event);
    eventId = AudioPolicyServerHandler::EventAudioServerCmd::ON_WAKEUP_CLOSE;
    audioPolicyServerHandler_->HandleServiceEvent(eventId, event);
    eventId = AudioPolicyServerHandler::EventAudioServerCmd::RECREATE_RENDERER_STREAM_EVENT;
    audioPolicyServerHandler_->HandleServiceEvent(eventId, event);
    eventId = AudioPolicyServerHandler::EventAudioServerCmd::RECREATE_CAPTURER_STREAM_EVENT;
    audioPolicyServerHandler_->HandleServiceEvent(eventId, event);
    eventId = AudioPolicyServerHandler::EventAudioServerCmd::ON_WAKEUP_CLOSE;
    audioPolicyServerHandler_->HandleServiceEvent(eventId, event);
    eventId = AudioPolicyServerHandler::EventAudioServerCmd::MICROPHONE_BLOCKED;
    audioPolicyServerHandler_->HandleServiceEvent(eventId, event);
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
    eventId = AudioPolicyServerHandler::EventAudioServerCmd::PREFERRED_DEVICE_SET;
    audioPolicyServerHandler_->HandleOtherServiceSecondEvent(eventId, event);
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
    std::vector<AudioRendererFilter> filterList = {AudioRendererFilter()};
    audioPolicyServerHandler_->clientCbRendererInfoMap_[clientPid] = filterList;
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
    std::vector<AudioCapturerFilter> filterList = {AudioCapturerFilter()};
    audioPolicyServerHandler_->clientCbCapturerFilterMap_[clientPid] = filterList;
    audioPolicyServerHandler_->GetCallbackCapturerInfoList(clientPid);
    EXPECT_EQ(audioPolicyServerHandler_->audioPolicyClientProxyAPSCbsMap_.size(), 0);
}

/**
 * @tc.name  : SetCallbackCapturerInfo_001
 * @tc.number: SetCallbackCapturerInfo_001
 * @tc.desc  : Test SetCallbackCapturerInfo method when set audioCapturerFilter into clientCbCapturerFilterMap_.
 */
HWTEST(AudioPolicyServerHandlerUnitTest, SetCallbackCapturerInfo_001, TestSize.Level1)
{
    auto audioPolicyServerHandler_ = std::make_shared<AudioPolicyServerHandler>();
    EXPECT_NE(audioPolicyServerHandler_, nullptr);
    AudioCapturerInfo audioCapturerInfo;
    int32_t uid = -1;
    int32_t ret = audioPolicyServerHandler_->SetCallbackCapturerInfo(audioCapturerInfo, uid);
    EXPECT_EQ(ret, ERR_PERMISSION_DENIED);
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
    EXPECT_EQ(ret, ERR_PERMISSION_DENIED);
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
    AudioDeviceDescriptor preOutputDeviceInfo;
    AudioStreamDeviceChangeReasonExt reason = AudioStreamDeviceChangeReasonExt::ExtEnum::UNKNOWN;
    bool ret = audioPolicyServerHandler_->SendRendererDeviceChangeEvent(clientPid,
        sessionId, outputDeviceInfo, reason, preOutputDeviceInfo);
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


/**
 * @tc.name  : HandleAdaptiveSpatialRenderingEnabledChangeEvent_001
 * @tc.number: HandleAdaptiveSpatialRenderingEnabledChangeEvent_001
 * @tc.desc  : Test HandleAdaptiveSpatialRenderingEnabledChangeForAnyDeviceEvent when eventContextObj is nullptr.
 */
HWTEST(AudioPolicyServerHandlerUnitTest, HandleAdaptiveSpatialRenderingEnabledChangeEvent_001, TestSize.Level2)
{
    auto audioPolicyServerHandler_ = std::make_shared<AudioPolicyServerHandler>();
    EXPECT_NE(audioPolicyServerHandler_, nullptr);
    int32_t clientPid = 1;
    std::shared_ptr<AudioPolicyClientHolder> cb = nullptr;
    audioPolicyServerHandler_->AddAudioPolicyClientProxyMap(clientPid, cb);
    AppExecFwk::InnerEvent::Pointer event =
        AppExecFwk::InnerEvent::Get(AudioPolicyServerHandler::EventAudioServerCmd::NN_STATE_CHANGE, 0);
    int32_t ret =
        audioPolicyServerHandler_->SetClientCallbacksEnable(CallbackChange::CALLBACK_SET_MICROPHONE_BLOCKED, false);
    audioPolicyServerHandler_->HandleAdaptiveSpatialRenderingEnabledChangeForAnyDeviceEvent(event);
    audioPolicyServerHandler_->SetClientCallbacksEnable(
        CallbackChange::CALLBACK_ADAPTIVE_SPATIAL_RENDERING_ENABLED_CHANGE, true);
    audioPolicyServerHandler_->HandleAdaptiveSpatialRenderingEnabledChangeForAnyDeviceEvent(event);
    EXPECT_EQ(audioPolicyServerHandler_->audioPolicyClientProxyAPSCbsMap_.size(), 1);
}

/**
 * @tc.name  : HandleDeviceConfigChangedEvent_Test_001
 * @tc.number: HandleDeviceConfigChangedEvent_Test_001
 * @tc.desc  : Test HandleDeviceConfigChangedEvent function.
 */
HWTEST(AudioPolicyServerHandlerUnitTest, HandleOtherServiceEvent_Test_001, TestSize.Level2)
{
    auto audioPolicyServerHandler_ = std::make_shared<AudioPolicyServerHandler>();
    EXPECT_NE(audioPolicyServerHandler_, nullptr);
    auto eventContextObj = std::make_shared<AudioPolicyServerHandler::EventContextObj>();
    ASSERT_NE(eventContextObj, nullptr);
    std::shared_ptr<AudioDeviceDescriptor> devDesc = std::make_shared<AudioDeviceDescriptor>(
        DeviceType::DEVICE_TYPE_NEARLINK, DeviceRole::OUTPUT_DEVICE);
    eventContextObj->descriptor = devDesc;
    int32_t clientPid = 1;
    std::shared_ptr<AudioPolicyClientHolder> cb = nullptr;
    audioPolicyServerHandler_->AddAudioPolicyClientProxyMap(clientPid, cb);
    AppExecFwk::InnerEvent::Pointer event = AppExecFwk::InnerEvent::Get(
        AudioPolicyServerHandler::EventAudioServerCmd::DEVICE_CONFIG_CHANGED, eventContextObj);
    int32_t ret =
        audioPolicyServerHandler_->SetClientCallbacksEnable(CallbackChange::CALLBACK_SET_MICROPHONE_BLOCKED, false);
    audioPolicyServerHandler_->SetClientCallbacksEnable(
        CallbackChange::CALLBACK_HEAD_TRACKING_ENABLED_CHANGE, true);
    uint32_t eventId = AudioPolicyServerHandler::EventAudioServerCmd::DEVICE_CONFIG_CHANGED;
    audioPolicyServerHandler_->HandleOtherServiceSecondEvent(eventId, event);
    EXPECT_EQ(audioPolicyServerHandler_->audioPolicyClientProxyAPSCbsMap_.size(), 1);
}

/**
 * @tc.name  : HandleSystemAppVolumeChangeEvent_Normal
 * @tc.number: HandleSystemAppVolumeChangeEvent_001
 * @tc.desc  : Test HandleSystemAppVolumeChangeEvent when callback is enabled and client registered
 */
HWTEST(AudioPolicyServerHandlerUnitTest, HandleSystemAppVolumeChangeEvent_Normal, TestSize.Level2)
{
    // Arrange: Setup test conditions
    auto audioPolicyServerHandler_ = std::make_shared<AudioPolicyServerHandler>();

    auto eventContextObj = std::make_shared<AudioPolicyServerHandler::EventContextObj>();
    eventContextObj->appUid = 1001;
    eventContextObj->volumeEvent.volumeType = STREAM_MUSIC;
    eventContextObj->volumeEvent.volume = 5;
    eventContextObj->volumeEvent.updateUi = true;
    AppExecFwk::InnerEvent::Pointer event = AppExecFwk::InnerEvent::Get(
        AudioPolicyServerHandler::EventAudioServerCmd::SYSTEM_APP_VOLUME_CHANGE_EVENT, eventContextObj);

    auto volumeChangeCb = std::make_shared<AudioPolicyClientHolder>(nullptr);
    int32_t clientPid = IPCSkeleton::GetCallingPid();
    audioPolicyServerHandler_->SetClientCallbacksEnable(CallbackChange::CALLBACK_APP_VOLUME_CHANGE, true);
    audioPolicyServerHandler_->AddAudioPolicyClientProxyMap(clientPid, volumeChangeCb);

    // Act: Call the function under test
    audioPolicyServerHandler_->HandleSystemAppVolumeChangeEvent(event);

    // Assert: Verify client registered successfully
    EXPECT_EQ(1, static_cast<int32_t>(audioPolicyServerHandler_->audioPolicyClientProxyAPSCbsMap_.size()));

    // Cleanup
    audioPolicyServerHandler_->RemoveAudioPolicyClientProxyMap(clientPid);
}

/**
 * @tc.name  : HandleSystemAppVolumeChangeEvent_CallbackDisabled
 * @tc.number: HandleSystemAppVolumeChangeEvent_002
 * @tc.desc  : Test HandleSystemAppVolumeChangeEvent when callback is disabled
 */
HWTEST(AudioPolicyServerHandlerUnitTest, HandleSystemAppVolumeChangeEvent_CallbackDisabled, TestSize.Level2)
{
    // Arrange: Setup test conditions
    auto audioPolicyServerHandler_ = std::make_shared<AudioPolicyServerHandler>();

    auto eventContextObj = std::make_shared<AudioPolicyServerHandler::EventContextObj>();
    eventContextObj->appUid = 1001;
    eventContextObj->volumeEvent.volumeType = STREAM_MUSIC;
    eventContextObj->volumeEvent.volume = 5;
    AppExecFwk::InnerEvent::Pointer event = AppExecFwk::InnerEvent::Get(
        AudioPolicyServerHandler::EventAudioServerCmd::SYSTEM_APP_VOLUME_CHANGE_EVENT, eventContextObj);

    auto volumeChangeCb = std::make_shared<AudioPolicyClientHolder>(nullptr);
    int32_t clientPid = IPCSkeleton::GetCallingPid();
    audioPolicyServerHandler_->SetClientCallbacksEnable(CallbackChange::CALLBACK_APP_VOLUME_CHANGE, false);
    audioPolicyServerHandler_->AddAudioPolicyClientProxyMap(clientPid, volumeChangeCb);

    // Act: Call the function under test
    audioPolicyServerHandler_->HandleSystemAppVolumeChangeEvent(event);

    // Assert: Verify client registered but callback is disabled
    EXPECT_EQ(1, static_cast<int32_t>(audioPolicyServerHandler_->audioPolicyClientProxyAPSCbsMap_.size()));

    // Cleanup
    audioPolicyServerHandler_->RemoveAudioPolicyClientProxyMap(clientPid);
}
/**
 * @tc.name  : HandlePreferredOutputDeviceUpdated_Deduplication_001
 * @tc.number: HandlePreferredOutputDeviceUpdated_Deduplication_001
 * @tc.desc  : Test deduplication - same device should not trigger duplicate callback.
 */
HWTEST(AudioPolicyServerHandlerUnitTest, HandlePreferredOutputDeviceUpdated_Deduplication_001, TestSize.Level2)
{
    auto audioPolicyServerHandler_ = std::make_shared<AudioPolicyServerHandler>();
    ASSERT_NE(audioPolicyServerHandler_, nullptr);

    int32_t clientPid = 1;
    auto mockClientHolder = std::make_shared<AudioPolicyClientHolder>(nullptr);
    ASSERT_NE(mockClientHolder, nullptr);

    // Setup client and enable callback
    audioPolicyServerHandler_->AddAudioPolicyClientProxyMap(clientPid, mockClientHolder);
    audioPolicyServerHandler_->SetClientCallbacksEnable(
        CallbackChange::CALLBACK_PREFERRED_OUTPUT_DEVICE_CHANGE, true);

    // First call - should send callback and record device
    AppExecFwk::InnerEvent::Pointer event = AppExecFwk::InnerEvent::Get(
    AudioPolicyServerHandler::EventAudioServerCmd::PREFERRED_OUTPUT_DEVICE_UPDATED);
    audioPolicyServerHandler_->HandlePreferredOutputDeviceUpdated();

    // Second call with same device - should skip (deduplication)
    audioPolicyServerHandler_->HandlePreferredOutputDeviceUpdated();

    // Cleanup
    audioPolicyServerHandler_->RemoveAudioPolicyClientProxyMap(clientPid);
    EXPECT_EQ(audioPolicyServerHandler_->audioPolicyClientProxyAPSCbsMap_.size(), 0);
}

/**
 * @tc.name  : HandlePreferredOutputDeviceUpdated_Deduplication_002
 * @tc.number: HandlePreferredOutputDeviceUpdated_Deduplication_002
 * @tc.desc  : Test deduplication - different device should trigger callback.
 */
HWTEST(AudioPolicyServerHandlerUnitTest, HandlePreferredOutputDeviceUpdated_Deduplication_002, TestSize.Level2)
{
    auto audioPolicyServerHandler_ = std::make_shared<AudioPolicyServerHandler>();
    ASSERT_NE(audioPolicyServerHandler_, nullptr);

    int32_t clientPid = 1;
    auto mockClientHolder = std::make_shared<AudioPolicyClientHolder>(nullptr);
    ASSERT_NE(mockClientHolder, nullptr);

    // Setup client and enable callback
    audioPolicyServerHandler_->AddAudioPolicyClientProxyMap(clientPid, mockClientHolder);
    audioPolicyServerHandler_->SetClientCallbacksEnable(
        CallbackChange::CALLBACK_PREFERRED_OUTPUT_DEVICE_CHANGE, true);

    // First call
    AppExecFwk::InnerEvent::Pointer event = AppExecFwk::InnerEvent::Get(
    AudioPolicyServerHandler::EventAudioServerCmd::PREFERRED_OUTPUT_DEVICE_UPDATED);
    audioPolicyServerHandler_->HandlePreferredOutputDeviceUpdated();

    // Second call - should process (different device scenario)
    audioPolicyServerHandler_->HandlePreferredOutputDeviceUpdated();

    // Cleanup
    audioPolicyServerHandler_->RemoveAudioPolicyClientProxyMap(clientPid);
    EXPECT_EQ(audioPolicyServerHandler_->audioPolicyClientProxyAPSCbsMap_.size(), 0);
}

/**
 * @tc.name  : HandlePreferredInputDeviceUpdated_Deduplication_001
 * @tc.number: HandlePreferredInputDeviceUpdated_Deduplication_001
 * @tc.desc  : Test deduplication - same input device should not trigger duplicate callback.
 */
HWTEST(AudioPolicyServerHandlerUnitTest, HandlePreferredInputDeviceUpdated_Deduplication_001, TestSize.Level2)
{
    auto audioPolicyServerHandler_ = std::make_shared<AudioPolicyServerHandler>();
    ASSERT_NE(audioPolicyServerHandler_, nullptr);

    int32_t clientPid = 1;
    auto mockClientHolder = std::make_shared<AudioPolicyClientHolder>(nullptr);
    ASSERT_NE(mockClientHolder, nullptr);

    // Setup client and enable callback
    audioPolicyServerHandler_->AddAudioPolicyClientProxyMap(clientPid, mockClientHolder);
    audioPolicyServerHandler_->SetClientCallbacksEnable(
        CallbackChange::CALLBACK_PREFERRED_INPUT_DEVICE_CHANGE, true);

    // First call - should send callback and record device
    audioPolicyServerHandler_->HandlePreferredInputDeviceUpdated();

    // Second call with same device - should skip (deduplication)
    audioPolicyServerHandler_->HandlePreferredInputDeviceUpdated();

    // Cleanup
    audioPolicyServerHandler_->RemoveAudioPolicyClientProxyMap(clientPid);
    EXPECT_EQ(audioPolicyServerHandler_->audioPolicyClientProxyAPSCbsMap_.size(), 0);
}

/**
 * @tc.name  : RemoveAudioPolicyClientProxyMap_ClearTracking_001
 * @tc.number: RemoveAudioPolicyClientProxyMap_ClearTracking_001
 * @tc.desc  : Test that RemoveAudioPolicyClientProxyMap clears tracking maps.
 */
HWTEST(AudioPolicyServerHandlerUnitTest, RemoveAudioPolicyClientProxyMap_ClearTracking_001, TestSize.Level2)
{
    auto audioPolicyServerHandler_ = std::make_shared<AudioPolicyServerHandler>();
    ASSERT_NE(audioPolicyServerHandler_, nullptr);

    int32_t clientPid = 1;
    auto mockClientHolder = std::make_shared<AudioPolicyClientHolder>(nullptr);
    audioPolicyServerHandler_->AddAudioPolicyClientProxyMap(clientPid, mockClientHolder);

    // Setup tracking entries by calling the handler functions
    audioPolicyServerHandler_->SetClientCallbacksEnable(
        CallbackChange::CALLBACK_PREFERRED_OUTPUT_DEVICE_CHANGE, true);
    AppExecFwk::InnerEvent::Pointer event = AppExecFwk::InnerEvent::Get(
    AudioPolicyServerHandler::EventAudioServerCmd::PREFERRED_OUTPUT_DEVICE_UPDATED);
    audioPolicyServerHandler_->HandlePreferredOutputDeviceUpdated();

    audioPolicyServerHandler_->SetClientCallbacksEnable(
        CallbackChange::CALLBACK_PREFERRED_INPUT_DEVICE_CHANGE, true);
    audioPolicyServerHandler_->HandlePreferredInputDeviceUpdated();

    // Remove client - should clear tracking entries
    audioPolicyServerHandler_->RemoveAudioPolicyClientProxyMap(clientPid);

    // Verify client is removed
    EXPECT_EQ(audioPolicyServerHandler_->audioPolicyClientProxyAPSCbsMap_.size(), 0);
}

/**
 * @tc.name  : IsSameDeviceDescPtr_Deduplication_001
 * @tc.number: IsSameDeviceDescPtr_Deduplication_001
 * @tc.desc  : Test IsSameDeviceDescPtr for device comparison in deduplication.
 */
HWTEST(AudioPolicyServerHandlerUnitTest, IsSameDeviceDescPtr_Deduplication_001, TestSize.Level2)
{
    // Test same device type
    auto deviceDesc1 = std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_SPEAKER, OUTPUT_DEVICE);
    auto deviceDesc2 = std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_SPEAKER, OUTPUT_DEVICE);
    EXPECT_TRUE(deviceDesc1->IsSameDeviceDescPtr(deviceDesc2));

    // Test different device type
    auto deviceDesc3 = std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_BLUETOOTH_A2DP, OUTPUT_DEVICE);
    EXPECT_FALSE(deviceDesc1->IsSameDeviceDescPtr(deviceDesc3));

    // Test same BT device with same macAddress
    auto btDevice1 = std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_BLUETOOTH_A2DP, OUTPUT_DEVICE);
    btDevice1->macAddress_ = "00:11:22:33:44:55";
    auto btDevice2 = std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_BLUETOOTH_A2DP, OUTPUT_DEVICE);
    btDevice2->macAddress_ = "00:11:22:33:44:55";
    EXPECT_TRUE(btDevice1->IsSameDeviceDescPtr(btDevice2));

    // Test same BT device with different macAddress
    auto btDevice3 = std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_BLUETOOTH_A2DP, OUTPUT_DEVICE);
    btDevice3->macAddress_ = "AA:BB:CC:DD:EE:FF";
    EXPECT_FALSE(btDevice1->IsSameDeviceDescPtr(btDevice3));

    // Test null device descriptor
    std::shared_ptr<AudioDeviceDescriptor> nullDevice = nullptr;
    EXPECT_FALSE(deviceDesc1->IsSameDeviceDescPtr(nullDevice));
}

// Constants for testing ShouldSkipDuplicateCallback methods
const int32_t TEST_CLIENT_PID = 1001;
const int32_t TEST_UID = 10001;
const StreamUsage TEST_STREAM_USAGE = STREAM_USAGE_MUSIC;
const SourceType TEST_SOURCE_TYPE = SOURCE_TYPE_MIC;
const std::string TEST_NETWORK_ID = "test_network_id";
const std::string TEST_MAC_ADDRESS_1 = "00:11:22:33:44:55";
const std::string TEST_MAC_ADDRESS_2 = "00:11:22:33:44:56";

// Helper function: Create device descriptor for testing
std::shared_ptr<AudioDeviceDescriptor> CreateTestDeviceDesc(
    DeviceType type, const std::string& mac, const std::string& networkId)
{
    auto desc = std::make_shared<AudioDeviceDescriptor>();
    desc->deviceType_ = type;
    desc->macAddress_ = mac;
    desc->networkId_ = networkId;
    return desc;
}

// Helper function: Create device list for testing
std::vector<std::shared_ptr<AudioDeviceDescriptor>> CreateTestDeviceList(
    std::initializer_list<std::pair<DeviceType, std::string>> devices)
{
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> list;
    for (auto& d : devices) {
        list.push_back(CreateTestDeviceDesc(d.first, d.second, TEST_NETWORK_ID));
    }
    return list;
}

/**
 * @tc.name  : Test SendRecordControllerEnabledChangeCallback API
 * @tc.type  : FUNC
 * @tc.number: SendRecordControllerEnabledChangeCallback_001
 * @tc.desc  : Test SendRecordControllerEnabledChangeCallback interface with enabled=true.
 */
HWTEST(AudioPolicyServerHandlerUnitTest, SendRecordControllerEnabledChangeCallback_001, TestSize.Level1)
{
    auto handler = std::make_shared<AudioPolicyServerHandler>();
    SystemRecordControllerChangeInfo changeInfo;
    changeInfo.enabled = true;
    changeInfo.uid = TEST_UID;
    changeInfo.sourceType = SOURCE_TYPE_MIC;
    bool ret = handler->SendRecordControllerEnabledChangeCallback(changeInfo);
    EXPECT_TRUE(ret);
}

/**
 * @tc.name  : Test SendRecordControllerEnabledChangeCallback API
 * @tc.type  : FUNC
 * @tc.number: SendRecordControllerEnabledChangeCallback_002
 * @tc.desc  : Test SendRecordControllerEnabledChangeCallback interface with enabled=false.
 */
HWTEST(AudioPolicyServerHandlerUnitTest, SendRecordControllerEnabledChangeCallback_002, TestSize.Level1)
{
    auto handler = std::make_shared<AudioPolicyServerHandler>();
    SystemRecordControllerChangeInfo changeInfo;
    changeInfo.enabled = false;
    changeInfo.uid = TEST_UID;
    changeInfo.sourceType = SOURCE_TYPE_CAMCORDER;
    bool ret = handler->SendRecordControllerEnabledChangeCallback(changeInfo);
    EXPECT_TRUE(ret);
}

/**
 * @tc.name  : Test ClearPreferredDeviceDedupCache API
 * @tc.type  : FUNC
 * @tc.number: ClearPreferredDeviceDedupCache_Output_001
 * @tc.desc  : Test ClearPreferredDeviceDedupCache clears output dedup cache and renderer info map.
 */
HWTEST(AudioPolicyServerHandlerUnitTest, ClearPreferredDeviceDedupCache_Output_001, TestSize.Level1)
{
    auto handler = std::make_shared<AudioPolicyServerHandler>();
    ASSERT_NE(handler, nullptr);

    int32_t clientPid = TEST_CLIENT_PID;
    int32_t uid = TEST_UID;
    StreamUsage streamUsage = TEST_STREAM_USAGE;
    auto devices = CreateTestDeviceList({{DEVICE_TYPE_SPEAKER, TEST_MAC_ADDRESS_1}});

    // Populate clientCbRendererInfoMap_
    AudioRendererFilter filter;
    filter.uid = uid;
    filter.rendererInfo.streamUsage = streamUsage;
    handler->clientCbRendererInfoMap_[clientPid] = {filter};

    handler->ClearPreferredDeviceDedupCache(clientPid, CallbackChange::CALLBACK_PREFERRED_OUTPUT_DEVICE_CHANGE);

    EXPECT_EQ(handler->clientCbRendererInfoMap_.count(clientPid), 0);
}

/**
 * @tc.name  : Test ClearPreferredDeviceDedupCache API
 * @tc.type  : FUNC
 * @tc.number: ClearPreferredDeviceDedupCache_Input_001
 * @tc.desc  : Test ClearPreferredDeviceDedupCache clears input dedup cache and capturer filter map.
 */
HWTEST(AudioPolicyServerHandlerUnitTest, ClearPreferredDeviceDedupCache_Input_001, TestSize.Level1)
{
    auto handler = std::make_shared<AudioPolicyServerHandler>();
    ASSERT_NE(handler, nullptr);

    int32_t clientPid = TEST_CLIENT_PID;
    int32_t uid = TEST_UID;
    SourceType sourceType = TEST_SOURCE_TYPE;
    auto devices = CreateTestDeviceList({{DEVICE_TYPE_MIC, TEST_MAC_ADDRESS_1}});

    // Populate clientCbCapturerFilterMap_
    AudioCapturerFilter capturerFilter;
    capturerFilter.uid = uid;
    capturerFilter.capturerInfo.sourceType = sourceType;
    handler->clientCbCapturerFilterMap_[clientPid] = {capturerFilter};

    handler->ClearPreferredDeviceDedupCache(clientPid, CallbackChange::CALLBACK_PREFERRED_INPUT_DEVICE_CHANGE);

    EXPECT_EQ(handler->clientCbCapturerFilterMap_.count(clientPid), 0);
}

/**
 * @tc.name  : Test ClearPreferredDeviceDedupCache API
 * @tc.type  : FUNC
 * @tc.number: ClearPreferredDeviceDedupCache_NoEffectOnOtherClient_001
 * @tc.desc  : Test ClearPreferredDeviceDedupCache only clears the target client, not other clients.
 */
HWTEST(AudioPolicyServerHandlerUnitTest, ClearPreferredDeviceDedupCache_NoEffectOnOtherClient_001, TestSize.Level1)
{
    auto handler = std::make_shared<AudioPolicyServerHandler>();
    ASSERT_NE(handler, nullptr);

    int32_t targetPid = TEST_CLIENT_PID;
    int32_t otherPid = TEST_CLIENT_PID + 1;
    int32_t uid = TEST_UID;
    StreamUsage streamUsage = TEST_STREAM_USAGE;
    auto devices = CreateTestDeviceList({{DEVICE_TYPE_SPEAKER, TEST_MAC_ADDRESS_1}});

    AudioRendererFilter filter;
    filter.uid = uid;
    filter.rendererInfo.streamUsage = streamUsage;
    handler->clientCbRendererInfoMap_[targetPid] = {filter};
    handler->clientCbRendererInfoMap_[otherPid] = {filter};

    handler->ClearPreferredDeviceDedupCache(targetPid, CallbackChange::CALLBACK_PREFERRED_OUTPUT_DEVICE_CHANGE);

    // Target client's cache should be cleared
    EXPECT_EQ(handler->clientCbRendererInfoMap_.count(targetPid), 0);
    // Other client's cache should remain
    EXPECT_EQ(handler->clientCbRendererInfoMap_.count(otherPid), 1);
}

/**
 * @tc.name  : Test SetClientCallbacksEnable with dedup cache clearing
 * @tc.type  : FUNC
 * @tc.number: SetClientCallbacksEnable_ClearDedupCache_Output_001
 * @tc.desc  : Test SetClientCallbacksEnable(false) clears output dedup cache and renderer info.
 */
HWTEST(AudioPolicyServerHandlerUnitTest, SetClientCallbacksEnable_ClearDedupCache_Output_001, TestSize.Level1)
{
    auto handler = std::make_shared<AudioPolicyServerHandler>();
    ASSERT_NE(handler, nullptr);

    int32_t clientPid = getpid();
    int32_t uid = TEST_UID;
    StreamUsage streamUsage = TEST_STREAM_USAGE;
    auto devices = CreateTestDeviceList({{DEVICE_TYPE_SPEAKER, TEST_MAC_ADDRESS_1}});

    // Enable callback and populate dedup cache
    handler->clientCallbacksMap_[clientPid][CallbackChange::CALLBACK_PREFERRED_OUTPUT_DEVICE_CHANGE] = true;
    AudioRendererFilter filter;
    filter.uid = uid;
    filter.rendererInfo.streamUsage = streamUsage;
    handler->clientCbRendererInfoMap_[clientPid] = {filter};

    // Disable callback - should clear dedup cache and renderer info
    int32_t ret = handler->SetClientCallbacksEnable(
        CallbackChange::CALLBACK_PREFERRED_OUTPUT_DEVICE_CHANGE, false);

    EXPECT_EQ(ret, AUDIO_OK);
    EXPECT_EQ(handler->clientCbRendererInfoMap_.count(clientPid), 0);
}

/**
 * @tc.name  : Test SetClientCallbacksEnable with dedup cache clearing
 * @tc.type  : FUNC
 * @tc.number: SetClientCallbacksEnable_ClearDedupCache_Input_001
 * @tc.desc  : Test SetClientCallbacksEnable(false) clears input dedup cache and capturer filter.
 */
HWTEST(AudioPolicyServerHandlerUnitTest, SetClientCallbacksEnable_ClearDedupCache_Input_001, TestSize.Level1)
{
    auto handler = std::make_shared<AudioPolicyServerHandler>();
    ASSERT_NE(handler, nullptr);

    int32_t clientPid = getpid();
    int32_t uid = TEST_UID;
    SourceType sourceType = TEST_SOURCE_TYPE;
    auto devices = CreateTestDeviceList({{DEVICE_TYPE_MIC, TEST_MAC_ADDRESS_1}});

    // Enable callback and populate dedup cache
    handler->clientCallbacksMap_[clientPid][CallbackChange::CALLBACK_PREFERRED_INPUT_DEVICE_CHANGE] = true;
    AudioCapturerFilter capturerFilter;
    capturerFilter.uid = uid;
    capturerFilter.capturerInfo.sourceType = sourceType;
    handler->clientCbCapturerFilterMap_[clientPid] = {capturerFilter};

    // Disable callback - should clear dedup cache and capturer filter
    int32_t ret = handler->SetClientCallbacksEnable(
        CallbackChange::CALLBACK_PREFERRED_INPUT_DEVICE_CHANGE, false);

    EXPECT_EQ(ret, AUDIO_OK);
    EXPECT_EQ(handler->clientCbCapturerFilterMap_.count(clientPid), 0);
}

/**
 * @tc.name  : Test SetClientCallbacksEnable with dedup cache clearing
 * @tc.type  : FUNC
 * @tc.number: SetClientCallbacksEnable_EnableTrueNoClear_001
 * @tc.desc  : Test SetClientCallbacksEnable(true) does NOT clear dedup cache.
 */
HWTEST(AudioPolicyServerHandlerUnitTest, SetClientCallbacksEnable_EnableTrueNoClear_001, TestSize.Level1)
{
    auto handler = std::make_shared<AudioPolicyServerHandler>();
    ASSERT_NE(handler, nullptr);

    int32_t clientPid = TEST_CLIENT_PID;
    int32_t uid = TEST_UID;
    StreamUsage streamUsage = TEST_STREAM_USAGE;
    SourceType sourceType = TEST_SOURCE_TYPE;
    auto devices = CreateTestDeviceList({{DEVICE_TYPE_SPEAKER, TEST_MAC_ADDRESS_1}});

    AudioRendererFilter filter;
    filter.uid = uid;
    filter.rendererInfo.streamUsage = streamUsage;
    handler->clientCbRendererInfoMap_[clientPid] = {filter};
    AudioCapturerFilter capturerFilter;
    capturerFilter.uid = uid;
    capturerFilter.capturerInfo.sourceType = sourceType;
    handler->clientCbCapturerFilterMap_[clientPid] = {capturerFilter};

    // Enable callback - should NOT clear dedup cache
    int32_t ret = handler->SetClientCallbacksEnable(
        CallbackChange::CALLBACK_PREFERRED_OUTPUT_DEVICE_CHANGE, true);

    EXPECT_EQ(ret, AUDIO_OK);
    EXPECT_EQ(handler->clientCbRendererInfoMap_.count(clientPid), 1);

    ret = handler->SetClientCallbacksEnable(
        CallbackChange::CALLBACK_PREFERRED_INPUT_DEVICE_CHANGE, true);

    EXPECT_EQ(ret, AUDIO_OK);
    EXPECT_EQ(handler->clientCbCapturerFilterMap_.count(clientPid), 1);
}

/**
 * @tc.name  : SetSystemVolumeChangeCallbackInfo_NewUid
 * @tc.number: SetSystemVolumeChangeCallbackInfo_001
 * @tc.desc  : Test if branch true when uid not found in filter list
 */
HWTEST(AudioPolicyServerHandlerUnitTest, SetSystemVolumeChangeCallbackInfo_NewUid, TestSize.Level1)
{
    auto handler = std::make_shared<AudioPolicyServerHandler>();
    SystemVolumeFilter filter;
    filter.uid = 1001;

    handler->SetSystemVolumeChangeCallbackInfo(CLIENT_ID, filter);
    auto &filterList = handler->clientSystemVolumeFilterMap_[CLIENT_ID];
    EXPECT_EQ(filterList.size(), 1);
    EXPECT_EQ(filterList[0].uid, 1001);
}

/**
 * @tc.name  : SetSystemVolumeChangeCallbackInfo_DuplicateUid
 * @tc.number: SetSystemVolumeChangeCallbackInfo_002
 * @tc.desc  : Test if branch false when uid already exists in filter list
 */
HWTEST(AudioPolicyServerHandlerUnitTest, SetSystemVolumeChangeCallbackInfo_DuplicateUid, TestSize.Level1)
{
    auto handler = std::make_shared<AudioPolicyServerHandler>();
    SystemVolumeFilter filter;
    filter.uid = 1001;

    handler->SetSystemVolumeChangeCallbackInfo(CLIENT_ID, filter);
    handler->SetSystemVolumeChangeCallbackInfo(CLIENT_ID, filter);
    auto &filterList = handler->clientSystemVolumeFilterMap_[CLIENT_ID];
    EXPECT_EQ(filterList.size(), 1);
}

/**
 * @tc.name  : RemoveSystemVolumeChangeCallbackInfo_ExistingUid
 * @tc.number: RemoveSystemVolumeChangeCallbackInfo_001
 * @tc.desc  : Test if branch true when uid found in filter list
 */
HWTEST(AudioPolicyServerHandlerUnitTest, RemoveSystemVolumeChangeCallbackInfo_ExistingUid, TestSize.Level1)
{
    auto handler = std::make_shared<AudioPolicyServerHandler>();
    SystemVolumeFilter filter;
    filter.uid = 1001;
    handler->SetSystemVolumeChangeCallbackInfo(CLIENT_ID, filter);

    handler->RemoveSystemVolumeChangeCallbackInfo(CLIENT_ID, filter);
    EXPECT_TRUE(handler->clientSystemVolumeFilterMap_.find(CLIENT_ID) ==
        handler->clientSystemVolumeFilterMap_.end());
}

/**
 * @tc.name  : RemoveSystemVolumeChangeCallbackInfo_NonExistingUid
 * @tc.number: RemoveSystemVolumeChangeCallbackInfo_002
 * @tc.desc  : Test if branch false when uid not found in filter list
 */
HWTEST(AudioPolicyServerHandlerUnitTest, RemoveSystemVolumeChangeCallbackInfo_NonExistingUid, TestSize.Level1)
{
    auto handler = std::make_shared<AudioPolicyServerHandler>();
    SystemVolumeFilter filter1;
    filter1.uid = 1001;
    handler->SetSystemVolumeChangeCallbackInfo(CLIENT_ID, filter1);

    SystemVolumeFilter filter2;
    filter2.uid = 2002;
    handler->RemoveSystemVolumeChangeCallbackInfo(CLIENT_ID, filter2);
    auto &filterList = handler->clientSystemVolumeFilterMap_[CLIENT_ID];
    EXPECT_EQ(filterList.size(), 1);
    EXPECT_EQ(filterList[0].uid, 1001);
}

/**
 * @tc.name  : RemoveSystemVolumeChangeCallbackInfo_ListNotEmpty
 * @tc.number: RemoveSystemVolumeChangeCallbackInfo_003
 * @tc.desc  : Test if branch false when filter list is not empty after removal
 */
HWTEST(AudioPolicyServerHandlerUnitTest, RemoveSystemVolumeChangeCallbackInfo_ListNotEmpty, TestSize.Level1)
{
    auto handler = std::make_shared<AudioPolicyServerHandler>();
    SystemVolumeFilter filter1;
    filter1.uid = 1001;
    SystemVolumeFilter filter2;
    filter2.uid = 2002;
    handler->SetSystemVolumeChangeCallbackInfo(CLIENT_ID, filter1);
    handler->SetSystemVolumeChangeCallbackInfo(CLIENT_ID, filter2);

    handler->RemoveSystemVolumeChangeCallbackInfo(CLIENT_ID, filter1);
    auto &filterList = handler->clientSystemVolumeFilterMap_[CLIENT_ID];
    EXPECT_EQ(filterList.size(), 1);
}
/**
 * @tc.name  : HasVolumeKeyEventCallbacks_ClientPidNotFound
 * @tc.number: HasVolumeKeyEventCallbacks_001
 * @tc.desc  : Test HasVolumeKeyEventCallbacks returns false when clientPid not in clientCallbacksMap_
 */
HWTEST(AudioPolicyServerHandlerUnitTest, HasVolumeKeyEventCallbacks_ClientPidNotFound, TestSize.Level1)
{
    auto handler = std::make_shared<AudioPolicyServerHandler>();
    handler->clientCallbacksMap_.clear();

    bool result = handler->HasVolumeKeyEventCallbacks(9999);
    EXPECT_FALSE(result);
}

/**
 * @tc.name  : HasVolumeKeyEventCallbacks_NoVolumeCallbacks
 * @tc.number: HasVolumeKeyEventCallbacks_002
 * @tc.desc  : Test HasVolumeKeyEventCallbacks returns false when clientPid in map but no volume callbacks
 */
HWTEST(AudioPolicyServerHandlerUnitTest, HasVolumeKeyEventCallbacks_NoVolumeCallbacks, TestSize.Level1)
{
    auto handler = std::make_shared<AudioPolicyServerHandler>();
    handler->clientCallbacksMap_.clear();
    int32_t clientPid = 1000;
    handler->clientCallbacksMap_[clientPid] = {};

    bool result = handler->HasVolumeKeyEventCallbacks(clientPid);
    EXPECT_FALSE(result);
}

/**
 * @tc.name  : HasVolumeKeyEventCallbacks_WithVolumeKeyEventCallback
 * @tc.number: HasVolumeKeyEventCallbacks_003
 * @tc.desc  : Test HasVolumeKeyEventCallbacks returns true when CALLBACK_SET_VOLUME_KEY_EVENT is enabled
 */
HWTEST(AudioPolicyServerHandlerUnitTest, HasVolumeKeyEventCallbacks_WithVolumeKeyEventCallback, TestSize.Level1)
{
    auto handler = std::make_shared<AudioPolicyServerHandler>();
    handler->clientCallbacksMap_.clear();
    int32_t clientPid = 1000;
    handler->clientCallbacksMap_[clientPid][CALLBACK_SET_VOLUME_KEY_EVENT] = true;

    bool result = handler->HasVolumeKeyEventCallbacks(clientPid);
    EXPECT_TRUE(result);

    handler->clientCallbacksMap_.clear();
}

/**
 * @tc.name  : Test HandleSpatialAudioSourceTypeChange dispatches outside the lock
 * @tc.type  : FUNC
 * @tc.number: HandleSpatialAudioSourceTypeChange_001
 * @tc.desc  : Callback must be invoked after handleMapMutex_ is released (lock-free dispatch).
 */
HWTEST(AudioPolicyServerHandlerUnitTest, HandleSpatialAudioSourceTypeChange_001, TestSize.Level1)
{
    auto handler = std::make_shared<AudioPolicyServerHandler>();
    sptr<MockAudioPolicyClient> client = new MockAudioPolicyClient();
    auto holder = std::make_shared<AudioPolicyClientHolder>(client);
    int32_t clientPid = 1000;
    handler->audioPolicyClientProxyAPSCbsMap_[clientPid] = holder;
    handler->clientCallbacksMap_[clientPid][CALLBACK_SPATIAL_AUDIO_SOURCE_TYPE_CHANGE] = true;

    bool lockFreeAtCallback = false;
    EXPECT_CALL(*client, OnSpatialAudioSourceTypeChange(_))
        .WillOnce(testing::Invoke([&handler, &lockFreeAtCallback](int32_t mode) {
            EXPECT_EQ(mode, SPATIAL_AUDIO_SOURCE_TYPE_MULTI_CHANNEL);
            lockFreeAtCallback = handler->handleMapMutex_.try_lock();
            if (lockFreeAtCallback) {
                handler->handleMapMutex_.unlock();
            }
            return ERR_OK;
        }));

    auto eventContextObj = std::make_shared<AudioPolicyServerHandler::EventContextObj>();
    eventContextObj->mode = SPATIAL_AUDIO_SOURCE_TYPE_MULTI_CHANNEL;
    auto event = InnerEvent::Get(
        AudioPolicyServerHandler::EventAudioServerCmd::EFFECT_MODE_CHANGE_FOR_ANY_DEVICE, eventContextObj);
    handler->HandleSpatialAudioSourceTypeChange(event);

    EXPECT_TRUE(lockFreeAtCallback);
    handler->clientCallbacksMap_.clear();
    handler->audioPolicyClientProxyAPSCbsMap_.clear();
}

/**
 * @tc.name  : Test HandleSpatialAudioSourceTypeChange skips clients without callback enabled
 * @tc.type  : FUNC
 * @tc.number: HandleSpatialAudioSourceTypeChange_002
 * @tc.desc  : Callback must not be sent when CALLBACK_SPATIAL_AUDIO_SOURCE_TYPE_CHANGE is disabled.
 */
HWTEST(AudioPolicyServerHandlerUnitTest, HandleSpatialAudioSourceTypeChange_002, TestSize.Level1)
{
    auto handler = std::make_shared<AudioPolicyServerHandler>();
    sptr<MockAudioPolicyClient> client = new MockAudioPolicyClient();
    auto holder = std::make_shared<AudioPolicyClientHolder>(client);
    int32_t clientPid = 1001;
    handler->audioPolicyClientProxyAPSCbsMap_[clientPid] = holder;
    handler->clientCallbacksMap_[clientPid][CALLBACK_SET_VOLUME_KEY_EVENT] = true;

    EXPECT_CALL(*client, OnSpatialAudioSourceTypeChange(_)).Times(0);

    auto eventContextObj = std::make_shared<AudioPolicyServerHandler::EventContextObj>();
    eventContextObj->mode = SPATIAL_AUDIO_SOURCE_TYPE_STEREO;
    auto event = InnerEvent::Get(
        AudioPolicyServerHandler::EventAudioServerCmd::EFFECT_MODE_CHANGE_FOR_ANY_DEVICE, eventContextObj);
    handler->HandleSpatialAudioSourceTypeChange(event);

    handler->clientCallbacksMap_.clear();
    handler->audioPolicyClientProxyAPSCbsMap_.clear();
}

} // namespace AudioStandard
} // namespace OHOS
