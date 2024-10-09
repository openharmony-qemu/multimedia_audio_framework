/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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
#define LOG_TAG "AudioSaSdk"
#endif

#include "audio_sasdk.h"
#include "audio_policy_manager.h"
#include "audio_service_log.h"

namespace OHOS {
namespace AudioStandard {
using namespace std;

AudioSaSdk::AudioSaSdk()
{
    AUDIO_DEBUG_LOG("Construct AudioSaSdk");
}

AudioSaSdk::~AudioSaSdk()
{
    AUDIO_DEBUG_LOG("Destruct AudioSaSdk");
}

AudioSaSdk *AudioSaSdk::GetInstance()
{
    static AudioSaSdk audioSaSdk;
    return &audioSaSdk;
}

bool AudioSaSdk::IsStreamActive(SaSdkAudioVolumeType streamType)
{
    return AudioPolicyManager::GetInstance().IsStreamActive(static_cast<AudioVolumeType>(streamType));
}
} // namespace AudioStandard
} // namespace OHOS
