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

#include "audio_mute_factor_manager.h"
#include "audio_log.h"

namespace OHOS {
namespace AudioStandard {

AudioMuteFactorManager& AudioMuteFactorManager::GetInstance()
{
    static AudioMuteFactorManager instance_;
    return instance_;
}

AudioMuteFactorManager::AudioMuteFactorManager()
{
    AUDIO_INFO_LOG("Construct");
}

AudioMuteFactorManager::~AudioMuteFactorManager()
{
    AUDIO_INFO_LOG("Destruct");
}

bool AudioMuteFactorManager::GetMdmMuteStatus() const
{
    return isMdmMute_;
}

void AudioMuteFactorManager::SetMdmMuteStatus(const bool mdmMute)
{
    isMdmMute_ = mdmMute;
}

} // namespace AudioStandard
} // namespace OHOS