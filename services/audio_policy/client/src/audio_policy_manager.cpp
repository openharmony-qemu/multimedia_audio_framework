/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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
#define LOG_TAG "AudioPolicyManager"
#endif

#include "audio_policy_manager.h"
#include "audio_errors.h"
#include "audio_server_death_recipient.h"
#include "audio_policy_log.h"
#include "audio_utils.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace AudioStandard {
using namespace std;
using namespace std::chrono_literals;

static sptr<IAudioPolicy> g_apProxy = nullptr;
mutex g_apProxyMutex;
constexpr int64_t SLEEP_TIME = 1;
constexpr int32_t RETRY_TIMES = 10;
const unsigned int TIME_OUT_SECONDS = 10;
constexpr auto SLEEP_TIMES_RETYT_FAILED = 1min;
std::mutex g_cBMapMutex;
std::mutex g_cBDiedMapMutex;
std::unordered_map<int32_t, std::weak_ptr<AudioRendererPolicyServiceDiedCallback>> AudioPolicyManager::rendererCBMap_;
std::vector<std::weak_ptr<AudioStreamPolicyServiceDiedCallback>> AudioPolicyManager::audioStreamCBMap_;
std::unordered_map<int32_t, sptr<AudioClientTrackerCallbackStub>> AudioPolicyManager::clientTrackerStubMap_;

inline bool RegisterDeathRecipientInner(sptr<IRemoteObject> object)
{
    pid_t pid = 0;
    sptr<AudioServerDeathRecipient> deathRecipient = new(std::nothrow) AudioServerDeathRecipient(pid);
    CHECK_AND_RETURN_RET_LOG(deathRecipient != nullptr, false, "deathRecipient is null");
    deathRecipient->SetNotifyCb(
        [] (pid_t pid) { AudioPolicyManager::AudioPolicyServerDied(pid); });
    AUDIO_DEBUG_LOG("Register audio policy server death recipient");
    CHECK_AND_RETURN_RET_LOG(object->AddDeathRecipient(deathRecipient), false, "AddDeathRecipient failed");
    return true;
}

inline sptr<IAudioPolicy> GetAudioPolicyProxyFromSamgr()
{
    auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    CHECK_AND_RETURN_RET_LOG(samgr != nullptr, nullptr, "samgr init failed.");
    sptr<IRemoteObject> object = samgr->GetSystemAbility(AUDIO_POLICY_SERVICE_ID);
    CHECK_AND_RETURN_RET_LOG(object != nullptr, nullptr, "Object is NULL.");
    sptr<IAudioPolicy> apProxy = iface_cast<IAudioPolicy>(object);
    CHECK_AND_RETURN_RET_LOG(apProxy != nullptr, nullptr, "Init apProxy is NULL.");
    return apProxy;
}

const sptr<IAudioPolicy> AudioPolicyManager::GetAudioPolicyManagerProxy()
{
    AUDIO_DEBUG_LOG("In");
    lock_guard<mutex> lock(g_apProxyMutex);

    if (g_apProxy != nullptr) {
        return g_apProxy;
    }

    sptr<IAudioPolicy> gsp = GetAudioPolicyProxyFromSamgr();
    CHECK_AND_RETURN_RET_LOG(gsp, nullptr, "gsp is null");

    AUDIO_DEBUG_LOG("Init g_apProxy is assigned.");

    if (RegisterDeathRecipientInner(gsp->AsObject())) {
        g_apProxy = gsp;
    }

    return gsp;
}

static const sptr<IAudioPolicy> RecoverAndGetAudioPolicyManagerProxy()
{
    AUDIO_DEBUG_LOG("In");
    lock_guard<mutex> lock(g_apProxyMutex);
    if (g_apProxy != nullptr) {
        sptr<IRemoteObject> object = g_apProxy->AsObject();
        if (object != nullptr && !object->IsObjectDead()) {
            AUDIO_INFO_LOG("direct return g_apProxy");
            return g_apProxy;
        }
    }

    sptr<IAudioPolicy> gsp = GetAudioPolicyProxyFromSamgr();
    CHECK_AND_RETURN_RET_LOG(gsp, nullptr, "gsp is null");

    AUDIO_DEBUG_LOG("Init g_apProxy is assigned.");
    CHECK_AND_RETURN_RET_LOG(RegisterDeathRecipientInner(gsp->AsObject()), nullptr, "RegisterDeathRecipient failed");

    g_apProxy = gsp;
    return gsp;
}

int32_t AudioPolicyManager::RegisterPolicyCallbackClientFunc(const sptr<IAudioPolicy> &gsp)
{
    AudioXCollie audioXCollie("AudioPolicyManager::RegisterPolicyCallbackClientFunc", TIME_OUT_SECONDS);
    std::unique_lock<std::mutex> lock(registerCallbackMutex_);
    if (audioPolicyClientStubCB_ == nullptr) {
        audioPolicyClientStubCB_ = new(std::nothrow) AudioPolicyClientStubImpl();
    }
    sptr<IRemoteObject> object = audioPolicyClientStubCB_->AsObject();
    if (object == nullptr) {
        AUDIO_ERR_LOG("audioPolicyClientStubCB_->AsObject is nullptr");
        lock.unlock();
        return ERROR;
    }
    lock.unlock();

    int32_t ret = gsp->RegisterPolicyCallbackClient(object);
    if (ret == SUCCESS) {
        isAudioPolicyClientRegisted_ = true;
    }
    return ret;
}

void AudioPolicyManager::RecoverAudioPolicyCallbackClient()
{
    std::unique_lock<std::mutex> lock(registerCallbackMutex_);
    if (audioPolicyClientStubCB_ == nullptr) {
        AUDIO_ERR_LOG("audioPolicyClientStubCB_ is null.");
        return;
    }
    lock.unlock();

    int32_t retry = RETRY_TIMES;
    sptr<IAudioPolicy> gsp = nullptr;
    while (retry--) {
        // Sleep and wait for 1 second;
        sleep(SLEEP_TIME);
        gsp = RecoverAndGetAudioPolicyManagerProxy();
        if (gsp != nullptr) {
            AUDIO_INFO_LOG("Reconnect audio policy service success!");
            break;
        }
        if (retry == 0) {
            AUDIO_WARNING_LOG("Reconnect audio policy service %{public}d times, sleep ", RETRY_TIMES);
            std::this_thread::sleep_for(SLEEP_TIMES_RETYT_FAILED);
            retry = RETRY_TIMES;
        }
    }

    CHECK_AND_RETURN_LOG(gsp != nullptr, "Reconnect audio policy service fail!");

    sptr<IRemoteObject> object = audioPolicyClientStubCB_->AsObject();
    if (object == nullptr) {
        AUDIO_ERR_LOG("RegisterPolicyCallbackClientFunc: audioPolicyClientStubCB_->AsObject is nullptr");
        return;
    }

    gsp->RegisterPolicyCallbackClient(object);
    if (audioPolicyClientStubCB_->HasMicStateChangeCallback()) {
        AUDIO_INFO_LOG("RecoverAudioPolicyCallbackClient has micStateChangeCallback");
        gsp->SetClientCallbacksEnable(CALLBACK_MICMUTE_STATE_CHANGE, true);
    }

    for (auto enumIndex : CALLBACK_ENUMS) {
        auto &[mutex, isEnable] = callbackChangeInfos_[enumIndex];
        std::lock_guard<std::mutex> lock(mutex);
        if (isEnable) {
            gsp->SetClientCallbacksEnable(enumIndex, true);
        }
    }
}

void AudioPolicyManager::AudioPolicyServerDied(pid_t pid)
{
    GetInstance().ResetClientTrackerStubMap();
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
        if (g_apProxy != nullptr) {
            sptr<IRemoteObject> object = g_apProxy->AsObject();
            if (object == nullptr || object->IsObjectDead()) {
                AUDIO_INFO_LOG("assign g_apProxy to nullptr");
                g_apProxy = nullptr;
            }
        }
    }
    GetInstance().RecoverAudioPolicyCallbackClient();

    {
        std::lock_guard<std::mutex> lockCbMap(g_cBDiedMapMutex);
        if (audioStreamCBMap_.size() != 0) {
            for (auto it = audioStreamCBMap_.begin(); it != audioStreamCBMap_.end();) {
                auto cb = (*it).lock();
                if (cb == nullptr) {
                    it = audioStreamCBMap_.erase(it);
                    continue;
                }
                cb->OnAudioPolicyServiceDied();
                ++it;
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

int32_t AudioPolicyManager::SetSystemVolumeLevel(AudioVolumeType volumeType, int32_t volumeLevel, bool isLegacy,
    int32_t volumeFlag)
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, -1, "audio policy manager proxy is NULL.");

    if (isLegacy) {
        return gsp->SetSystemVolumeLevelLegacy(volumeType, volumeLevel);
    }
    return gsp->SetSystemVolumeLevel(volumeType, volumeLevel, volumeFlag);
}

int32_t AudioPolicyManager::SetRingerModeLegacy(AudioRingerMode ringMode)
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, -1, "audio policy manager proxy is NULL.");
    return gsp->SetRingerModeLegacy(ringMode);
}

