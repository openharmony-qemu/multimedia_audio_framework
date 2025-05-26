/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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

#include "iservice_registry.h"
#include "system_ability_definition.h"

#include "audio_errors.h"
#include "audio_policy_log.h"

#include "audio_utils.h"
#include "audio_policy_proxy.h"
#include "audio_server_death_recipient.h"

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
std::weak_ptr<AudioCapturerPolicyServiceDiedCallback> AudioPolicyManager::capturerCB_;
std::vector<std::weak_ptr<AudioStreamPolicyServiceDiedCallback>> AudioPolicyManager::audioStreamCBMap_;
std::unordered_map<int32_t, sptr<AudioClientTrackerCallbackStub>> AudioPolicyManager::clientTrackerStubMap_;

static bool RegisterDeathRecipientInner(sptr<IRemoteObject> object)
{
    pid_t pid = 0;
    pid_t uid = 0;
    sptr<AudioServerDeathRecipient> deathRecipient = new(std::nothrow) AudioServerDeathRecipient(pid, uid);
    CHECK_AND_RETURN_RET_LOG(deathRecipient != nullptr, false, "deathRecipient is null");
    deathRecipient->SetNotifyCb(
        [] (pid_t pid, pid_t uid) { AudioPolicyManager::AudioPolicyServerDied(pid, uid); });
    AUDIO_DEBUG_LOG("Register audio policy server death recipient");
    CHECK_AND_RETURN_RET_LOG(object->AddDeathRecipient(deathRecipient), false, "AddDeathRecipient failed");
    return true;
}

static sptr<IAudioPolicy> GetAudioPolicyProxyFromSamgr()
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
    std::unique_lock<std::mutex> lockRegisterCallbackMutex(registerCallbackMutex_);
    if (audioPolicyClientStubCB_ == nullptr) {
        AUDIO_ERR_LOG("audioPolicyClientStubCB_ is null.");
        return;
    }
    lockRegisterCallbackMutex.unlock();

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
            SetCallbackStreamInfo(enumIndex);
            gsp->SetClientCallbacksEnable(enumIndex, true);
        }
    }
}

int32_t AudioPolicyManager::SetCallbackStreamInfo(const CallbackChange &callbackChange)
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, -1, "audio policy manager proxy is NULL.");

    int32_t ret = SUCCESS;
    if (callbackChange == CALLBACK_PREFERRED_OUTPUT_DEVICE_CHANGE) {
        for (auto &rendererInfo : rendererInfos_) {
            ret = gsp->SetCallbackRendererInfo(rendererInfo);
        }
    } else if (callbackChange == CALLBACK_PREFERRED_INPUT_DEVICE_CHANGE) {
        for (auto &capturerInfo : capturerInfos_) {
            ret = gsp->SetCallbackCapturerInfo(capturerInfo);
        }
    }
    return ret;
}

void AudioPolicyManager::AudioPolicyServerDied(pid_t pid, pid_t uid)
{
    GetInstance().ResetClientTrackerStubMap();
    if (auto capturerCb = capturerCB_.lock()) {
        capturerCb->OnAudioPolicyServiceDied();
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

int32_t AudioPolicyManager::SetSelfAppVolumeLevel(int32_t volumeLevel, int32_t volumeFlag)
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, -1, "audio policy manager proxy is NULL.");
    return gsp->SetSelfAppVolumeLevel(volumeLevel, volumeFlag);
}

int32_t AudioPolicyManager::SetAppVolumeLevel(int32_t appUid, int32_t volumeLevel, int32_t volumeFlag)
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, -1, "audio policy manager proxy is NULL.");
    return gsp->SetAppVolumeLevel(appUid, volumeLevel, volumeFlag);
}

int32_t AudioPolicyManager::SetAppVolumeMuted(int32_t appUid, bool muted, int32_t volumeFlag)
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, -1, "audio policy manager proxy is NULL.");
    return gsp->SetAppVolumeMuted(appUid, muted, volumeFlag);
}

bool AudioPolicyManager::IsAppVolumeMute(int32_t appUid, bool muted)
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, -1, "audio policy manager proxy is NULL.");
    return gsp->IsAppVolumeMute(appUid, muted);
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

