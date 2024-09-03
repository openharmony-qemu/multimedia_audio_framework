/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#ifndef AUDIO_RENDERER_SINK_INTF_H
#define AUDIO_RENDERER_SINK_INTF_H

#include "audio_hdiadapter_info.h"

#ifdef __cplusplus
extern "C" {
#endif

// should be same with IAudioSinkAttr in i_audio_renderer_sink.h
typedef struct {
    const char *adapterName;
    uint32_t openMicSpeaker;
    enum HdiAdapterFormat format;
    uint32_t sampleRate;
    uint32_t channel;
    float volume;
    const char *filePath;
    const char *deviceNetworkId;
    int32_t deviceType;
    uint64_t channelLayout;
} SinkAttr;

struct RendererSinkAdapter {
    int32_t deviceClass;
    void* wapper;
    int32_t (*RendererSinkInit)(struct RendererSinkAdapter *adapter, const SinkAttr *attr);
    void (*RendererSinkDeInit)(struct RendererSinkAdapter *adapter);
    int32_t (*RendererSinkStart)(struct RendererSinkAdapter *adapter);
    int32_t (*RendererSinkPause)(struct RendererSinkAdapter *adapter);
    int32_t (*RendererSinkResume)(struct RendererSinkAdapter *adapter);
    int32_t (*RendererSinkStop)(struct RendererSinkAdapter *adapter);
    int32_t (*RendererRenderFrame)(struct RendererSinkAdapter *adapter, char *data, uint64_t len, uint64_t *writeLen);
    int32_t (*RendererSinkSetVolume)(struct RendererSinkAdapter *adapter, float left, float right);
    int32_t (*RendererSinkGetVolume)(struct RendererSinkAdapter *adapter, float *left, float *right);
    int32_t (*RendererSinkGetLatency)(struct RendererSinkAdapter *adapter, uint32_t *latency);
    int32_t (*RendererRegCallback)(struct RendererSinkAdapter *adapter, int8_t *cb, int8_t *userdata);
    int32_t (*RendererSinkGetPresentationPosition)(struct RendererSinkAdapter *adapter, uint64_t* frames,
        int64_t* timeSec, int64_t* timeNanoSec);
    int32_t (*RendererSinkFlush)(struct RendererSinkAdapter *adapter);
    int32_t (*RendererSinkReset)(struct RendererSinkAdapter *adapter);
    int32_t (*RendererSinkSetBufferSize)(struct RendererSinkAdapter *adapter, uint32_t sizeMs);
    int32_t (*RendererSinkOffloadRunningLockInit)(struct RendererSinkAdapter *adapter);
    int32_t (*RendererSinkOffloadRunningLockLock)(struct RendererSinkAdapter *adapter);
    int32_t (*RendererSinkOffloadRunningLockUnlock)(struct RendererSinkAdapter *adapter);
    int32_t (*RendererSinkSetPaPower)(struct RendererSinkAdapter *adapter, int32_t flag);
    int32_t (*RendererSinkSetPriPaPower)(struct RendererSinkAdapter *adapter);
    int32_t (*RendererSinkUpdateAppsUid) (struct RendererSinkAdapter *adapter, const int32_t appsUid[MAX_MIX_CHANNELS],
        const size_t size);
};

int32_t FillinSinkWapper(const char *device, const char *deviceNetworkId, struct RendererSinkAdapter *adapter);
int32_t IAudioRendererSinkInit(struct RendererSinkAdapter *adapter, const SinkAttr *attr);
void IAudioRendererSinkDeInit(struct RendererSinkAdapter *adapter);
int32_t IAudioRendererSinkStart(struct RendererSinkAdapter *adapter);
int32_t IAudioRendererSinkStop(struct RendererSinkAdapter *adapter);
int32_t IAudioRendererSinkPause(struct RendererSinkAdapter *adapter);
int32_t IAudioRendererSinkResume(struct RendererSinkAdapter *adapter);
int32_t IAudioRendererSinkRenderFrame(struct RendererSinkAdapter *adapter, char *data, uint64_t len,
    uint64_t *writeLen);
int32_t IAudioRendererSinkSetVolume(struct RendererSinkAdapter *adapter, float left, float right);
int32_t IAudioRendererSinkGetVolume(struct RendererSinkAdapter *adapter, float *left, float *right);
int32_t IAudioRendererSinkGetLatency(struct RendererSinkAdapter *adapter, uint32_t *latency);
int32_t IAudioRendererSinkRegCallback(struct RendererSinkAdapter *adapter, int8_t *cb, int8_t *userdata);
int32_t IAudioRendererSinkGetPresentationPosition(struct RendererSinkAdapter *adapter, uint64_t* frames,
    int64_t* timeSec, int64_t* timeNanoSec);
int32_t IAudioRendererSinkFlush(struct RendererSinkAdapter *adapter);
int32_t IAudioRendererSinkReset(struct RendererSinkAdapter *adapter);
int32_t IAudioRendererSinkSetBufferSize(struct RendererSinkAdapter *adapter, uint32_t sizeMs);
int32_t IAudioRendererSinkOffloadRunningLockInit(struct RendererSinkAdapter *adapter);
int32_t IAudioRendererSinkOffloadRunningLockLock(struct RendererSinkAdapter *adapter);
int32_t IAudioRendererSinkOffloadRunningLockUnlock(struct RendererSinkAdapter *adapter);
int32_t IAudioRendererSinkSetPaPower(struct RendererSinkAdapter *adapter, int32_t flag);
int32_t IAudioRendererSinkSetPriPaPower(struct RendererSinkAdapter *adapter);
int32_t IAudioRendererSinkUpdateAppsUid(struct RendererSinkAdapter *adapter, const int32_t appsUid[MAX_MIX_CHANNELS],
    const size_t size);

#ifdef __cplusplus
}
#endif
#endif // AUDIO_RENDERER_SINK_INTF_H