int32_t AudioPolicyManager::SetRingerMode(AudioRingerMode ringMode)
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, -1, "audio policy manager proxy is NULL.");
    return gsp->SetRingerMode(ringMode);
}

AudioRingerMode AudioPolicyManager::GetRingerMode()
{
    AudioXCollie audioXCollie("AudioPolicyManager::GetRingerMode", TIME_OUT_SECONDS);
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

int32_t AudioPolicyManager::SetMicrophoneMutePersistent(const bool isMute, const PolicyType type)
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, -1, "audio policy manager proxy is NULL.");
    return gsp->SetMicrophoneMutePersistent(isMute, type);
}

bool AudioPolicyManager::GetPersistentMicMuteState()
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, -1, "audio policy manager proxy is NULL.");
    return gsp->GetPersistentMicMuteState();
}

bool AudioPolicyManager::IsMicrophoneMuteLegacy()
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, -1, "audio policy manager proxy is NULL.");
    if (!isAudioPolicyClientRegisted_) {
        RegisterPolicyCallbackClientFunc(gsp);
    }

    return gsp->IsMicrophoneMuteLegacy();
}

bool AudioPolicyManager::IsMicrophoneMute()
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, -1, "audio policy manager proxy is NULL.");
    if (!isAudioPolicyClientRegisted_) {
        RegisterPolicyCallbackClientFunc(gsp);
    }

    return gsp->IsMicrophoneMute();
}

AudioScene AudioPolicyManager::GetAudioScene()
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, AUDIO_SCENE_DEFAULT, "audio policy manager proxy is NULL.");
    return gsp->GetAudioScene();
}

AudioStreamType AudioPolicyManager::GetSystemActiveVolumeType(const int32_t clientUid)
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, STREAM_DEFAULT, "audio policy manager proxy is NULL.");
    return gsp->GetSystemActiveVolumeType(clientUid);
}

int32_t AudioPolicyManager::GetSystemVolumeLevel(AudioVolumeType volumeType)
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, -1, "audio policy manager proxy is NULL.");
    return gsp->GetSystemVolumeLevel(volumeType);
}

int32_t AudioPolicyManager::SetStreamMute(AudioVolumeType volumeType, bool mute, bool isLegacy)
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, -1, "audio policy manager proxy is NULL.");
    if (isLegacy) {
        return gsp->SetStreamMuteLegacy(volumeType, mute);
    }
    return gsp->SetStreamMute(volumeType, mute);
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

std::vector<sptr<AudioDeviceDescriptor>> AudioPolicyManager::GetDevicesInner(DeviceFlag deviceFlag)
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    if (gsp == nullptr) {
        AUDIO_ERR_LOG("audio policy manager proxy is NULL.");
        std::vector<sptr<AudioDeviceDescriptor>> deviceInfo;
        return deviceInfo;
    }
    return gsp->GetDevicesInner(deviceFlag);
}

std::vector<sptr<AudioDeviceDescriptor>> AudioPolicyManager::GetPreferredOutputDeviceDescriptors(
    AudioRendererInfo &rendererInfo)
{
    AudioXCollie audioXCollie("AudioPolicyManager::GetPreferredOutputDeviceDescriptors", TIME_OUT_SECONDS);
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

int32_t AudioPolicyManager::SetClientCallbacksEnable(const CallbackChange &callbackchange, const bool &enable)
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, -1, "audio policy manager proxy is NULL.");
    return gsp->SetClientCallbacksEnable(callbackchange, enable);
}

int32_t AudioPolicyManager::RegisterFocusInfoChangeCallback(const int32_t clientId,
    const std::shared_ptr<AudioFocusInfoChangeCallback> &callback)
{
    AUDIO_DEBUG_LOG("AudioPolicyManager::RegisterFocusInfoChangeCallback");
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, ERR_INVALID_PARAM,
        "RegisterFocusInfoChangeCallback: callback is nullptr");

    if (!isAudioPolicyClientRegisted_) {
        const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
        CHECK_AND_RETURN_RET_LOG(gsp != nullptr, ERROR, "audio policy manager proxy is NULL.");
        int32_t ret = RegisterPolicyCallbackClientFunc(gsp);
        if (ret != SUCCESS) {
            return ret;
        }
    }

    std::lock_guard<std::mutex> lockCbMap(callbackChangeInfos_[CALLBACK_FOCUS_INFO_CHANGE].mutex);
    audioPolicyClientStubCB_->AddFocusInfoChangeCallback(callback);
    size_t callbackSize = audioPolicyClientStubCB_->GetFocusInfoChangeCallbackSize();
    if (callbackSize == 1) {
        callbackChangeInfos_[CALLBACK_FOCUS_INFO_CHANGE].isEnable = true;
        SetClientCallbacksEnable(CALLBACK_FOCUS_INFO_CHANGE, true);
    }

    return SUCCESS;
}

int32_t AudioPolicyManager::UnregisterFocusInfoChangeCallback(const int32_t clientId)
{
    AUDIO_DEBUG_LOG("AudioPolicyManager::UnregisterFocusInfoChangeCallback");
    std::lock_guard<std::mutex> lockCbMap(callbackChangeInfos_[CALLBACK_FOCUS_INFO_CHANGE].mutex);
    if (audioPolicyClientStubCB_ != nullptr) {
        audioPolicyClientStubCB_->RemoveFocusInfoChangeCallback();
        if (audioPolicyClientStubCB_->GetFocusInfoChangeCallbackSize() == 0) {
            callbackChangeInfos_[CALLBACK_FOCUS_INFO_CHANGE].isEnable = false;
            SetClientCallbacksEnable(CALLBACK_FOCUS_INFO_CHANGE, false);
        }
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

    CHECK_AND_RETURN_RET_LOG(callback != nullptr, ERR_INVALID_PARAM, "callback is nullptr");

    if (!isAudioPolicyClientRegisted_) {
        const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
        CHECK_AND_RETURN_RET_LOG(gsp != nullptr, ERR_INVALID_PARAM, "audio policy manager proxy is NULL.");
        int32_t ret = RegisterPolicyCallbackClientFunc(gsp);
        if (ret != SUCCESS) {
            return ret;
        }
    }
    std::lock_guard<std::mutex> lockCbMap(callbackChangeInfos_[CALLBACK_SET_RINGER_MODE].mutex);
    if (audioPolicyClientStubCB_ != nullptr) {
        audioPolicyClientStubCB_->AddRingerModeCallback(callback);
        size_t callbackSize = audioPolicyClientStubCB_->GetRingerModeCallbackSize();
        if (callbackSize == 1) {
            callbackChangeInfos_[CALLBACK_SET_RINGER_MODE].isEnable = true;
            SetClientCallbacksEnable(CALLBACK_SET_RINGER_MODE, true);
        }
    }
    return SUCCESS;
}

int32_t AudioPolicyManager::UnsetRingerModeCallback(const int32_t clientId)
{
    AUDIO_DEBUG_LOG("Remove all ringer mode callbacks");
    std::lock_guard<std::mutex> lockCbMap(callbackChangeInfos_[CALLBACK_SET_RINGER_MODE].mutex);
    if (audioPolicyClientStubCB_ != nullptr) {
        audioPolicyClientStubCB_->RemoveRingerModeCallback();
        if (audioPolicyClientStubCB_->GetRingerModeCallbackSize() == 0) {
            callbackChangeInfos_[CALLBACK_SET_RINGER_MODE].isEnable = false;
            SetClientCallbacksEnable(CALLBACK_SET_RINGER_MODE, false);
        }
    }
    return SUCCESS;
}

int32_t AudioPolicyManager::UnsetRingerModeCallback(const int32_t clientId,
    const std::shared_ptr<AudioRingerModeCallback> &callback)
{
    AUDIO_DEBUG_LOG("Remove one ringer mode callback");
    std::lock_guard<std::mutex> lockCbMap(callbackChangeInfos_[CALLBACK_SET_RINGER_MODE].mutex);
    if (audioPolicyClientStubCB_ != nullptr) {
        audioPolicyClientStubCB_->RemoveRingerModeCallback(callback);
        if (audioPolicyClientStubCB_->GetRingerModeCallbackSize() == 0) {
            callbackChangeInfos_[CALLBACK_SET_RINGER_MODE].isEnable = false;
            SetClientCallbacksEnable(CALLBACK_SET_RINGER_MODE, false);
        }
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

    CHECK_AND_RETURN_RET_LOG(callback != nullptr, ERR_INVALID_PARAM, "SetDeviceChangeCallback: callback is nullptr");

    if (!isAudioPolicyClientRegisted_) {
        const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
        CHECK_AND_RETURN_RET_LOG(gsp != nullptr, -1, "audio policy manager proxy is NULL.");
        int32_t ret = RegisterPolicyCallbackClientFunc(gsp);
        if (ret != SUCCESS) {
            return ret;
        }
    }

    std::lock_guard<std::mutex> lockCbMap(callbackChangeInfos_[CALLBACK_SET_DEVICE_CHANGE].mutex);
    if (audioPolicyClientStubCB_ != nullptr) {
        audioPolicyClientStubCB_->AddDeviceChangeCallback(flag, callback);
        size_t callbackSize = audioPolicyClientStubCB_->GetDeviceChangeCallbackSize();
        if (callbackSize == 1) {
            callbackChangeInfos_[CALLBACK_SET_DEVICE_CHANGE].isEnable = true;
            SetClientCallbacksEnable(CALLBACK_SET_DEVICE_CHANGE, true);
        }
    }
    return SUCCESS;
}

int32_t AudioPolicyManager::SetMicrophoneBlockedCallback(const int32_t clientId,
    const std::shared_ptr<AudioManagerMicrophoneBlockedCallback> &callback)
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, -1, "audio policy manager proxy is NULL.");
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, ERR_INVALID_PARAM,
        "SetMicrophoneBlockedCallback: callback is nullptr");
    if (!isAudioPolicyClientRegisted_) {
        int32_t ret = RegisterPolicyCallbackClientFunc(gsp);
        if (ret != SUCCESS) {
            return ret;
        }
    }
    std::lock_guard<std::mutex> lockCbMap(callbackChangeInfos_[CALLBACK_SET_MICROPHONE_BLOCKED].mutex);
    if (audioPolicyClientStubCB_ != nullptr) {
        audioPolicyClientStubCB_->AddMicrophoneBlockedCallback(clientId, callback);
        size_t callbackSize = audioPolicyClientStubCB_->GetMicrophoneBlockedCallbackSize();
        if (callbackSize == 1) {
            callbackChangeInfos_[CALLBACK_SET_MICROPHONE_BLOCKED].isEnable = true;
            SetClientCallbacksEnable(CALLBACK_SET_MICROPHONE_BLOCKED, true);
        }
    }
    return SUCCESS;
}

