/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#include "audio_policy_manager.h"
#include "audio_errors.h"
#include "audio_policy_proxy.h"
#include "audio_server_death_recipient.h"
#include "audio_log.h"
#include "audio_utils.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace AudioStandard {
using namespace std;

static sptr<IAudioPolicy> g_apProxy = nullptr;
mutex g_apProxyMutex;
constexpr int64_t SLEEP_TIME = 1;
constexpr int32_t RETRY_TIMES = 3;
std::mutex g_cBMapMutex;
std::mutex g_cBDiedMapMutex;
std::unordered_map<int32_t, std::weak_ptr<AudioRendererPolicyServiceDiedCallback>> AudioPolicyManager::rendererCBMap_;
sptr<AudioPolicyClientStubImpl> AudioPolicyManager::audioStaticPolicyClientStubCB_;
std::vector<std::shared_ptr<AudioStreamPolicyServiceDiedCallback>> AudioPolicyManager::audioStreamCBMap_;

inline const sptr<IAudioPolicy> GetAudioPolicyManagerProxy()
{
    AUDIO_DEBUG_LOG("Start to get audio manager service proxy.");
    lock_guard<mutex> lock(g_apProxyMutex);

    if (g_apProxy == nullptr) {
        auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
        CHECK_AND_RETURN_RET_LOG(samgr != nullptr, nullptr, "samgr init failed.");

        sptr<IRemoteObject> object = samgr->GetSystemAbility(AUDIO_POLICY_SERVICE_ID);
        CHECK_AND_RETURN_RET_LOG(object != nullptr, nullptr, "Object is NULL.");

        g_apProxy = iface_cast<IAudioPolicy>(object);
        CHECK_AND_RETURN_RET_LOG(g_apProxy != nullptr, nullptr, "Init g_apProxy is NULL.");

        AUDIO_DEBUG_LOG("Init g_apProxy is assigned.");
        pid_t pid = 0;
        sptr<AudioServerDeathRecipient> deathRecipient_ = new(std::nothrow) AudioServerDeathRecipient(pid);
        if (deathRecipient_ != nullptr) {
            deathRecipient_->SetNotifyCb(std::bind(&AudioPolicyManager::AudioPolicyServerDied,
                std::placeholders::_1));
            AUDIO_DEBUG_LOG("Register audio policy server death recipient");
            bool result = object->AddDeathRecipient(deathRecipient_);
            if (!result) {
                AUDIO_ERR_LOG("failed to add deathRecipient");
            }
        }
    }

    const sptr<IAudioPolicy> gsp = g_apProxy;
    return gsp;
}

int32_t AudioPolicyManager::RegisterPolicyCallbackClientFunc(const sptr<IAudioPolicy> &gsp)
{
    std::unique_lock<std::mutex> lock(registerCallbackMutex_);
    audioPolicyClientStubCB_ = new(std::nothrow) AudioPolicyClientStubImpl();
    audioStaticPolicyClientStubCB_ = audioPolicyClientStubCB_;
    sptr<IRemoteObject> object = audioPolicyClientStubCB_->AsObject();
    if (object == nullptr) {
        AUDIO_ERR_LOG("audioPolicyClientStubCB_->AsObject is nullptr");
        lock.unlock();
        return ERROR;
    }
    lock.unlock();

    return gsp->RegisterPolicyCallbackClient(object);
}

void AudioPolicyManager::RecoverAudioPolicyCallbackClient()
{
    if (audioStaticPolicyClientStubCB_ == nullptr) {
        AUDIO_ERR_LOG("audioPolicyClientStubCB_ is null.");
        return;
    }

    int32_t retry = RETRY_TIMES;
    sptr<IAudioPolicy> gsp = nullptr;
    while (retry--) {
        // Sleep and wait for 1 second;
        sleep(SLEEP_TIME);
        gsp = GetAudioPolicyManagerProxy();
        if (gsp != nullptr) {
            AUDIO_INFO_LOG("Reconnect audio policy service success!");
            break;
        }
    }

    CHECK_AND_RETURN_LOG(gsp != nullptr, "Reconnect audio policy service fail!");

    sptr<IRemoteObject> object = audioStaticPolicyClientStubCB_->AsObject();
    if (object == nullptr) {
        AUDIO_ERR_LOG("RegisterPolicyCallbackClientFunc: audioPolicyClientStubCB_->AsObject is nullptr");
        return;
    }

    gsp->RegisterPolicyCallbackClient(object);
}

void AudioPolicyManager::AudioPolicyServerDied(pid_t pid)
{
    if (g_apProxy == nullptr) {
        AUDIO_ERR_LOG("Audio policy server has already died!");
        return;
    }
    {
        std::lock_guard<std::mutex> lockCbMap(g_cBMapMutex);
        AUDIO_INFO_LOG("Audio policy server died: reestablish connection");
        std::shared_ptr<AudioRendererPolicyServiceDiedCallback> cb;
        for (auto it = rendererCBMap_.begin(); it != rendererCBMap_.end(); ++it) {
            cb = it->second.lock();
            if (cb != nullptr) {
                cb->OnAudioPolicyServiceDied();
            }
        }
    }
    {
        std::lock_guard<std::mutex> lock(g_apProxyMutex);
        g_apProxy = nullptr;
    }
    RecoverAudioPolicyCallbackClient();

    {
        std::lock_guard<std::mutex> lockCbMap(g_cBDiedMapMutex);
        if (audioStreamCBMap_.size() != 0) {
            for (auto it = audioStreamCBMap_.begin(); it != audioStreamCBMap_.end(); ++it) {
                if (*it != nullptr) {
                    (*it)->OnAudioPolicyServiceDied();
                }
            }
        }
    }
}

int32_t AudioPolicyManager::GetMaxVolumeLevel(AudioVolumeType volumeType)
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, -1, "audio policy manager proxy is NULL.");

    return gsp->GetMaxVolumeLevel(volumeType);
}

int32_t AudioPolicyManager::GetMinVolumeLevel(AudioVolumeType volumeType)
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, -1, "audio policy manager proxy is NULL.");

    return gsp->GetMinVolumeLevel(volumeType);
}

int32_t AudioPolicyManager::SetSystemVolumeLevel(AudioVolumeType volumeType, int32_t volumeLevel, API_VERSION api_v)
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, -1, "audio policy manager proxy is NULL.");

    return gsp->SetSystemVolumeLevel(volumeType, volumeLevel, api_v);
}

int32_t AudioPolicyManager::SetRingerMode(AudioRingerMode ringMode, API_VERSION api_v)
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, -1, "audio policy manager proxy is NULL.");
    return gsp->SetRingerMode(ringMode, api_v);
}

AudioRingerMode AudioPolicyManager::GetRingerMode()
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, RINGER_MODE_NORMAL, "audio policy manager proxy is NULL.");
    return gsp->GetRingerMode();
}

