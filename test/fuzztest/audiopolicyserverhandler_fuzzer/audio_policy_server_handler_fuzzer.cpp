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

#include <iostream>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include "audio_info.h"
#include "audio_policy_server.h"
#include "audio_policy_service.h"
#include "audio_device_info.h"
#include "audio_utils.h"
#include "accesstoken_kit.h"
#include "nativetoken_kit.h"
#include "token_setproc.h"
#include "access_token.h"
#include "audio_channel_blend.h"
#include "volume_ramp.h"
#include "audio_speed.h"

#include "audio_policy_utils.h"
#include "audio_stream_descriptor.h"
#include "audio_limiter_manager.h"
#include "dfx_msg_manager.h"

#include "audio_source_clock.h"
#include "capturer_clock_manager.h"
#include "hpae_policy_manager.h"
#include "audio_policy_state_monitor.h"
#include "audio_device_info.h"
#include "audio_server.h"
#include "audio_effect_volume.h"
#include "futex_tool.h"
#include "format_converter.h"
#include "audio_dump_pcm.h"
#include "audio_dump_pcm_private.h"
#include "audio_zone_service.h"

namespace OHOS {
namespace AudioStandard {
using namespace std;

static const uint8_t* RAW_DATA = nullptr;
static size_t g_dataSize = 0;
static size_t g_pos;
const size_t THRESHOLD = 10;
static int32_t NUM_2 = 2;
std::mutex paElementsMutex_;

typedef void (*TestFuncs)();

template<class T>
T GetData()
{
    T object {};
    size_t objectSize = sizeof(object);
    if (g_dataSize < g_pos) {
        return object;
    }
    if (RAW_DATA == nullptr || objectSize > g_dataSize - g_pos) {
        return object;
    }
    errno_t ret = memcpy_s(&object, objectSize, RAW_DATA + g_pos, objectSize);
    if (ret != EOK) {
        return {};
    }
    g_pos += objectSize;
    return object;
}

template<class T>
uint32_t GetArrLength(T& arr)
{
    if (arr == nullptr) {
        AUDIO_INFO_LOG("%{public}s: The array length is equal to 0", __func__);
        return 0;
    }
    return sizeof(arr) / sizeof(arr[0]);
}

vector<AudioDeviceUsage> AudioDeviceUsageVec = {
    MEDIA_OUTPUT_DEVICES,
    MEDIA_INPUT_DEVICES,
    ALL_MEDIA_DEVICES,
    CALL_OUTPUT_DEVICES,
    CALL_INPUT_DEVICES,
    ALL_CALL_DEVICES,
    D_ALL_DEVICES,
};

vector<AudioStreamType> AudioStreamTypeVec = {
    STREAM_DEFAULT,
    STREAM_VOICE_CALL,
    STREAM_MUSIC,
    STREAM_RING,
    STREAM_MEDIA,
    STREAM_VOICE_ASSISTANT,
    STREAM_SYSTEM,
    STREAM_ALARM,
    STREAM_NOTIFICATION,
    STREAM_BLUETOOTH_SCO,
    STREAM_ENFORCED_AUDIBLE,
    STREAM_DTMF,
    STREAM_TTS,
    STREAM_ACCESSIBILITY,
    STREAM_RECORDING,
    STREAM_MOVIE,
    STREAM_GAME,
    STREAM_SPEECH,
    STREAM_SYSTEM_ENFORCED,
    STREAM_ULTRASONIC,
    STREAM_WAKEUP,
    STREAM_VOICE_MESSAGE,
    STREAM_NAVIGATION,
    STREAM_INTERNAL_FORCE_STOP,
    STREAM_SOURCE_VOICE_CALL,
    STREAM_VOICE_COMMUNICATION,
    STREAM_VOICE_RING,
    STREAM_VOICE_CALL_ASSISTANT,
    STREAM_CAMCORDER,
    STREAM_APP,
    STREAM_TYPE_MAX,
    STREAM_ALL,
};

void AddAudioPolicyClientProxyMapFuzzTest()
{
    auto audioPolicyServerHandler_ = std::make_shared<AudioPolicyServerHandler>();
    if (audioPolicyServerHandler_ == nullptr) {
        return;
    }
    int32_t clientPid = GetData<int32_t>();
    std::shared_ptr<AudioPolicyClientHolder> cb = nullptr;
    audioPolicyServerHandler_->AddAudioPolicyClientProxyMap(clientPid, cb);
    audioPolicyServerHandler_->AddAudioPolicyClientProxyMap(clientPid, cb);
    std::shared_ptr<AudioPolicyClientHolder> cb2 = nullptr;
    audioPolicyServerHandler_->AddAudioPolicyClientProxyMap(clientPid, cb2);
}

void RemoveAudioPolicyClientProxyMapFuzzTest()
{
    auto audioPolicyServerHandler_ = std::make_shared<AudioPolicyServerHandler>();
    if (audioPolicyServerHandler_ == nullptr) {
        return;
    }
    int32_t clientPid = GetData<int32_t>();
    audioPolicyServerHandler_->RemoveAudioPolicyClientProxyMap(clientPid);
}

void AddExternInterruptCbsMapFuzzTest()
{
    auto audioPolicyServerHandler_ = std::make_shared<AudioPolicyServerHandler>();
    if (audioPolicyServerHandler_ == nullptr) {
        return;
    }
    int32_t clientPid = GetData<int32_t>();
    std::shared_ptr<AudioInterruptCallback> audioInterruptCallback = nullptr;
    audioPolicyServerHandler_->AddExternInterruptCbsMap(clientPid, audioInterruptCallback);
}

void AddAvailableDeviceChangeMapFuzzTest()
{
    auto audioPolicyServerHandler_ = std::make_shared<AudioPolicyServerHandler>();
    if (audioPolicyServerHandler_ == nullptr) {
        return;
    }
    int32_t clientPid = GetData<int32_t>();
    uint32_t usageCount = GetData<uint32_t>() % AudioDeviceUsageVec.size();
    AudioDeviceUsage usage = AudioDeviceUsageVec[usageCount];
    std::shared_ptr<AudioPolicyManagerListenerCallback> cb = nullptr;
    audioPolicyServerHandler_->AddAvailableDeviceChangeMap(1, AudioDeviceUsage::ALL_CALL_DEVICES, cb);
    audioPolicyServerHandler_->AddAvailableDeviceChangeMap(1, AudioDeviceUsage::ALL_MEDIA_DEVICES, cb);
    audioPolicyServerHandler_->AddAvailableDeviceChangeMap(clientPid, AudioDeviceUsage::CALL_INPUT_DEVICES, cb);
    audioPolicyServerHandler_->RemoveAvailableDeviceChangeMap(clientPid, usage);
}

void AddDistributedRoutingRoleChangeCbsMapFuzzTest()
{
    auto audioPolicyServerHandler_ = std::make_shared<AudioPolicyServerHandler>();
    if (audioPolicyServerHandler_ == nullptr) {
        return;
    }
    int32_t clientPid = GetData<int32_t>();
    sptr<IStandardAudioRoutingManagerListener> cb = nullptr;
    audioPolicyServerHandler_->AddDistributedRoutingRoleChangeCbsMap(clientPid, cb);
}

void SendDeviceChangedCallbackFuzzTest()
{
    auto audioPolicyServerHandler_ = std::make_shared<AudioPolicyServerHandler>();
    if (audioPolicyServerHandler_ == nullptr) {
        return;
    }
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> descForCb = {};
    bool isConnected = GetData<uint32_t>() % NUM_2;
    audioPolicyServerHandler_->SendDeviceChangedCallback(descForCb, isConnected);
}

void SendAvailableDeviceChangeFuzzTest()
{
    auto audioPolicyServerHandler_ = std::make_shared<AudioPolicyServerHandler>();
    if (audioPolicyServerHandler_ == nullptr) {
        return;
    }
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> descForCb = {};
    bool isConnected = GetData<uint32_t>() % NUM_2;
    audioPolicyServerHandler_->SendAvailableDeviceChange(descForCb, isConnected);
}

void SendAudioSessionDeactiveCallbackFuzzTest()
{
    auto audioPolicyServerHandler_ = std::make_shared<AudioPolicyServerHandler>();
    if (audioPolicyServerHandler_ == nullptr) {
        return;
    }
    std::pair<int32_t, AudioSessionDeactiveEvent> sessionDeactivePair;
    audioPolicyServerHandler_->SendAudioSessionDeactiveCallback(sessionDeactivePair);
}

void SendActiveVolumeTypeChangeCallbackFuzzTest()
{
    auto audioPolicyServerHandler_ = std::make_shared<AudioPolicyServerHandler>();
    if (audioPolicyServerHandler_ == nullptr) {
        return;
    }
    uint32_t index = GetData<uint32_t>() % AudioStreamTypeVec.size();
    AudioVolumeType streamType = AudioStreamTypeVec[index];
    audioPolicyServerHandler_->SendActiveVolumeTypeChangeCallback(streamType);
}

void SendMicStateUpdatedCallbackFuzzTest()
{
    auto audioPolicyServerHandler_ = std::make_shared<AudioPolicyServerHandler>();
    if (audioPolicyServerHandler_ == nullptr) {
        return;
    }
    MicStateChangeEvent micStateChangeEvent;
    micStateChangeEvent.mute = GetData<uint32_t>() % NUM_2;
    audioPolicyServerHandler_->SendMicStateUpdatedCallback(micStateChangeEvent);
}

void SendMicStateWithClientIdCallbackFuzzTest()
{
    auto audioPolicyServerHandler_ = std::make_shared<AudioPolicyServerHandler>();
    if (audioPolicyServerHandler_ == nullptr) {
        return;
    }
    int32_t clientPid = GetData<int32_t>();
    MicStateChangeEvent micStateChangeEvent;
    micStateChangeEvent.mute = GetData<uint32_t>() % NUM_2;
    audioPolicyServerHandler_->SendMicStateWithClientIdCallback(micStateChangeEvent, clientPid);
}

void SendInterruptEventInternalCallbackFuzzTest()
{
    auto audioPolicyServerHandler_ = std::make_shared<AudioPolicyServerHandler>();
    if (audioPolicyServerHandler_ == nullptr) {
        return;
    }
    InterruptEventInternal interruptEventInternal;
    audioPolicyServerHandler_->SendInterruptEventInternalCallback(interruptEventInternal);
}

void SendInterruptEventWithStreamIdCallbackFuzzTest()
{
    auto audioPolicyServerHandler_ = std::make_shared<AudioPolicyServerHandler>();
    if (audioPolicyServerHandler_ == nullptr) {
        return;
    }
    InterruptEventInternal interruptEventInternal;
    uint32_t streamId = GetData<uint32_t>();
    audioPolicyServerHandler_->SendInterruptEventWithStreamIdCallback(interruptEventInternal, streamId);
}

void SendRendererDeviceChangeEventFuzzTest()
{
    auto audioPolicyServerHandler_ = std::make_shared<AudioPolicyServerHandler>();
    if (audioPolicyServerHandler_ == nullptr) {
        return;
    }
    int32_t clientPid = 1;
    uint64_t sessionId = 0;
    AudioDeviceDescriptor outputDeviceInfo;
    AudioStreamDeviceChangeReasonExt reason = AudioStreamDeviceChangeReasonExt::ExtEnum::UNKNOWN;
    audioPolicyServerHandler_->SendRendererDeviceChangeEvent(clientPid, sessionId, outputDeviceInfo, reason);
}

void SendCapturerCreateEventFuzzTest()
{
    std::lock_guard<std::mutex> lock(paElementsMutex_);
    auto audioPolicyServerHandler_ = std::make_shared<AudioPolicyServerHandler>();
    if (AudioCoreService::GetCoreService() == nullptr) {
        return;
    }
    AudioCoreService::GetCoreService()->Init();
    if (audioPolicyServerHandler_ == nullptr || AudioCoreService::GetCoreService()->eventEntry_ == nullptr) {
        return;
    }
    AudioCapturerInfo capturerInfo;
    AudioStreamInfo streamInfo;
    uint64_t sessionId = 0;
    bool isSync = GetData<uint32_t>() % NUM_2;
    int32_t error = 0;
    audioPolicyServerHandler_->SendCapturerCreateEvent(capturerInfo, streamInfo, sessionId, isSync, error);
}

void SendCapturerRemovedEventFuzzTest()
{
    std::lock_guard<std::mutex> lock(paElementsMutex_);
    auto audioPolicyServerHandler_ = std::make_shared<AudioPolicyServerHandler>();
    if (AudioCoreService::GetCoreService() == nullptr) {
        return;
    }
    AudioCoreService::GetCoreService()->Init();
    if (audioPolicyServerHandler_ == nullptr || AudioCoreService::GetCoreService()->eventEntry_ == nullptr) {
        return;
    }
    uint64_t sessionId = 0;
    bool isSync = GetData<uint32_t>() % NUM_2;
    audioPolicyServerHandler_->SendCapturerRemovedEvent(sessionId, isSync);
}

void SendRingerModeUpdatedCallbackFuzzTest()
{
    auto audioPolicyServerHandler_ = std::make_shared<AudioPolicyServerHandler>();
    if (audioPolicyServerHandler_ == nullptr) {
        return;
    }
    AudioRingerMode ringMode = GetData<AudioRingerMode>();
    audioPolicyServerHandler_->SendRingerModeUpdatedCallback(ringMode);
}
 
void SendAppVolumeChangeCallbackFuzzTest()
{
    auto audioPolicyServerHandler_ = std::make_shared<AudioPolicyServerHandler>();
    if (audioPolicyServerHandler_ == nullptr) {
        return;
    }
    int32_t appUid = GetData<int32_t>();
    VolumeEvent volumeEvent;
    audioPolicyServerHandler_->SendAppVolumeChangeCallback(appUid, volumeEvent);
}

void SendInterruptEventCallbackForAudioSessionFuzzTest()
{
    auto audioPolicyServerHandler_ = std::make_shared<AudioPolicyServerHandler>();
    if (audioPolicyServerHandler_ == nullptr) {
        return;
    }
    InterruptEventInternal interruptEvent;
    AudioInterrupt audioInterrupt;
    audioPolicyServerHandler_->SendInterruptEventCallbackForAudioSession(interruptEvent, audioInterrupt);
}
 
void SendInterruptEventWithClientIdCallbackFuzzTest()
{
    auto audioPolicyServerHandler_ = std::make_shared<AudioPolicyServerHandler>();
    if (audioPolicyServerHandler_ == nullptr) {
        return;
    }
    InterruptEventInternal interruptEvent;
    int32_t clientPid = GetData<int32_t>();
    audioPolicyServerHandler_->SendInterruptEventWithClientIdCallback(interruptEvent, clientPid);
}
 
void SendDistributedRoutingRoleChangeFuzzTest()
{
    auto audioPolicyServerHandler_ = std::make_shared<AudioPolicyServerHandler>();
    if (audioPolicyServerHandler_ == nullptr) {
        return;
    }
    std::shared_ptr<AudioDeviceDescriptor> descriptor;
    CastType type = GetData<CastType>();
    audioPolicyServerHandler_->SendDistributedRoutingRoleChange(descriptor, type);
}

void SendSpatializatonEnabledChangeForAnyDeviceEventFuzzTest()
{
    auto audioPolicyServerHandler_ = std::make_shared<AudioPolicyServerHandler>();
    if (audioPolicyServerHandler_ == nullptr) {
        return;
    }
    std::shared_ptr<AudioDeviceDescriptor> selectedAudioDevice;
    bool enabled = GetData<uint32_t>() % NUM_2;
    audioPolicyServerHandler_->SendSpatializatonEnabledChangeForAnyDeviceEvent(selectedAudioDevice, enabled);
}
 
void SendSpatializatonEnabledChangeForCurrentDeviceEventFuzzTest()
{
    auto audioPolicyServerHandler_ = std::make_shared<AudioPolicyServerHandler>();
    if (audioPolicyServerHandler_ == nullptr) {
        return;
    }
    bool enabled = GetData<uint32_t>() % NUM_2;
    audioPolicyServerHandler_->SendSpatializatonEnabledChangeForCurrentDeviceEvent(enabled);
}

void SendHeadTrackingEnabledChangeForAnyDeviceEventFuzzTest()
{
    auto audioPolicyServerHandler_ = std::make_shared<AudioPolicyServerHandler>();
    if (audioPolicyServerHandler_ == nullptr) {
        return;
    }
    std::shared_ptr<AudioDeviceDescriptor> selectedAudioDevice;
    bool enabled = GetData<uint32_t>() % NUM_2;
    audioPolicyServerHandler_->SendHeadTrackingEnabledChangeForAnyDeviceEvent(selectedAudioDevice, enabled);
}
 
void SendPipeStreamCleanEventFuzzTest()
{
    auto audioPolicyServerHandler_ = std::make_shared<AudioPolicyServerHandler>();
    if (audioPolicyServerHandler_ == nullptr) {
        return;
    }
    AudioPipeType pipeType = GetData<AudioPipeType>();
    audioPolicyServerHandler_->SendPipeStreamCleanEvent(pipeType);
}

void HandleVolumeChangeCallbackFuzzTest()
{
    auto audioPolicyServerHandler_ = std::make_shared<AudioPolicyServerHandler>();
    if (audioPolicyServerHandler_ == nullptr) {
        return;
    }
    int32_t clientId = GetData<int32_t>();
    VolumeEvent volumeEvent;
    std::shared_ptr<AudioPolicyClientHolder> audioPolicyClient = nullptr;
    audioPolicyServerHandler_->HandleVolumeChangeCallback(clientId, audioPolicyClient, volumeEvent);
}
 
void HandleHeadTrackingDeviceChangeEventFuzzTest()
{
    auto audioPolicyServerHandler_ = std::make_shared<AudioPolicyServerHandler>();
    if (audioPolicyServerHandler_ == nullptr) {
        return;
    }
    AppExecFwk::InnerEvent::Pointer event =
        AppExecFwk::InnerEvent::Get(AudioPolicyServerHandler::EventAudioServerCmd::NN_STATE_CHANGE, 0);
    audioPolicyServerHandler_->SetClientCallbacksEnable(CallbackChange::CALLBACK_SET_MICROPHONE_BLOCKED, false);
    audioPolicyServerHandler_->HandleHeadTrackingDeviceChangeEvent(event);
}
 
void HandleCapturerCreateEventFuzzTest()
{
    auto audioPolicyServerHandler_ = std::make_shared<AudioPolicyServerHandler>();
    if (audioPolicyServerHandler_ == nullptr) {
        return;
    }
    AppExecFwk::InnerEvent::Pointer event =
        AppExecFwk::InnerEvent::Get(AudioPolicyServerHandler::EventAudioServerCmd::NN_STATE_CHANGE, 0);
    audioPolicyServerHandler_->SetClientCallbacksEnable(CallbackChange::CALLBACK_SET_MICROPHONE_BLOCKED, false);
    audioPolicyServerHandler_->HandleCapturerCreateEvent(event);
}
 
void HandleCapturerRemovedEventFuzzTest()
{
    auto audioPolicyServerHandler_ = std::make_shared<AudioPolicyServerHandler>();
    if (audioPolicyServerHandler_ == nullptr) {
        return;
    }
    AppExecFwk::InnerEvent::Pointer event =
        AppExecFwk::InnerEvent::Get(AudioPolicyServerHandler::EventAudioServerCmd::NN_STATE_CHANGE, 0);
    audioPolicyServerHandler_->SetClientCallbacksEnable(CallbackChange::CALLBACK_SET_MICROPHONE_BLOCKED, false);
    audioPolicyServerHandler_->HandleCapturerRemovedEvent(event);
}
 
void HandleSpatializatonEnabledChangeForAnyDeviceEventFuzzTest()
{
    auto audioPolicyServerHandler_ = std::make_shared<AudioPolicyServerHandler>();
    if (audioPolicyServerHandler_ == nullptr) {
        return;
    }
    AppExecFwk::InnerEvent::Pointer event =
        AppExecFwk::InnerEvent::Get(AudioPolicyServerHandler::EventAudioServerCmd::NN_STATE_CHANGE, 0);
    audioPolicyServerHandler_->SetClientCallbacksEnable(CallbackChange::CALLBACK_SET_MICROPHONE_BLOCKED, false);
    audioPolicyServerHandler_->HandleSpatializatonEnabledChangeForAnyDeviceEvent(event);
}
 
void HandleSpatializatonEnabledChangeForCurrentDeviceEventFuzzTest()
{
    auto audioPolicyServerHandler_ = std::make_shared<AudioPolicyServerHandler>();
    if (audioPolicyServerHandler_ == nullptr) {
        return;
    }
    AppExecFwk::InnerEvent::Pointer event =
        AppExecFwk::InnerEvent::Get(AudioPolicyServerHandler::EventAudioServerCmd::NN_STATE_CHANGE, 0);
    audioPolicyServerHandler_->SetClientCallbacksEnable(CallbackChange::CALLBACK_SET_MICROPHONE_BLOCKED, false);
    audioPolicyServerHandler_->HandleSpatializatonEnabledChangeForCurrentDeviceEvent(event);
}

void HandleHeadTrackingEnabledChangeForAnyDeviceEventFuzzTest()
{
    auto audioPolicyServerHandler_ = std::make_shared<AudioPolicyServerHandler>();
    if (audioPolicyServerHandler_ == nullptr) {
        return;
    }
    AppExecFwk::InnerEvent::Pointer event =
        AppExecFwk::InnerEvent::Get(AudioPolicyServerHandler::EventAudioServerCmd::NN_STATE_CHANGE, 0);
    audioPolicyServerHandler_->SetClientCallbacksEnable(CallbackChange::CALLBACK_SET_MICROPHONE_BLOCKED, false);
    audioPolicyServerHandler_->HandleHeadTrackingEnabledChangeForAnyDeviceEvent(event);
}

void HandleAudioZoneEventFuzzTest()
{
    auto audioPolicyServerHandler_ = std::make_shared<AudioPolicyServerHandler>();
    if (audioPolicyServerHandler_ == nullptr) {
        return;
    }
    AppExecFwk::InnerEvent::Pointer event =
        AppExecFwk::InnerEvent::Get(AudioPolicyServerHandler::EventAudioServerCmd::NN_STATE_CHANGE, 0);
    audioPolicyServerHandler_->SetClientCallbacksEnable(CallbackChange::CALLBACK_SET_MICROPHONE_BLOCKED, false);
    audioPolicyServerHandler_->HandleAudioZoneEvent(event);
}
 
void RemoveDistributedRoutingRoleChangeCbsMapFuzzTest()
{
    auto audioPolicyServerHandler_ = std::make_shared<AudioPolicyServerHandler>();
    if (audioPolicyServerHandler_ == nullptr) {
        return;
    }
    int32_t clientId = GetData<int32_t>();
    audioPolicyServerHandler_->RemoveDistributedRoutingRoleChangeCbsMap(clientId);
}
 
void HandleAudioSessionDeactiveCallbackFuzzTest()
{
    auto audioPolicyServerHandler_ = std::make_shared<AudioPolicyServerHandler>();
    if (audioPolicyServerHandler_ == nullptr) {
        return;
    }
    AppExecFwk::InnerEvent::Pointer event =
        AppExecFwk::InnerEvent::Get(AudioPolicyServerHandler::EventAudioServerCmd::NN_STATE_CHANGE, 0);
    audioPolicyServerHandler_->SetClientCallbacksEnable(CallbackChange::CALLBACK_SET_MICROPHONE_BLOCKED, false);
    audioPolicyServerHandler_->HandleAudioSessionDeactiveCallback(event);
}
 
void HandleRequestCateGoryEventFuzzTest()
{
    auto audioPolicyServerHandler_ = std::make_shared<AudioPolicyServerHandler>();
    if (audioPolicyServerHandler_ == nullptr) {
        return;
    }
    AppExecFwk::InnerEvent::Pointer event =
        AppExecFwk::InnerEvent::Get(AudioPolicyServerHandler::EventAudioServerCmd::NN_STATE_CHANGE, 0);
    audioPolicyServerHandler_->SetClientCallbacksEnable(CallbackChange::CALLBACK_SET_MICROPHONE_BLOCKED, false);
    audioPolicyServerHandler_->HandleRequestCateGoryEvent(event);
}
 
void HandleAbandonCateGoryEventFuzzTest()
{
    auto audioPolicyServerHandler_ = std::make_shared<AudioPolicyServerHandler>();
    if (audioPolicyServerHandler_ == nullptr) {
        return;
    }
    AppExecFwk::InnerEvent::Pointer event =
        AppExecFwk::InnerEvent::Get(AudioPolicyServerHandler::EventAudioServerCmd::NN_STATE_CHANGE, 0);
    audioPolicyServerHandler_->SetClientCallbacksEnable(CallbackChange::CALLBACK_SET_MICROPHONE_BLOCKED, false);
    audioPolicyServerHandler_->HandleAbandonCateGoryEvent(event);
}
 
void HandleFocusInfoChangeEventFuzzTest()
{
    auto audioPolicyServerHandler_ = std::make_shared<AudioPolicyServerHandler>();
    if (audioPolicyServerHandler_ == nullptr) {
        return;
    }
    AppExecFwk::InnerEvent::Pointer event =
        AppExecFwk::InnerEvent::Get(AudioPolicyServerHandler::EventAudioServerCmd::NN_STATE_CHANGE, 0);
    audioPolicyServerHandler_->SetClientCallbacksEnable(CallbackChange::CALLBACK_SET_MICROPHONE_BLOCKED, false);
    audioPolicyServerHandler_->HandleFocusInfoChangeEvent(event);
}
 
void HandleActiveVolumeTypeChangeEventFuzzTest()
{
    auto audioPolicyServerHandler_ = std::make_shared<AudioPolicyServerHandler>();
    if (audioPolicyServerHandler_ == nullptr) {
        return;
    }
    AppExecFwk::InnerEvent::Pointer event =
        AppExecFwk::InnerEvent::Get(AudioPolicyServerHandler::EventAudioServerCmd::NN_STATE_CHANGE, 0);
    audioPolicyServerHandler_->SetClientCallbacksEnable(CallbackChange::CALLBACK_SET_MICROPHONE_BLOCKED, false);
    audioPolicyServerHandler_->HandleActiveVolumeTypeChangeEvent(event);
}
 
void HandleAppVolumeChangeEventFuzzTest()
{
    auto audioPolicyServerHandler_ = std::make_shared<AudioPolicyServerHandler>();
    if (audioPolicyServerHandler_ == nullptr) {
        return;
    }
    AppExecFwk::InnerEvent::Pointer event =
        AppExecFwk::InnerEvent::Get(AudioPolicyServerHandler::EventAudioServerCmd::NN_STATE_CHANGE, 0);
    audioPolicyServerHandler_->SetClientCallbacksEnable(CallbackChange::CALLBACK_SET_MICROPHONE_BLOCKED, false);
    audioPolicyServerHandler_->HandleAppVolumeChangeEvent(event);
}
 
void HandleRingerModeUpdatedEventFuzzTest()
{
    auto audioPolicyServerHandler_ = std::make_shared<AudioPolicyServerHandler>();
    if (audioPolicyServerHandler_ == nullptr) {
        return;
    }
    AppExecFwk::InnerEvent::Pointer event =
        AppExecFwk::InnerEvent::Get(AudioPolicyServerHandler::EventAudioServerCmd::NN_STATE_CHANGE, 0);
    audioPolicyServerHandler_->SetClientCallbacksEnable(CallbackChange::CALLBACK_SET_MICROPHONE_BLOCKED, false);
    audioPolicyServerHandler_->HandleRingerModeUpdatedEvent(event);
}
 
void HandleMicStateUpdatedEventFuzzTest()
{
    auto audioPolicyServerHandler_ = std::make_shared<AudioPolicyServerHandler>();
    if (audioPolicyServerHandler_ == nullptr) {
        return;
    }
    AppExecFwk::InnerEvent::Pointer event =
        AppExecFwk::InnerEvent::Get(AudioPolicyServerHandler::EventAudioServerCmd::NN_STATE_CHANGE, 0);
    audioPolicyServerHandler_->SetClientCallbacksEnable(CallbackChange::CALLBACK_SET_MICROPHONE_BLOCKED, false);
    audioPolicyServerHandler_->HandleMicStateUpdatedEvent(event);
}
 
void HandleMicStateUpdatedEventWithClientIdFuzzTest()
{
    auto audioPolicyServerHandler_ = std::make_shared<AudioPolicyServerHandler>();
    if (audioPolicyServerHandler_ == nullptr) {
        return;
    }
    AppExecFwk::InnerEvent::Pointer event =
        AppExecFwk::InnerEvent::Get(AudioPolicyServerHandler::EventAudioServerCmd::NN_STATE_CHANGE, 0);
    audioPolicyServerHandler_->SetClientCallbacksEnable(CallbackChange::CALLBACK_SET_MICROPHONE_BLOCKED, false);
    audioPolicyServerHandler_->HandleMicStateUpdatedEventWithClientId(event);
}
 
void HandleInterruptEventFuzzTest()
{
    auto audioPolicyServerHandler_ = std::make_shared<AudioPolicyServerHandler>();
    if (audioPolicyServerHandler_ == nullptr) {
        return;
    }
    AppExecFwk::InnerEvent::Pointer event =
        AppExecFwk::InnerEvent::Get(AudioPolicyServerHandler::EventAudioServerCmd::NN_STATE_CHANGE, 0);
    audioPolicyServerHandler_->SetClientCallbacksEnable(CallbackChange::CALLBACK_SET_MICROPHONE_BLOCKED, false);
    audioPolicyServerHandler_->HandleInterruptEvent(event);
}

void HandleInterruptEventForAudioSessionFuzzTest()
{
    auto audioPolicyServerHandler_ = std::make_shared<AudioPolicyServerHandler>();
    if (audioPolicyServerHandler_ == nullptr) {
        return;
    }
    AppExecFwk::InnerEvent::Pointer event =
        AppExecFwk::InnerEvent::Get(AudioPolicyServerHandler::EventAudioServerCmd::NN_STATE_CHANGE, 0);
    audioPolicyServerHandler_->SetClientCallbacksEnable(CallbackChange::CALLBACK_SET_MICROPHONE_BLOCKED, false);
    audioPolicyServerHandler_->HandleInterruptEventForAudioSession(event);
}
 
void HandleInterruptEventWithClientIdFuzzTest()
{
    auto audioPolicyServerHandler_ = std::make_shared<AudioPolicyServerHandler>();
    if (audioPolicyServerHandler_ == nullptr) {
        return;
    }
    AppExecFwk::InnerEvent::Pointer event =
        AppExecFwk::InnerEvent::Get(AudioPolicyServerHandler::EventAudioServerCmd::NN_STATE_CHANGE, 0);
    audioPolicyServerHandler_->SetClientCallbacksEnable(CallbackChange::CALLBACK_SET_MICROPHONE_BLOCKED, false);
    audioPolicyServerHandler_->HandleInterruptEventWithClientId(event);
}
 
void HandleDistributedRoutingRoleChangeEventFuzzTest()
{
    auto audioPolicyServerHandler_ = std::make_shared<AudioPolicyServerHandler>();
    if (audioPolicyServerHandler_ == nullptr) {
        return;
    }
    AppExecFwk::InnerEvent::Pointer event =
        AppExecFwk::InnerEvent::Get(AudioPolicyServerHandler::EventAudioServerCmd::NN_STATE_CHANGE, 0);
    audioPolicyServerHandler_->SetClientCallbacksEnable(CallbackChange::CALLBACK_SET_MICROPHONE_BLOCKED, false);
    audioPolicyServerHandler_->HandleDistributedRoutingRoleChangeEvent(event);
}
 
void HandleVolumeKeyEventToRssWhenAccountsChangeFuzzTest()
{
    auto audioPolicyServerHandler_ = std::make_shared<AudioPolicyServerHandler>();
    if (audioPolicyServerHandler_ == nullptr) {
        return;
    }
    AppExecFwk::InnerEvent::Pointer event =
        AppExecFwk::InnerEvent::Get(AudioPolicyServerHandler::EventAudioServerCmd::NN_STATE_CHANGE, 0);
    std::shared_ptr<AudioPolicyServerHandler::EventContextObj> eventContextObj =
        event->GetSharedObject<AudioPolicyServerHandler::EventContextObj>();
    audioPolicyServerHandler_->HandleVolumeKeyEventToRssWhenAccountsChange(eventContextObj);
}

TestFuncs g_testFuncs[] = {
    AddAudioPolicyClientProxyMapFuzzTest,
    RemoveAudioPolicyClientProxyMapFuzzTest,
    AddExternInterruptCbsMapFuzzTest,
    AddAvailableDeviceChangeMapFuzzTest,
    AddDistributedRoutingRoleChangeCbsMapFuzzTest,
    SendDeviceChangedCallbackFuzzTest,
    SendAvailableDeviceChangeFuzzTest,
    SendAudioSessionDeactiveCallbackFuzzTest,
    SendActiveVolumeTypeChangeCallbackFuzzTest,
    SendMicStateUpdatedCallbackFuzzTest,
    SendMicStateWithClientIdCallbackFuzzTest,
    SendInterruptEventInternalCallbackFuzzTest,
    SendInterruptEventWithStreamIdCallbackFuzzTest,
    SendRendererDeviceChangeEventFuzzTest,
    SendCapturerCreateEventFuzzTest,
    SendCapturerRemovedEventFuzzTest,
    SendRingerModeUpdatedCallbackFuzzTest,
    SendAppVolumeChangeCallbackFuzzTest,
    SendInterruptEventCallbackForAudioSessionFuzzTest,
    SendInterruptEventWithClientIdCallbackFuzzTest,
    SendDistributedRoutingRoleChangeFuzzTest,
    SendSpatializatonEnabledChangeForAnyDeviceEventFuzzTest,
    SendSpatializatonEnabledChangeForCurrentDeviceEventFuzzTest,
    SendHeadTrackingEnabledChangeForAnyDeviceEventFuzzTest,
    SendPipeStreamCleanEventFuzzTest,
    HandleVolumeChangeCallbackFuzzTest,
    HandleHeadTrackingDeviceChangeEventFuzzTest,
    HandleCapturerCreateEventFuzzTest,
    HandleCapturerRemovedEventFuzzTest,
    HandleSpatializatonEnabledChangeForAnyDeviceEventFuzzTest,
    HandleSpatializatonEnabledChangeForCurrentDeviceEventFuzzTest,
    HandleHeadTrackingEnabledChangeForAnyDeviceEventFuzzTest,
    HandleAudioZoneEventFuzzTest,
    RemoveDistributedRoutingRoleChangeCbsMapFuzzTest,
    HandleAudioSessionDeactiveCallbackFuzzTest,
    HandleRequestCateGoryEventFuzzTest,
    HandleAbandonCateGoryEventFuzzTest,
    HandleFocusInfoChangeEventFuzzTest,
    HandleActiveVolumeTypeChangeEventFuzzTest,
    HandleAppVolumeChangeEventFuzzTest,
    HandleRingerModeUpdatedEventFuzzTest,
    HandleMicStateUpdatedEventFuzzTest,
    HandleMicStateUpdatedEventWithClientIdFuzzTest,
    HandleInterruptEventFuzzTest,
    HandleInterruptEventForAudioSessionFuzzTest,
    HandleInterruptEventWithClientIdFuzzTest,
    HandleDistributedRoutingRoleChangeEventFuzzTest,
    HandleVolumeKeyEventToRssWhenAccountsChangeFuzzTest,
};

void FuzzTest(const uint8_t* rawData, size_t size)
{
    if (rawData == nullptr) {
        return;
    }

    // initialize data
    RAW_DATA = rawData;
    g_dataSize = size;
    g_pos = 0;

    uint32_t code = GetData<uint32_t>();
    uint32_t len = GetArrLength(g_testFuncs);
    if (len > 0) {
        g_testFuncs[code % len]();
    } else {
        AUDIO_INFO_LOG("%{public}s: The len length is equal to 0", __func__);
    }

    return;
}
} // namespace AudioStandard
} // namesapce OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    if (size < OHOS::AudioStandard::THRESHOLD) {
        return 0;
    }

    OHOS::AudioStandard::FuzzTest(data, size);
    return 0;
}
