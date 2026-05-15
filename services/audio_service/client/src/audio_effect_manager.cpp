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
#ifndef LOG_TAG
#define LOG_TAG "AudioEffectManager"
#endif

#include "audio_effect_manager.h"

#include "audio_errors.h"
#include "audio_service_log.h"
#include "audio_policy_manager.h"

namespace OHOS {
namespace AudioStandard {
using namespace std;
AudioEffectManager *AudioEffectManager::GetInstance()
{
    static AudioEffectManager audioEffectManager;
    return &audioEffectManager;
}

int32_t AudioEffectManager::GetSupportedAudioEffectProperty(AudioEffectPropertyArray &propertyArray)
{
    return AudioPolicyManager::GetInstance().GetSupportedAudioEffectProperty(propertyArray);
}

int32_t AudioEffectManager::SetAudioEffectProperty(const AudioEffectPropertyArray &propertyArray)
{
    return AudioPolicyManager::GetInstance().SetAudioEffectProperty(propertyArray);
}

int32_t AudioEffectManager::GetAudioEffectProperty(AudioEffectPropertyArray &propertyArray)
{
    return AudioPolicyManager::GetInstance().GetAudioEffectProperty(propertyArray);
}

bool AudioEffectManager::IsAudioSeparationEffectSupported()
{
    return AudioPolicyManager::GetInstance().IsAudioSeparationEffectSupported();
}

int32_t AudioEffectManager::SetAudioSeparationEffectEnabled(bool enabled, int32_t uid, int64_t streamId)
{
    return AudioPolicyManager::GetInstance().SetAudioSeparationEffectEnabled(enabled, uid, streamId);
}

int32_t AudioEffectManager::SetAudioSeparationEffectVolume(AudioSeparationVolumeType type, double volume)
{
    return AudioPolicyManager::GetInstance().SetAudioSeparationEffectVolume(type, volume);
}

int32_t AudioEffectManager::OnAudioSeparationEffectEnabledChange(
    const std::shared_ptr<AudioSeparationEffectEnabledChangeCallback> &callback)
{
    return AudioPolicyManager::GetInstance().OnAudioSeparationEffectEnabledChange(callback);
}

int32_t AudioEffectManager::OffAudioSeparationEffectEnabledChange()
{
    return AudioPolicyManager::GetInstance().OffAudioSeparationEffectEnabledChange();
}

} // namespace AudioStandard
} // namespace OHOS
