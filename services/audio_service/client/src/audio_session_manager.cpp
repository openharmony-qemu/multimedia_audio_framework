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
#ifndef LOG_TAG
#define LOG_TAG "AudioSessionManager"
#endif

#include "audio_session_manager.h"

#include "audio_errors.h"
#include "audio_service_log.h"
#include "audio_policy_manager.h"

namespace OHOS {
namespace AudioStandard {
using namespace std;
AudioSessionManager *AudioSessionManager::GetInstance()
{
    static AudioSessionManager audioSessionManager;
    return &audioSessionManager;
}

int32_t AudioSessionManager::ActivateAudioSession(const AudioSessionStrategy &strategy)
{
    AUDIO_INFO_LOG("Activate audio session with strategy: %{public}d", static_cast<int32_t>(strategy.concurrencyMode));
    int32_t ret = AudioPolicyManager::GetInstance().ActivateAudioSession(strategy);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "ActivateAudioSession failed, ret:%{public}d", ret);

    std::lock_guard<std::mutex> lock(setDefaultOutputDeviceMutex_);
    if (setDefaultOutputDevice_) {
        AUDIO_INFO_LOG("need retain SetDefaultOutputDevice");
        AudioPolicyManager::GetInstance().SetDefaultOutputDevice(setDeviceType_);
    }

    RegisterAudioPolicyServerDiedCb();
    restoreParame_.RecordAudioSessionOpt(AudioSessionRestoreParame::OperationType::AUDIO_SESSION_ACTIVATE,
        static_cast<int32_t>(strategy.concurrencyMode));
    return ret;
}

int32_t AudioSessionManager::DeactivateAudioSession()
{
    AUDIO_INFO_LOG("in");
    int32_t ret = AudioPolicyManager::GetInstance().DeactivateAudioSession();
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "DeactivateAudioSession failed, ret:%{public}d", ret);

    restoreParame_.OnAudioSessionDeactive();
    return ret;
}

bool AudioSessionManager::IsAudioSessionActivated()
{
    AUDIO_INFO_LOG("in");
    return AudioPolicyManager::GetInstance().IsAudioSessionActivated();
}

int32_t AudioSessionManager::SetAudioSessionCallback(const std::shared_ptr<AudioSessionCallback> &audioSessionCallback)
{
    AUDIO_INFO_LOG("in");
    CHECK_AND_RETURN_RET_LOG(audioSessionCallback != nullptr, ERR_INVALID_PARAM, "audioSessionCallback is null");

    int32_t result = AudioPolicyManager::GetInstance().SetAudioSessionCallback(audioSessionCallback);
    CHECK_AND_RETURN_RET_LOG(result == SUCCESS, ERR_OPERATION_FAILED,
        "SetAudioSessionCallback result:%{public}d", result);
    return result;
}

int32_t AudioSessionManager::UnsetAudioSessionCallback()
{
    AUDIO_INFO_LOG("Unset all audio session callbacks");
    int32_t result = AudioPolicyManager::GetInstance().UnsetAudioSessionCallback();
    CHECK_AND_RETURN_RET_LOG(result == SUCCESS, ERR_OPERATION_FAILED,
        "UnsetAudioSessionCallback(all) result:%{public}d", result);
    return result;
}

int32_t AudioSessionManager::UnsetAudioSessionCallback(
    const std::shared_ptr<AudioSessionCallback> &audioSessionCallback)
{
    AUDIO_INFO_LOG("Unset one audio session callback");
    CHECK_AND_RETURN_RET_LOG(audioSessionCallback != nullptr, ERR_INVALID_PARAM, "audioSessionCallback is null");

    int32_t result = AudioPolicyManager::GetInstance().UnsetAudioSessionCallback(audioSessionCallback);
    CHECK_AND_RETURN_RET_LOG(result == SUCCESS, ERR_OPERATION_FAILED,
        "UnsetAudioSessionCallback result:%{public}d", result);
    return result;
}

int32_t AudioSessionManager::SetAudioSessionScene(const AudioSessionScene audioSessionScene)
{
    AUDIO_INFO_LOG("Set audio session scene: %{public}d", static_cast<int32_t>(audioSessionScene));

    int32_t result = AudioPolicyManager::GetInstance().SetAudioSessionScene(audioSessionScene);
    CHECK_AND_RETURN_RET_LOG(result == SUCCESS, result, "SetAudioSessionScene failed, result:%{public}d", result);

    RegisterAudioPolicyServerDiedCb();
    restoreParame_.RecordAudioSessionOpt(AudioSessionRestoreParame::OperationType::AUDIO_SESSION_SET_SCENE,
        static_cast<int32_t>(audioSessionScene));
    return result;
}

