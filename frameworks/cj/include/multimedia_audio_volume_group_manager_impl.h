/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
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

#ifndef MULTIMEDIA_AUDIO_VOLUME_GROUP_MANAGER_IMPL_H
#define MULTIMEDIA_AUDIO_VOLUME_GROUP_MANAGER_IMPL_H
#include "cj_common_ffi.h"
#include "native/ffi_remote_data.h"
#include "audio_group_manager.h"
#include "audio_system_manager.h"
#include "multimedia_audio_ffi.h"
#include "multimedia_audio_volume_group_manager_callback.h"

namespace OHOS {
namespace AudioStandard {
class MMAAudioVolumeGroupManagerImpl : public OHOS::FFI::FFIData {
    DECL_TYPE(MMAAudioVolumeGroupManagerImpl, OHOS::FFI::FFIData)
public:
    MMAAudioVolumeGroupManagerImpl(int32_t groupId);
    ~MMAAudioVolumeGroupManagerImpl()
    {
        audioMngr_ = nullptr;
    }
    int32_t GetMaxVolume(int32_t volumeType);
    int32_t GetMinVolume(int32_t volumeType);
    int32_t GetRingerMode() const;
    float GetSystemVolumeInDb(int32_t volumeType, int32_t volumeLevel, int32_t deviceType);
    int32_t GetVolume(int32_t volumeType);
    bool IsMicrophoneMute();
    bool IsMute(int32_t volumeType);
    bool IsVolumeUnadjustable();
    float GetMaxAmplitudeForOutputDevice(const int32_t deviceId);
    float GetMaxAmplitudeForInputDevice(const int32_t deviceId);
    void RegisterCallback(int32_t callbackType, void (*callback)(), int32_t *errorCode);
private:
    AudioSystemManager *audioMngr_ = nullptr;
    int32_t cachedClientId_ = -1;
    std::shared_ptr<AudioGroupManager> audioGroupMngr_ = nullptr;
    std::shared_ptr<CjAudioRingerModeCallback> audioRingerModeCallback_ = nullptr;
    std::shared_ptr<CjAudioManagerMicStateChangeCallback> micStateChangeCallback_ = nullptr;
};
} // namespace AudioStandard
} // namespace OHOS
#endif // MULTIMEDIA_AUDIO_VOLUME_GROUP_MANAGER_IMPL_H
