/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License")
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "multimedia_audio_volume_group_manager_impl.h"
#include "cj_lambda.h"
#include "audio_policy_log.h"
#include "multimedia_audio_common.h"
#include "multimedia_audio_error.h"

namespace OHOS {
namespace AudioStandard {
extern "C" {
MMAAudioVolumeGroupManagerImpl::MMAAudioVolumeGroupManagerImpl(int32_t groupId)
{
    audioMngr_ = AudioSystemManager::GetInstance();
    audioGroupMngr_ = audioMngr_->GetGroupManager(groupId);
    audioRingerModeCallback_ = std::make_shared<CjAudioRingerModeCallback>();
    micStateChangeCallback_ = std::make_shared<CjAudioManagerMicStateChangeCallback>();
    cachedClientId_ = getpid();
}
int32_t MMAAudioVolumeGroupManagerImpl::GetMaxVolume(int32_t volumeType)
{
    if (audioGroupMngr_ == nullptr) {
        AUDIO_ERR_LOG("invalid audio group manager instance");
        return CJ_ERR_INVALID_VALUE;
    }
    auto ret = audioGroupMngr_->GetMaxVolume(GetNativeAudioVolumeType(volumeType));
    return ret;
}

int32_t MMAAudioVolumeGroupManagerImpl::GetMinVolume(int32_t volumeType)
{
    if (audioGroupMngr_ == nullptr) {
        AUDIO_ERR_LOG("invalid audio group manager instance");
        return CJ_ERR_INVALID_VALUE;
    }
    auto ret = audioGroupMngr_->GetMinVolume(GetNativeAudioVolumeType(volumeType));
    return ret;
}

int32_t MMAAudioVolumeGroupManagerImpl::GetRingerMode() const
{
    if (audioGroupMngr_ == nullptr) {
        AUDIO_ERR_LOG("invalid audio group manager instance");
        return CJ_ERR_INVALID_VALUE;
    }
    auto ret = audioGroupMngr_->GetRingerMode();
    return static_cast<int32_t>(ret);
}

float MMAAudioVolumeGroupManagerImpl::GetSystemVolumeInDb(int32_t volumeType, int32_t volumeLevel, int32_t deviceType)
{
    if (audioGroupMngr_ == nullptr) {
        AUDIO_ERR_LOG("invalid audio group manager instance");
        return CJ_ERR_INVALID_RETURN_FLOAT_VALUE;
    }
    auto ret = audioGroupMngr_->GetSystemVolumeInDb(GetNativeAudioVolumeType(volumeType), volumeLevel,
        static_cast<DeviceType>(deviceType));
    return ret;
}

int32_t MMAAudioVolumeGroupManagerImpl::GetVolume(int32_t volumeType)
{
    if (audioGroupMngr_ == nullptr) {
        AUDIO_ERR_LOG("invalid audio group manager instance");
        return CJ_ERR_INVALID_VALUE;
    }
    auto ret = audioGroupMngr_->GetVolume(GetNativeAudioVolumeType(volumeType));
    return ret;
}

bool MMAAudioVolumeGroupManagerImpl::IsMicrophoneMute()
{
    if (audioGroupMngr_ == nullptr) {
        AUDIO_ERR_LOG("invalid audio group manager instance");
        return false;
    }
    auto ret = audioGroupMngr_->IsMicrophoneMute();
    return ret;
}

bool MMAAudioVolumeGroupManagerImpl::IsMute(int32_t volumeType)
{
    bool isMute{ false };
    if (audioGroupMngr_ == nullptr) {
        AUDIO_ERR_LOG("invalid audio group manager instance");
        return isMute;
    }
    auto ret = audioGroupMngr_->IsStreamMute(GetNativeAudioVolumeType(volumeType), isMute);
    if (ret != NATIVE_SUCCESS) {
        AUDIO_ERR_LOG("failed to get mute status.");
    }
    return isMute;
}

bool MMAAudioVolumeGroupManagerImpl::IsVolumeUnadjustable()
{
    if (audioGroupMngr_ == nullptr) {
        AUDIO_ERR_LOG("invalid audio group manager instance");
        return false;
    }
    auto ret = audioGroupMngr_->IsVolumeUnadjustable();
    return ret;
}

float MMAAudioVolumeGroupManagerImpl::GetMaxAmplitudeForOutputDevice(const int32_t deviceId)
{
    if (audioGroupMngr_ == nullptr) {
        AUDIO_ERR_LOG("invalid audio group manager instance");
        return CJ_ERR_INVALID_RETURN_FLOAT_VALUE;
    }
    auto ret = audioGroupMngr_->GetMaxAmplitude(deviceId);
    if (ret < 0) {
        AUDIO_ERR_LOG("failed to get MaxAmplitude.");
    }
    return ret;
}

float MMAAudioVolumeGroupManagerImpl::GetMaxAmplitudeForInputDevice(const int32_t deviceId)
{
    if (audioGroupMngr_ == nullptr) {
        AUDIO_ERR_LOG("invalid audio group manager instance");
        return CJ_ERR_INVALID_RETURN_FLOAT_VALUE;
    }
    auto ret = audioGroupMngr_->GetMaxAmplitude(deviceId);
    if (ret < 0) {
        AUDIO_ERR_LOG("failed to get MaxAmplitude.");
    }
    return ret;
}

void MMAAudioVolumeGroupManagerImpl::RegisterCallback(int32_t callbackType, void (*callback)(), int32_t *errorCode)
{
    if (errorCode == nullptr) {
        AUDIO_ERR_LOG("invalid pointer");
        return;
    }
    if (audioGroupMngr_ == nullptr) {
        AUDIO_ERR_LOG("invalid audio group manager instance");
        *errorCode = CJ_ERR_SYSTEM;
        return;
    }
    switch (callbackType) {
        case AudioVolumeGroupManagerCallbackType::RING_MODE_CHANGE: {
            auto func = CJLambda::Create(reinterpret_cast<void (*)(int32_t)>(callback));
            if (func == nullptr) {
                AUDIO_ERR_LOG("Register RING_MODE_CHANGE event failure!");
                *errorCode = CJ_ERR_SYSTEM;
                return;
            }
            audioRingerModeCallback_->RegisterFunc(func);
            audioGroupMngr_->SetRingerModeCallback(cachedClientId_, audioRingerModeCallback_);
            break;
        }
        case AudioVolumeGroupManagerCallbackType::MICSTATE_CHANGE: {
            auto func = CJLambda::Create(reinterpret_cast<void (*)(CMicStateChangeEvent)>(callback));
            if (func == nullptr) {
                AUDIO_ERR_LOG("Register MICSTATE_CHANGE event failure!");
                *errorCode = CJ_ERR_SYSTEM;
                return;
            }
            micStateChangeCallback_->RegisterFunc(func);
            audioGroupMngr_->SetMicStateChangeCallback(micStateChangeCallback_);
            break;
        }
        default:
            AUDIO_ERR_LOG("No such callback supported");
    }
}
}
} // namespace AudioStandard
} // namespace OHOS
