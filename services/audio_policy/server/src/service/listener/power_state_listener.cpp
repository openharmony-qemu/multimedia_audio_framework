/*
 * Copyright (c) 2023-2023 Huawei Device Co., Ltd.
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
#ifndef LOG_TAG
#define LOG_TAG "PowerStateListener"
#endif

#include "power_state_listener.h"

#include "suspend/sync_sleep_callback_ipc_interface_code.h"
#include "hibernate/sync_hibernate_callback_ipc_interface_code.h"
#include "audio_policy_server.h"

namespace OHOS {
namespace AudioStandard {
using namespace std::chrono_literals;
const int32_t AUDIO_INTERRUPT_SESSION_ID = 10000;

void PowerListerMethods::InitAudioInterruptInfo(AudioInterrupt& audioInterrupt)
{
    audioInterrupt.contentType = ContentType::CONTENT_TYPE_UNKNOWN;
    audioInterrupt.streamUsage = StreamUsage::STREAM_USAGE_UNKNOWN;
    audioInterrupt.audioFocusType.streamType = AudioStreamType::STREAM_INTERNAL_FORCE_STOP;
    audioInterrupt.audioFocusType.sourceType = SOURCE_TYPE_INVALID;
    audioInterrupt.audioFocusType.isPlay = true;
    audioInterrupt.streamId = AUDIO_INTERRUPT_SESSION_ID;
    audioInterrupt.pid = getpid();
}

int32_t PowerStateListenerStub::OnRemoteRequest(uint32_t code, MessageParcel &data,
    MessageParcel &reply, MessageOption &option)
{
    AUDIO_DEBUG_LOG("code = %{public}d, flag = %{public}d", code, option.GetFlags());
    std::u16string descriptor = PowerStateListenerStub::GetDescriptor();
    std::u16string remoteDescriptor = data.ReadInterfaceToken();
    CHECK_AND_RETURN_RET_LOG(descriptor == remoteDescriptor, -1, "Descriptor not match");

    int32_t ret = ERR_OK;
    switch (code) {
        case static_cast<int32_t>(PowerMgr::SyncSleepCallbackInterfaceCode::CMD_ON_SYNC_SLEEP):
            ret = OnSyncSleepCallbackStub(data);
            break;
        case static_cast<int32_t>(PowerMgr::SyncSleepCallbackInterfaceCode::CMD_ON_SYNC_WAKEUP):
            ret = OnSyncWakeupCallbackStub(data);
            break;
        default:
            ret = IPCObjectStub::OnRemoteRequest(code, data, reply, option);
            break;
    }

    return ret;
}

int32_t PowerStateListenerStub::OnSyncSleepCallbackStub(MessageParcel &data)
{
    bool forceSleep = data.ReadBool();
    OnSyncSleep(forceSleep);

    return ERR_OK;
}

int32_t PowerStateListenerStub::OnSyncWakeupCallbackStub(MessageParcel &data)
{
    bool forceSleep = data.ReadBool();
    OnSyncWakeup(forceSleep);

    return ERR_OK;
}

PowerStateListener::PowerStateListener(const sptr<AudioPolicyServer> audioPolicyServer)
    : audioPolicyServer_(audioPolicyServer) {}

void PowerStateListener::OnSyncSleep(bool OnForceSleep)
{
    CHECK_AND_RETURN_LOG(OnForceSleep, "OnSyncSleep not force sleep");

    AUDIO_INFO_LOG("OnSyncSleep, try to control audio focus");
    ControlAudioFocus(true);
}

void PowerStateListener::OnSyncWakeup(bool OnForceSleep)
{
    CHECK_AND_RETURN_LOG(OnForceSleep, "OnSyncWakeup not force sleep");

    AUDIO_INFO_LOG("OnSyncWakeup, try to release audio focus");
    ControlAudioFocus(false);
}

void PowerStateListener::ControlAudioFocus(bool applyFocus)
{
    if (audioPolicyServer_ == nullptr) {
        AUDIO_ERR_LOG("audioPolicyServer_ is nullptr");
        return;
    }
    std::lock_guard<std::mutex> lock(focusMutex_);
    AUDIO_INFO_LOG("control audio focus, want: %{public}d, last: %{public}d", applyFocus, isAudioFocusApplied_);

    AudioInterrupt audioInterrupt;
    PowerListerMethods::InitAudioInterruptInfo(audioInterrupt);

    int32_t result = -1;
    if (applyFocus == true && isAudioFocusApplied_ == false) {
        result = audioPolicyServer_->ActivateAudioInterrupt(audioInterrupt);
        if (result == SUCCESS) {
            isAudioFocusApplied_ = true;
            audioPolicyServer_->CheckConnectedDevice();
        } else {
            AUDIO_WARNING_LOG("Activate audio interrupt failed, err = %{public}d", result);
        }
    } else if (applyFocus == false && isAudioFocusApplied_ == true) {
        result = audioPolicyServer_->DeactivateAudioInterrupt(audioInterrupt);
        if (result == SUCCESS) {
            isAudioFocusApplied_ = false;
            thread switchThread(&AudioPolicyServer::SetDeviceConnectedFlagFalseAfterDuration, audioPolicyServer_);
            switchThread.detach();
        } else {
            AUDIO_WARNING_LOG("Deactivate audio interrupt failed, err = %{public}d", result);
        }
    }
}

int32_t SyncHibernateListenerStub::OnRemoteRequest(uint32_t code, MessageParcel &data,
    MessageParcel &reply, MessageOption &option)
{
    AUDIO_DEBUG_LOG("code = %{public}d, flag = %{public}d", code, option.GetFlags());
    std::u16string descriptor = SyncHibernateListenerStub::GetDescriptor();
    std::u16string remoteDescriptor = data.ReadInterfaceToken();
    CHECK_AND_RETURN_RET_LOG(descriptor == remoteDescriptor, -1, "Descriptor not match");
 
    int32_t ret = ERR_OK;
    switch (code) {
        case static_cast<int32_t>(PowerMgr::SyncHibernateCallbackInterfaceCode::CMD_ON_SYNC_HIBERNATE):
            ret = OnSyncHibernateCallbackStub();
            break;
        case static_cast<int32_t>(PowerMgr::SyncHibernateCallbackInterfaceCode::CMD_ON_SYNC_WAKEUP):
            ret = OnSyncWakeupCallbackStub();
            break;
        default:
            ret = IPCObjectStub::OnRemoteRequest(code, data, reply, option);
            break;
    }
 
    return ret;
}
 
int32_t SyncHibernateListenerStub::OnSyncHibernateCallbackStub()
{
    OnSyncHibernate();
 
    return ERR_OK;
}
 
int32_t SyncHibernateListenerStub::OnSyncWakeupCallbackStub()
{
    OnSyncWakeup();
 
    return ERR_OK;
}
 
SyncHibernateListener::SyncHibernateListener(const sptr<AudioPolicyServer> audioPolicyServer)
    : audioPolicyServer_(audioPolicyServer) {}
 
void SyncHibernateListener::OnSyncHibernate()
{
    AUDIO_INFO_LOG("OnSyncHibernate in hibernate");
    ControlAudioFocus(true);
    CHECK_AND_RETURN_LOG(audioPolicyServer_, "audioPolicyServer_ is nullptr");
    audioPolicyServer_->CheckHibernateState(true);
}
 
void SyncHibernateListener::OnSyncWakeup(bool hibernateResult)
{
    AUDIO_INFO_LOG("OnSyncWakeup in hibernate");
    ControlAudioFocus(false);
    CHECK_AND_RETURN_LOG(audioPolicyServer_, "audioPolicyServer_ is nullptr");
    audioPolicyServer_->CheckHibernateState(false);
    audioPolicyServer_->UpdateSafeVolumeByS4();
}
 
void SyncHibernateListener::ControlAudioFocus(bool isHibernate)
{
    if (audioPolicyServer_ == nullptr) {
        AUDIO_ERR_LOG("audioPolicyServer_ is nullptr");
        return;
    }
    
    AudioInterrupt audioInterrupt;
    PowerListerMethods::InitAudioInterruptInfo(audioInterrupt);
 
    int32_t result = -1;
    if (isHibernate) {
        result = audioPolicyServer_->ActivateAudioInterrupt(audioInterrupt);
        if (result != SUCCESS) {
            AUDIO_WARNING_LOG("Sync hibernate activate audio interrupt failed, err = %{public}d", result);
        }
        audioPolicyServer_->CheckConnectedDevice();
    } else {
        result = audioPolicyServer_->DeactivateAudioInterrupt(audioInterrupt);
        if (result != SUCCESS) {
            AUDIO_WARNING_LOG("Sync hibernate deactivate audio interrupt failed, err = %{public}d", result);
        }
        thread switchThread(&AudioPolicyServer::SetDeviceConnectedFlagFalseAfterDuration, audioPolicyServer_);
        switchThread.detach();
    }
}
} // namespace AudioStandard
} // namespace OHOS