int32_t AudioPolicyManager::UnsetDeviceChangeCallback(const int32_t clientId, DeviceFlag flag,
    std::shared_ptr<AudioManagerDeviceChangeCallback> &cb)
{
    AUDIO_DEBUG_LOG("AudioPolicyManager::UnsetDeviceChangeCallback");
    std::lock_guard<std::mutex> lockCbMap(callbackChangeInfos_[CALLBACK_SET_DEVICE_CHANGE].mutex);
    if (audioPolicyClientStubCB_ != nullptr) {
        audioPolicyClientStubCB_->RemoveDeviceChangeCallback(flag, cb);
        if (audioPolicyClientStubCB_->GetDeviceChangeCallbackSize() == 0) {
            callbackChangeInfos_[CALLBACK_SET_DEVICE_CHANGE].isEnable = false;
            SetClientCallbacksEnable(CALLBACK_SET_DEVICE_CHANGE, false);
        }
    }
    return SUCCESS;
}

int32_t AudioPolicyManager::SetPreferredOutputDeviceChangeCallback(const int32_t clientId,
    const std::shared_ptr<AudioPreferredOutputDeviceChangeCallback> &callback)
{
    AUDIO_DEBUG_LOG("AudioPolicyManager::SetPreferredOutputDeviceChangeCallback");
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, ERR_INVALID_PARAM, "callback is nullptr");

    if (!isAudioPolicyClientRegisted_) {
        const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
        CHECK_AND_RETURN_RET_LOG(gsp != nullptr, -1, "audio policy manager proxy is NULL.");
        int32_t ret = RegisterPolicyCallbackClientFunc(gsp);
        if (ret != SUCCESS) {
            return ret;
        }
    }

    std::lock_guard<std::mutex> lockCbMap(callbackChangeInfos_[CALLBACK_PREFERRED_OUTPUT_DEVICE_CHANGE].mutex);
    if (audioPolicyClientStubCB_ != nullptr) {
        audioPolicyClientStubCB_->AddPreferredOutputDeviceChangeCallback(callback);
        size_t callbackSize = audioPolicyClientStubCB_->GetPreferredOutputDeviceChangeCallbackSize();
        if (callbackSize == 1) {
            callbackChangeInfos_[CALLBACK_PREFERRED_OUTPUT_DEVICE_CHANGE].isEnable = true;
            SetClientCallbacksEnable(CALLBACK_PREFERRED_OUTPUT_DEVICE_CHANGE, true);
        }
    }
    return SUCCESS;
}

int32_t AudioPolicyManager::SetPreferredInputDeviceChangeCallback(
    const std::shared_ptr<AudioPreferredInputDeviceChangeCallback> &callback)
{
    AUDIO_DEBUG_LOG("AudioPolicyManager::SetPreferredInputDeviceChangeCallback");
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, ERR_INVALID_PARAM, "callback is nullptr");

    if (!isAudioPolicyClientRegisted_) {
        const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
        CHECK_AND_RETURN_RET_LOG(gsp != nullptr, -1, "audio policy manager proxy is NULL.");
        int32_t ret = RegisterPolicyCallbackClientFunc(gsp);
        if (ret != SUCCESS) {
            return ret;
        }
    }

    std::lock_guard<std::mutex> lockCbMap(callbackChangeInfos_[CALLBACK_PREFERRED_INPUT_DEVICE_CHANGE].mutex);
    if (audioPolicyClientStubCB_ != nullptr) {
        audioPolicyClientStubCB_->AddPreferredInputDeviceChangeCallback(callback);
        size_t callbackSize = audioPolicyClientStubCB_->GetPreferredInputDeviceChangeCallbackSize();
        if (callbackSize == 1) {
            callbackChangeInfos_[CALLBACK_PREFERRED_INPUT_DEVICE_CHANGE].isEnable = true;
            SetClientCallbacksEnable(CALLBACK_PREFERRED_INPUT_DEVICE_CHANGE, true);
        }
    }
    return SUCCESS;
}

int32_t AudioPolicyManager::UnsetPreferredOutputDeviceChangeCallback(const int32_t clientId)
{
    AUDIO_DEBUG_LOG("AudioPolicyManager::UnsetPreferredOutputDeviceChangeCallback");
    std::lock_guard<std::mutex> lockCbMap(callbackChangeInfos_[CALLBACK_PREFERRED_OUTPUT_DEVICE_CHANGE].mutex);
    if (audioPolicyClientStubCB_ != nullptr) {
        audioPolicyClientStubCB_->RemovePreferredOutputDeviceChangeCallback();
        if (audioPolicyClientStubCB_->GetPreferredOutputDeviceChangeCallbackSize() == 0) {
            callbackChangeInfos_[CALLBACK_PREFERRED_OUTPUT_DEVICE_CHANGE].isEnable = false;
            SetClientCallbacksEnable(CALLBACK_PREFERRED_OUTPUT_DEVICE_CHANGE, false);
        }
    }
    return SUCCESS;
}

int32_t AudioPolicyManager::UnsetPreferredInputDeviceChangeCallback()
{
    AUDIO_DEBUG_LOG("AudioPolicyManager::UnsetPreferredInputDeviceChangeCallback");
    std::lock_guard<std::mutex> lockCbMap(callbackChangeInfos_[CALLBACK_PREFERRED_INPUT_DEVICE_CHANGE].mutex);
    if (audioPolicyClientStubCB_ != nullptr) {
        audioPolicyClientStubCB_->RemovePreferredInputDeviceChangeCallback();
        if (audioPolicyClientStubCB_->GetPreferredInputDeviceChangeCallbackSize() == 0) {
            callbackChangeInfos_[CALLBACK_PREFERRED_INPUT_DEVICE_CHANGE].isEnable = false;
            SetClientCallbacksEnable(CALLBACK_PREFERRED_INPUT_DEVICE_CHANGE, false);
        }
    }
    return SUCCESS;
}

int32_t AudioPolicyManager::UnsetMicrophoneBlockedCallback(const int32_t clientId,
    const std::shared_ptr<AudioManagerMicrophoneBlockedCallback> &callback)
{
    std::lock_guard<std::mutex> lockCbMap(callbackChangeInfos_[CALLBACK_SET_MICROPHONE_BLOCKED].mutex);
    if (audioPolicyClientStubCB_ != nullptr) {
        audioPolicyClientStubCB_->RemoveMicrophoneBlockedCallback(clientId, callback);
        if (audioPolicyClientStubCB_->GetMicrophoneBlockedCallbackSize() == 0) {
            callbackChangeInfos_[CALLBACK_SET_MICROPHONE_BLOCKED].isEnable = false;
            SetClientCallbacksEnable(CALLBACK_SET_MICROPHONE_BLOCKED, false);
        }
    }
    return SUCCESS;
}

