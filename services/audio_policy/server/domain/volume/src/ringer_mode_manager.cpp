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

#ifndef LOG_TAG
#define LOG_TAG "RingerModeManager"
#endif

#include "ringer_mode_manager.h"

#include "audio_policy_log.h"
#include "audio_policy_manager_factory.h"
#include "parameters.h"

namespace OHOS {
namespace AudioStandard {

RingerModeManager::RingerModeManager()
    : audioPolicyManager_(AudioPolicyManagerFactory::GetAudioPolicyManager())
{
}

void RingerModeManager::Init()
{
    supportVibrator_ = system::GetBoolParameter("const.vibrator.support_vibrator", true);
}

bool RingerModeManager::IsRingerModeMute() const
{
    return ringerModeMute_.load();
}

void RingerModeManager::SetRingerModeMute(bool flag)
{
    ringerModeMute_.store(flag);
}

int32_t RingerModeManager::ResetRingerModeMute()
{
    audioPolicyManager_.ClearDeviceNoMuteForRinger();
    SetRingerModeMute(true);
    return SUCCESS;
}

bool RingerModeManager::IsRingerModeValid(AudioRingerMode ringMode) const
{
    bool result = false;
    switch (ringMode) {
        case RINGER_MODE_SILENT:
        case RINGER_MODE_VIBRATE:
        case RINGER_MODE_NORMAL:
            result = true;
            break;
        default:
            result = false;
            AUDIO_ERR_LOG("IsRingerModeValid: ringMode[%{public}d] is not supported", ringMode);
            break;
    }
    return result;
}

bool RingerModeManager::ShouldUpdateRingerMode(int32_t volumeLevel) const
{
    AudioRingerMode currentRingerMode = audioPolicyManager_.GetRingerMode();
    return (currentRingerMode == RINGER_MODE_NORMAL && volumeLevel == 0) ||
           (currentRingerMode != RINGER_MODE_NORMAL && volumeLevel > 0);
}

AudioRingerMode RingerModeManager::GetRingerModeForVolumeLevel(int32_t volumeLevel) const
{
    AudioRingerMode ringerMode = (volumeLevel > 0) ? RINGER_MODE_NORMAL :
        (supportVibrator_ ? RINGER_MODE_VIBRATE : RINGER_MODE_SILENT);
    if (!supportVibrator_) {
        AUDIO_INFO_LOG("The device does not support vibration");
    }
    return ringerMode;
}

AudioRingerMode RingerModeManager::GetRingerModeForMute(bool mute) const
{
    AudioRingerMode ringerMode = mute ?
        (supportVibrator_ ? RINGER_MODE_VIBRATE : RINGER_MODE_SILENT) : RINGER_MODE_NORMAL;
    if (!supportVibrator_) {
        AUDIO_INFO_LOG("The device does not support vibration");
    }
    return ringerMode;
}

}  // namespace AudioStandard
}  // namespace OHOS