int32_t AudioPolicyManager::SetAudioScene(AudioScene scene)
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, -1, "audio policy manager proxy is NULL.");
    return gsp->SetAudioScene(scene);
}

int32_t AudioPolicyManager::SetMicrophoneMute(bool isMute)
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, -1, "audio policy manager proxy is NULL.");
    return gsp->SetMicrophoneMute(isMute);
}

int32_t AudioPolicyManager::SetMicrophoneMuteAudioConfig(bool isMute)
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, -1, "audio policy manager proxy is NULL.");
    return gsp->SetMicrophoneMuteAudioConfig(isMute);
}

bool AudioPolicyManager::IsMicrophoneMute(API_VERSION api_v)
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, -1, "audio policy manager proxy is NULL.");
    return gsp->IsMicrophoneMute(api_v);
}

AudioScene AudioPolicyManager::GetAudioScene()
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, AUDIO_SCENE_DEFAULT, "audio policy manager proxy is NULL.");
    return gsp->GetAudioScene();
}

int32_t AudioPolicyManager::GetSystemVolumeLevel(AudioVolumeType volumeType)
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, -1, "audio policy manager proxy is NULL.");
    return gsp->GetSystemVolumeLevel(volumeType);
}

int32_t AudioPolicyManager::SetStreamMute(AudioVolumeType volumeType, bool mute, API_VERSION api_v)
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, -1, "audio policy manager proxy is NULL.");
    return gsp->SetStreamMute(volumeType, mute, api_v);
}

bool AudioPolicyManager::GetStreamMute(AudioVolumeType volumeType)
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, false, "audio policy manager proxy is NULL.");
    return gsp->GetStreamMute(volumeType);
}

int32_t AudioPolicyManager::SetLowPowerVolume(int32_t streamId, float volume)
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, -1, "audio policy manager proxy is NULL.");
    return gsp->SetLowPowerVolume(streamId, volume);
}

float AudioPolicyManager::GetLowPowerVolume(int32_t streamId)
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, -1, "audio policy manager proxy is NULL.");
    return gsp->GetLowPowerVolume(streamId);
}

float AudioPolicyManager::GetSingleStreamVolume(int32_t streamId)
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, -1, "audio policy manager proxy is NULL.");
    return gsp->GetSingleStreamVolume(streamId);
}

bool AudioPolicyManager::IsStreamActive(AudioVolumeType volumeType)
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, false, "audio policy manager proxy is NULL.");
    return gsp->IsStreamActive(volumeType);
}

int32_t AudioPolicyManager::SelectOutputDevice(sptr<AudioRendererFilter> audioRendererFilter,
    std::vector<sptr<AudioDeviceDescriptor>> audioDeviceDescriptors)
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, -1, "audio policy manager proxy is NULL.");
    return gsp->SelectOutputDevice(audioRendererFilter, audioDeviceDescriptors);
}

std::string AudioPolicyManager::GetSelectedDeviceInfo(int32_t uid, int32_t pid, AudioStreamType streamType)
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, "", "audio policy manager proxy is NULL.");
    return gsp->GetSelectedDeviceInfo(uid, pid, streamType);
}

int32_t AudioPolicyManager::SelectInputDevice(sptr<AudioCapturerFilter> audioCapturerFilter,
    std::vector<sptr<AudioDeviceDescriptor>> audioDeviceDescriptors)
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, -1, "audio policy manager proxy is NULL.");
    return gsp->SelectInputDevice(audioCapturerFilter, audioDeviceDescriptors);
}

std::vector<sptr<AudioDeviceDescriptor>> AudioPolicyManager::GetDevices(DeviceFlag deviceFlag)
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    if (gsp == nullptr) {
        AUDIO_ERR_LOG("GetDevices: audio policy manager proxy is NULL.");
        std::vector<sptr<AudioDeviceDescriptor>> deviceInfo;
        return deviceInfo;
    }
    return gsp->GetDevices(deviceFlag);
}

std::vector<sptr<AudioDeviceDescriptor>> AudioPolicyManager::GetPreferredOutputDeviceDescriptors(
    AudioRendererInfo &rendererInfo)
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    if (gsp == nullptr) {
        AUDIO_ERR_LOG("GetPreferredOutputDeviceDescriptors: audio policy manager proxy is NULL.");
        std::vector<sptr<AudioDeviceDescriptor>> deviceInfo;
        return deviceInfo;
    }
    return gsp->GetPreferredOutputDeviceDescriptors(rendererInfo);
}

std::vector<sptr<AudioDeviceDescriptor>> AudioPolicyManager::GetPreferredInputDeviceDescriptors(
    AudioCapturerInfo &captureInfo)
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    if (gsp == nullptr) {
        AUDIO_ERR_LOG("audio policy manager proxy is NULL.");
        std::vector<sptr<AudioDeviceDescriptor>> deviceInfo;
        return deviceInfo;
    }
    return gsp->GetPreferredInputDeviceDescriptors(captureInfo);
}

int32_t AudioPolicyManager::GetAudioFocusInfoList(std::list<std::pair<AudioInterrupt, AudioFocuState>> &focusInfoList,
    const int32_t zoneID)
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, -1, "audio policy manager proxy is NULL.");
    return gsp->GetAudioFocusInfoList(focusInfoList, zoneID);
}

int32_t AudioPolicyManager::RegisterFocusInfoChangeCallback(const int32_t clientId,
    const std::shared_ptr<AudioFocusInfoChangeCallback> &callback)
{
    AUDIO_DEBUG_LOG("AudioPolicyManager::RegisterFocusInfoChangeCallback");
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, ERROR, "audio policy manager proxy is NULL.");
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, ERR_INVALID_PARAM,
        "RegisterFocusInfoChangeCallback: callback is nullptr");

    if (audioPolicyClientStubCB_ == nullptr) {
        int32_t ret = RegisterPolicyCallbackClientFunc(gsp);
        if (ret != SUCCESS) {
            return ret;
        }
    }

    audioPolicyClientStubCB_->AddFocusInfoChangeCallback(callback);
    return SUCCESS;
}

int32_t AudioPolicyManager::UnregisterFocusInfoChangeCallback(const int32_t clientId)
{
    AUDIO_DEBUG_LOG("AudioPolicyManager::UnregisterFocusInfoChangeCallback");
    if (audioPolicyClientStubCB_ != nullptr) {
        audioPolicyClientStubCB_->RemoveFocusInfoChangeCallback();
    }
    return SUCCESS;
}

#ifdef FEATURE_DTMF_TONE
std::vector<int32_t> AudioPolicyManager::GetSupportedTones()
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    if (gsp == nullptr) {
        AUDIO_ERR_LOG("audio policy manager proxy is NULL.");
        std::vector<int> lSupportedToneList = {};
        return lSupportedToneList;
    }
    return gsp->GetSupportedTones();
}