int32_t AudioPolicyManager::SetMicStateChangeCallback(const int32_t clientId,
    const std::shared_ptr<AudioManagerMicStateChangeCallback> &callback)
{
    AUDIO_DEBUG_LOG("AudioPolicyManager::SetMicStateChangeCallback");
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, ERR_INVALID_PARAM, "callback is nullptr");

    if (!isAudioPolicyClientRegisted_) {
        const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
        CHECK_AND_RETURN_RET_LOG(gsp != nullptr, ERROR, "audio policy manager proxy is NULL.");
        int32_t ret = RegisterPolicyCallbackClientFunc(gsp);
        if (ret != SUCCESS) {
            return ret;
        }
    }

    std::lock_guard<std::mutex> lockCbMap(callbackChangeInfos_[CALLBACK_SET_MIC_STATE_CHANGE].mutex);
    if (audioPolicyClientStubCB_ != nullptr) {
        audioPolicyClientStubCB_->AddMicStateChangeCallback(callback);
        size_t callbackSize = audioPolicyClientStubCB_->GetMicStateChangeCallbackSize();
        if (callbackSize == 1) {
            callbackChangeInfos_[CALLBACK_SET_MIC_STATE_CHANGE].isEnable = true;
            SetClientCallbacksEnable(CALLBACK_SET_MIC_STATE_CHANGE, true);
        }
    }
    return SUCCESS;
}

int32_t AudioPolicyManager::UnsetMicStateChangeCallback(
    const std::shared_ptr<AudioManagerMicStateChangeCallback> &callback)
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, ERROR, "audio policy manager proxy is NULL.");
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, ERR_INVALID_PARAM, "callback is nullptr");
    CHECK_AND_RETURN_RET_LOG(audioPolicyClientStubCB_ != nullptr, ERR_INVALID_OPERATION,
        "audioPolicyClientStubCB is nullptr");
    std::lock_guard<std::mutex> lockCbMap(callbackChangeInfos_[CALLBACK_SET_MIC_STATE_CHANGE].mutex);
    if (audioPolicyClientStubCB_ != nullptr) {
        audioPolicyClientStubCB_->RemoveMicStateChangeCallback();
        if (audioPolicyClientStubCB_->GetMicStateChangeCallbackSize() == 0) {
            callbackChangeInfos_[CALLBACK_SET_MIC_STATE_CHANGE].isEnable = false;
            SetClientCallbacksEnable(CALLBACK_SET_MIC_STATE_CHANGE, false);
        }
    }
    return SUCCESS;
}

int32_t AudioPolicyManager::SetAudioInterruptCallback(const uint32_t sessionID,
    const std::shared_ptr<AudioInterruptCallback> &callback, uint32_t clientUid, const int32_t zoneID)
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, ERROR, "audio policy manager proxy is NULL.");
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, ERR_INVALID_PARAM, "callback is nullptr");

    sptr<AudioPolicyManagerListenerStub> listener = new(std::nothrow) AudioPolicyManagerListenerStub();
    CHECK_AND_RETURN_RET_LOG(listener != nullptr, ERROR, "object null");
    listener->SetInterruptCallback(callback);

    sptr<IRemoteObject> object = listener->AsObject();
    CHECK_AND_RETURN_RET_LOG(object != nullptr, ERROR, "listenerStub->AsObject is nullptr..");

    return gsp->SetAudioInterruptCallback(sessionID, object, clientUid, zoneID);
}

int32_t AudioPolicyManager::UnsetAudioInterruptCallback(const uint32_t sessionID, const int32_t zoneID)
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, -1, "audio policy manager proxy is NULL.");
    return gsp->UnsetAudioInterruptCallback(sessionID, zoneID);
}

int32_t AudioPolicyManager::SetQueryClientTypeCallback(const std::shared_ptr<AudioQueryClientTypeCallback> &callback)
{
    AUDIO_ERR_LOG("In");
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, ERROR, "audio policy manager proxy is NULL.");
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, ERR_INVALID_PARAM, "callback is nullptr");

    sptr<AudioPolicyManagerListenerStub> listener = new(std::nothrow) AudioPolicyManagerListenerStub();
    CHECK_AND_RETURN_RET_LOG(listener != nullptr, ERROR, "object null");
    listener->SetQueryClientTypeCallback(callback);

    sptr<IRemoteObject> object = listener->AsObject();
    CHECK_AND_RETURN_RET_LOG(object != nullptr, ERROR, "listenerStub->AsObject is nullptr.");

    return gsp->SetQueryClientTypeCallback(object);
}

int32_t AudioPolicyManager::ActivateAudioInterrupt(
    const AudioInterrupt &audioInterrupt, const int32_t zoneID, const bool isUpdatedAudioStrategy)
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, -1, "audio policy manager proxy is NULL.");
    return gsp->ActivateAudioInterrupt(audioInterrupt, zoneID, isUpdatedAudioStrategy);
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
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, ERR_INVALID_PARAM, "volume back is nullptr");

    if (!isAudioPolicyClientRegisted_) {
        const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
        CHECK_AND_RETURN_RET_LOG(gsp != nullptr, ERROR, "audio policy manager proxy is NULL.");
        int32_t ret = RegisterPolicyCallbackClientFunc(gsp);
        if (ret != SUCCESS) {
            return ret;
        }
    }

    std::lock_guard<std::mutex> lockCbMap(callbackChangeInfos_[CALLBACK_SET_VOLUME_KEY_EVENT].mutex);
    if (audioPolicyClientStubCB_ != nullptr) {
        audioPolicyClientStubCB_->AddVolumeKeyEventCallback(callback);
        size_t callbackSize = audioPolicyClientStubCB_->GetVolumeKeyEventCallbackSize();
        if (callbackSize == 1) {
            callbackChangeInfos_[CALLBACK_SET_VOLUME_KEY_EVENT].isEnable = true;
            SetClientCallbacksEnable(CALLBACK_SET_VOLUME_KEY_EVENT, true);
        }
    }
    return SUCCESS;
}

int32_t AudioPolicyManager::UnsetVolumeKeyEventCallback(
    const std::shared_ptr<VolumeKeyEventCallback> &callback)
{
    AUDIO_DEBUG_LOG("UnsetVolumeKeyEventCallback");
    std::lock_guard<std::mutex> lockCbMap(callbackChangeInfos_[CALLBACK_SET_VOLUME_KEY_EVENT].mutex);
    if (audioPolicyClientStubCB_ != nullptr) {
        audioPolicyClientStubCB_->RemoveVolumeKeyEventCallback(callback);
        if (audioPolicyClientStubCB_->GetVolumeKeyEventCallbackSize() == 0) {
            callbackChangeInfos_[CALLBACK_SET_VOLUME_KEY_EVENT].isEnable = false;
            SetClientCallbacksEnable(CALLBACK_SET_VOLUME_KEY_EVENT, false);
        }
    }
    return SUCCESS;
}

int32_t AudioPolicyManager::RegisterAudioRendererEventListener(
    const std::shared_ptr<AudioRendererStateChangeCallback> &callback)
{
    AUDIO_DEBUG_LOG("in");
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, ERR_INVALID_PARAM, "RendererEvent Listener callback is nullptr");

    if (!isAudioPolicyClientRegisted_) {
        const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
        CHECK_AND_RETURN_RET_LOG(gsp != nullptr, ERROR, "audio policy manager proxy is NULL.");
        int32_t ret = RegisterPolicyCallbackClientFunc(gsp);
        if (ret != SUCCESS) {
            return ret;
        }
    }

    std::lock_guard<std::mutex> lockCbMap(callbackChangeInfos_[CALLBACK_RENDERER_STATE_CHANGE].mutex);
    audioPolicyClientStubCB_->AddRendererStateChangeCallback(callback);
    size_t callbackSize = audioPolicyClientStubCB_->GetRendererStateChangeCallbackSize();
    if (callbackSize == 1) {
        callbackChangeInfos_[CALLBACK_RENDERER_STATE_CHANGE].isEnable = true;
        SetClientCallbacksEnable(CALLBACK_RENDERER_STATE_CHANGE, true);
    }
    isAudioRendererEventListenerRegistered = true;
    return SUCCESS;
}

int32_t AudioPolicyManager::UnregisterAudioRendererEventListener(
    const std::vector<std::shared_ptr<AudioRendererStateChangeCallback>> &callbacks)
{
    AUDIO_DEBUG_LOG("in");
    std::lock_guard<std::mutex> lockCbMap(callbackChangeInfos_[CALLBACK_RENDERER_STATE_CHANGE].mutex);
    if ((audioPolicyClientStubCB_ != nullptr) && isAudioRendererEventListenerRegistered) {
        audioPolicyClientStubCB_->RemoveRendererStateChangeCallback(callbacks);
        if (audioPolicyClientStubCB_->GetRendererStateChangeCallbackSize() == 0) {
            callbackChangeInfos_[CALLBACK_RENDERER_STATE_CHANGE].isEnable = false;
            SetClientCallbacksEnable(CALLBACK_RENDERER_STATE_CHANGE, false);
        }
        isAudioRendererEventListenerRegistered = false;
    }
    return SUCCESS;
}

