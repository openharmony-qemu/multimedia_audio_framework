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

#include "multimedia_audio_ffi.h"
#include "multimedia_audio_error.h"
#include "multimedia_audio_manager_impl.h"
#include "multimedia_audio_volume_group_manager_impl.h"
#include "multimedia_audio_volume_manager_impl.h"

#include "audio_log.h"

using namespace OHOS::FFI;
// using namespace OHOS::AudioStandard::MMAAudioCapturerImpl;

namespace OHOS {
namespace AudioStandard {
extern "C" {
/* Static Method */
int64_t FfiMMACreateAudioManager(int32_t *errorCode)
{
    auto mgr = FFIData::Create<MMAAudioManagerImpl>();
    if (mgr == nullptr) {
        *errorCode = CJ_ERR_SYSTEM;
        AUDIO_ERR_LOG("FfiMMACreateAudioManager failed.");
        return CJ_ERR_INVALID_RETURN_VALUE;
    }
    *errorCode = SUCCESS_CODE;
    return mgr->GetID();
}

/* Audio Manager */
int64_t FfiMMAAudioManagerGetVolumeManager(int64_t id, int32_t *errorCode)
{
    auto mgr = FFIData::GetData<MMAAudioManagerImpl>(id);
    if (mgr == nullptr) {
        *errorCode = CJ_ERR_SYSTEM;
        AUDIO_ERR_LOG("FfiMMAAudioManagerGetVolumeManager failed.");
        return CJ_ERR_INVALID_RETURN_VALUE;
    }
    *errorCode = SUCCESS_CODE;
    return mgr->GetVolumeManager(errorCode);
}

/* Audio Volume Manager */
int64_t FfiMMAAVMGetVolumeGroupManager(int64_t id, int32_t groupId, int32_t *errorCode)
{
    auto mgr = FFIData::GetData<MMAAudioVolumeManagerImpl>(id);
    if (mgr == nullptr) {
        *errorCode = CJ_ERR_SYSTEM;
        AUDIO_ERR_LOG("FfiMMAAVMGetVolumeGroupManager failed.");
        return CJ_ERR_INVALID_RETURN_VALUE;
    }
    *errorCode = SUCCESS_CODE;
    return mgr->GetVolumeGroupManager(groupId, errorCode);
}

/* Audio Volumne Group Manager */
int32_t FfiMMAAVGMGetMaxVolume(int64_t id, int32_t volumeType, int32_t *errorCode)
{
    auto inst = FFIData::GetData<MMAAudioVolumeGroupManagerImpl>(id);
    if (inst == nullptr) {
        *errorCode = CJ_ERR_SYSTEM;
        AUDIO_ERR_LOG("FfiMMAAVGMGetMaxVolume error");
        return CJ_ERR_INVALID_RETURN_VALUE;
    }
    *errorCode = SUCCESS_CODE;
    return inst->GetMaxVolume(volumeType);
}

int32_t FfiMMAAVGMGetMinVolume(int64_t id, int32_t volumeType, int32_t *errorCode)
{
    auto inst = FFIData::GetData<MMAAudioVolumeGroupManagerImpl>(id);
    if (inst == nullptr) {
        *errorCode = CJ_ERR_SYSTEM;
        AUDIO_ERR_LOG("FfiMMAAVGMGetMinVolume error");
        return CJ_ERR_INVALID_RETURN_VALUE;
    }
    *errorCode = SUCCESS_CODE;
    return inst->GetMinVolume(volumeType);
}

int32_t FfiMMAAVGMGetRingerMode(int64_t id, int32_t *errorCode)
{
    auto inst = FFIData::GetData<MMAAudioVolumeGroupManagerImpl>(id);
    if (inst == nullptr) {
        *errorCode = CJ_ERR_SYSTEM;
        AUDIO_ERR_LOG("FfiMMAAVGMGetRingerMode error");
        return CJ_ERR_INVALID_RETURN_VALUE;
    }
    *errorCode = SUCCESS_CODE;
    return inst->GetRingerMode();
}

float FfiMMAAVGMGetSystemVolumeInDb(int64_t id, int32_t volumeType, int32_t volumeLevel, int32_t device,
    int32_t *errorCode)
{
    auto inst = FFIData::GetData<MMAAudioVolumeGroupManagerImpl>(id);
    if (inst == nullptr) {
        *errorCode = CJ_ERR_SYSTEM;
        AUDIO_ERR_LOG("FfiMMAAVGMGetSystemVolumeInDb error");
        return CJ_ERR_INVALID_RETURN_VALUE;
    }
    *errorCode = SUCCESS_CODE;
    return inst->GetSystemVolumeInDb(volumeType, volumeLevel, device);
}

int32_t FfiMMAAVGMGetVolume(int64_t id, int32_t volumeType, int32_t *errorCode)
{
    auto inst = FFIData::GetData<MMAAudioVolumeGroupManagerImpl>(id);
    if (inst == nullptr) {
        *errorCode = CJ_ERR_SYSTEM;
        AUDIO_ERR_LOG("FfiMMAAVGMGetVolume error");
        return CJ_ERR_INVALID_RETURN_VALUE;
    }
    *errorCode = SUCCESS_CODE;
    return inst->GetVolume(volumeType);
}

bool FfiMMAAVGMIsMicrophoneMute(int64_t id, int32_t *errorCode)
{
    auto inst = FFIData::GetData<MMAAudioVolumeGroupManagerImpl>(id);
    if (inst == nullptr) {
        *errorCode = CJ_ERR_SYSTEM;
        AUDIO_ERR_LOG("Get FfiMMAAVGMIsMicrophoneMute error");
        return CJ_ERR_INVALID_RETURN_VALUE;
    }
    *errorCode = SUCCESS_CODE;
    return inst->IsMicrophoneMute();
}

bool FfiMMAAVGMIsMute(int64_t id, int32_t volumeType, int32_t *errorCode)
{
    auto inst = FFIData::GetData<MMAAudioVolumeGroupManagerImpl>(id);
    if (inst == nullptr) {
        *errorCode = CJ_ERR_SYSTEM;
        AUDIO_ERR_LOG("Get FfiMMAAVGMIsMute error");
        return CJ_ERR_INVALID_RETURN_VALUE;
    }
    *errorCode = SUCCESS_CODE;
    return inst->IsMute(volumeType);
}

bool FfiMMAAVGMIsVolumeUnadjustable(int64_t id, int32_t *errorCode)
{
    auto inst = FFIData::GetData<MMAAudioVolumeGroupManagerImpl>(id);
    if (inst == nullptr) {
        *errorCode = CJ_ERR_SYSTEM;
        AUDIO_ERR_LOG("Get FfiMMAAVGMIsVolumeUnadjustable error");
        return CJ_ERR_INVALID_RETURN_VALUE;
    }
    *errorCode = SUCCESS_CODE;
    return inst->IsVolumeUnadjustable();
}
}
} // namespace AudioStandard
} // namespace OHOS