int32_t AudioPolicyManager::SetSystemVolumeLevelWithDevice(AudioVolumeType volumeType, int32_t volumeLevel,
    DeviceType deviceType, int32_t volumeFlag)
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, -1, "audio policy manager proxy is NULL.");

    return gsp->SetSystemVolumeLevelWithDevice(volumeType, volumeLevel, deviceType, volumeFlag);
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

int32_t AudioPolicyManager::GetSelfAppVolumeLevel()
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, -1, "audio policy manager proxy is NULL.");
    return gsp->GetSelfAppVolumeLevel();
}

int32_t AudioPolicyManager::GetAppVolumeLevel(int32_t appUid)
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, -1, "audio policy manager proxy is NULL.");
    return gsp->GetAppVolumeLevel(appUid);
}

int32_t AudioPolicyManager::GetSystemVolumeLevel(AudioVolumeType volumeType)
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, -1, "audio policy manager proxy is NULL.");
    return gsp->GetSystemVolumeLevel(volumeType);
}

int32_t AudioPolicyManager::SetStreamMute(AudioVolumeType volumeType, bool mute, bool isLegacy,
    const DeviceType &deviceType)
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, -1, "audio policy manager proxy is NULL.");
    if (isLegacy) {
        return gsp->SetStreamMuteLegacy(volumeType, mute, deviceType);
    }
    return gsp->SetStreamMute(volumeType, mute, deviceType);
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

int32_t AudioPolicyManager::SetCallbackRendererInfo(const AudioRendererInfo &rendererInfo)
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, -1, "audio policy manager proxy is NULL.");
    return gsp->SetCallbackRendererInfo(rendererInfo);
}

int32_t AudioPolicyManager::SetCallbackCapturerInfo(const AudioCapturerInfo &capturerInfo)
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, -1, "audio policy manager proxy is NULL.");
    return gsp->SetCallbackCapturerInfo(capturerInfo);
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
std::vector<int32_t> AudioPolicyManager::GetSupportedTones(const std::string &countryCode)
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    if (gsp == nullptr) {
        AUDIO_ERR_LOG("audio policy manager proxy is NULL.");
        std::vector<int> lSupportedToneList = {};
        return lSupportedToneList;
    }
    return gsp->GetSupportedTones(countryCode);
}

std::shared_ptr<ToneInfo> AudioPolicyManager::GetToneConfig(int32_t ltonetype, const std::string &countryCode)
{
    AUDIO_DEBUG_LOG("AudioPolicyManager::GetToneConfig");

    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, nullptr, "audio policy manager proxy is NULL.");
    return gsp->GetToneConfig(ltonetype, countryCode);
}
#endif

int32_t AudioPolicyManager::SetSelfAppVolumeChangeCallback(
    const std::shared_ptr<AudioManagerAppVolumeChangeCallback> &callback)
{
    AUDIO_DEBUG_LOG("enter set self volume change callback");
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, ERR_INVALID_PARAM, "callback is nullptr");
    if (!isAudioPolicyClientRegisted_) {
        const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
        CHECK_AND_RETURN_RET_LOG(gsp != nullptr, ERR_INVALID_PARAM, "audio policy manager proxy is NULL.");
        int32_t ret = RegisterPolicyCallbackClientFunc(gsp);
        if (ret != SUCCESS) {
            return ret;
        }
    }

    std::lock_guard<std::mutex> lockCbMap(callbackChangeInfos_[CALLBACK_SELF_APP_VOLUME_CHANGE].mutex);
    if (audioPolicyClientStubCB_ != nullptr) {
        audioPolicyClientStubCB_->AddSelfAppVolumeChangeCallback(getuid(), callback);
        size_t callbackSize = audioPolicyClientStubCB_->GetSelfAppVolumeChangeCallbackSize();
        if (callbackSize == 1) {
            callbackChangeInfos_[CALLBACK_SELF_APP_VOLUME_CHANGE].isEnable = true;
            SetClientCallbacksEnable(CALLBACK_SELF_APP_VOLUME_CHANGE, true);
        }
    }
    return SUCCESS;
}