int32_t AudioPolicyManager::UnregisterAudioRendererEventListener(
    const std::shared_ptr<AudioRendererStateChangeCallback> &callback)
{
    AUDIO_DEBUG_LOG("in");
    std::lock_guard<std::mutex> lockCbMap(callbackChangeInfos_[CALLBACK_RENDERER_STATE_CHANGE].mutex);
    if ((audioPolicyClientStubCB_ != nullptr) && isAudioRendererEventListenerRegistered) {
        audioPolicyClientStubCB_->RemoveRendererStateChangeCallback(callback);
        if (audioPolicyClientStubCB_->GetRendererStateChangeCallbackSize() == 0) {
            callbackChangeInfos_[CALLBACK_RENDERER_STATE_CHANGE].isEnable = false;
            SetClientCallbacksEnable(CALLBACK_RENDERER_STATE_CHANGE, false);
        }
        isAudioRendererEventListenerRegistered = false;
    }
    return SUCCESS;
}

int32_t AudioPolicyManager::RegisterAudioCapturerEventListener(const int32_t clientPid,
    const std::shared_ptr<AudioCapturerStateChangeCallback> &callback)
{
    AUDIO_DEBUG_LOG("AudioPolicyManager::RegisterAudioCapturerEventListener");

    CHECK_AND_RETURN_RET_LOG(callback != nullptr, ERR_INVALID_PARAM, "Capturer Event Listener callback is nullptr");

    if (!isAudioPolicyClientRegisted_) {
        const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
        CHECK_AND_RETURN_RET_LOG(gsp != nullptr, ERROR, "audio policy manager proxy is NULL.");
        int32_t ret = RegisterPolicyCallbackClientFunc(gsp);
        if (ret != SUCCESS) {
            return ret;
        }
    }

    std::lock_guard<std::mutex> lockCbMap(callbackChangeInfos_[CALLBACK_CAPTURER_STATE_CHANGE].mutex);
    audioPolicyClientStubCB_->AddCapturerStateChangeCallback(callback);
    size_t callbackSize = audioPolicyClientStubCB_->GetCapturerStateChangeCallbackSize();
    if (callbackSize == 1) {
        callbackChangeInfos_[CALLBACK_CAPTURER_STATE_CHANGE].isEnable = true;
        SetClientCallbacksEnable(CALLBACK_CAPTURER_STATE_CHANGE, true);
    }
    isAudioCapturerEventListenerRegistered = true;
    return SUCCESS;
}

int32_t AudioPolicyManager::UnregisterAudioCapturerEventListener(const int32_t clientPid)
{
    AUDIO_DEBUG_LOG("AudioPolicyManager::UnregisterAudioCapturerEventListener");
    std::lock_guard<std::mutex> lockCbMap(callbackChangeInfos_[CALLBACK_CAPTURER_STATE_CHANGE].mutex);
    if ((audioPolicyClientStubCB_ != nullptr) && isAudioCapturerEventListenerRegistered) {
        audioPolicyClientStubCB_->RemoveCapturerStateChangeCallback();
        if (audioPolicyClientStubCB_->GetCapturerStateChangeCallbackSize() == 0) {
            callbackChangeInfos_[CALLBACK_CAPTURER_STATE_CHANGE].isEnable = false;
            SetClientCallbacksEnable(CALLBACK_CAPTURER_STATE_CHANGE, false);
        }
        isAudioCapturerEventListenerRegistered = false;
    }
    return SUCCESS;
}

int32_t AudioPolicyManager::RegisterDeviceChangeWithInfoCallback(
    const uint32_t sessionID, const std::weak_ptr<DeviceChangeWithInfoCallback> &callback)
{
    AUDIO_DEBUG_LOG("In");

    if (callback.expired()) {
        AUDIO_ERR_LOG("callback is expired");
        return ERR_INVALID_PARAM;
    }

    if (!isAudioPolicyClientRegisted_) {
        const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
        CHECK_AND_RETURN_RET_LOG(gsp != nullptr, ERROR, "audio policy manager proxy is NULL.");
        int32_t ret = RegisterPolicyCallbackClientFunc(gsp);
        if (ret != SUCCESS) {
            return ret;
        }
    }

    std::lock_guard<std::mutex> lockCbMap(callbackChangeInfos_[CALLBACK_DEVICE_CHANGE_WITH_INFO].mutex);
    if (audioPolicyClientStubCB_ != nullptr) {
        audioPolicyClientStubCB_->AddDeviceChangeWithInfoCallback(sessionID, callback);
        size_t callbackSize = audioPolicyClientStubCB_->GetDeviceChangeWithInfoCallbackkSize();
        if (callbackSize == 1) {
            callbackChangeInfos_[CALLBACK_DEVICE_CHANGE_WITH_INFO].isEnable = true;
            SetClientCallbacksEnable(CALLBACK_DEVICE_CHANGE_WITH_INFO, true);
        }
    }
    return SUCCESS;
}

int32_t AudioPolicyManager::UnregisterDeviceChangeWithInfoCallback(const uint32_t sessionID)
{
    AUDIO_DEBUG_LOG("In");
    std::lock_guard<std::mutex> lockCbMap(callbackChangeInfos_[CALLBACK_DEVICE_CHANGE_WITH_INFO].mutex);
    if (audioPolicyClientStubCB_ != nullptr) {
        audioPolicyClientStubCB_->RemoveDeviceChangeWithInfoCallback(sessionID);
        if (audioPolicyClientStubCB_->GetDeviceChangeWithInfoCallbackkSize() == 0) {
            callbackChangeInfos_[CALLBACK_DEVICE_CHANGE_WITH_INFO].isEnable = false;
            SetClientCallbacksEnable(CALLBACK_DEVICE_CHANGE_WITH_INFO, false);
        }
    }
    return SUCCESS;
}

int32_t AudioPolicyManager::RegisterTracker(AudioMode &mode, AudioStreamChangeInfo &streamChangeInfo,
    const std::shared_ptr<AudioClientTracker> &clientTrackerObj)
{
    AUDIO_DEBUG_LOG("AudioPolicyManager::RegisterTracker");
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, ERROR, "audio policy manager proxy is NULL.");

    std::lock_guard<std::mutex> lock(clientTrackerStubMutex_);
    sptr<AudioClientTrackerCallbackStub> callback = new(std::nothrow) AudioClientTrackerCallbackStub();
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, ERROR, "clientTrackerCbStub: memory allocation failed");

    callback->SetClientTrackerCallback(clientTrackerObj);

    sptr<IRemoteObject> object = callback->AsObject();
    CHECK_AND_RETURN_RET_LOG(object != nullptr, ERROR, "clientTrackerCbStub: IPC object creation failed");

    int32_t ret = gsp->RegisterTracker(mode, streamChangeInfo, object);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERROR, "RegisterTracker failed");
    int32_t sessionId = mode == AUDIO_MODE_PLAYBACK ? streamChangeInfo.audioRendererChangeInfo.sessionId :
        streamChangeInfo.audioCapturerChangeInfo.sessionId;
    clientTrackerStubMap_[sessionId] = callback;
    return ret;
}

int32_t AudioPolicyManager::UpdateTracker(AudioMode &mode, AudioStreamChangeInfo &streamChangeInfo)
{
    AUDIO_DEBUG_LOG("AudioPolicyManager::UpdateTracker");
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, ERROR, "audio policy manager proxy is NULL.");
    int32_t ret = gsp->UpdateTracker(mode, streamChangeInfo);
    CheckAndRemoveClientTrackerStub(mode, streamChangeInfo);
    return ret;
}

void AudioPolicyManager::FetchOutputDeviceForTrack(AudioStreamChangeInfo &streamChangeInfo,
    const AudioStreamDeviceChangeReasonExt reason)
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    if (gsp != nullptr) {
        gsp->FetchOutputDeviceForTrack(streamChangeInfo, reason);
    } else {
        AUDIO_ERR_LOG("audio policy manager proxy is NULL.");
    }
}

void AudioPolicyManager::FetchInputDeviceForTrack(AudioStreamChangeInfo &streamChangeInfo)
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    if (gsp != nullptr) {
        gsp->FetchInputDeviceForTrack(streamChangeInfo);
    } else {
        AUDIO_ERR_LOG("audio policy manager proxy is NULL.");
    }
}

bool AudioPolicyManager::CheckRecordingCreate(uint32_t appTokenId, uint64_t appFullTokenId, int32_t appUid,
    SourceType sourceType)
{
    AUDIO_ERR_LOG("Not supported operation");
    return false;
}

bool AudioPolicyManager::CheckRecordingStateChange(uint32_t appTokenId, uint64_t appFullTokenId, int32_t appUid,
    AudioPermissionState state)
{
    AUDIO_ERR_LOG("Not supported operation");
    return false;
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

int32_t AudioPolicyManager::GetPreferredOutputStreamType(AudioRendererInfo &rendererInfo)
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, AUDIO_FLAG_INVALID, "audio policy manager proxy is NULL.");
    return gsp->GetPreferredOutputStreamType(rendererInfo);
}

