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
    return ret;
}

int32_t AudioSessionManager::DeactivateAudioSession()
{
    AUDIO_INFO_LOG("in");
    return AudioPolicyManager::GetInstance().DeactivateAudioSession();
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
    return AudioPolicyManager::GetInstance().SetAudioSessionScene(audioSessionScene);
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

} // namespace AudioStandard
} // namespace OHOS
