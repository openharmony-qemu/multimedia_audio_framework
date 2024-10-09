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
#include "audio_log.h"
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
    return AudioPolicyManager::GetInstance().ActivateAudioSession(strategy);
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
} // namespace AudioStandard
} // namespace OHOS