std::shared_ptr<ToneInfo> AudioPolicyManager::GetToneConfig(int32_t ltonetype)
{
    AUDIO_DEBUG_LOG("AudioPolicyManager::GetToneConfig");

    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, nullptr, "audio policy manager proxy is NULL.");
    return gsp->GetToneConfig(ltonetype);
}
#endif

int32_t AudioPolicyManager::SetDeviceActive(InternalDeviceType deviceType, bool active)
{
    AUDIO_INFO_LOG("SetDeviceActive deviceType: %{public}d, active: %{public}d", deviceType, active);
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, -1, "audio policy manager proxy is NULL.");
    return gsp->SetDeviceActive(deviceType, active);
}

bool AudioPolicyManager::IsDeviceActive(InternalDeviceType deviceType)
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, false, "audio policy manager proxy is NULL.");
    return gsp->IsDeviceActive(deviceType);
}

DeviceType AudioPolicyManager::GetActiveOutputDevice()
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, DEVICE_TYPE_INVALID, "audio policy manager proxy is NULL.");
    return gsp->GetActiveOutputDevice();
}

DeviceType AudioPolicyManager::GetActiveInputDevice()
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, DEVICE_TYPE_INVALID, "audio policy manager proxy is NULL.");
    return gsp->GetActiveInputDevice();
}

int32_t AudioPolicyManager::SetRingerModeCallback(const int32_t clientId,
    const std::shared_ptr<AudioRingerModeCallback> &callback, API_VERSION api_v)
{
    AUDIO_DEBUG_LOG("AudioPolicyManager::SetRingerModeCallback");
    if (api_v == API_8 && !PermissionUtil::VerifySystemPermission()) {
        AUDIO_ERR_LOG("SetRingerModeCallback: No system permission");
        return ERR_PERMISSION_DENIED;
    }

    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, ERR_INVALID_PARAM, "audio policy manager proxy is NULL.");
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, ERR_INVALID_PARAM, "callback is nullptr");

    if (audioPolicyClientStubCB_ == nullptr) {
        int32_t ret = RegisterPolicyCallbackClientFunc(gsp);
        if (ret != SUCCESS) {
            return ret;
        }
    }
    audioPolicyClientStubCB_->AddRingerModeCallback(callback);
    return SUCCESS;
}

int32_t AudioPolicyManager::UnsetRingerModeCallback(const int32_t clientId)
{
    AUDIO_DEBUG_LOG("Remove all ringer mode callbacks");
    if (audioPolicyClientStubCB_ != nullptr) {
        audioPolicyClientStubCB_->RemoveRingerModeCallback();
    }
    return SUCCESS;
}

int32_t AudioPolicyManager::UnsetRingerModeCallback(const int32_t clientId,
    const std::shared_ptr<AudioRingerModeCallback> &callback)
{
    AUDIO_DEBUG_LOG("Remove one ringer mode callback");
    if (audioPolicyClientStubCB_ != nullptr) {
        audioPolicyClientStubCB_->RemoveRingerModeCallback(callback);
    }
    return SUCCESS;
}

int32_t AudioPolicyManager::SetDeviceChangeCallback(const int32_t clientId, const DeviceFlag flag,
    const std::shared_ptr<AudioManagerDeviceChangeCallback> &callback)
{
    AUDIO_DEBUG_LOG("AudioPolicyManager::SetDeviceChangeCallback");
    bool hasSystemPermission = PermissionUtil::VerifySystemPermission();
    switch (flag) {
        case NONE_DEVICES_FLAG:
        case DISTRIBUTED_OUTPUT_DEVICES_FLAG:
        case DISTRIBUTED_INPUT_DEVICES_FLAG:
        case ALL_DISTRIBUTED_DEVICES_FLAG:
        case ALL_L_D_DEVICES_FLAG:
            if (!hasSystemPermission) {
                AUDIO_ERR_LOG("SetDeviceChangeCallback: No system permission");
                return ERR_PERMISSION_DENIED;
            }
            break;
        default:
            break;
    }

    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, -1, "audio policy manager proxy is NULL.");
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, ERR_INVALID_PARAM, "SetDeviceChangeCallback: callback is nullptr");

    if (audioPolicyClientStubCB_ == nullptr) {
        int32_t ret = RegisterPolicyCallbackClientFunc(gsp);
        if (ret != SUCCESS) {
            return ret;
        }
    }

    audioPolicyClientStubCB_->AddDeviceChangeCallback(flag, callback);
    return SUCCESS;
}

int32_t AudioPolicyManager::UnsetDeviceChangeCallback(const int32_t clientId, DeviceFlag flag)
{
    AUDIO_DEBUG_LOG("AudioPolicyManager::UnsetDeviceChangeCallback");
    if (audioPolicyClientStubCB_ != nullptr) {
        audioPolicyClientStubCB_->RemoveDeviceChangeCallback();
    }
    return SUCCESS;
}

int32_t AudioPolicyManager::SetPreferredOutputDeviceChangeCallback(const int32_t clientId,
    const std::shared_ptr<AudioPreferredOutputDeviceChangeCallback> &callback)
{
    AUDIO_DEBUG_LOG("AudioPolicyManager::SetPreferredOutputDeviceChangeCallback");
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, -1, "audio policy manager proxy is NULL.");
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, ERR_INVALID_PARAM, "callback is nullptr");

    if (audioPolicyClientStubCB_ == nullptr) {
        int32_t ret = RegisterPolicyCallbackClientFunc(gsp);
        if (ret != SUCCESS) {
            return ret;
        }
    }

    audioPolicyClientStubCB_->AddPreferredOutputDeviceChangeCallback(callback);
    return SUCCESS;
}

int32_t AudioPolicyManager::SetPreferredInputDeviceChangeCallback(
    const std::shared_ptr<AudioPreferredInputDeviceChangeCallback> &callback)
{
    AUDIO_DEBUG_LOG("AudioPolicyManager::SetPreferredInputDeviceChangeCallback");
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, -1, "audio policy manager proxy is NULL.");
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, ERR_INVALID_PARAM, "callback is nullptr");

    if (audioPolicyClientStubCB_ == nullptr) {
        int32_t ret = RegisterPolicyCallbackClientFunc(gsp);
        if (ret != SUCCESS) {
            return ret;
        }
    }

    audioPolicyClientStubCB_->AddPreferredInputDeviceChangeCallback(callback);
    return SUCCESS;
}

int32_t AudioPolicyManager::UnsetPreferredOutputDeviceChangeCallback(const int32_t clientId)
{
    AUDIO_DEBUG_LOG("AudioPolicyManager::UnsetPreferredOutputDeviceChangeCallback");
    if (audioPolicyClientStubCB_ != nullptr) {
        audioPolicyClientStubCB_->RemovePreferredOutputDeviceChangeCallback();
    }
    return SUCCESS;
}

