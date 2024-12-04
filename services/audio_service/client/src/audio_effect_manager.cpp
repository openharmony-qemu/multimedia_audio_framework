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
#define LOG_TAG "AudioEffectManager"
#endif

#include "audio_effect_manager.h"

#include "audio_errors.h"
#include "audio_service_log.h"
#include "audio_policy_manager.h"
#include "audio_utils.h"
#include "i_audio_stream.h"

namespace OHOS {
namespace AudioStandard {
using namespace std;
AudioEffectManager *AudioEffectManager::GetInstance()
{
    static AudioEffectManager audioEffectManager;
    return &audioEffectManager;
}

int32_t AudioEffectManager::GetSupportedAudioEffectProperty(AudioEffectPropertyArrayV3 &propertyArray)
{
    return AudioPolicyManager::GetInstance().GetSupportedAudioEffectProperty(propertyArray);
}

int32_t AudioEffectManager::SetAudioEffectProperty(const AudioEffectPropertyArrayV3 &propertyArray)
{
    return AudioPolicyManager::GetInstance().SetAudioEffectProperty(propertyArray);
}

int32_t AudioEffectManager::GetAudioEffectProperty(AudioEffectPropertyArrayV3 &propertyArray)
{
    return AudioPolicyManager::GetInstance().GetAudioEffectProperty(propertyArray);
}

} // namespace AudioStandard
} // namespace OHOS