int32_t AudioPolicyManager::UnsetSelfAppVolumeCallback(
    const std::shared_ptr<AudioManagerAppVolumeChangeCallback> &callback)
{
    AUDIO_DEBUG_LOG("enter unset self volume change callback");
    std::lock_guard<std::mutex> lockCbMap(callbackChangeInfos_[CALLBACK_SELF_APP_VOLUME_CHANGE].mutex);
    if (audioPolicyClientStubCB_ == nullptr) {
        AUDIO_ERR_LOG("audioPolicyClientStubCB_ is error");
        return ERR_NULL_POINTER;
    }
    if (callback != nullptr) {
        AUDIO_DEBUG_LOG("callback is not null");
        audioPolicyClientStubCB_->RemoveSelfAppVolumeChangeCallback(getuid(), callback);
    } else {
        AUDIO_DEBUG_LOG("callback is null");
        audioPolicyClientStubCB_->RemoveAllSelfAppVolumeChangeCallback(getuid());
    }
    if (audioPolicyClientStubCB_->GetSelfAppVolumeChangeCallbackSize() == 0) {
        callbackChangeInfos_[CALLBACK_SELF_APP_VOLUME_CHANGE].isEnable = false;
        SetClientCallbacksEnable(CALLBACK_SELF_APP_VOLUME_CHANGE, false);
    }
    return SUCCESS;
}

int32_t AudioPolicyManager::UnsetAppVolumeCallbackForUid(
    const std::shared_ptr<AudioManagerAppVolumeChangeCallback> &callback)
{
    AUDIO_DEBUG_LOG("enter");
    if (!PermissionUtil::VerifySystemPermission()) {
        AUDIO_ERR_LOG("SetAppVolumeChangeCallbackForUid: No system permission");
        return ERR_PERMISSION_DENIED;
    }
    std::lock_guard<std::mutex> lockCbMap(callbackChangeInfos_[CALLBACK_APP_VOLUME_CHANGE].mutex);
    if (audioPolicyClientStubCB_ == nullptr) {
        AUDIO_ERR_LOG("audioPolicyClientStubCB_ is error");
        return ERR_NULL_POINTER;
    }
    audioPolicyClientStubCB_->RemoveAppVolumeChangeForUidCallback(callback);
    if (audioPolicyClientStubCB_->GetAppVolumeChangeCallbackForUidSize() == 0) {
        callbackChangeInfos_[CALLBACK_APP_VOLUME_CHANGE].isEnable = false;
        SetClientCallbacksEnable(CALLBACK_APP_VOLUME_CHANGE, false);
    }
    return SUCCESS;
}

int32_t AudioPolicyManager::SetAppVolumeChangeCallbackForUid(const int32_t appUid,
    const std::shared_ptr<AudioManagerAppVolumeChangeCallback> &callback)
{
    AUDIO_DEBUG_LOG("enter set volume change callback for uid");
    if (!PermissionUtil::VerifySystemPermission()) {
        AUDIO_ERR_LOG("SetAppVolumeChangeCallbackForUid: No system permission");
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

    std::lock_guard<std::mutex> lockCbMap(callbackChangeInfos_[CALLBACK_APP_VOLUME_CHANGE].mutex);
    if (audioPolicyClientStubCB_ != nullptr) {
        audioPolicyClientStubCB_->AddAppVolumeChangeForUidCallback(appUid, callback);
        size_t callbackSize = audioPolicyClientStubCB_->GetAppVolumeChangeCallbackForUidSize();
        if (callbackSize == 1) {
            callbackChangeInfos_[CALLBACK_APP_VOLUME_CHANGE].isEnable = true;
            SetClientCallbacksEnable(CALLBACK_APP_VOLUME_CHANGE, true);
        }
    }
    return SUCCESS;
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

int32_t AudioPolicyManager::SetAudioSceneChangeCallback(const int32_t clientId,
    const std::shared_ptr<AudioManagerAudioSceneChangedCallback> &callback)
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, -1, "audio policy manager proxy is NULL.");
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, ERR_INVALID_PARAM,
        "AudioManagerAudioSceneChangedCallback: callback is nullptr");
    if (!isAudioPolicyClientRegisted_) {
        int32_t ret = RegisterPolicyCallbackClientFunc(gsp);
        if (ret != SUCCESS) {
            return ret;
        }
    }
    std::lock_guard<std::mutex> lockCbMap(callbackChangeInfos_[CALLBACK_SET_AUDIO_SCENE_CHANGE].mutex);
    if (audioPolicyClientStubCB_ != nullptr) {
        audioPolicyClientStubCB_->AddAudioSceneChangedCallback(clientId, callback);
        size_t callbackSize = audioPolicyClientStubCB_->GetAudioSceneChangedCallbackSize();
        if (callbackSize == 1) {
            callbackChangeInfos_[CALLBACK_SET_AUDIO_SCENE_CHANGE].isEnable = true;
            SetClientCallbacksEnable(CALLBACK_SET_AUDIO_SCENE_CHANGE, true);
        }
    }
    return SUCCESS;
}