int32_t AudioPolicyManager::UnsetPreferredInputDeviceChangeCallback()
{
    AUDIO_DEBUG_LOG("AudioPolicyManager::UnsetPreferredInputDeviceChangeCallback");
    if (audioPolicyClientStubCB_ != nullptr) {
        audioPolicyClientStubCB_->RemovePreferredInputDeviceChangeCallback();
    }
    return SUCCESS;
}

int32_t AudioPolicyManager::SetMicStateChangeCallback(const int32_t clientId,
    const std::shared_ptr<AudioManagerMicStateChangeCallback> &callback)
{
    AUDIO_DEBUG_LOG("AudioPolicyManager::SetMicStateChangeCallback");
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, ERROR, "audio policy manager proxy is NULL.");
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, ERR_INVALID_PARAM, "callback is nullptr");

    if (audioPolicyClientStubCB_ == nullptr) {
        int32_t ret = RegisterPolicyCallbackClientFunc(gsp);
        if (ret != SUCCESS) {
            return ret;
        }
    }

    audioPolicyClientStubCB_->AddMicStateChangeCallback(callback);
    return SUCCESS;
}

int32_t AudioPolicyManager::SetAudioInterruptCallback(const uint32_t sessionID,
    const std::shared_ptr<AudioInterruptCallback> &callback, const int32_t zoneID)
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, ERROR, "audio policy manager proxy is NULL.");
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, ERR_INVALID_PARAM, "callback is nullptr");

    sptr<AudioPolicyManagerListenerStub> listener = new(std::nothrow) AudioPolicyManagerListenerStub();
    CHECK_AND_RETURN_RET_LOG(listener != nullptr, ERROR, "object null");
    listener->SetInterruptCallback(callback);

    sptr<IRemoteObject> object = listener->AsObject();
    CHECK_AND_RETURN_RET_LOG(object != nullptr, ERROR, "listenerStub->AsObject is nullptr..");

    return gsp->SetAudioInterruptCallback(sessionID, object, zoneID);
}

int32_t AudioPolicyManager::UnsetAudioInterruptCallback(const uint32_t sessionID, const int32_t zoneID)
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, -1, "audio policy manager proxy is NULL.");
    return gsp->UnsetAudioInterruptCallback(sessionID, zoneID);
}

int32_t AudioPolicyManager::ActivateAudioInterrupt(const AudioInterrupt &audioInterrupt, const int32_t zoneID)
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, -1, "audio policy manager proxy is NULL.");
    return gsp->ActivateAudioInterrupt(audioInterrupt, zoneID);
}

int32_t AudioPolicyManager::DeactivateAudioInterrupt(const AudioInterrupt &audioInterrupt, const int32_t zoneID)
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, -1, "audio policy manager proxy is NULL.");
    return gsp->DeactivateAudioInterrupt(audioInterrupt, zoneID);
}

int32_t AudioPolicyManager::SetAudioManagerInterruptCallback(const int32_t clientId,
    const std::shared_ptr<AudioInterruptCallback> &callback)
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, ERROR, "audio policy manager proxy is NULL.");
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, ERR_INVALID_PARAM, "callback is nullptr");

    std::unique_lock<std::mutex> lock(listenerStubMutex_);
    sptr<AudioPolicyManagerListenerStub> interruptListenerStub = new(std::nothrow) AudioPolicyManagerListenerStub();
    CHECK_AND_RETURN_RET_LOG(interruptListenerStub != nullptr, ERROR, "object null");
    interruptListenerStub->SetInterruptCallback(callback);

    sptr<IRemoteObject> object = interruptListenerStub->AsObject();
    CHECK_AND_RETURN_RET_LOG(object != nullptr, ERROR, "onInterruptListenerStub->AsObject is nullptr.");
    lock.unlock();

    return gsp->SetAudioManagerInterruptCallback(clientId, object);
}

int32_t AudioPolicyManager::UnsetAudioManagerInterruptCallback(const int32_t clientId)
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, -1, "audio policy manager proxy is NULL.");
    return gsp->UnsetAudioManagerInterruptCallback(clientId);
}

int32_t AudioPolicyManager::RequestAudioFocus(const int32_t clientId, const AudioInterrupt &audioInterrupt)
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, -1, "audio policy manager proxy is NULL.");
    return gsp->RequestAudioFocus(clientId, audioInterrupt);
}

int32_t AudioPolicyManager::AbandonAudioFocus(const int32_t clientId, const AudioInterrupt &audioInterrupt)
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, -1, "audio policy manager proxy is NULL.");
    return gsp->AbandonAudioFocus(clientId, audioInterrupt);
}

AudioStreamType AudioPolicyManager::GetStreamInFocus(const int32_t zoneID)
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, STREAM_DEFAULT, "audio policy manager proxy is NULL.");
    return gsp->GetStreamInFocus(zoneID);
}

int32_t AudioPolicyManager::GetSessionInfoInFocus(AudioInterrupt &audioInterrupt, const int32_t zoneID)
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, -1, "audio policy manager proxy is NULL.");
    return gsp->GetSessionInfoInFocus(audioInterrupt, zoneID);
}

int32_t AudioPolicyManager::SetVolumeKeyEventCallback(const int32_t clientPid,
    const std::shared_ptr<VolumeKeyEventCallback> &callback, API_VERSION api_v)
{
    AUDIO_INFO_LOG("SetVolumeKeyEventCallback: client: %{public}d", clientPid);
    if (api_v == API_8 && !PermissionUtil::VerifySystemPermission()) {
        AUDIO_ERR_LOG("SetVolumeKeyEventCallback: No system permission");
        return ERR_PERMISSION_DENIED;
    }
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, ERROR, "audio policy manager proxy is NULL.");

    CHECK_AND_RETURN_RET_LOG(callback != nullptr, ERR_INVALID_PARAM, "volume back is nullptr");

    if (audioPolicyClientStubCB_ == nullptr) {
        int32_t ret = RegisterPolicyCallbackClientFunc(gsp);
        if (ret != SUCCESS) {
            return ret;
        }
    }

    audioPolicyClientStubCB_->AddVolumeKeyEventCallback(callback);
    return SUCCESS;
}

int32_t AudioPolicyManager::UnsetVolumeKeyEventCallback(const int32_t clientPid)
{
    AUDIO_DEBUG_LOG("AudioPolicyManager::UnsetVolumeKeyEventCallback");
    if (audioPolicyClientStubCB_ != nullptr) {
        audioPolicyClientStubCB_->RemoveVolumeKeyEventCallback();
    }
    return SUCCESS;
}

