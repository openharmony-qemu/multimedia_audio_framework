/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "audio_asr_client_manager.h"
#include "audio_service_proxy.h"
#include "audio_log.h"


namespace OHOS {
namespace AudioStandard {
AudioAsrClientManager &AudioAsrClientManager::GetInstance()
{
    static AudioAsrClientManager instance;
    return instance;
}

int32_t AudioAsrClientManager::SetAsrAecMode(const AsrAecMode asrAecMode)
{
    const sptr<IStandardAudioService> gasp = AudioServiceProxy::GetAudioSystemManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gasp != nullptr, 0, "Audio service unavailable.");
    return gasp->SetAsrAecMode(static_cast<int32_t>(asrAecMode));
}

int32_t AudioAsrClientManager::GetAsrAecMode(AsrAecMode &asrAecMode)
{
    const sptr<IStandardAudioService> gasp = AudioServiceProxy::GetAudioSystemManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gasp != nullptr, 0, "Audio service unavailable.");
    int32_t mode = 0;
    int32_t ret = gasp->GetAsrAecMode(mode);
    CHECK_AND_RETURN_RET_LOG(ret == 0, AUDIO_ERR, "Get AsrAec Mode audio parameters failed");
    asrAecMode = static_cast<AsrAecMode>(mode);
    return 0;
}

int32_t AudioAsrClientManager::SetAsrNoiseSuppressionMode(const AsrNoiseSuppressionMode asrNoiseSuppressionMode)
{
    const sptr<IStandardAudioService> gasp = AudioServiceProxy::GetAudioSystemManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gasp != nullptr, 0, "Audio service unavailable.");
    return gasp->SetAsrNoiseSuppressionMode(static_cast<int32_t>(asrNoiseSuppressionMode));
}

int32_t AudioAsrClientManager::GetAsrNoiseSuppressionMode(AsrNoiseSuppressionMode &asrNoiseSuppressionMode)
{
    const sptr<IStandardAudioService> gasp = AudioServiceProxy::GetAudioSystemManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gasp != nullptr, 0, "Audio service unavailable.");
    int32_t mode = 0;
    int32_t ret = gasp->GetAsrNoiseSuppressionMode(mode);
    CHECK_AND_RETURN_RET_LOG(ret == 0, AUDIO_ERR, "Get AsrAec Mode audio parameters failed");
    asrNoiseSuppressionMode = static_cast<AsrNoiseSuppressionMode>(mode);
    return 0;
}

int32_t AudioAsrClientManager::SetAsrWhisperDetectionMode(const AsrWhisperDetectionMode asrWhisperDetectionMode)
{
    const sptr<IStandardAudioService> gasp = AudioServiceProxy::GetAudioSystemManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gasp != nullptr, 0, "Audio service unavailable.");
    return gasp->SetAsrWhisperDetectionMode(static_cast<int32_t>(asrWhisperDetectionMode));
}

int32_t AudioAsrClientManager::GetAsrWhisperDetectionMode(AsrWhisperDetectionMode &asrWhisperDetectionMode)
{
    const sptr<IStandardAudioService> gasp = AudioServiceProxy::GetAudioSystemManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gasp != nullptr, 0, "Audio service unavailable.");
    int32_t mode = 0;
    int32_t ret = gasp->GetAsrWhisperDetectionMode(mode);
    CHECK_AND_RETURN_RET_LOG(ret == 0, AUDIO_ERR, "Get AsrWhisperDetection Mode audio parameters failed");
    asrWhisperDetectionMode = static_cast<AsrWhisperDetectionMode>(mode);
    return 0;
}

int32_t AudioAsrClientManager::SetAsrVoiceControlMode(const AsrVoiceControlMode asrVoiceControlMode, bool on)
{
    const sptr<IStandardAudioService> gasp = AudioServiceProxy::GetAudioSystemManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gasp != nullptr, 0, "Audio service unavailable.");
    return gasp->SetAsrVoiceControlMode(static_cast<int32_t>(asrVoiceControlMode), on);
}

int32_t AudioAsrClientManager::SetAsrVoiceMuteMode(const AsrVoiceMuteMode asrVoiceMuteMode, bool on)
{
    const sptr<IStandardAudioService> gasp = AudioServiceProxy::GetAudioSystemManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gasp != nullptr, 0, "Audio service unavailable.");
    return gasp->SetAsrVoiceMuteMode(static_cast<int32_t>(asrVoiceMuteMode), on);
}
} // namespace AudioStandard
} // namespace OHOS
