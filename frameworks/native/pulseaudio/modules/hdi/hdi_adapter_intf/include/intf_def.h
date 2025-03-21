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

#ifndef INTF_DEF_H
#define INTF_DEF_H

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "audio_hdi_log.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_MIX_CHANNELS 32
#define PA_MAX_OUTPUTS_PER_SOURCE 256

// device class
enum ClassType : uint32_t {
    CLASS_TYPE_PRIMARY = 0,
    CLASS_TYPE_A2DP,
    CLASS_TYPE_FILE,
    CLASS_TYPE_REMOTE,
    CLASS_TYPE_USB,
    CLASS_TYPE_OFFLOAD,
    CLASS_TYPE_MULTICHANNEL,
    CLASS_TYPE_DP,
    CLASS_TYPE_NUM,
};

// same with AudioSampleFormat in audio_info.h
enum AudioSampleFormatIntf {
    SAMPLE_U8 = 0,
    SAMPLE_S16 = 1,
    SAMPLE_S24 = 2,
    SAMPLE_S32 = 3,
    SAMPLE_F32 = 4,
    INVALID_WIDTH = -1,
};

struct SinkAdapterAttr {
    const char *adapterName;
    uint32_t openMicSpeaker;
    int32_t format;
    uint32_t sampleRate;
    uint32_t channel;
    float volume;
    const char *filePath;
    const char *deviceNetworkId;
    int32_t deviceType;
    uint64_t channelLayout;
    const char *aux;
};

struct SinkAdapter {
    uint32_t renderId;
    const char *deviceClass;

    int32_t (*SinkAdapterInit)(struct SinkAdapter *adapter, const struct SinkAdapterAttr *attr);
    void (*SinkAdapterDeInit)(struct SinkAdapter *adapter);
    int32_t (*SinkAdapterStart)(struct SinkAdapter *adapter);
    int32_t (*SinkAdapterStop)(struct SinkAdapter *adapter);
    int32_t (*SinkAdapterResume)(struct SinkAdapter *adapter);
    int32_t (*SinkAdapterPause)(struct SinkAdapter *adapter);
    int32_t (*SinkAdapterFlush)(struct SinkAdapter *adapter);
    int32_t (*SinkAdapterReset)(struct SinkAdapter *adapter);
    int32_t (*SinkAdapterRenderFrame)(struct SinkAdapter *adapter, char *data, uint64_t len, uint64_t *writeLen);

    int32_t (*SinkAdapterSetVolume)(struct SinkAdapter *adapter, float left, float right);
    int32_t (*SinkAdapterGetVolume)(struct SinkAdapter *adapter, float *left, float *right);

    int32_t (*SinkAdapterGetLatency)(struct SinkAdapter *adapter, uint32_t *latency);
    int32_t (*SinkAdapterGetPresentationPosition)(struct SinkAdapter *adapter, uint64_t *frames, int64_t *timeSec,
        int64_t *timeNanoSec);

    int32_t (*SinkAdapterGetAudioScene)(struct SinkAdapter *adapter);

    int32_t (*SinkAdapterSetPaPower)(struct SinkAdapter *adapter, int32_t flag);
    int32_t (*SinkAdapterSetPriPaPower)(struct SinkAdapter *adapter);

    int32_t (*SinkAdapterUpdateAppsUid)(struct SinkAdapter *adapter, const int32_t appsUid[MAX_MIX_CHANNELS],
        const size_t size);

    // offload extend function
    int32_t (*SinkAdapterRegistOffloadHdiCallback)(struct SinkAdapter *adapter, int8_t *rawCallback, int8_t *userdata);
    int32_t (*SinkAdapterSetBufferSize)(struct SinkAdapter *adapter, uint32_t sizeMs);
    int32_t (*SinkAdapterLockOffloadRunningLock)(struct SinkAdapter *adapter);
    int32_t (*SinkAdapterUnLockOffloadRunningLock)(struct SinkAdapter *adapter);

    // remote extend function
    int32_t (*SinkAdapterSplitRenderFrame)(struct SinkAdapter *adapter, char *data, uint64_t len, uint64_t *writeLen,
        const char *streamType);

     // primary extend function
    int32_t (*SinkSetDeviceConnectedFlag)(struct SinkAdapter *adapter, bool flag);
};

struct SourceAdapterAttr {
    const char *adapterName;
    uint32_t openMicSpeaker;
    int32_t format;
    uint32_t sampleRate;
    uint32_t channel;
    float volume;
    uint32_t bufferSize;
    bool isBigEndian;
    const char *filePath;
    const char *deviceNetworkId;
    int32_t deviceType;
    int32_t sourceType;
    uint64_t channelLayout;
    bool hasEcConfig;
    int32_t formatEc;
    uint32_t sampleRateEc;
    uint32_t channelEc;
};

struct SourceAdapterFrameDesc {
    char *frame;
    uint64_t frameLen;
};

struct SourceAdapter {
    uint32_t captureId;
    const char *deviceClass;
    struct SourceAdapterAttr *attr;

    int32_t (*SourceAdapterInit)(struct SourceAdapter *adapter, const struct SourceAdapterAttr *attr);
    void (*SourceAdapterDeInit)(struct SourceAdapter *adapter);

    int32_t (*SourceAdapterStart)(struct SourceAdapter *adapter);
    int32_t (*SourceAdapterStop)(struct SourceAdapter *adapter);
    int32_t (*SourceAdapterCaptureFrame)(struct SourceAdapter *adapter, char *frame, uint64_t requestBytes,
        uint64_t *replyBytes);
    int32_t (*SourceAdapterCaptureFrameWithEc)(struct SourceAdapter *adapter, struct SourceAdapterFrameDesc *fdesc,
        uint64_t *replyBytes, struct SourceAdapterFrameDesc *fdescEc, uint64_t *replyBytesEc);

    int32_t (*SourceAdapterSetVolume)(struct SourceAdapter *adapter, float left, float right);
    int32_t (*SourceAdapterGetVolume)(struct SourceAdapter *adapter, float *left, float *right);
    int32_t (*SourceAdapterSetMute)(struct SourceAdapter *adapter, bool isMute);
    bool (*SourceAdapterGetMute)(struct SourceAdapter *adapter);

    int32_t (*SourceAdapterUpdateAppsUid)(struct SourceAdapter *adapter,
        const int32_t appsUid[PA_MAX_OUTPUTS_PER_SOURCE], const size_t size);
};

typedef enum EcType {
    EC_NONE = 0,
    EC_SAME_ADAPTER,
    EC_DIFFERENT_ADAPTER
} EcType;

typedef enum MicRefSwitch {
    REF_OFF = 0,
    REF_ON
} MicRefSwitch;

#ifdef __cplusplus
}
#endif
#endif // INTF_DEF_H