int32_t AudioPolicyManager::RegisterAudioRendererEventListener(const int32_t clientPid,
    const std::shared_ptr<AudioRendererStateChangeCallback> &callback)
{
    AUDIO_DEBUG_LOG("AudioPolicyManager::RegisterAudioRendererEventListener");
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, ERROR, "audio policy manager proxy is NULL.");
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, ERR_INVALID_PARAM, "RendererEvent Listener callback is nullptr");

    if (audioPolicyClientStubCB_ == nullptr) {
        int32_t ret = RegisterPolicyCallbackClientFunc(gsp);
        if (ret != SUCCESS) {
            return ret;
        }
    }

    audioPolicyClientStubCB_->AddRendererStateChangeCallback(callback);
    isAudioRendererEventListenerRegistered = true;
    return SUCCESS;
}

int32_t AudioPolicyManager::UnregisterAudioRendererEventListener(const int32_t clientPid)
{
    AUDIO_DEBUG_LOG("AudioPolicyManager::UnregisterAudioRendererEventListener");
    if ((audioPolicyClientStubCB_ != nullptr) && isAudioRendererEventListenerRegistered) {
        audioPolicyClientStubCB_->RemoveRendererStateChangeCallback();
        isAudioRendererEventListenerRegistered = false;
    }
    return SUCCESS;
}

int32_t AudioPolicyManager::RegisterAudioCapturerEventListener(const int32_t clientPid,
    const std::shared_ptr<AudioCapturerStateChangeCallback> &callback)
{
    AUDIO_DEBUG_LOG("AudioPolicyManager::RegisterAudioCapturerEventListener");
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, ERROR, "audio policy manager proxy is NULL.");

    CHECK_AND_RETURN_RET_LOG(callback != nullptr, ERR_INVALID_PARAM, "Capturer Event Listener callback is nullptr");

    if (audioPolicyClientStubCB_ == nullptr) {
        int32_t ret = RegisterPolicyCallbackClientFunc(gsp);
        if (ret != SUCCESS) {
            return ret;
        }
    }

    audioPolicyClientStubCB_->AddCapturerStateChangeCallback(callback);
    isAudioCapturerEventListenerRegistered = true;
    return SUCCESS;
}

int32_t AudioPolicyManager::UnregisterAudioCapturerEventListener(const int32_t clientPid)
{
    AUDIO_DEBUG_LOG("AudioPolicyManager::UnregisterAudioCapturerEventListener");
    if ((audioPolicyClientStubCB_ != nullptr) && isAudioCapturerEventListenerRegistered) {
        audioPolicyClientStubCB_->RemoveCapturerStateChangeCallback();
        isAudioCapturerEventListenerRegistered = false;
    }
    return SUCCESS;
}

int32_t AudioPolicyManager::RegisterOutputDeviceChangeWithInfoCallback(
    const uint32_t sessionID, const std::shared_ptr<OutputDeviceChangeWithInfoCallback> &callback)
{
    AUDIO_DEBUG_LOG("Entered %{public}s", __func__);
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    if (gsp == nullptr) {
        AUDIO_ERR_LOG("RegisterAudioRendererEventListener: audio policy manager proxy is NULL.");
        return ERROR;
    }

    if (callback == nullptr) {
        AUDIO_ERR_LOG("RegisterAudioRendererEventListener: RendererEvent Listener callback is nullptr");
        return ERR_INVALID_PARAM;
    }

    if (audioPolicyClientStubCB_ == nullptr) {
        int32_t ret = RegisterPolicyCallbackClientFunc(gsp);
        if (ret != SUCCESS) {
            return ret;
        }
    }

    audioPolicyClientStubCB_->AddOutputDeviceChangeWithInfoCallback(sessionID, callback);
    return SUCCESS;
}

int32_t AudioPolicyManager::UnregisterOutputDeviceChangeWithInfoCallback(const uint32_t sessionID)
{
    AUDIO_DEBUG_LOG("Entered %{public}s", __func__);
    if (audioPolicyClientStubCB_ != nullptr) {
        audioPolicyClientStubCB_->RemoveOutputDeviceChangeWithInfoCallback(sessionID);
    }
    return SUCCESS;
}

int32_t AudioPolicyManager::RegisterTracker(AudioMode &mode, AudioStreamChangeInfo &streamChangeInfo,
    const std::shared_ptr<AudioClientTracker> &clientTrackerObj)
{
    AUDIO_DEBUG_LOG("AudioPolicyManager::RegisterTracker");
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, ERROR, "audio policy manager proxy is NULL.");

    std::unique_lock<std::mutex> lock(clientTrackerStubMutex_);
    sptr<AudioClientTrackerCallbackStub> callback = new(std::nothrow) AudioClientTrackerCallbackStub();
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, ERROR, "clientTrackerCbStub: memory allocation failed");

    callback->SetClientTrackerCallback(clientTrackerObj);

    sptr<IRemoteObject> object = callback->AsObject();
    CHECK_AND_RETURN_RET_LOG(object != nullptr, ERROR, "clientTrackerCbStub: IPC object creation failed");
    lock.unlock();

    return gsp->RegisterTracker(mode, streamChangeInfo, object);
}

int32_t AudioPolicyManager::UpdateTracker(AudioMode &mode, AudioStreamChangeInfo &streamChangeInfo)
{
    AUDIO_DEBUG_LOG("AudioPolicyManager::UpdateTracker");
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, ERROR, "audio policy manager proxy is NULL.");
    return gsp->UpdateTracker(mode, streamChangeInfo);
}

bool AudioPolicyManager::CheckRecordingCreate(uint32_t appTokenId, uint64_t appFullTokenId, int32_t appUid,
    SourceType sourceType)
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, false, "audio policy manager proxy is NULL.");
    return gsp->CheckRecordingCreate(appTokenId, appFullTokenId, appUid, sourceType);
}

bool AudioPolicyManager::CheckRecordingStateChange(uint32_t appTokenId, uint64_t appFullTokenId, int32_t appUid,
    AudioPermissionState state)
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, false, "audio policy manager proxy is NULL.");
    return gsp->CheckRecordingStateChange(appTokenId, appFullTokenId, appUid, state);
}

int32_t AudioPolicyManager::ReconfigureAudioChannel(const uint32_t &count, DeviceType deviceType)
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, -1, "audio policy manager proxy is NULL.");
    return gsp->ReconfigureAudioChannel(count, deviceType);
}

int32_t AudioPolicyManager::GetAudioLatencyFromXml()
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, -1, "audio policy manager proxy is NULL.");
    return gsp->GetAudioLatencyFromXml();
}

uint32_t AudioPolicyManager::GetSinkLatencyFromXml()
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, 0, "audio policy manager proxy is NULL.");
    return gsp->GetSinkLatencyFromXml();
}

int32_t AudioPolicyManager::GetCurrentRendererChangeInfos(
    vector<unique_ptr<AudioRendererChangeInfo>> &audioRendererChangeInfos)
{
    AUDIO_DEBUG_LOG("GetCurrentRendererChangeInfos");
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, -1, "audio policy manager proxy is NULL.");
    return gsp->GetCurrentRendererChangeInfos(audioRendererChangeInfos);
}

