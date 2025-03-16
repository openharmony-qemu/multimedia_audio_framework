/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef AUDIO_HDIADAPTER_INFO_H
#define AUDIO_HDIADAPTER_INFO_H

#ifndef MAX_MIX_CHANNELS
#define MAX_MIX_CHANNELS 128
#endif
#define PA_MAX_OUTPUTS_PER_SOURCE 256

#include <inttypes.h>
#include <stdbool.h>

// should be same with AudioSampleFormat in audio_info.h
enum HdiAdapterFormat {
    SAMPLE_U8 = 0,
    SAMPLE_S16 = 1,
    SAMPLE_S24 = 2,
    SAMPLE_S32 = 3,
    SAMPLE_F32 = 4,
    INVALID_WIDTH = -1
};

enum RenderCallbackType {
    CB_NONBLOCK_WRITE_COMPLETED = 0,
    CB_DRAIN_COMPLETED = 1,
    CB_FLUSH_COMPLETED = 2,
    CB_RENDER_FULL = 3,
    CB_ERROR_OCCUR = 4,
};

#ifdef __cplusplus
extern "C" {
#endif

typedef enum EcType {
    EC_NONE = 0,
    EC_SAME_ADAPTER,
    EC_DIFFERENT_ADAPTER
} EcType;

typedef enum MicRefSwitch {
    REF_OFF = 0,
    REF_ON
} MicRefSwitch;

typedef struct FrameDesc {
    char *frame;
    uint64_t frameLen;
} FrameDesc;

typedef struct CaptureAttr {
    // usage attrs
    int32_t sourceType;
    // device attrs
    const char *adapterName;
    int32_t deviceType;
    // common audio attrs
    uint32_t sampleRate;
    uint32_t channelCount;
    enum HdiAdapterFormat format;
    bool isBigEndian;
    uint32_t openMicSpeaker;
} CaptureAttr;

#ifdef __cplusplus
}
#endif

#endif