int32_t AudioPolicyManager::UnsetAudioSceneChangeCallback(
    const std::shared_ptr<AudioManagerAudioSceneChangedCallback> &callback)
{
    std::lock_guard<std::mutex> lockCbMap(callbackChangeInfos_[CALLBACK_SET_AUDIO_SCENE_CHANGE].mutex);
    if (audioPolicyClientStubCB_ != nullptr) {
        audioPolicyClientStubCB_->RemoveAudioSceneChangedCallback(callback);
        if (audioPolicyClientStubCB_->GetAudioSceneChangedCallbackSize() == 0) {
            callbackChangeInfos_[CALLBACK_SET_AUDIO_SCENE_CHANGE].isEnable = false;
            SetClientCallbacksEnable(CALLBACK_SET_AUDIO_SCENE_CHANGE, false);
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
    AUDIO_INFO_LOG("In");
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
    AudioInterrupt &audioInterrupt, const int32_t zoneID, const bool isUpdatedAudioStrategy)
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

AudioStreamType AudioPolicyManager::GetStreamInFocusByUid(const int32_t uid, const int32_t zoneID)
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, STREAM_DEFAULT, "audio policy manager proxy is NULL.");
    return gsp->GetStreamInFocusByUid(uid, zoneID);
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

int32_t AudioPolicyManager::ReconfigureAudioChannel(const uint32_t &count, DeviceType deviceType)
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, -1, "audio policy manager proxy is NULL.");
    return gsp->ReconfigureAudioChannel(count, deviceType);
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
    vector<shared_ptr<AudioRendererChangeInfo>> &audioRendererChangeInfos)
{
    AUDIO_DEBUG_LOG("GetCurrentRendererChangeInfos");
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, -1, "audio policy manager proxy is NULL.");
    return gsp->GetCurrentRendererChangeInfos(audioRendererChangeInfos);
}

int32_t AudioPolicyManager::GetCurrentCapturerChangeInfos(
    vector<shared_ptr<AudioCapturerChangeInfo>> &audioCapturerChangeInfos)
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

int32_t AudioPolicyManager::RegisterAudioPolicyServerDiedCb(const int32_t clientPid,
    const std::shared_ptr<AudioCapturerPolicyServiceDiedCallback> &callback)
{
    std::lock_guard<std::mutex> lockCbMap(g_cBMapMutex);
    capturerCB_ = callback;
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

int32_t AudioPolicyManager::GetHardwareOutputSamplingRate(const std::shared_ptr<AudioDeviceDescriptor> &desc)
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

int32_t AudioPolicyManager::ConfigDistributedRoutingRole(
    std::shared_ptr<AudioDeviceDescriptor> descriptor, CastType type)
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

bool AudioPolicyManager::IsSpatializationEnabledForCurrentDevice()
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, false, "audio policy manager proxy is NULL.");
    return gsp->IsSpatializationEnabledForCurrentDevice();
}

int32_t AudioPolicyManager::SetSpatializationEnabled(const bool enable)
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, ERROR, "audio policy manager proxy is NULL.");
    return gsp->SetSpatializationEnabled(enable);
}

int32_t AudioPolicyManager::SetSpatializationEnabled(
    const std::shared_ptr<AudioDeviceDescriptor> &selectedAudioDevice, const bool enable)
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

int32_t AudioPolicyManager::SetHeadTrackingEnabled(
    const std::shared_ptr<AudioDeviceDescriptor> &selectedAudioDevice, const bool enable)
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