int32_t AudioPolicyManager::GetCurrentCapturerChangeInfos(
    vector<unique_ptr<AudioCapturerChangeInfo>> &audioCapturerChangeInfos)
{
    AUDIO_DEBUG_LOG("GetCurrentCapturerChangeInfos");
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, ERROR, "audio policy manager proxy is NULL.");
    return gsp->GetCurrentCapturerChangeInfos(audioCapturerChangeInfos);
}

int32_t AudioPolicyManager::UpdateStreamState(const int32_t clientUid,
    StreamSetState streamSetState, AudioStreamType audioStreamType)
{
    AUDIO_DEBUG_LOG("UpdateStreamState");
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, ERROR, "audio policy manager proxy is NULL.");
    return  gsp->UpdateStreamState(clientUid, streamSetState, audioStreamType);
}

int32_t AudioPolicyManager::GetVolumeGroupInfos(std::string networkId, std::vector<sptr<VolumeGroupInfo>> &infos)
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, ERROR, "audio policy manager proxy is NULL.");
    return gsp->GetVolumeGroupInfos(networkId, infos);
}

int32_t AudioPolicyManager::GetNetworkIdByGroupId(int32_t groupId, std::string &networkId)
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, ERROR, "GetNetworkIdByGroupId failed, g_apProxy is nullptr.");
    return gsp->GetNetworkIdByGroupId(groupId, networkId);
}

bool AudioPolicyManager::IsAudioRendererLowLatencySupported(const AudioStreamInfo &audioStreamInfo)
{
    AUDIO_DEBUG_LOG("IsAudioRendererLowLatencySupported");
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, -1, "audio policy manager proxy is NULL.");
    return gsp->IsAudioRendererLowLatencySupported(audioStreamInfo);
}

int32_t AudioPolicyManager::SetSystemSoundUri(const std::string &key, const std::string &uri)
{
    AUDIO_DEBUG_LOG("SetSystemSoundUri: [%{public}s]: [%{public}s]", key.c_str(), uri.c_str());
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, ERROR, "audio policy manager proxy is NULL.");

    return gsp->SetSystemSoundUri(key, uri);
}

std::string AudioPolicyManager::GetSystemSoundUri(const std::string &key)
{
    AUDIO_DEBUG_LOG("GetSystemSoundUri: %{public}s", key.c_str());
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, "", "audio policy manager proxy is NULL.");

    return gsp->GetSystemSoundUri(key);
}

float AudioPolicyManager::GetMinStreamVolume()
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, ERROR, "audio policy manager proxy is NULL.");
    return gsp->GetMinStreamVolume();
}

float AudioPolicyManager::GetMaxStreamVolume()
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, ERROR, "audio policy manager proxy is NULL.");
    return gsp->GetMaxStreamVolume();
}

int32_t AudioPolicyManager::RegisterAudioPolicyServerDiedCb(const int32_t clientPid,
    const std::weak_ptr<AudioRendererPolicyServiceDiedCallback> &callback)
{
    std::lock_guard<std::mutex> lockCbMap(g_cBMapMutex);
    rendererCBMap_[clientPid] = callback;
    return SUCCESS;
}

int32_t AudioPolicyManager::UnregisterAudioPolicyServerDiedCb(const int32_t clientPid)
{
    std::lock_guard<std::mutex> lockCbMap(g_cBMapMutex);
    AUDIO_DEBUG_LOG("client pid: %{public}d", clientPid);
    for (auto it = rendererCBMap_.begin(); it != rendererCBMap_.end(); ++it) {
        rendererCBMap_.erase(getpid());
    }
    return SUCCESS;
}

int32_t AudioPolicyManager::RegisterAudioStreamPolicyServerDiedCb(
    const std::shared_ptr<AudioStreamPolicyServiceDiedCallback> &callback)
{
    std::lock_guard<std::mutex> lockCbMap(g_cBDiedMapMutex);
    AUDIO_DEBUG_LOG("RegisterAudioStreamPolicyServerDiedCb");
    auto iter = find(audioStreamCBMap_.begin(), audioStreamCBMap_.end(), callback);
    if (iter == audioStreamCBMap_.end()) {
        audioStreamCBMap_.emplace_back(callback);
    }

    return SUCCESS;
}

int32_t AudioPolicyManager::UnregisterAudioStreamPolicyServerDiedCb(
    const std::shared_ptr<AudioStreamPolicyServiceDiedCallback> &callback)
{
    std::lock_guard<std::mutex> lockCbMap(g_cBDiedMapMutex);
    AUDIO_DEBUG_LOG("UnregisterAudioStreamPolicyServerDiedCb");
    auto iter = find(audioStreamCBMap_.begin(), audioStreamCBMap_.end(), callback);
    if (iter != audioStreamCBMap_.end()) {
        audioStreamCBMap_.erase(iter);
    }

    return SUCCESS;
}

int32_t AudioPolicyManager::GetMaxRendererInstances()
{
    AUDIO_DEBUG_LOG("AudioPolicyManager::GetMaxRendererInstances");
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, ERROR, "audio policy manager proxy is NULL.");

    return gsp->GetMaxRendererInstances();
}

bool AudioPolicyManager::IsVolumeUnadjustable()
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, false, "audio policy manager proxy is NULL.");
    return gsp->IsVolumeUnadjustable();
}

int32_t AudioPolicyManager::AdjustVolumeByStep(VolumeAdjustType adjustType)
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, ERROR, "audio policy manager proxy is NULL.");
    return gsp->AdjustVolumeByStep(adjustType);
}

int32_t AudioPolicyManager::AdjustSystemVolumeByStep(AudioVolumeType volumeType, VolumeAdjustType adjustType)
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, ERROR, "audio policy manager proxy is NULL.");
    return gsp->AdjustSystemVolumeByStep(volumeType, adjustType);
}

float AudioPolicyManager::GetSystemVolumeInDb(AudioVolumeType volumeType, int32_t volumeLevel, DeviceType deviceType)
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, ERROR, "audio policy manager proxy is NULL.");
    return gsp->GetSystemVolumeInDb(volumeType, volumeLevel, deviceType);
}

int32_t AudioPolicyManager::QueryEffectSceneMode(SupportedEffectConfig &supportedEffectConfig)
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, -1, "audio policy manager proxy is NULL.");
    int error = gsp->QueryEffectSceneMode(supportedEffectConfig);
    return error;
}

int32_t AudioPolicyManager::SetPlaybackCapturerFilterInfos(const AudioPlaybackCaptureConfig &config,
    uint32_t appTokenId)
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, ERROR, "audio policy manager proxy is NULL.");

    return gsp->SetPlaybackCapturerFilterInfos(config, appTokenId);
}

int32_t AudioPolicyManager::SetCaptureSilentState(bool state)
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    if (gsp == nullptr) {
        AUDIO_ERR_LOG("SetCaptureSilentState: audio policy manager proxy is NULL");
        return ERROR;
    }

    return gsp->SetCaptureSilentState(state);
}

