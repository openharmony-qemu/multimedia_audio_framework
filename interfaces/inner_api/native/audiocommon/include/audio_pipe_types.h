/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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
#ifndef AUDIO_PIPE_TYPES_H
#define AUDIO_PIPE_TYPES_H

#include <cstdint>

namespace OHOS {
namespace AudioStandard {

constexpr uint32_t PIPE_ID_INVALID = 0;

enum AudioPipeRole : int32_t {
    PIPE_ROLE_OUTPUT = 0,
    PIPE_ROLE_INPUT,
    PIPE_ROLE_NONE,
};

enum AudioPipeStatus : int32_t {
    PIPE_STATUS_OPEN = 0,
    PIPE_STATUS_CLOSE,
    PIPE_STATUS_RUNNING,
    PIPE_STATUS_STANDBY, // stop status
};

enum HdiAdapterType : uint32_t {
    HDI_ADAPTER_TYPE_UNKNOWN = 0,
    HDI_ADAPTER_TYPE_PRIMARY,
    HDI_ADAPTER_TYPE_A2DP,
    HDI_ADAPTER_TYPE_USB,
    HDI_ADAPTER_TYPE_DP,
    HDI_ADAPTER_TYPE_REMOTE,
    HDI_ADAPTER_TYPE_HEARING_AID,
    HDI_ADAPTER_TYPE_ACCESSORY,
    HDI_ADAPTER_TYPE_SLE,
    HDI_ADAPTER_TYPE_VA,
};

} // namespace AudioStandard
} // namespace OHOS

enum AudioFlag : uint32_t {
    AUDIO_FLAG_NONE = 0x0,
    AUDIO_OUTPUT_FLAG_NORMAL = 0x1,
    AUDIO_OUTPUT_FLAG_DIRECT = 0x2,
    AUDIO_OUTPUT_FLAG_HD = 0x4,
    AUDIO_OUTPUT_FLAG_MULTICHANNEL = 0x8,
    AUDIO_OUTPUT_FLAG_LOWPOWER = 0x10,
    AUDIO_OUTPUT_FLAG_FAST = 0x20,
    AUDIO_OUTPUT_FLAG_VOIP = 0x40,
    AUDIO_OUTPUT_FLAG_VOIP_FAST = 0x80,
    AUDIO_OUTPUT_FLAG_HWDECODING = 0x100,
    AUDIO_OUTPUT_FLAG_COMPRESS_OFFLOAD = 0x200,
    AUDIO_OUTPUT_FLAG_MODEM_COMMUNICATION = 0x400,
    AUDIO_OUTPUT_FLAG_INTERPHONE = 0x1000000,
    AUDIO_OUTPUT_FLAG_3DA_DIRECT = 0x800,
    AUDIO_INPUT_FLAG_NORMAL = 0x1000,
    AUDIO_INPUT_FLAG_FAST = 0x2000,
    AUDIO_INPUT_FLAG_VOIP = 0x4000,
    AUDIO_INPUT_FLAG_VOIP_FAST = 0x8000,
    AUDIO_INPUT_FLAG_WAKEUP = 0x10000,
    AUDIO_INPUT_FLAG_AI = 0x20000,
    AUDIO_INPUT_FLAG_UNPROCESS = 0x40000,
    AUDIO_INPUT_FLAG_ULTRASONIC = 0x80000,
    AUDIO_INPUT_FLAG_VOICE_RECOGNITION = 0x100000,
    AUDIO_INPUT_FLAG_RAW_AI = 0x200000,
    AUDIO_INPUT_FLAG_OFFLOAD = 0x400000,
    AUDIO_INPUT_FLAG_INTERPHONE = 0x800000,
    AUDIO_INPUT_FLAG_LIVE = 0x2000000,
    AUDIO_INPUT_FLAG_CAMCORDER = 0x8000000,
    AUDIO_FLAG_MAX,
};

#endif // AUDIO_PIPE_TYPES_H