int32_t AudioPolicyManager::RegisterSpatializationEnabledForCurrentDeviceEventListener(
    const std::shared_ptr<AudioSpatializationEnabledChangeForCurrentDeviceCallback> &callback)
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

    std::lock_guard<std::mutex>
        lockCbMap(callbackChangeInfos_[CALLBACK_SPATIALIZATION_ENABLED_CHANGE_FOR_CURRENT_DEVICE].mutex);
    if (audioPolicyClientStubCB_ != nullptr) {
        audioPolicyClientStubCB_->AddSpatializationEnabledChangeForCurrentDeviceCallback(callback);
        size_t callbackSize = audioPolicyClientStubCB_->GetSpatializationEnabledChangeForCurrentDeviceCallbackSize();
        if (callbackSize == 1) {
            callbackChangeInfos_[CALLBACK_SPATIALIZATION_ENABLED_CHANGE_FOR_CURRENT_DEVICE].isEnable = true;
            SetClientCallbacksEnable(CALLBACK_SPATIALIZATION_ENABLED_CHANGE_FOR_CURRENT_DEVICE, true);
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

int32_t AudioPolicyManager::RegisterNnStateEventListener(const std::shared_ptr<AudioNnStateChangeCallback> &callback)
{
    AUDIO_DEBUG_LOG("Start to register");
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, ERR_INVALID_PARAM, "callback is nullptr");

    if (!isAudioPolicyClientRegisted_) {
        const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
        CHECK_AND_RETURN_RET_LOG(gsp != nullptr, -1, "audio policy manager proxy is NULL.");
        int32_t ret = RegisterPolicyCallbackClientFunc(gsp);
        CHECK_AND_RETURN_RET(ret == SUCCESS, ret);
    }

    std::lock_guard<std::mutex> lockCbMap(callbackChangeInfos_[CALLBACK_NN_STATE_CHANGE].mutex);
    CHECK_AND_RETURN_RET(audioPolicyClientStubCB_ != nullptr, SUCCESS);
    audioPolicyClientStubCB_->AddNnStateChangeCallback(callback);
    if (audioPolicyClientStubCB_->GetNnStateChangeCallbackSize() == 1) {
        callbackChangeInfos_[CALLBACK_NN_STATE_CHANGE].isEnable = true;
        SetClientCallbacksEnable(CALLBACK_NN_STATE_CHANGE, true);
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

int32_t AudioPolicyManager::UnregisterSpatializationEnabledForCurrentDeviceEventListener()
{
    AUDIO_DEBUG_LOG("Start to unregister");
    std::lock_guard<std::mutex>
        lockCbMap(callbackChangeInfos_[CALLBACK_SPATIALIZATION_ENABLED_CHANGE_FOR_CURRENT_DEVICE].mutex);
    if (audioPolicyClientStubCB_ != nullptr) {
        audioPolicyClientStubCB_->RemoveSpatializationEnabledChangeForCurrentDeviceCallback();
        if (audioPolicyClientStubCB_->GetSpatializationEnabledChangeForCurrentDeviceCallbackSize() == 0) {
            callbackChangeInfos_[CALLBACK_SPATIALIZATION_ENABLED_CHANGE_FOR_CURRENT_DEVICE].isEnable = false;
            SetClientCallbacksEnable(CALLBACK_SPATIALIZATION_ENABLED_CHANGE_FOR_CURRENT_DEVICE, false);
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

int32_t AudioPolicyManager::UnregisterNnStateEventListener()
{
    AUDIO_DEBUG_LOG("Start to unregister");
    std::lock_guard<std::mutex> lockCbMap(callbackChangeInfos_[CALLBACK_NN_STATE_CHANGE].mutex);
    CHECK_AND_RETURN_RET(audioPolicyClientStubCB_ != nullptr, SUCCESS);
    audioPolicyClientStubCB_->RemoveNnStateChangeCallback();
    if (audioPolicyClientStubCB_->GetNnStateChangeCallbackSize() == 0) {
        callbackChangeInfos_[CALLBACK_NN_STATE_CHANGE].isEnable = false;
        SetClientCallbacksEnable(CALLBACK_NN_STATE_CHANGE, false);
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

int32_t AudioPolicyManager::UnsetAudioSessionCallback(
    const std::shared_ptr<AudioSessionCallback> &audioSessionCallback)
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
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, SPATIALIZATION_SCENE_TYPE_MUSIC, "audio policy manager proxy is NULL.");
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
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, SPATIALIZATION_SCENE_TYPE_DEFAULT, "audio policy manager proxy is NULL.");
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

int32_t AudioPolicyManager::SetAudioClientInfoMgrCallback(
    const std::shared_ptr<AudioClientInfoMgrCallback> &callback)
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, ERROR, "audio policy manager proxy is NULL.");
    if (callback == nullptr) {
        return ERR_INVALID_PARAM;
    };

    sptr<AudioPolicyManagerListenerStub> listener = new (std::nothrow) AudioPolicyManagerListenerStub();
    CHECK_AND_RETURN_RET_LOG(listener != nullptr, ERROR, "object null");
    listener->SetAudioClientInfoMgrCallback(callback);

    sptr<IRemoteObject> object = listener->AsObject();
    CHECK_AND_RETURN_RET_LOG(object != nullptr, ERROR, "listenerStub->AsObject is nullptr.");

    return gsp->SetAudioClientInfoMgrCallback(object);
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
    Trace trace("AudioPolicyManager::ActivateAudioConcurrency:" + std::to_string(pipeType));
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

int32_t AudioPolicyManager::GetSupportedAudioEffectProperty(AudioEffectPropertyArray &propertyArray)
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, ERR_INVALID_PARAM, "audio policy manager proxy is NULL.");
    return gsp->GetSupportedAudioEffectProperty(propertyArray);
}

int32_t AudioPolicyManager::GetSupportedAudioEnhanceProperty(AudioEnhancePropertyArray &propertyArray)
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, ERR_INVALID_PARAM, "audio policy manager proxy is NULL.");
    return gsp->GetSupportedAudioEnhanceProperty(propertyArray);
}

int32_t AudioPolicyManager::SetAudioEffectProperty(const AudioEffectPropertyArray &propertyArray)
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, ERR_INVALID_PARAM, "audio policy manager proxy is NULL.");
    return gsp->SetAudioEffectProperty(propertyArray);
}

