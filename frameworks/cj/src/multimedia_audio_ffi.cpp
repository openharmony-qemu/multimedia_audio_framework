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
int64_t FfiMMAGetVolumeManager(int64_t id, int32_t *errorCode)
{
    auto mgr = FFIData::Create<MMAAudioVolumeManagerImpl>();
    if (mgr == nullptr) {
        *errorCode = CJ_ERR_SYSTEM;
        AUDIO_ERR_LOG("FfiMMACreateAudioManager failed.");
        return CJ_ERR_INVALID_RETURN_VALUE;
    }
    *errorCode = SUCCESS_CODE;
    return mgr->GetID();
}

/* Audio Volume Manager */
int64_t FfiMMAGetVolumeGroupManager(int64_t id, int32_t *errorCode)
{
    auto mgr = FFIData::Create<MMAAudioVolumeGroupManagerImpl>();
    if (mgr == nullptr) {
        *errorCode = CJ_ERR_SYSTEM;
        AUDIO_ERR_LOG("FfiMMACreateAudioManager failed.");
        return CJ_ERR_INVALID_RETURN_VALUE;
    }
    *errorCode = SUCCESS_CODE;
    return mgr->GetID();
}

/* Audio Volumne Group Manager */
int32_t FfiMMAGetMaxVolume(int64_t id, int32_t volumeType, int32_t *errorCode)
{
    auto inst = FFIData::GetData<MMAAudioVolumeGroupManagerImpl>(id);
    if (inst != nullptr) {
        *errorCode = CJ_ERR_SYSTEM;
        AUDIO_ERR_LOG("Get MMAAudioVolumeGroupManagerImpl error");
        return CJ_ERR_INVALID_RETURN_VALUE;
    }
    *errorCode = SUCCESS_CODE;
    return inst->GetMaxVolume(volumeType);
}

int32_t FfiMMAGetMinVolume(int64_t id, int32_t volumeType, int32_t *errorCode)
{
    auto inst = FFIData::GetData<MMAAudioVolumeGroupManagerImpl>(id);
    if (inst != nullptr) {
        *errorCode = CJ_ERR_SYSTEM;
        AUDIO_ERR_LOG("Get MMAAudioVolumeGroupManagerImpl error");
        return CJ_ERR_INVALID_RETURN_VALUE;
    }
    *errorCode = SUCCESS_CODE;
    return inst->GetMinVolume(volumeType);
}

int32_t FfiMMAGetRingerMode(int64_t id, int32_t *errorCode)
{
    auto inst = FFIData::GetData<MMAAudioVolumeGroupManagerImpl>(id);
    if (inst != nullptr) {
        *errorCode = CJ_ERR_SYSTEM;
        AUDIO_ERR_LOG("Get MMAAudioVolumeGroupManagerImpl error");
        return CJ_ERR_INVALID_RETURN_VALUE;
    }
    *errorCode = SUCCESS_CODE;
    return inst->GetRingerMode();
}

float FfiMMAGetSystemVolumeInDb(int64_t id, int32_t volumeType, int32_t volumeLevel, int32_t device, int32_t *errorCode)
{
    auto inst = FFIData::GetData<MMAAudioVolumeGroupManagerImpl>(id);
    if (inst != nullptr) {
        *errorCode = CJ_ERR_SYSTEM;
        AUDIO_ERR_LOG("Get MMAAudioVolumeGroupManagerImpl error");
        return CJ_ERR_INVALID_RETURN_VALUE;
    }
    *errorCode = SUCCESS_CODE;
    return inst->GetSystemVolumeInDb(volumeType, volumeLevel, device);
}

int32_t FfiMMAGetVolume(int64_t id, int32_t volumeType, int32_t *errorCode)
{
    auto inst = FFIData::GetData<MMAAudioVolumeGroupManagerImpl>(id);
    if (inst != nullptr) {
        *errorCode = CJ_ERR_SYSTEM;
        AUDIO_ERR_LOG("Get MMAAudioVolumeGroupManagerImpl error");
        return CJ_ERR_INVALID_RETURN_VALUE;
    }
    *errorCode = SUCCESS_CODE;
    return inst->GetVolume(volumeType);
}

bool FfiMMAIsMicrophoneMute(int64_t id, int32_t *errorCode)
{
    auto inst = FFIData::GetData<MMAAudioVolumeGroupManagerImpl>(id);
    if (inst != nullptr) {
        *errorCode = CJ_ERR_SYSTEM;
        AUDIO_ERR_LOG("Get MMAAudioVolumeGroupManagerImpl error");
        return CJ_ERR_INVALID_RETURN_VALUE;
    }
    *errorCode = SUCCESS_CODE;
    return inst->IsMicrophoneMute();
}

bool FfiMMAIsMute(int64_t id, int32_t volumeType, int32_t *errorCode)
{
    auto inst = FFIData::GetData<MMAAudioVolumeGroupManagerImpl>(id);
    if (inst != nullptr) {
        *errorCode = CJ_ERR_SYSTEM;
        AUDIO_ERR_LOG("Get MMAAudioVolumeGroupManagerImpl error");
        return CJ_ERR_INVALID_RETURN_VALUE;
    }
    *errorCode = SUCCESS_CODE;
    return inst->IsMute(volumeType);
}

bool FfiMMAIsVolumeUnadjustable(int64_t id, int32_t *errorCode)
{
    auto inst = FFIData::GetData<MMAAudioVolumeGroupManagerImpl>(id);
    if (inst != nullptr) {
        *errorCode = CJ_ERR_SYSTEM;
        AUDIO_ERR_LOG("Get MMAAudioVolumeGroupManagerImpl error");
        return CJ_ERR_INVALID_RETURN_VALUE;
    }
    *errorCode = SUCCESS_CODE;
    return inst->IsVolumeUnadjustable();
}
}
} // namespace AudioStandard
} // namespace OHOS