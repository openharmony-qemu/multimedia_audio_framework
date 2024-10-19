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

#ifndef MULTIMEDIA_AUDIO_FFI_H
#define MULTIMEDIA_AUDIO_FFI_H
#include <cstdint>
#include "cj_common_ffi.h"
#include "native/ffi_remote_data.h"

namespace OHOS {
namespace AudioStandard {
extern "C" {
// MMA is the addreviation of MultimediaAudio

// Static Method
FFI_EXPORT int64_t FfiMMACreateAudioManager(int32_t *errorCode);

// Audio Manager
FFI_EXPORT int64_t FfiMMAAudioManagerGetVolumeManager(int64_t id, int32_t *errorCode);

// Audio Volumne Manager
// AVM is the addreviation of Audio Volume Manager
FFI_EXPORT int64_t FfiMMAAVMGetVolumeGroupManager(int64_t id, int32_t groupId, int32_t *errorCode);

// Audio Volumne Group Manager
// AVGM is the addreviation of Audio Volume Group Manager
FFI_EXPORT int32_t FfiMMAAVGMGetMaxVolume(int64_t id, int32_t volumeType, int32_t *errorCode);
FFI_EXPORT int32_t FfiMMAAVGMGetMinVolume(int64_t id, int32_t volumeType, int32_t *errorCode);
FFI_EXPORT int32_t FfiMMAAVGMGetRingerMode(int64_t id, int32_t *errorCode);
FFI_EXPORT float FfiMMAAVGMGetSystemVolumeInDb(int64_t id, int32_t volumeType, int32_t volumeLevel, int32_t device,
    int32_t *errorCode);
FFI_EXPORT int32_t FfiMMAAVGMGetVolume(int64_t id, int32_t volumeType, int32_t *errorCode);
FFI_EXPORT bool FfiMMAAVGMIsMicrophoneMute(int64_t id, int32_t *errorCode);
FFI_EXPORT bool FfiMMAAVGMIsMute(int64_t id, int32_t volumeType, int32_t *errorCode);
FFI_EXPORT bool FfiMMAAVGMIsVolumeUnadjustable(int64_t id, int32_t *errorCode);
}
} // namespace AudioStandard
} // namespace OHOS
#endif // MULTIMEDIA_AUDIO_FFI_H