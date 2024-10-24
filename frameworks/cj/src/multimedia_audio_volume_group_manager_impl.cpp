/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#include "audio_info.h"
#include "audio_log.h"
#include "multimedia_audio_common.h"
#include "multimedia_audio_error.h"

namespace OHOS {
namespace AudioStandard {
extern "C" {
MMAAudioVolumeGroupManagerImpl::MMAAudioVolumeGroupManagerImpl(int32_t groupId)
{
    audioMngr_ = AudioSystemManager::GetInstance();
    audioGroupMngr_ = audioMngr_->GetGroupManager(groupId);
}
int32_t MMAAudioVolumeGroupManagerImpl::GetMaxVolume(int32_t volumeType)
{
    auto ret = audioGroupMngr_->GetMaxVolume(static_cast<AudioVolumeType>(volumeType));
    return ret;
}

int32_t MMAAudioVolumeGroupManagerImpl::GetMinVolume(int32_t volumeType)
{
    auto ret = audioGroupMngr_->GetMinVolume(static_cast<AudioVolumeType>(volumeType));
    return ret;
}

int32_t MMAAudioVolumeGroupManagerImpl::GetRingerMode() const
{
    auto ret = audioGroupMngr_->GetRingerMode();
    return static_cast<int32_t>(ret);
}

float MMAAudioVolumeGroupManagerImpl::GetSystemVolumeInDb(int32_t volumeType, int32_t volumeLevel, int32_t deviceType)
{
    auto ret = audioGroupMngr_->GetSystemVolumeInDb(static_cast<AudioVolumeType>(volumeType), volumeLevel,
        static_cast<DeviceType>(deviceType));
    return ret;
}

int32_t MMAAudioVolumeGroupManagerImpl::GetVolume(int32_t volumeType)
{
    auto ret = audioGroupMngr_->GetVolume(static_cast<AudioVolumeType>(volumeType));
    return ret;
}

bool MMAAudioVolumeGroupManagerImpl::IsMicrophoneMute()
{
    auto ret = audioGroupMngr_->IsMicrophoneMute();
    return ret;
}

bool MMAAudioVolumeGroupManagerImpl::IsMute(int32_t volumeType)
{
    bool isMute{ false };
    auto ret = audioGroupMngr_->IsStreamMute(static_cast<AudioVolumeType>(volumeType), isMute);
    if (ret != NATIVE_SUCCESS) {
        AUDIO_ERR_LOG("failed to get mute status.");
    }
    return isMute;
}

bool MMAAudioVolumeGroupManagerImpl::IsVolumeUnadjustable()
{
    auto ret = audioGroupMngr_->IsVolumeUnadjustable();
    return ret;
}
}
} // namespace AudioStandard
} // namespace OHOS