int32_t AudioSessionManager::SetAudioSessionStateChangeCallback(
    const std::shared_ptr<AudioSessionStateChangedCallback> &stateChangedCallback)
{
    AUDIO_INFO_LOG("in");
    CHECK_AND_RETURN_RET_LOG(stateChangedCallback != nullptr, ERR_INVALID_PARAM, "stateChangedCallback is null");

    int32_t result = AudioPolicyManager::GetInstance().SetAudioSessionStateChangeCallback(stateChangedCallback);
    CHECK_AND_RETURN_RET_LOG(result == SUCCESS, ERR_OPERATION_FAILED,
        "SetAudioSessionStateChangeCallback result:%{public}d", result);
    return result;
}

int32_t AudioSessionManager::UnsetAudioSessionStateChangeCallback()
{
    AUDIO_INFO_LOG("Unset all audio session state callbacks");
    int32_t result = AudioPolicyManager::GetInstance().UnsetAudioSessionStateChangeCallback();
    CHECK_AND_RETURN_RET_LOG(result == SUCCESS, ERR_OPERATION_FAILED,
        "UnsetAudioSessionStateChangeCallback(all) result:%{public}d", result);
    return result;
}

int32_t AudioSessionManager::UnsetAudioSessionStateChangeCallback(
    const std::shared_ptr<AudioSessionStateChangedCallback> &stateChangedCallback)
{
    AUDIO_INFO_LOG("Unset one audio session state callback");
    CHECK_AND_RETURN_RET_LOG(stateChangedCallback != nullptr, ERR_INVALID_PARAM, "stateChangedCallback is null");

    int32_t result = AudioPolicyManager::GetInstance().UnsetAudioSessionStateChangeCallback(stateChangedCallback);
    CHECK_AND_RETURN_RET_LOG(result == SUCCESS, ERR_OPERATION_FAILED,
        "UnsetAudioSessionStateChangeCallback result:%{public}d", result);
    return result;
}

int32_t AudioSessionManager::GetDefaultOutputDevice(DeviceType &deviceType)
{
    AUDIO_INFO_LOG("GetDefaultOutputDevice");
    return AudioPolicyManager::GetInstance().GetDefaultOutputDevice(deviceType);
}

int32_t AudioSessionManager::SetDefaultOutputDevice(DeviceType deviceType)
{
    AUDIO_INFO_LOG("SetDefaultOutputDevice with deviceType: %{public}d", static_cast<int32_t>(deviceType));
    int32_t ret = AudioPolicyManager::GetInstance().SetDefaultOutputDevice(deviceType);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "SetDefaultOutputDevice failed ret:%{public}d", ret);

    AUDIO_INFO_LOG("SetDefaultOutputDevice successful.");
    std::lock_guard<std::mutex> lock(setDefaultOutputDeviceMutex_);
    setDefaultOutputDevice_ = true;
    setDeviceType_ = deviceType;

    RegisterAudioPolicyServerDiedCb();
    return ret;
}

int32_t AudioSessionManager::SetAudioSessionCurrentDeviceChangeCallback(
    const std::shared_ptr<AudioSessionCurrentDeviceChangedCallback> &deviceChangedCallback)
{
    AUDIO_INFO_LOG("in");
    CHECK_AND_RETURN_RET_LOG(deviceChangedCallback != nullptr, ERR_INVALID_PARAM, "deviceChangedCallback is nullptr");

    int32_t ret = AudioPolicyManager::GetInstance().SetAudioSessionCurrentDeviceChangeCallback(deviceChangedCallback);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret,
        "SetAudioSessionCurrentDeviceChangeCallback ret:%{public}d", ret);
    return ret;
}

int32_t AudioSessionManager::UnsetAudioSessionCurrentDeviceChangeCallback()
{
    AUDIO_INFO_LOG("Unset all audio session device callbacks");
    int32_t ret = AudioPolicyManager::GetInstance().UnsetAudioSessionCurrentDeviceChangeCallback();
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret,
        "UnsetAudioSessionCurrentDeviceChangeCallback(all) ret:%{public}d", ret);
    return ret;
}