int32_t AudioPolicyManager::GetPreferredInputStreamType(AudioCapturerInfo &capturerInfo)
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, AUDIO_FLAG_INVALID, "audio policy manager proxy is NULL.");
    return gsp->GetPreferredInputStreamType(capturerInfo);
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
    StreamSetState streamSetState, StreamUsage streamUsage)
{
    AUDIO_DEBUG_LOG("UpdateStreamState");
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, ERROR, "audio policy manager proxy is NULL.");
    return  gsp->UpdateStreamState(clientUid, streamSetState, streamUsage);
}

int32_t AudioPolicyManager::GetVolumeGroupInfos(std::string networkId, std::vector<sptr<VolumeGroupInfo>> &infos)
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, ERROR, "audio policy manager proxy is NULL.");
    return gsp->GetVolumeGroupInfos(networkId, infos);
}

int32_t AudioPolicyManager::GetNetworkIdByGroupId(int32_t groupId, std::string &networkId)
{
    AudioXCollie audioXCollie("AudioPolicyManager::GetNetworkIdByGroupId", TIME_OUT_SECONDS);
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
    const std::shared_ptr<AudioRendererPolicyServiceDiedCallback> &callback)
{
    std::lock_guard<std::mutex> lockCbMap(g_cBMapMutex);
    if (rendererCBMap_.count(clientPid)) {
        rendererCBMap_.erase(clientPid);
    }
    rendererCBMap_[clientPid] = callback;
    return SUCCESS;
}

int32_t AudioPolicyManager::UnregisterAudioPolicyServerDiedCb(const int32_t clientPid)
{
    std::lock_guard<std::mutex> lockCbMap(g_cBMapMutex);
    AUDIO_DEBUG_LOG("client pid: %{public}d", clientPid);
    rendererCBMap_.erase(getpid());
    return SUCCESS;
}

int32_t AudioPolicyManager::RegisterAudioStreamPolicyServerDiedCb(
    const std::shared_ptr<AudioStreamPolicyServiceDiedCallback> &callback)
{
    std::lock_guard<std::mutex> lockCbMap(g_cBDiedMapMutex);
    AUDIO_DEBUG_LOG("RegisterAudioStreamPolicyServerDiedCb");
    audioStreamCBMap_.emplace_back(callback);

    return SUCCESS;
}

int32_t AudioPolicyManager::UnregisterAudioStreamPolicyServerDiedCb(
    const std::shared_ptr<AudioStreamPolicyServiceDiedCallback> &callback)
{
    std::lock_guard<std::mutex> lockCbMap(g_cBDiedMapMutex);
    AUDIO_DEBUG_LOG("UnregisterAudioStreamPolicyServerDiedCb");

    audioStreamCBMap_.erase(std::remove_if(audioStreamCBMap_.begin(), audioStreamCBMap_.end(),
        [&callback] (const weak_ptr<AudioStreamPolicyServiceDiedCallback> &cb) {
            auto sharedCb = cb.lock();
            if (sharedCb == callback || sharedCb == nullptr) {
                return true;
            }
            return false;
        }), audioStreamCBMap_.end());

    return SUCCESS;
}

int32_t AudioPolicyManager::CheckMaxRendererInstances()
{
    AUDIO_DEBUG_LOG("AudioPolicyManager::CheckMaxRendererInstances");
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, ERROR, "audio policy manager proxy is NULL.");

    return gsp->CheckMaxRendererInstances();
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

bool AudioPolicyManager::IsVgsVolumeSupported()
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, ERROR, "audio policy manager proxy is NULL.");
    return gsp->IsVgsVolumeSupported();
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

bool AudioPolicyManager::IsSpatializationEnabled(const std::string address)
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, false, "audio policy manager proxy is NULL.");
    return gsp->IsSpatializationEnabled(address);
}

int32_t AudioPolicyManager::SetSpatializationEnabled(const bool enable)
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, ERROR, "audio policy manager proxy is NULL.");
    return gsp->SetSpatializationEnabled(enable);
}

int32_t AudioPolicyManager::SetSpatializationEnabled(const sptr<AudioDeviceDescriptor> &selectedAudioDevice,
    const bool enable)
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, ERROR, "audio policy manager proxy is NULL.");
    return gsp->SetSpatializationEnabled(selectedAudioDevice, enable);
}

bool AudioPolicyManager::IsHeadTrackingEnabled()
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, false, "audio policy manager proxy is NULL.");
    return gsp->IsHeadTrackingEnabled();
}

bool AudioPolicyManager::IsHeadTrackingEnabled(const std::string address)
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, false, "audio policy manager proxy is NULL.");
    return gsp->IsHeadTrackingEnabled(address);
}

int32_t AudioPolicyManager::SetHeadTrackingEnabled(const bool enable)
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, ERROR, "audio policy manager proxy is NULL.");
    return gsp->SetHeadTrackingEnabled(enable);
}

int32_t AudioPolicyManager::SetHeadTrackingEnabled(const sptr<AudioDeviceDescriptor> &selectedAudioDevice,
    const bool enable)
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, ERROR, "audio policy manager proxy is NULL.");
    return gsp->SetHeadTrackingEnabled(selectedAudioDevice, enable);
}

int32_t AudioPolicyManager::RegisterSpatializationEnabledEventListener(
    const std::shared_ptr<AudioSpatializationEnabledChangeCallback> &callback)
{
    AUDIO_DEBUG_LOG("Start to register");
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, ERR_INVALID_PARAM, "callback is nullptr");

    if (!isAudioPolicyClientRegisted_) {
        const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
        CHECK_AND_RETURN_RET_LOG(gsp != nullptr, -1, "audio policy manager proxy is NULL.");
        int32_t ret = RegisterPolicyCallbackClientFunc(gsp);
        if (ret != SUCCESS) {
            return ret;
        }
    }

    std::lock_guard<std::mutex> lockCbMap(callbackChangeInfos_[CALLBACK_SPATIALIZATION_ENABLED_CHANGE].mutex);
    if (audioPolicyClientStubCB_ != nullptr) {
        audioPolicyClientStubCB_->AddSpatializationEnabledChangeCallback(callback);
        size_t callbackSize = audioPolicyClientStubCB_->GetSpatializationEnabledChangeCallbackSize();
        if (callbackSize == 1) {
            callbackChangeInfos_[CALLBACK_SPATIALIZATION_ENABLED_CHANGE].isEnable = true;
            SetClientCallbacksEnable(CALLBACK_SPATIALIZATION_ENABLED_CHANGE, true);
        }
    }
    return SUCCESS;
}

int32_t AudioPolicyManager::RegisterHeadTrackingEnabledEventListener(
    const std::shared_ptr<AudioHeadTrackingEnabledChangeCallback> &callback)
{
    AUDIO_DEBUG_LOG("Start to register");
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, ERR_INVALID_PARAM, "callback is nullptr");

    if (!isAudioPolicyClientRegisted_) {
        const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
        CHECK_AND_RETURN_RET_LOG(gsp != nullptr, -1, "audio policy manager proxy is NULL.");
        int32_t ret = RegisterPolicyCallbackClientFunc(gsp);
        if (ret != SUCCESS) {
            return ret;
        }
    }

    std::lock_guard<std::mutex> lockCbMap(callbackChangeInfos_[CALLBACK_HEAD_TRACKING_ENABLED_CHANGE].mutex);
    if (audioPolicyClientStubCB_ != nullptr) {
        audioPolicyClientStubCB_->AddHeadTrackingEnabledChangeCallback(callback);
        size_t callbackSize = audioPolicyClientStubCB_->GetHeadTrackingEnabledChangeCallbacSize();
        if (callbackSize == 1) {
            callbackChangeInfos_[CALLBACK_HEAD_TRACKING_ENABLED_CHANGE].isEnable = true;
            SetClientCallbacksEnable(CALLBACK_HEAD_TRACKING_ENABLED_CHANGE, true);
        }
    }
    return SUCCESS;
}

int32_t AudioPolicyManager::UnregisterSpatializationEnabledEventListener()
{
    AUDIO_DEBUG_LOG("Start to unregister");
    std::lock_guard<std::mutex> lockCbMap(callbackChangeInfos_[CALLBACK_SPATIALIZATION_ENABLED_CHANGE].mutex);
    if (audioPolicyClientStubCB_ != nullptr) {
        audioPolicyClientStubCB_->RemoveSpatializationEnabledChangeCallback();
        if (audioPolicyClientStubCB_->GetSpatializationEnabledChangeCallbackSize() == 0) {
            callbackChangeInfos_[CALLBACK_SPATIALIZATION_ENABLED_CHANGE].isEnable = false;
            SetClientCallbacksEnable(CALLBACK_SPATIALIZATION_ENABLED_CHANGE, false);
        }
    }
    return SUCCESS;
}