int32_t AudioPolicyManager::GetHardwareOutputSamplingRate(const sptr<AudioDeviceDescriptor> &desc)
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, ERROR, "audio policy manager proxy is NULL.");
    return gsp->GetHardwareOutputSamplingRate(desc);
}

vector<sptr<MicrophoneDescriptor>> AudioPolicyManager::GetAudioCapturerMicrophoneDescriptors(int32_t sessionID)
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    if (gsp == nullptr) {
        AUDIO_ERR_LOG("audio policy manager proxy is NULL.");
        std::vector<sptr<MicrophoneDescriptor>> descs;
        return descs;
    }
    return gsp->GetAudioCapturerMicrophoneDescriptors(sessionID);
}

vector<sptr<MicrophoneDescriptor>> AudioPolicyManager::GetAvailableMicrophones()
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    if (gsp == nullptr) {
        AUDIO_ERR_LOG("audio policy manager proxy is NULL.");
        std::vector<sptr<MicrophoneDescriptor>> descs;
        return descs;
    }
    return gsp->GetAvailableMicrophones();
}

int32_t AudioPolicyManager::SetDeviceAbsVolumeSupported(const std::string &macAddress, const bool support)
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, ERROR, "audio policy manager proxy is NULL.");
    return gsp->SetDeviceAbsVolumeSupported(macAddress, support);
}

bool AudioPolicyManager::IsAbsVolumeScene()
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, ERROR, "audio policy manager proxy is NULL.");
    return gsp->IsAbsVolumeScene();
}

int32_t AudioPolicyManager::SetA2dpDeviceVolume(const std::string &macAddress, const int32_t volume,
    const bool updateUi)
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, ERROR, "audio policy manager proxy is NULL.");
    return gsp->SetA2dpDeviceVolume(macAddress, volume, updateUi);
}

std::vector<std::unique_ptr<AudioDeviceDescriptor>> AudioPolicyManager::GetAvailableDevices(AudioDeviceUsage usage)
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    if (gsp == nullptr) {
        AUDIO_ERR_LOG("GetAvailableMicrophones: audio policy manager proxy is NULL.");
        std::vector<unique_ptr<AudioDeviceDescriptor>> descs;
        return descs;
    }
    return gsp->GetAvailableDevices(usage);
}

int32_t AudioPolicyManager::SetAvailableDeviceChangeCallback(const int32_t clientId, const AudioDeviceUsage usage,
    const std::shared_ptr<AudioManagerAvailableDeviceChangeCallback>& callback)
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, -1, "audio policy manager proxy is NULL.");
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, ERR_INVALID_PARAM, "callback is nullptr");

    auto deviceChangeCbStub = new(std::nothrow) AudioPolicyManagerListenerStub();
    CHECK_AND_RETURN_RET_LOG(deviceChangeCbStub != nullptr, ERROR, "object null");

    deviceChangeCbStub->SetAvailableDeviceChangeCallback(callback);

    sptr<IRemoteObject> object = deviceChangeCbStub->AsObject();
    if (object == nullptr) {
        AUDIO_ERR_LOG("listenerStub->AsObject is nullptr..");
        delete deviceChangeCbStub;
        return ERROR;
    }

    return gsp->SetAvailableDeviceChangeCallback(clientId, usage, object);
}

int32_t AudioPolicyManager::UnsetAvailableDeviceChangeCallback(const int32_t clientId, AudioDeviceUsage usage)
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, -1, "audio policy manager proxy is NULL.");
    return gsp->UnsetAvailableDeviceChangeCallback(clientId, usage);
}

int32_t AudioPolicyManager::ConfigDistributedRoutingRole(sptr<AudioDeviceDescriptor> descriptor, CastType type)
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, -1, "audio policy manager proxy is NULL.");
    return gsp->ConfigDistributedRoutingRole(descriptor, type);
}

int32_t AudioPolicyManager::SetDistributedRoutingRoleCallback(
    const std::shared_ptr<AudioDistributedRoutingRoleCallback> &callback)
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, -1, "audio policy manager proxy is NULL.");
    if (callback == nullptr) {
        AUDIO_ERR_LOG("SetDistributedRoutingRoleCallback: callback is nullptr");
        return ERR_INVALID_PARAM;
    }

    std::unique_lock<std::mutex> lock(listenerStubMutex_);
    auto activeDistributedRoutingRoleCb = new(std::nothrow) AudioRoutingManagerListenerStub();
    if (activeDistributedRoutingRoleCb == nullptr) {
        AUDIO_ERR_LOG("SetDistributedRoutingRoleCallback: object is nullptr");
        return ERROR;
    }
    activeDistributedRoutingRoleCb->SetDistributedRoutingRoleCallback(callback);
    sptr<IRemoteObject> object = activeDistributedRoutingRoleCb->AsObject();
    if (object == nullptr) {
        AUDIO_ERR_LOG("SetDistributedRoutingRoleCallback: listenerStub is nullptr.");
        delete activeDistributedRoutingRoleCb;
        return ERROR;
    }
    return gsp->SetDistributedRoutingRoleCallback(object);
}

int32_t AudioPolicyManager::UnsetDistributedRoutingRoleCallback()
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, -1, "audio policy manager proxy is NULL.");
    return gsp->UnsetDistributedRoutingRoleCallback();
}

bool AudioPolicyManager::IsSpatializationEnabled()
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, false, "audio policy manager proxy is NULL.");
    return gsp->IsSpatializationEnabled();
}

int32_t AudioPolicyManager::SetSpatializationEnabled(const bool enable)
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, ERROR, "audio policy manager proxy is NULL.");
    return gsp->SetSpatializationEnabled(enable);
}

bool AudioPolicyManager::IsHeadTrackingEnabled()
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, false, "audio policy manager proxy is NULL.");
    return gsp->IsHeadTrackingEnabled();
}

int32_t AudioPolicyManager::SetHeadTrackingEnabled(const bool enable)
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, ERROR, "audio policy manager proxy is NULL.");
    return gsp->SetHeadTrackingEnabled(enable);
}

int32_t AudioPolicyManager::RegisterSpatializationEnabledEventListener(
    const std::shared_ptr<AudioSpatializationEnabledChangeCallback> &callback)
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, ERROR, "audio policy manager proxy is NULL.");

    CHECK_AND_RETURN_RET_LOG(callback != nullptr, ERR_INVALID_PARAM, "Spatialization Enabled callback is nullptr");

    sptr<AudioSpatializationEnabledChangeListenerStub> spatializationEnabledChangeListenerStub =
        new(std::nothrow) AudioSpatializationEnabledChangeListenerStub();
    if (spatializationEnabledChangeListenerStub == nullptr) {
        AUDIO_ERR_LOG("RegisterSpatializationEnabledEventListener: object null");
        return ERROR;
    }

    spatializationEnabledChangeListenerStub->SetCallback(callback);

    sptr<IRemoteObject> object = spatializationEnabledChangeListenerStub->AsObject();
    CHECK_AND_RETURN_RET_LOG(object != nullptr, ERROR,
        "SpatializationEnabledChangeListener IPC object creation failed");

    return gsp->RegisterSpatializationEnabledEventListener(object);
}