int32_t AudioSessionManager::UnsetAudioSessionCurrentDeviceChangeCallback(
    const std::shared_ptr<AudioSessionCurrentDeviceChangedCallback> &deviceChangedCallback)
{
    AUDIO_INFO_LOG("Unset one audio session device callback");
    CHECK_AND_RETURN_RET_LOG(deviceChangedCallback != nullptr, ERR_INVALID_PARAM, "deviceChangedCallback is nullptr");

    int32_t ret = AudioPolicyManager::GetInstance().UnsetAudioSessionCurrentDeviceChangeCallback(deviceChangedCallback);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "UnsetAudioSessionCurrentDeviceChangeCallback ret:%{public}d", ret);
    return ret;
}

void AudioSessionManager::RegisterAudioPolicyServerDiedCb()
{
    std::lock_guard<std::mutex> lock(sessionManagerRestoreMutex_);
    CHECK_AND_RETURN(!policyServerDiedCbRegistered_);

    sessionManagerRestoreCb_ = std::make_shared<AudioSessionManagerServiceDiedRestore>();
    CHECK_AND_RETURN_LOG(sessionManagerRestoreCb_ != nullptr,
        "get AudioSessionManagerServiceDiedRestore malloc failed.");

    AudioPolicyManager::GetInstance().RegisterAudioPolicyServerDiedCb(sessionManagerRestoreCb_);

    std::shared_ptr<AudioSessionManagerStateCallback> stateCallback =
        std::make_shared<AudioSessionManagerStateCallback>();
    CHECK_AND_RETURN_LOG(stateCallback != nullptr, "Failed to create AudioSessionState callback!");
    SetAudioSessionStateChangeCallback(stateCallback);

    std::shared_ptr<AudioSessionManagerDeactivedCallback> activeStateCallback =
        std::make_shared<AudioSessionManagerDeactivedCallback>();
    CHECK_AND_RETURN_LOG(activeStateCallback != nullptr, "Failed to create AudioSession actived callback!");
    SetAudioSessionCallback(activeStateCallback);

    policyServerDiedCbRegistered_ = true;
    AUDIO_INFO_LOG("Audio session RegisterAudioPolicyServerDiedCb successed");
}

void AudioSessionManager::OnAudioSessionDeactive(const AudioSessionDeactiveEvent &deactiveEvent)
{
    restoreParame_.OnAudioSessionDeactive();
}

void AudioSessionManager::OnAudioSessionStateChanged(const AudioSessionStateChangedEvent &stateChangedEvent)
{
    restoreParame_.OnAudioSessionStateChanged(stateChangedEvent.stateChangeHint);
}

bool AudioSessionManager::Restore()
{
    AUDIO_INFO_LOG("start restore audio session manager parame.");

    // restore devicetype
    {
        std::lock_guard<std::mutex> lock(setDefaultOutputDeviceMutex_);
        if (setDefaultOutputDevice_) {
            AUDIO_INFO_LOG("restore need SetDefaultOutputDevice");
            auto ret = AudioPolicyManager::GetInstance().SetDefaultOutputDevice(setDeviceType_);
            CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, false, "SetDefaultOutputDevice failed, ret:%{public}d", ret);
        }
    }

    // restore active and setscene
    bool restoreResult = restoreParame_.RestoreParame();
    AUDIO_INFO_LOG("end restore audio session manager parame.");

    return restoreResult;
}

void AudioSessionManagerServiceDiedRestore::OnAudioPolicyServiceDied()
{
    int32_t tryCounter = 10;
    uint32_t sleepTime = 300000;
    bool restoreResult = false;
    while (!restoreResult && tryCounter > 0) {
        tryCounter--;
        usleep(sleepTime);

        auto sessionManager = AudioSessionManager::GetInstance();
        CHECK_AND_RETURN_LOG(sessionManager != nullptr, "can not get audio sessionManager, session is nullptr.");

        restoreResult = sessionManager->Restore();
    }
}

void AudioSessionManagerDeactivedCallback::OnAudioSessionDeactive(const AudioSessionDeactiveEvent &deactiveEvent)
{
    auto sessionManager = AudioSessionManager::GetInstance();
    CHECK_AND_RETURN_LOG(sessionManager != nullptr, "can not get audio sessionManager, session is nullptr.");

    sessionManager->OnAudioSessionDeactive(deactiveEvent);
}

