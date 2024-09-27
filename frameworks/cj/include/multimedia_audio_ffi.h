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
struct CAudioCapturerInfo {
    int32_t capturerFlags;
    int32_t source;
};

struct CAudioStreamInfo {
    int32_t channels;
    int32_t encodingType;
    int32_t sampleFormat;
    int32_t samplingRate;
    int64_t channelLayout;
};

struct CAudioCapturerOptions {
    CAudioCapturerInfo audioCapturerInfo;
    CAudioStreamInfo audioStreamInfo;
};

struct COptionArr {
    CArrI32 arr;
    bool hasValue;
};

struct CDeviceDescriptor {
    char *address;
    CArrI32 channelCounts;
    CArrI32 channelMasks;
    int32_t deviceRole;
    int32_t deviceType;
    char *displayName;
    COptionArr encodingTypes;
    int32_t id;
    char *name;
    CArrI32 sampleRates;
};

struct CArrDeviceDescriptor {
    CDeviceDescriptor *head;
    int64_t size;
};

struct CAudioCapturerChangeInfo {
    CAudioCapturerInfo audioCapturerInfo;
    CArrDeviceDescriptor deviceDescriptors;
    int32_t streamId;
    bool muted;
};

struct CDeviceChangeAction {
    CArrDeviceDescriptor deviceDescriptors;
    int32_t changeType;
};

struct CArrAudioCapturerChangeInfo {
    CAudioCapturerChangeInfo *head;
    int64_t size;
};

struct CInterruptEvent {
    int32_t eventType;
    int32_t forceType;
    int32_t hintType;
};

enum AudioCapturerCallbackType : int32_t {
    AUDIO_CAPTURER_CHANGE = 0,
    AUDIO_INTERRUPT,
    INPUT_DEVICE_CHANGE,
    MARK_REACH,
    PERIOD_REACH,
    READ_DATA,
    STATE_CHANGE
};

enum AudioStreamManagerCallbackType : int32_t { CAPTURER_CHANGE = 0, RENDERER_CHANGE };

enum AudioRoutingManagerCallbackType : int32_t {
    DEVICE_CHANGE = 0,
    MICROPHONE_BLOCKED,
    INPUT_DEVICE_CHANGE_FOR_CAPTURER_INFO,
    OUTPUT_DEVICE_CHANGE_FOR_RENDERER_INFO
};

// Audio Capturer
// MMA is the addreviation of MultimediaAudio
FFI_EXPORT int64_t FfiMMACreateAudioCapturer(CAudioCapturerOptions options, int32_t *errorCode);
FFI_EXPORT int32_t FfiMMAAudioCapturerGetState(int64_t id, int32_t *errorCode);
FFI_EXPORT uint32_t FfiMMAAudioCapturerGetStreamId(int64_t id, int32_t *errorCode);
FFI_EXPORT int64_t FfiMMAAudioCapturerGetAudioTime(int64_t id, int32_t *errorCode);
FFI_EXPORT uint32_t FfiMMAAudioCapturerGetBufferSize(int64_t id, int32_t *errorCode);
FFI_EXPORT uint32_t FfiMMAAudioCapturerGetOverflowCount(int64_t id, int32_t *errorCode);
FFI_EXPORT void FfiMMAAudioCapturerStart(int64_t id, int32_t *errorCode);
FFI_EXPORT void FfiMMAAudioCapturerStop(int64_t id, int32_t *errorCode);
FFI_EXPORT void FfiMMAAudioCapturerRelease(int64_t id, int32_t *errorCode);
FFI_EXPORT CAudioCapturerChangeInfo FfiMMAAudioCapturerGetAudioCapturerChangeInfo(int64_t id, int32_t *errorCode);
FFI_EXPORT CArrDeviceDescriptor FfiMMAAudioCapturerGetInputDevices(int64_t id, int32_t *errorCode);
FFI_EXPORT CAudioCapturerInfo FfiMMAAudioCapturerGetCapturerInfo(int64_t id, int32_t *errorCode);
FFI_EXPORT CAudioStreamInfo FfiMMAAudioCapturerGetStreamInfo(int64_t id, int32_t *errorCode);
FFI_EXPORT void FfiMMAAudioCapturerOn(int64_t id, int32_t callbackType, void (*callback)(), int32_t *errorCode);
FFI_EXPORT void FfiMMAAudioCapturerOnWithFrame(int64_t id, int32_t callbackType, void (*callback)(), int64_t frame,
    int32_t *errorCode);

// Audio Manager
FFI_EXPORT int64_t FfiMMACreateAudioManager(int32_t *errorCode);
FFI_EXPORT int64_t FfiMMAAudioManagerGetRoutingManager(int64_t id, int32_t *errorCode);
FFI_EXPORT int64_t FfiMMAAudioManagerGetStreamManager(int64_t id, int32_t *errorCode);
FFI_EXPORT int32_t FfiMMAAudioManagerGetAudioScene(int64_t id, int32_t *errorCode);
FFI_EXPORT int64_t FfiMMAAudioManagerGetVolumeManager(int64_t id, int32_t *errorCode);

// Audio Stream Manager
// ASM is the addreviation of Audio Stream Manager
FFI_EXPORT bool FfiMMAASMIsActive(int64_t id, int32_t volumeType, int32_t *errorCode);
FFI_EXPORT CArrAudioCapturerChangeInfo FfiMMAASMGetCurrentAudioCapturerInfoArray(int64_t id, int32_t *errorCode);
FFI_EXPORT CArrI32 FfiMMAASMGetAudioEffectInfoArray(int64_t id, int32_t usage, int32_t *errorCode);
FFI_EXPORT void FfiMMAASMOn(int64_t id, int32_t callbackType, void (*callback)(), int32_t *errorCode);

// Audio Routing Manager
// ARM is the addreviation of Audio Routing Manager
FFI_EXPORT void FfiMMAARMSetCommunicationDevice(int64_t id, int32_t deviceType, bool active, int32_t *errorCode);
FFI_EXPORT bool FfiMMAARMIsCommunicationDeviceActive(int64_t id, int32_t deviceType, int32_t *errorCode);
FFI_EXPORT CArrDeviceDescriptor FfiMMAARMGetDevices(int64_t id, int32_t deviceFlag, int32_t *errorCode);
FFI_EXPORT CArrDeviceDescriptor FfiMMAARMGetPreferredInputDeviceForCapturerInfo(int64_t id,
    CAudioCapturerInfo capturerInfo, int32_t *errorCode);
FFI_EXPORT void FfiMMAARMOn(int64_t id, int32_t callbackType, void (*callback)(), int32_t *errorCode);
FFI_EXPORT void FfiMMAARMOnWithFlags(int64_t id, int32_t callbackType, void (*callback)(), int32_t flags,
    int32_t *errorCode);
FFI_EXPORT void FfiMMAARMWOnithCapturerInfo(int64_t id, int32_t callbackType, void (*callback)(),
    CAudioCapturerInfo capturerInfo, int32_t *errorCode);

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