int32_t AudioPolicyManager::UnregisterHeadTrackingEnabledEventListener()
{
    AUDIO_DEBUG_LOG("Start to unregister");
    std::lock_guard<std::mutex> lockCbMap(callbackChangeInfos_[CALLBACK_HEAD_TRACKING_ENABLED_CHANGE].mutex);
    if (audioPolicyClientStubCB_ != nullptr) {
        audioPolicyClientStubCB_->RemoveHeadTrackingEnabledChangeCallback();
        if (audioPolicyClientStubCB_->GetHeadTrackingEnabledChangeCallbacSize() == 0) {
            callbackChangeInfos_[CALLBACK_HEAD_TRACKING_ENABLED_CHANGE].isEnable = false;
            SetClientCallbacksEnable(CALLBACK_HEAD_TRACKING_ENABLED_CHANGE, false);
        }
    }
    return SUCCESS;
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

int32_t AudioPolicyManager::CreateAudioInterruptZone(const std::set<int32_t> &pids, const int32_t zoneID)
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, ERROR, "audio policy manager proxy is NULL.");

    return gsp->CreateAudioInterruptZone(pids, zoneID);
}

int32_t AudioPolicyManager::AddAudioInterruptZonePids(const std::set<int32_t> &pids, const int32_t zoneID)
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, ERROR, "audio policy manager proxy is NULL.");

    return gsp->AddAudioInterruptZonePids(pids, zoneID);
}

int32_t AudioPolicyManager::RemoveAudioInterruptZonePids(const std::set<int32_t> &pids, const int32_t zoneID)
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

ConverterConfig AudioPolicyManager::GetConverterConfig()
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    if (gsp == nullptr) {
        AUDIO_ERR_LOG("audio policy manager proxy is NULL.");
        return ConverterConfig();
    }
    return gsp->GetConverterConfig();
}

bool AudioPolicyManager::IsHighResolutionExist()
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    if (gsp == nullptr) {
        AUDIO_ERR_LOG("audio policy manager proxy is NULL.");
        return false;
    }
    bool gspIsHighResolutionExist = gsp->IsHighResolutionExist();
    return gspIsHighResolutionExist;
}

int32_t AudioPolicyManager::SetHighResolutionExist(bool highResExist)
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    if (gsp == nullptr) {
        AUDIO_ERR_LOG("audio policy manager proxy is NULL.");
        return -1;
    }
    gsp->SetHighResolutionExist(highResExist);
    return SUCCESS;
}

int32_t AudioPolicyManager::ActivateAudioSession(const AudioSessionStrategy &strategy)
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, ERROR, "audio policy manager proxy is NULL.");
    if (!isAudioPolicyClientRegisted_) {
        int32_t result = RegisterPolicyCallbackClientFunc(gsp);
        if (result != SUCCESS) {
            AUDIO_ERR_LOG("Failed to register policy callback clent");
            return result;
        }
    }
    return gsp->ActivateAudioSession(strategy);
}

int32_t AudioPolicyManager::DeactivateAudioSession()
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, ERROR, "audio policy manager proxy is NULL.");
    return gsp->DeactivateAudioSession();
}

bool AudioPolicyManager::IsAudioSessionActivated()
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, false, "audio policy manager proxy is NULL.");
    return gsp->IsAudioSessionActivated();
}

int32_t AudioPolicyManager::SetAudioSessionCallback(const std::shared_ptr<AudioSessionCallback> &audioSessionCallback)
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, ERROR, "audio policy manager proxy is NULL.");
    CHECK_AND_RETURN_RET_LOG(audioSessionCallback != nullptr, ERR_INVALID_PARAM, "audioSessionCallback is nullptr");

    int32_t result = SUCCESS;
    if (!isAudioPolicyClientRegisted_) {
        result = RegisterPolicyCallbackClientFunc(gsp);
        if (result != SUCCESS) {
            AUDIO_ERR_LOG("Failed to register policy callback clent");
            return result;
        }
    }
    if (audioPolicyClientStubCB_ == nullptr) {
        AUDIO_ERR_LOG("audioPolicyClientStubCB_ is null");
        return ERROR_ILLEGAL_STATE;
    }

    result = audioPolicyClientStubCB_->AddAudioSessionCallback(audioSessionCallback);
    if (result != SUCCESS) {
        AUDIO_ERR_LOG("Failed to add audio session callback.");
        return result;
    }

    std::lock_guard<std::mutex> lockCbMap(callbackChangeInfos_[CALLBACK_AUDIO_SESSION].mutex);
    if (audioPolicyClientStubCB_->GetAudioSessionCallbackSize() == 1) {
        // Notify audio server that the client has registerd one listener.
        callbackChangeInfos_[CALLBACK_AUDIO_SESSION].isEnable = true;
        SetClientCallbacksEnable(CALLBACK_AUDIO_SESSION, true);
    }
    return result;
}

int32_t AudioPolicyManager::UnsetAudioSessionCallback()
{
    if (audioPolicyClientStubCB_ == nullptr) {
        AUDIO_ERR_LOG("audioPolicyClientStubCB_ is null");
        return ERROR_ILLEGAL_STATE;
    }

    int32_t result = audioPolicyClientStubCB_->RemoveAudioSessionCallback();
    if (result != SUCCESS) {
        AUDIO_ERR_LOG("Failed to remove all audio session callbacks.");
        return result;
    }

    std::lock_guard<std::mutex> lockCbMap(callbackChangeInfos_[CALLBACK_AUDIO_SESSION].mutex);
    if (audioPolicyClientStubCB_->GetAudioSessionCallbackSize() == 0) {
        // Notify audio server that all of the client listeners have been unregisterd.
        callbackChangeInfos_[CALLBACK_AUDIO_SESSION].isEnable = false;
        SetClientCallbacksEnable(CALLBACK_AUDIO_SESSION, false);
    }
    return result;
}

int32_t AudioPolicyManager::UnsetAudioSessionCallback(const std::shared_ptr<AudioSessionCallback> &audioSessionCallback)
{
    if (audioPolicyClientStubCB_ == nullptr) {
        AUDIO_ERR_LOG("audioPolicyClientStubCB_ is null");
        return ERROR_ILLEGAL_STATE;
    }
    int32_t result = audioPolicyClientStubCB_->RemoveAudioSessionCallback(audioSessionCallback);
    if (result != SUCCESS) {
        AUDIO_ERR_LOG("Failed to remove the audio session callback.");
        return result;
    }

    std::lock_guard<std::mutex> lockCbMap(callbackChangeInfos_[CALLBACK_AUDIO_SESSION].mutex);
    if (audioPolicyClientStubCB_->GetAudioSessionCallbackSize() == 0) {
        // Notify audio server that all of the client listeners have been unregisterd.
        callbackChangeInfos_[CALLBACK_AUDIO_SESSION].isEnable = false;
        SetClientCallbacksEnable(CALLBACK_AUDIO_SESSION, false);
    }
    return result;
}

AudioSpatializationSceneType AudioPolicyManager::GetSpatializationSceneType()
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, SPATIALIZATION_SCENE_TYPE_DEFAULT, "audio policy manager proxy is NULL.");
    return gsp->GetSpatializationSceneType();
}

int32_t AudioPolicyManager::SetSpatializationSceneType(const AudioSpatializationSceneType spatializationSceneType)
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, ERROR, "audio policy manager proxy is NULL.");
    return gsp->SetSpatializationSceneType(spatializationSceneType);
}

float AudioPolicyManager::GetMaxAmplitude(const int32_t deviceId)
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, ERROR, "audio policy manager proxy is NULL.");
    return gsp->GetMaxAmplitude(deviceId);
}

int32_t AudioPolicyManager::DisableSafeMediaVolume()
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, ERROR, "audio policy manager proxy is NULL.");
    return gsp->DisableSafeMediaVolume();
}

bool AudioPolicyManager::IsHeadTrackingDataRequested(const std::string &macAddress)
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, false, "audio policy manager proxy is NULL.");
    return gsp->IsHeadTrackingDataRequested(macAddress);
}

int32_t AudioPolicyManager::RegisterHeadTrackingDataRequestedEventListener(const std::string &macAddress,
    const std::shared_ptr<HeadTrackingDataRequestedChangeCallback> &callback)
{
    AUDIO_DEBUG_LOG("Start to register");
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, ERR_INVALID_PARAM, "callback is nullptr");

    if (!isAudioPolicyClientRegisted_) {
        const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
        CHECK_AND_RETURN_RET_LOG(gsp != nullptr, -1, "audio policy manager proxy is NULL.");
        int32_t ret = RegisterPolicyCallbackClientFunc(gsp);
        if (ret != SUCCESS) {
            return ret;
        }
    }

    std::lock_guard<std::mutex> lockCbMap(callbackChangeInfos_[CALLBACK_HEAD_TRACKING_DATA_REQUESTED_CHANGE].mutex);
    if (audioPolicyClientStubCB_ != nullptr) {
        audioPolicyClientStubCB_->AddHeadTrackingDataRequestedChangeCallback(macAddress, callback);
        size_t callbackSize = audioPolicyClientStubCB_->GetHeadTrackingDataRequestedChangeCallbackSize();
        if (callbackSize == 1) {
            callbackChangeInfos_[CALLBACK_HEAD_TRACKING_DATA_REQUESTED_CHANGE].isEnable = true;
            SetClientCallbacksEnable(CALLBACK_HEAD_TRACKING_DATA_REQUESTED_CHANGE, true);
        }
    }
    return SUCCESS;
}