void AudioSessionManagerStateCallback::OnAudioSessionStateChanged(
    const AudioSessionStateChangedEvent &stateChangedEvent)
{
    auto sessionManager = AudioSessionManager::GetInstance();
    CHECK_AND_RETURN_LOG(sessionManager != nullptr, "can not get audio sessionManager, session is nullptr.");

    sessionManager->OnAudioSessionStateChanged(stateChangedEvent);
}

void AudioSessionRestoreParame::OnAudioSessionDeactive()
{
    std::lock_guard<std::mutex> lock(actionsMutex_);
    actions_.clear();
}

void AudioSessionRestoreParame::OnAudioSessionStateChanged(AudioSessionStateChangeHint stateChangeHint)
{
    if ((stateChangeHint == AudioSessionStateChangeHint::STOP) ||
        (stateChangeHint == AudioSessionStateChangeHint::TIME_OUT_STOP) ||
        (stateChangeHint == AudioSessionStateChangeHint::PAUSE)) {
        std::lock_guard<std::mutex> lock(actionsMutex_);
        actions_.clear();
    }
}

void AudioSessionRestoreParame::RecordAudioSessionOpt(const OperationType type, const int32_t value)
{
    std::lock_guard<std::mutex> lock(actionsMutex_);
    if (actions_.empty()) {
        auto action = std::make_unique<AudioSessionAction>(type, value);
        CHECK_AND_RETURN_LOG(action != nullptr, "get action failed, malloc failed.");
        actions_.push_back(std::move(action));
        return;
    }

    // Deduplication of continuous and repeated operations
    if (actions_.back() != nullptr && actions_.back()->type == type) {
        actions_.back()->optValue = value;
        return;
    }

    // iscontinuously repeated operation record
    auto action = std::make_unique<AudioSessionAction>(type, value);
    CHECK_AND_RETURN_LOG(action != nullptr, "get action failed, malloc failed.");
    actions_.push_back(std::move(action));

    // The previous active operation needs to be deleted and the set scene operation needs to be deduplicated.
    if (type == AudioSessionRestoreParame::OperationType::AUDIO_SESSION_ACTIVATE) {
        for (auto it = actions_.begin(); it != std::prev(actions_.end());) {
            CHECK_AND_CONTINUE(*it != nullptr);
            if ((*it)->type == AudioSessionRestoreParame::OperationType::AUDIO_SESSION_ACTIVATE) {
                it = actions_.erase(it);
            } else {
                ++it;
            }
        }

        bool firstFound = false;
        for (int idx = actions_.size() - 1; idx >= 0; idx--) {
            CHECK_AND_CONTINUE(actions_[idx] != nullptr);
            if (actions_[idx]->type != OperationType::AUDIO_SESSION_SET_SCENE) {
                continue;
            }

            if (!firstFound) {
                firstFound = true;
            } else {
                actions_.erase(actions_.begin() + idx);
            }
        }
    }
}

bool AudioSessionRestoreParame::RestoreParame()
{
    int32_t ret;
    std::lock_guard<std::mutex> lock(actionsMutex_);
    for (auto it = actions_.begin(); it != actions_.end(); ++it) {
        CHECK_AND_CONTINUE(*it != nullptr);

        AUDIO_INFO_LOG("AudioSessionManager RestoreParame type = %{public}d, value = %{public}d.",
            (*it)->type, (*it)->optValue);

        if ((*it)->type == AudioSessionRestoreParame::OperationType::AUDIO_SESSION_ACTIVATE) {
            AudioSessionStrategy strategy;
            strategy.concurrencyMode = static_cast<AudioConcurrencyMode>((*it)->optValue);
            ret = AudioPolicyManager::GetInstance().ActivateAudioSession(strategy);
            CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, false, "Restore Activate parame failed, ret:%{public}d", ret);
        }

        if ((*it)->type == AudioSessionRestoreParame::OperationType::AUDIO_SESSION_SET_SCENE) {
            AudioSessionScene audioSessionScene = static_cast<AudioSessionScene>((*it)->optValue);
            ret = AudioPolicyManager::GetInstance().SetAudioSessionScene(audioSessionScene);
            CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, false, "Restore DeviceType parame failed, ret:%{public}d", ret);
        }
    }

    return true;
}

} // namespace AudioStandard
} // namespace OHOS
