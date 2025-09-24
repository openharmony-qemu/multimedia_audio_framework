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
    char* address;
    CArrI32 channelCounts;
    CArrI32 channelMasks;
    int32_t deviceRole;
    int32_t deviceType;
    char* displayName;
    COptionArr encodingTypes;
    int32_t id;
    char* name;
    CArrI32 sampleRates;
};

struct CArrDeviceDescriptor {
    CDeviceDescriptor* head;
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
    CAudioCapturerChangeInfo* head;
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
    AVAILABLE_DEVICE_CHANGE,
    INPUT_DEVICE_CHANGE_FOR_CAPTURER_INFO,
    OUTPUT_DEVICE_CHANGE_FOR_RENDERER_INFO
};

struct CVolumeEvent {
    int32_t volume;
    int32_t volumeType;
    bool updateUi;
};

enum AudioVolumeManagerCallbackType : int32_t { VOLUME_CHANGE = 0 };

enum AudioVolumeGroupManagerCallbackType : int32_t { RING_MODE_CHANGE = 0, MICSTATE_CHANGE };

struct CMicStateChangeEvent {
    bool mute;
};

struct CAudioStreamDeviceChangeInfo {
    int32_t changeReason;
    CArrDeviceDescriptor deviceDescriptors;
};

struct CAudioRendererInfo {
    int32_t usage;
    int32_t rendererFlags;
};

struct CAudioRendererOptions {
    CAudioRendererInfo audioRendererInfo;
    CAudioStreamInfo audioStreamInfo;
    int32_t privacyType;
};

struct CAudioRendererChangeInfo {
    CAudioRendererInfo rendererInfo;
    CArrDeviceDescriptor deviceDescriptors;
    int32_t streamId;
};

struct CArrAudioRendererChangeInfo {
    CAudioRendererChangeInfo* head;
    int64_t size;
};

enum AudioRendererCallbackType : int32_t {
    AR_AUDIO_INTERRUPT = 0,
    AR_MARK_REACH,
    AR_PERIOD_REACH,
    AR_STATE_CHANGE,
    AR_OUTPUT_DEVICE_CHANGE,
    AR_OUTPUT_DEVICE_CHANGE_WITH_INFO,
    AR_WRITE_DATA
};

struct CAudioSessionDeactiveEvent {
    int32_t deactiveReason;
};

struct CAudioSessionStrategy {
    int32_t concurrencyMode;
};
}
} // namespace AudioStandard
} // namespace OHOS
#endif // MULTIMEDIA_AUDIO_FFI_H
