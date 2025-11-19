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

#ifndef SINK_ADAPTER_H
#define SINK_ADAPTER_H

#include <stdio.h>
#include <stdint.h>
#include "intf_def.h"

#ifdef __cplusplus
extern "C" {
#endif

int32_t InitSinkAdapter(struct SinkAdapter *adapter, const char *deviceClass, const char *info);
void DeInitSinkAdapter(struct SinkAdapter *adapter);

int32_t SinkAdapterInit(struct SinkAdapter *adapter, const struct SinkAdapterAttr *attr);
void SinkAdapterDeInit(struct SinkAdapter *adapter);

int32_t SinkAdapterStart(struct SinkAdapter *adapter);
int32_t SinkAdapterStop(struct SinkAdapter *adapter);
int32_t SinkAdapterResume(struct SinkAdapter *adapter);
int32_t SinkAdapterPause(struct SinkAdapter *adapter);
int32_t SinkAdapterFlush(struct SinkAdapter *adapter);
int32_t SinkAdapterReset(struct SinkAdapter *adapter);
int32_t SinkAdapterRenderFrame(struct SinkAdapter *adapter, char *data, uint64_t len, uint64_t *writeLen);

int32_t SinkAdapterSetVolume(struct SinkAdapter *adapter, float left, float right);
int32_t SinkAdapterGetVolume(struct SinkAdapter *adapter, float *left, float *right);

int32_t SinkAdapterGetLatency(struct SinkAdapter *adapter, uint32_t *latency);
int32_t SinkAdapterGetPresentationPosition(struct SinkAdapter *adapter, uint64_t *frames, int64_t *timeSec,
    int64_t *timeNanoSec);

int32_t SinkAdapterGetAudioScene(struct SinkAdapter *adapter);

int32_t SinkAdapterSetPaPower(struct SinkAdapter *adapter, int32_t flag);
int32_t SinkAdapterSetPriPaPower(struct SinkAdapter *adapter);

int32_t SinkAdapterUpdateAppsUid(struct SinkAdapter *adapter, const int32_t appsUid[MAX_MIX_CHANNELS],
    const size_t size);

// offload extend function
int32_t SinkAdapterRegistOffloadHdiCallback(struct SinkAdapter *adapter, int8_t *rawCallback, int8_t *userdata);
int32_t SinkAdapterSetBufferSize(struct SinkAdapter *adapter, uint32_t sizeMs);
int32_t SinkAdapterLockOffloadRunningLock(struct SinkAdapter *adapter);
int32_t SinkAdapterUnLockOffloadRunningLock(struct SinkAdapter *adapter);

// remote extend function
int32_t SinkAdapterSplitRenderFrame(struct SinkAdapter *adapter, char *data, uint64_t len, uint64_t *writeLen,
    uint32_t splitStreamType);
int32_t SinkSetDeviceConnectedFlag(struct SinkAdapter *adapter, bool flag);

#ifdef __cplusplus
}
#endif
#endif // SINK_ADAPTER_H