int32_t AudioPolicyManager::RegisterHeadTrackingEnabledEventListener(
    const std::shared_ptr<AudioHeadTrackingEnabledChangeCallback> &callback)
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, ERROR, "audio policy manager proxy is NULL.");

    CHECK_AND_RETURN_RET_LOG(callback != nullptr, ERR_INVALID_PARAM, "Head Tracking Enabled callback is nullptr");

    sptr<AudioHeadTrackingEnabledChangeListenerStub> headTrackingEnabledChangeListenerStub =
        new(std::nothrow) AudioHeadTrackingEnabledChangeListenerStub();
    CHECK_AND_RETURN_RET_LOG(headTrackingEnabledChangeListenerStub != nullptr, ERROR, "object null");

    headTrackingEnabledChangeListenerStub->SetCallback(callback);

    sptr<IRemoteObject> object = headTrackingEnabledChangeListenerStub->AsObject();
    CHECK_AND_RETURN_RET_LOG(object != nullptr, ERROR,
        "HeadTrackingEnabledChangeListener IPC object creation failed");

    return gsp->RegisterHeadTrackingEnabledEventListener(object);
}

int32_t AudioPolicyManager::UnregisterSpatializationEnabledEventListener()
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, ERROR, "audio policy manager proxy is NULL.");
    return gsp->UnregisterSpatializationEnabledEventListener();
}

int32_t AudioPolicyManager::UnregisterHeadTrackingEnabledEventListener()
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, ERROR, "audio policy manager proxy is NULL.");
    return gsp->UnregisterHeadTrackingEnabledEventListener();
}

AudioSpatializationState AudioPolicyManager::GetSpatializationState(const StreamUsage streamUsage)
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    if (gsp == nullptr) {
        AUDIO_ERR_LOG("GetSpatializationState: audio policy manager proxy is NULL.");
        AudioSpatializationState spatializationState = {false, false};
        return spatializationState;
    }
    return gsp->GetSpatializationState(streamUsage);
}

bool AudioPolicyManager::IsSpatializationSupported()
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, false, "audio policy manager proxy is NULL.");
    return gsp->IsSpatializationSupported();
}

bool AudioPolicyManager::IsSpatializationSupportedForDevice(const std::string address)
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, false, "audio policy manager proxy is NULL.");
    return gsp->IsSpatializationSupportedForDevice(address);
}

bool AudioPolicyManager::IsHeadTrackingSupported()
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, false, "audio policy manager proxy is NULL.");
    return gsp->IsHeadTrackingSupported();
}

bool AudioPolicyManager::IsHeadTrackingSupportedForDevice(const std::string address)
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, false, "audio policy manager proxy is NULL.");
    return gsp->IsHeadTrackingSupportedForDevice(address);
}

int32_t AudioPolicyManager::UpdateSpatialDeviceState(const AudioSpatialDeviceState audioSpatialDeviceState)
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, ERROR, "audio policy manager proxy is NULL.");
    return gsp->UpdateSpatialDeviceState(audioSpatialDeviceState);
}

int32_t AudioPolicyManager::RegisterSpatializationStateEventListener(const uint32_t sessionID,
    const StreamUsage streamUsage, const std::shared_ptr<AudioSpatializationStateChangeCallback> &callback)
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, ERROR, "audio policy manager proxy is NULL.");

    CHECK_AND_RETURN_RET_LOG(callback != nullptr, ERR_INVALID_PARAM, "Spatialization state callback is nullptr");

    sptr<AudioSpatializationStateChangeListenerStub> spatializationStateChangeListenerStub =
        new(std::nothrow) AudioSpatializationStateChangeListenerStub();
    CHECK_AND_RETURN_RET_LOG(spatializationStateChangeListenerStub != nullptr, ERROR, "object null");

    spatializationStateChangeListenerStub->SetCallback(callback);

    sptr<IRemoteObject> object = spatializationStateChangeListenerStub->AsObject();
    CHECK_AND_RETURN_RET_LOG(object != nullptr, ERROR, "IPC object creation failed");

    return gsp->RegisterSpatializationStateEventListener(sessionID, streamUsage, object);
}

int32_t AudioPolicyManager::UnregisterSpatializationStateEventListener(const uint32_t sessionID)
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, ERROR, "audio policy manager proxy is NULL.");

    return gsp->UnregisterSpatializationStateEventListener(sessionID);
}

int32_t AudioPolicyManager::CreateAudioInterruptZone(const std::set<int32_t> pids, const int32_t zoneID)
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, ERROR, "audio policy manager proxy is NULL.");

    return gsp->CreateAudioInterruptZone(pids, zoneID);
}

int32_t AudioPolicyManager::AddAudioInterruptZonePids(const std::set<int32_t> pids, const int32_t zoneID)
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, ERROR, "audio policy manager proxy is NULL.");

    return gsp->AddAudioInterruptZonePids(pids, zoneID);
}

int32_t AudioPolicyManager::RemoveAudioInterruptZonePids(const std::set<int32_t> pids, const int32_t zoneID)
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, ERROR, "audio policy manager proxy is NULL.");

    return gsp->RemoveAudioInterruptZonePids(pids, zoneID);
}

int32_t AudioPolicyManager::ReleaseAudioInterruptZone(const int32_t zoneID)
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, ERROR, "audio policy manager proxy is NULL.");

    return gsp->ReleaseAudioInterruptZone(zoneID);
}

int32_t AudioPolicyManager::SetCallDeviceActive(InternalDeviceType deviceType, bool active, std::string address)
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    if (gsp == nullptr) {
        AUDIO_ERR_LOG("audio policy manager proxy is NULL.");
        return -1;
    }
    return gsp->SetCallDeviceActive(deviceType, active, address);
}

std::unique_ptr<AudioDeviceDescriptor> AudioPolicyManager::GetActiveBluetoothDevice()
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    if (gsp == nullptr) {
        AUDIO_ERR_LOG("audio policy manager proxy is NULL.");
        return make_unique<AudioDeviceDescriptor>();
    }
    return gsp->GetActiveBluetoothDevice();
}

int32_t AudioPolicyManager::NotifyCapturerAdded(AudioCapturerInfo capturerInfo, AudioStreamInfo streamInfo,
    uint32_t sessionId)
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    if (gsp == nullptr) {
        AUDIO_ERR_LOG("audio policy manager proxy is NULL.");
        return -1;
    }
    return gsp->NotifyCapturerAdded(capturerInfo, streamInfo, sessionId);
}
} // namespace AudioStandard
} // namespace OHOS
