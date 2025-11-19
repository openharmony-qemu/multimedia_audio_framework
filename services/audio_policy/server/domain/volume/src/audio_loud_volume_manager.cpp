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
#define LOG_TAG "AudioLoudVolumeManager"
#endif

#include "audio_loud_volume_manager.h"

#ifdef FEATURE_MULTIMODALINPUT_INPUT
#include "input_manager.h"
#endif
#include "parameters.h"
using namespace std;
namespace OHOS {
namespace AudioStandard {

#ifdef FEATURE_MULTIMODALINPUT_INPUT
LoudVolumeManager::LoudVolumeManager()
    : audioVolumeManager_(AudioVolumeManager::GetInstance()),
      audioActiveDevice_(AudioActiveDevice::GetInstance())

{
    loudVolumeModeEnable_ = system::GetBoolParameter("const.audio.loudvolume", false);
    AUDIO_INFO_LOG("create LoudVolumeManager");
};

LoudVolumeManager::~LoudVolumeManager()
{
    AUDIO_WARNING_LOG("dtor should not happen");
};

bool LoudVolumeManager::IsSkipCloseLoudVolType(AudioStreamType streamType)
{
    auto iter = CONCURRENCY_KSIP_CLOSE_LOUD_VOL_TYPE.find(VolumeUtils::GetVolumeTypeFromStreamType(streamType));
    if (iter != CONCURRENCY_KSIP_CLOSE_LOUD_VOL_TYPE.end()) {
        return true;
    }
    return false;
}

bool LoudVolumeManager::FindLoudVolStreamTypeEnable(
    AudioStreamType streamType, LoudVolumeHoldType &funcHoldType)
{
    auto iter = LOUD_VOL_STREAM_TYPE_ENABLE.find(VolumeUtils::GetVolumeTypeFromStreamType(streamType));
    if (iter != LOUD_VOL_STREAM_TYPE_ENABLE.end()) {
        funcHoldType = iter->second;
        return true;
    }
    return false;
}

void LoudVolumeManager::SetLoudVolumeHoldMap(LoudVolumeHoldType funcHoldType, bool state)
{
    std::lock_guard<std::mutex> lock(setLoudVolHoldMutex_);
    loudVolumeHoldMap_[funcHoldType] = state;
}

bool LoudVolumeManager::ClearLoudVolumeHoldMap(LoudVolumeHoldType funcHoldType)
{
    std::lock_guard<std::mutex> lock(setLoudVolHoldMutex_);
    auto it = loudVolumeHoldMap_.find(funcHoldType);
    if (it != loudVolumeHoldMap_.end()) {
        loudVolumeHoldMap_.erase(it);
        return true;
    }
    return false;
}

bool LoudVolumeManager::GetLoudVolumeHoldMap(LoudVolumeHoldType funcHoldType, bool &state)
{
    std::lock_guard<std::mutex> lock(setLoudVolHoldMutex_);
    if (loudVolumeHoldMap_.count(funcHoldType) > 0) {
        state = loudVolumeHoldMap_[funcHoldType];
        return true;
    }
    return false;
}

bool LoudVolumeManager::ReloadLoudVolumeModeSwitch(LoudVolumeHoldType funcHoldType, SetLoudVolMode setVolMode)
{
    bool isHolding = false;
    if (funcHoldType != LOUD_VOLUME_MODE_MUSIC && funcHoldType != LOUD_VOLUME_MODE_VOICE) {
        AUDIO_ERR_LOG("funHoldType error : %{public}d", funcHoldType);
        return false;
    }
    bool isInLoudVolumeMode = GetLoudVolumeHoldMap(funcHoldType, isHolding);
    switch (setVolMode) {
        case LOUD_VOLUME_SWITCH_ON:
            if (!isInLoudVolumeMode) {
                if (funcHoldType == LOUD_VOLUME_MODE_MUSIC) {
                    SetLoudVolumeHoldMap(funcHoldType, true);
                } else {
                    SetLoudVolumeHoldMap(funcHoldType, false);
                }
                audioVolumeManager_.SendLoudVolumeMode(funcHoldType, true, true);
            }
            break;
        case LOUD_VOLUME_SWITCH_OFF:
            if (isInLoudVolumeMode) {
                ClearLoudVolumeHoldMap(funcHoldType);
                audioVolumeManager_.SendLoudVolumeMode(funcHoldType, false);
            }
            break;
        case LOUD_VOLUME_SWITCH_PAUSE:
            if (isInLoudVolumeMode) {
                audioVolumeManager_.SendLoudVolumeMode(funcHoldType, false);
            }
            break;
        case LOUD_VOLUME_SWITCH_AUTO:
            if (isInLoudVolumeMode && isHolding) {
                audioVolumeManager_.SendLoudVolumeMode(funcHoldType, true);
            } else {
                AUDIO_INFO_LOG("no need load loud volume mode");
            }
            break;
        default:
            AUDIO_ERR_LOG("setVolMode error : %{public}d", setVolMode);
            return false;
    }

    return true;
}

bool LoudVolumeManager::ReloadLoudVolumeMode(
    const AudioStreamType streamInFocus, SetLoudVolMode setVolMode)
{
    if (!loudVolumeModeEnable_) {
        return false;
    }
    LoudVolumeHoldType funcHoldType = LOUD_VOLUME_MODE_INVALID;
    if (IsSkipCloseLoudVolType(streamInFocus)) {
        AUDIO_INFO_LOG("streamType = %{public}d, skip operation loud volume mode.", streamInFocus);
        return false;
    }
    if (!FindLoudVolStreamTypeEnable(streamInFocus, funcHoldType) ||
        audioActiveDevice_.GetCurrentOutputDeviceNetworkId() != LOCAL_NETWORK_ID ||
        audioActiveDevice_.GetCurrentOutputDeviceType() != DeviceType::DEVICE_TYPE_SPEAKER) {
        AUDIO_INFO_LOG("streamType = %{public}d, or deviceType = %{public}d not support loud volume mode.",
            streamInFocus, audioActiveDevice_.GetCurrentOutputDeviceType());
        return false;
    }

    return ReloadLoudVolumeModeSwitch(funcHoldType, setVolMode);
}

bool LoudVolumeManager::CheckLoudVolumeMode(const int32_t volLevel,
    const int32_t keyType, const AudioStreamType &streamInFocus)
{
    LoudVolumeHoldType funcHoldType = LOUD_VOLUME_MODE_INVALID;
    if (!loudVolumeModeEnable_ || !FindLoudVolStreamTypeEnable(streamInFocus, funcHoldType)) {
        return false;
    }
    std::lock_guard<std::mutex> lock(loudVolTrigTimeMutex_);
    constexpr int32_t MAX_TRIGGER_TIMES = 2;
    constexpr int32_t ENABLE_TRIGGER_TIMES = 1;
    constexpr int32_t MAX_LOUD_VOLUME_MSEC = 3000;
    constexpr int32_t MIN_LOUD_VOLUME_MSEC = 400;
    bool isHolding = false;

    struct timeval tv;
    struct timezone tz;
    gettimeofday(&tv, &tz);
    int64_t mSec = static_cast<int64_t>(tv.tv_sec * 1000 + tv.tv_usec / AUDIO_MS_PER_SECOND);

    int32_t volumeLevelMax = audioVolumeManager_.GetMaxVolumeLevel(streamInFocus);
    int32_t volumeLevelInInt = (keyType == OHOS::MMI::KeyEvent::KEYCODE_VOLUME_UP) ? volLevel + 1 : volLevel - 1;
    if (keyType == OHOS::MMI::KeyEvent::KEYCODE_VOLUME_UP && (volumeLevelInInt >= volumeLevelMax)) {
        if (GetLoudVolumeHoldMap(funcHoldType, isHolding)) {
            AUDIO_DEBUG_LOG("no need to repeatedly set loud volume mode.");
            return false;
        }
        if (triggerTime == MAX_TRIGGER_TIMES && mSec - upTriggerTimeMSec < MAX_LOUD_VOLUME_MSEC) {
            triggerTime = 0;
            bool ret = ReloadLoudVolumeMode(streamInFocus, LOUD_VOLUME_SWITCH_ON);
            CHECK_AND_RETURN_RET_LOG(ret != false, false, "set LoudVolume on error");
            return true;
        } else if (triggerTime == ENABLE_TRIGGER_TIMES && (mSec - upTriggerTimeMSec > MIN_LOUD_VOLUME_MSEC)) {
            triggerTime++;
        } else {
            triggerTime = ENABLE_TRIGGER_TIMES;
        }
        upTriggerTimeMSec = mSec;
    } else if (keyType == OHOS::MMI::KeyEvent::KEYCODE_VOLUME_DOWN && GetLoudVolumeHoldMap(funcHoldType, isHolding)) {
        upTriggerTimeMSec = mSec;
        triggerTime = ENABLE_TRIGGER_TIMES;
        bool ret = ReloadLoudVolumeMode(streamInFocus, LOUD_VOLUME_SWITCH_OFF);
        CHECK_AND_RETURN_RET_LOG(ret != false, false, "set LoudVolume off error");
        return true;
    } else {
        upTriggerTimeMSec = 0;
        triggerTime = 0;
    }
    return false;
}
#endif
}
}