int32_t AudioPolicyManager::GetAudioEffectProperty(AudioEffectPropertyArray &propertyArray)
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, ERR_INVALID_PARAM, "audio policy manager proxy is NULL.");
    return gsp->GetAudioEffectProperty(propertyArray);
}

int32_t AudioPolicyManager::SetAudioEnhanceProperty(const AudioEnhancePropertyArray &propertyArray)
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, ERR_INVALID_PARAM, "audio policy manager proxy is NULL.");
    return gsp->SetAudioEnhanceProperty(propertyArray);
}

int32_t AudioPolicyManager::GetAudioEnhanceProperty(AudioEnhancePropertyArray &propertyArray)
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, ERR_INVALID_PARAM, "audio policy manager proxy is NULL.");
    return gsp->GetAudioEnhanceProperty(propertyArray);
}

int32_t AudioPolicyManager::InjectInterruption(const std::string networkId, InterruptEvent &event)
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, -1, "audio policy manager proxy is NULL.");
    return gsp->InjectInterruption(networkId, event);
}

int32_t AudioPolicyManager::LoadSplitModule(const std::string &splitArgs, const std::string &networkId)
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, -1, "audio policy manager proxy is NULL.");
    return gsp->LoadSplitModule(splitArgs, networkId);
}

bool AudioPolicyManager::IsAllowedPlayback(const int32_t &uid, const int32_t &pid)
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, -1, "audio policy manager proxy is NULL.");
    return gsp->IsAllowedPlayback(uid, pid);
}

int32_t AudioPolicyManager::SetVoiceRingtoneMute(bool isMute)
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, -1, "audio policy manager proxy is NULL.");
    return gsp->SetVoiceRingtoneMute(isMute);
}

int32_t AudioPolicyManager::SetVirtualCall(const bool isVirtual)
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, -1, "audio policy manager proxy is NULL.");
    return gsp->SetVirtualCall(isVirtual);
}

int32_t AudioPolicyManager::SetQueryAllowedPlaybackCallback(
    const std::shared_ptr<AudioQueryAllowedPlaybackCallback> &callback)
{
    AUDIO_INFO_LOG("In");
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, ERROR, "audio policy manager proxy is NULL.");
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, ERR_INVALID_PARAM, "callback is nullptr");

    sptr<AudioPolicyManagerListenerStub> listener = new(std::nothrow) AudioPolicyManagerListenerStub();
    CHECK_AND_RETURN_RET_LOG(listener != nullptr, ERROR, "object null");
    listener->SetQueryAllowedPlaybackCallback(callback);

    sptr<IRemoteObject> object = listener->AsObject();
    CHECK_AND_RETURN_RET_LOG(object != nullptr, ERROR, "listenerStub->AsObject is nullptr.");

    return gsp->SetQueryAllowedPlaybackCallback(object);
}

AudioPolicyManager& AudioPolicyManager::GetInstance()
{
    static AudioPolicyManager policyManager;
    return policyManager;
}
} // namespace AudioStandard
} // namespace OHOS