int32_t AudioPolicyManager::UnregisterHeadTrackingDataRequestedEventListener(const std::string &macAddress)
{
    AUDIO_DEBUG_LOG("Start to unregister");
    std::lock_guard<std::mutex> lockCbMap(callbackChangeInfos_[CALLBACK_HEAD_TRACKING_DATA_REQUESTED_CHANGE].mutex);
    if (audioPolicyClientStubCB_ != nullptr) {
        audioPolicyClientStubCB_->RemoveHeadTrackingDataRequestedChangeCallback(macAddress);
        if (audioPolicyClientStubCB_->GetHeadTrackingDataRequestedChangeCallbackSize() == 0) {
            callbackChangeInfos_[CALLBACK_HEAD_TRACKING_DATA_REQUESTED_CHANGE].isEnable = false;
            SetClientCallbacksEnable(CALLBACK_HEAD_TRACKING_DATA_REQUESTED_CHANGE, false);
        }
    }
    return SUCCESS;
}
int32_t AudioPolicyManager::SetAudioDeviceRefinerCallback(const std::shared_ptr<AudioDeviceRefiner> &callback)
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, ERROR, "audio policy manager proxy is NULL.");
    if (callback == nullptr) {
        return ERR_INVALID_PARAM;
    };

    std::unique_lock<std::mutex> lock(listenerStubMutex_);
    auto activeDistributedRoutingRoleCb = new (std::nothrow) AudioRoutingManagerListenerStub();
    if (activeDistributedRoutingRoleCb == nullptr) {
        AUDIO_ERR_LOG("object is nullptr");
        return ERROR;
    }
    activeDistributedRoutingRoleCb->SetAudioDeviceRefinerCallback(callback);
    sptr<IRemoteObject> object = activeDistributedRoutingRoleCb->AsObject();
    if (object == nullptr) {
        AUDIO_ERR_LOG("listenerStub is nullptr");
        delete activeDistributedRoutingRoleCb;
        return ERROR;
    }

    return gsp->SetAudioDeviceRefinerCallback(object);
}

int32_t AudioPolicyManager::UnsetAudioDeviceRefinerCallback()
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, ERROR, "audio policy manager proxy is NULL.");
    return gsp->UnsetAudioDeviceRefinerCallback();
}

int32_t AudioPolicyManager::TriggerFetchDevice(AudioStreamDeviceChangeReasonExt reason)
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, ERROR, "audio policy manager proxy is NULL.");
    return gsp->TriggerFetchDevice(reason);
}

int32_t AudioPolicyManager::SetPreferredDevice(const PreferredType preferredType,
    const sptr<AudioDeviceDescriptor> &desc)
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, ERROR, "audio policy manager proxy is NULL.");
    return gsp->SetPreferredDevice(preferredType, desc);
}

AudioPolicyManager& AudioPolicyManager::GetInstance()
{
    static AudioPolicyManager policyManager;
    return policyManager;
}

int32_t AudioPolicyManager::MoveToNewPipe(const uint32_t sessionId, const AudioPipeType pipeType)
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, ERROR, "audio policy manager proxy is NULL.");
    return gsp->MoveToNewPipe(sessionId, pipeType);
}
int32_t AudioPolicyManager::SetAudioConcurrencyCallback(const uint32_t sessionID,
    const std::shared_ptr<AudioConcurrencyCallback> &callback)
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, ERROR, "audio policy manager proxy is NULL.");
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, ERR_INVALID_PARAM, "callback is nullptr");

    sptr<AudioConcurrencyStateListenerStub> listener = new(std::nothrow) AudioConcurrencyStateListenerStub();
    CHECK_AND_RETURN_RET_LOG(listener != nullptr, ERROR, "object null");
    listener->SetConcurrencyCallback(callback);

    sptr<IRemoteObject> object = listener->AsObject();
    CHECK_AND_RETURN_RET_LOG(object != nullptr, ERROR, "listenerStub->AsObject is nullptr.");

    return gsp->SetAudioConcurrencyCallback(sessionID, object);
}

int32_t AudioPolicyManager::UnsetAudioConcurrencyCallback(const uint32_t sessionID)
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, -1, "audio policy manager proxy is NULL.");
    return gsp->UnsetAudioConcurrencyCallback(sessionID);
}

int32_t AudioPolicyManager::ActivateAudioConcurrency(const AudioPipeType &pipeType)
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, -1, "audio policy manager proxy is NULL.");
    return gsp->ActivateAudioConcurrency(pipeType);
}

// When AudioPolicyServer died, clear client tracker stubs. New tracker stubs will be added
// in IAudioStream::RestoreAudioStream. Only called in AudioPolicyServerDied().
void AudioPolicyManager::ResetClientTrackerStubMap()
{
    std::lock_guard<std::mutex> lock(clientTrackerStubMutex_);
    for (auto it : clientTrackerStubMap_) {
        if (it.second != nullptr) {
            it.second->UnsetClientTrackerCallback();
        } else {
            AUDIO_WARNING_LOG("Client tracker stub is nullptr in local map");
        }
    }
    clientTrackerStubMap_.clear();
}

void AudioPolicyManager::CheckAndRemoveClientTrackerStub(const AudioMode &mode,
    const AudioStreamChangeInfo &streamChangeInfo)
{
    if (streamChangeInfo.audioRendererChangeInfo.rendererState != RENDERER_RELEASED &&
        streamChangeInfo.audioCapturerChangeInfo.capturerState != CAPTURER_RELEASED) {
        return;
    }
    int32_t sessionId = mode == AUDIO_MODE_PLAYBACK ? streamChangeInfo.audioRendererChangeInfo.sessionId :
        streamChangeInfo.audioCapturerChangeInfo.sessionId;
    RemoveClientTrackerStub(sessionId);
}

void AudioPolicyManager::RemoveClientTrackerStub(int32_t sessionId)
{
    std::unique_lock<std::mutex> lock(clientTrackerStubMutex_);
    if (clientTrackerStubMap_.find(sessionId) != clientTrackerStubMap_.end() &&
        clientTrackerStubMap_[sessionId] != nullptr) {
        clientTrackerStubMap_[sessionId]->UnsetClientTrackerCallback();
        clientTrackerStubMap_.erase(sessionId);
        AUDIO_INFO_LOG("Client tracker for session %{public}d removed", sessionId);
    } else {
        AUDIO_WARNING_LOG("Client tracker for session %{public}d not exist", sessionId);
    }
}

int32_t AudioPolicyManager::GetSupportedAudioEffectProperty(AudioEffectPropertyArrayV3 &propertyArray)
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, ERR_INVALID_PARAM, "audio policy manager proxy is NULL.");
    return gsp->GetSupportedAudioEffectProperty(propertyArray);
}

int32_t AudioPolicyManager::SetAudioEffectProperty(const AudioEffectPropertyArrayV3 &propertyArray)
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, ERR_INVALID_PARAM, "audio policy manager proxy is NULL.");
    return gsp->SetAudioEffectProperty(propertyArray);
}

int32_t AudioPolicyManager::GetAudioEffectProperty(AudioEffectPropertyArrayV3 &propertyArray)
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, ERR_INVALID_PARAM, "audio policy manager proxy is NULL.");
    return gsp->GetAudioEffectProperty(propertyArray);
}

int32_t AudioPolicyManager::InjectInterruption(const std::string networkId, InterruptEvent &event)
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, -1, "audio policy manager proxy is NULL.");
    return gsp->InjectInterruption(networkId, event);
}

int32_t AudioPolicyManager::SetDefaultOutputDevice(const DeviceType deviceType, const uint32_t sessionID,
    const StreamUsage streamUsage, bool isRunning)
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, -1, "audio policy manager proxy is NULL.");
    return gsp->SetDefaultOutputDevice(deviceType, sessionID, streamUsage, isRunning);
}

void AudioPolicyManager::SaveRemoteInfo(const std::string &networkId, DeviceType deviceType)
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    if (gsp != nullptr) {
        gsp->SaveRemoteInfo(networkId, deviceType);
    } else {
        AUDIO_ERR_LOG("audio policy manager proxy is NULL.");
    }
}

int32_t AudioPolicyManager::LoadSplitModule(const std::string &splitArgs, const std::string &networkId)
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, -1, "audio policy manager proxy is NULL.");
    return gsp->LoadSplitModule(splitArgs, networkId);
}

int32_t AudioPolicyManager::SetVoiceRingtoneMute(bool isMute)
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, -1, "audio policy manager proxy is NULL.");
    return gsp->SetVoiceRingtoneMute(isMute);
}
} // namespace AudioStandard
} // namespace OHOS
