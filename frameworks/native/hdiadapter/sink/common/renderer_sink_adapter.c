/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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
#ifndef LOG_TAG
#define LOG_TAG "RendererSinkAdapter"
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "renderer_sink_adapter.h"
#include "audio_hdi_log.h"

#ifdef __cplusplus
extern "C" {
#endif

const int32_t  SUCCESS = 0;
const int32_t  ERROR = -1;

const int32_t CLASS_TYPE_PRIMARY = 0;
const int32_t CLASS_TYPE_A2DP = 1;
const int32_t CLASS_TYPE_FILE = 2;
const int32_t CLASS_TYPE_REMOTE = 3;
const int32_t CLASS_TYPE_USB = 4;
const int32_t CLASS_TYPE_OFFLOAD = 5;
const int32_t CLASS_TYPE_MULTICHANNEL = 6;
const int32_t CLASS_TYPE_DP = 7;

const char *DEVICE_CLASS_PRIMARY = "primary";
const char *DEVICE_CLASS_USB = "usb";
const char *DEVICE_CLASS_A2DP = "a2dp";
const char *DEVICE_CLASS_FILE = "file_io";
const char *DEVICE_CLASS_REMOTE = "remote";
const char *DEVICE_CLASS_OFFLOAD = "offload";
const char *DEVICE_CLASS_MULTICHANNEL = "multichannel";
const char *DEVICE_CLASS_DP = "dp";

int32_t LoadSinkAdapter(const char *device, const char *deviceNetworkId, struct RendererSinkAdapter **sinkAdapter)
{
    AUDIO_INFO_LOG("%{public}s: device:[%{public}s]", __func__, device);
    CHECK_AND_RETURN_RET_LOG((device != NULL) && (sinkAdapter != NULL), ERROR, "Invalid parameter");

    struct RendererSinkAdapter *adapter = (struct RendererSinkAdapter *)calloc(1, sizeof(*adapter));
    CHECK_AND_RETURN_RET_LOG(adapter != NULL, ERROR, "alloc sink adapter failed");

    if (FillinSinkWapper(device, deviceNetworkId, adapter) != SUCCESS) {
        AUDIO_ERR_LOG("%{public}s: Device not supported", __func__);
        free(adapter);
        return ERROR;
    }
    // fill deviceClass for hdi_sink.c
    adapter->deviceClass = !strcmp(device, DEVICE_CLASS_PRIMARY) ? CLASS_TYPE_PRIMARY : adapter->deviceClass;
    adapter->deviceClass = !strcmp(device, DEVICE_CLASS_USB) ? CLASS_TYPE_USB : adapter->deviceClass;
    adapter->deviceClass = !strcmp(device, DEVICE_CLASS_A2DP) ? CLASS_TYPE_A2DP : adapter->deviceClass;
    adapter->deviceClass = !strcmp(device, DEVICE_CLASS_FILE) ? CLASS_TYPE_FILE : adapter->deviceClass;
    adapter->deviceClass = !strcmp(device, DEVICE_CLASS_REMOTE) ? CLASS_TYPE_REMOTE : adapter->deviceClass;
    adapter->deviceClass = !strcmp(device, DEVICE_CLASS_OFFLOAD) ? CLASS_TYPE_OFFLOAD : adapter->deviceClass;
    adapter->deviceClass = !strcmp(device, DEVICE_CLASS_MULTICHANNEL) ? CLASS_TYPE_MULTICHANNEL : adapter->deviceClass;
    adapter->deviceClass = !strcmp(device, DEVICE_CLASS_DP) ? CLASS_TYPE_DP : adapter->deviceClass;

    adapter->RendererSinkInit = IAudioRendererSinkInit;
    adapter->RendererSinkDeInit = IAudioRendererSinkDeInit;
    adapter->RendererSinkStart = IAudioRendererSinkStart;
    adapter->RendererSinkStop = IAudioRendererSinkStop;
    adapter->RendererSinkPause = IAudioRendererSinkPause;
    adapter->RendererSinkResume = IAudioRendererSinkResume;
    adapter->RendererRenderFrame = IAudioRendererSinkRenderFrame;
    adapter->RendererSplitRenderFrame = IAudioRendererSinkSplitRenderFrame;
    adapter->RendererSinkSetVolume = IAudioRendererSinkSetVolume;
    adapter->RendererSinkGetVolume = IAudioRendererSinkGetVolume;
    adapter->RendererSinkGetLatency = IAudioRendererSinkGetLatency;
    adapter->RendererRegCallback = IAudioRendererSinkRegCallback;
    adapter->RendererSinkGetPresentationPosition = IAudioRendererSinkGetPresentationPosition;
    adapter->RendererSinkFlush = IAudioRendererSinkFlush;
    adapter->RendererSinkReset = IAudioRendererSinkReset;
    adapter->RendererSinkSetBufferSize = IAudioRendererSinkSetBufferSize;
    adapter->RendererSinkOffloadRunningLockInit = IAudioRendererSinkOffloadRunningLockInit;
    adapter->RendererSinkOffloadRunningLockLock = IAudioRendererSinkOffloadRunningLockLock;
    adapter->RendererSinkOffloadRunningLockUnlock = IAudioRendererSinkOffloadRunningLockUnlock;
    adapter->RendererSinkSetPaPower = IAudioRendererSinkSetPaPower;
    adapter->RendererSinkSetPriPaPower = IAudioRendererSinkSetPriPaPower;
    adapter->RendererSinkUpdateAppsUid = IAudioRendererSinkUpdateAppsUid;

    *sinkAdapter = adapter;

    return SUCCESS;
}

int32_t UnLoadSinkAdapter(struct RendererSinkAdapter *sinkAdapter)
{
    CHECK_AND_RETURN_RET_LOG(sinkAdapter != NULL, ERROR, "Invalid parameter");

    free(sinkAdapter);
    return SUCCESS;
}

const char *GetDeviceClass(int32_t deviceClass)
{
    if (deviceClass == CLASS_TYPE_PRIMARY) {
        return DEVICE_CLASS_PRIMARY;
    } else if (deviceClass == CLASS_TYPE_USB) {
        return DEVICE_CLASS_USB;
    } else if (deviceClass == CLASS_TYPE_A2DP) {
        return DEVICE_CLASS_A2DP;
    } else if (deviceClass == CLASS_TYPE_FILE) {
        return DEVICE_CLASS_FILE;
    } else if (deviceClass == CLASS_TYPE_REMOTE) {
        return DEVICE_CLASS_REMOTE;
    } else if (deviceClass == CLASS_TYPE_OFFLOAD) {
        return DEVICE_CLASS_OFFLOAD;
    } else if (deviceClass == CLASS_TYPE_MULTICHANNEL) {
        return DEVICE_CLASS_MULTICHANNEL;
    } else if (deviceClass == CLASS_TYPE_DP) {
        return DEVICE_CLASS_DP;
    } else {
        return "";
    }
}
#ifdef __cplusplus
}
#endif
