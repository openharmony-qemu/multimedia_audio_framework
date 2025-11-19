/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#ifndef HDI_ADAPTER_INFO_H
#define HDI_ADAPTER_INFO_H

#include <stdint.h>

// id
enum HdiIdBase : uint32_t {
    HDI_ID_BASE_RENDER,
    HDI_ID_BASE_CAPTURE,
    HDI_ID_BASE_NUM,
};

enum HdiIdType : uint32_t {
    HDI_ID_TYPE_PRIMARY,
    HDI_ID_TYPE_FAST,
    HDI_ID_TYPE_REMOTE,
    HDI_ID_TYPE_REMOTE_FAST,
    HDI_ID_TYPE_REMOTE_OFFLOAD,
    HDI_ID_TYPE_FILE,
    HDI_ID_TYPE_BLUETOOTH,
    HDI_ID_TYPE_OFFLOAD,
    HDI_ID_TYPE_EAC3,
    HDI_ID_TYPE_MULTICHANNEL,
    HDI_ID_TYPE_WAKEUP,
    HDI_ID_TYPE_ACCESSORY,
    HDI_ID_TYPE_AI,
    HDI_ID_TYPE_VA,
    HDI_ID_TYPE_VIRTUAL_INJECTOR,
    HDI_ID_TYPE_NUM,
};

#define HDI_INVALID_ID 0xFFFFFFFF

#define HDI_ID_INFO_DEFAULT "default"
#define HDI_ID_INFO_DIRECT "direct"
#define HDI_ID_INFO_VOIP "voip"
#define HDI_ID_INFO_DP "dp"
#define HDI_ID_INFO_USB "usb"
#define HDI_ID_INFO_EC "ec"
#define HDI_ID_INFO_MIC_REF "mic_ref"
#define HDI_ID_INFO_MMAP "mmap"
#define HDI_ID_INFO_HEARING_AID "hearing_aid"
#define HDI_ID_INFO_ACCESSORY "accessory"
#define HDI_ID_INFO_DP_MULTICHANNEL "dp_multichannel"
#define HDI_ID_INFO_VA "va"

// device manager
enum HdiDeviceManagerType : uint32_t {
    HDI_DEVICE_MANAGER_TYPE_LOCAL,
    HDI_DEVICE_MANAGER_TYPE_BLUETOOTH,
    HDI_DEVICE_MANAGER_TYPE_REMOTE,
    HDI_DEVICE_MANAGER_TYPE_NUM,
};

// callback type
enum HdiAdapterCallbackType : uint32_t {
    HDI_CB_RENDER_STATE,
    HDI_CB_RENDER_PARAM,
    HDI_CB_CAPTURE_STATE,
    HDI_CB_CAPTURE_PARAM,
    HDI_CB_CAPTURE_WAKEUP,
    HDI_CB_TYPE_NUM,
};

enum AudioDrainType {
    AUDIO_DRAIN_EARLY_NOTIFY,
    AUDIO_DRAIN_ALL,
};

enum RenderCallbackType {
    CB_NONBLOCK_WRITE_COMPLETED = 0,
    CB_DRAIN_COMPLETED = 1,
    CB_FLUSH_COMPLETED = 2,
    CB_RENDER_FULL = 3,
    CB_ERROR_OCCUR = 4,
};

enum AudioFormatBit : uint32_t {
    PCM_8_BIT = 8,
    PCM_16_BIT = 16,
    PCM_24_BIT = 24,
    PCM_32_BIT = 32,
};

enum AudioByteSize : int32_t {
    BYTE_SIZE_SAMPLE_U8 = 1,
    BYTE_SIZE_SAMPLE_S16 = 2,
    BYTE_SIZE_SAMPLE_S24 = 3,
    BYTE_SIZE_SAMPLE_S32 = 4,
};

#define PRESTORE_INFO_AUDIO_BALANCE "audio_balance"
#define PRESTORE_INFO_AUDIO_MONO "audio_mono"
#define PRESTORE_INFO_AUDIO_BT_PARAM "audio_bt_param"

#define TIMEOUT_SECONDS_10 10
#define TIMEOUT_SECONDS_5 5
#define MAX_MIX_CHANNELS 32
#define PA_MAX_OUTPUTS_PER_SOURCE 256
#define SECOND_TO_MILLISECOND 1000
#define SECOND_TO_MICROSECOND 1000000
#define SECOND_TO_NANOSECOND 1000000000
#define MICROSECOND_PER_MILLISECOND 1000
#define NANOSECOND_TO_MICROSECOND 1000
#define BIT_IN_BYTE 8

#endif // HDI_ADAPTER_INFO_H
