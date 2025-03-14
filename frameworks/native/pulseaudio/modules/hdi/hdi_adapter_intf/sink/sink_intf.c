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

#ifndef LOG_TAG
#define LOG_TAG "SinkInterface"
#endif

#include "sink/sink_intf.h"
#include <stdlib.h>
#include "audio_hdi_log.h"
#include "common/hdi_adapter_info.h"
#include "sink/sink_adapter.h"

#ifdef __cplusplus
extern "C" {
#endif

static const char *g_deviceClassMap[CLASS_TYPE_NUM] = { "primary", "a2dp", "file_io", "remote", "usb", "offload",
    "multichannel", "dp" };

static void FillAdapterFuncPtr(struct SinkAdapter *adapter)
{
    CHECK_AND_RETURN_LOG(adapter != NULL, "adapter is nullptr");

    adapter->SinkAdapterInit = SinkAdapterInit;
    adapter->SinkAdapterDeInit = SinkAdapterDeInit;

    adapter->SinkAdapterStart = SinkAdapterStart;
    adapter->SinkAdapterStop = SinkAdapterStop;
    adapter->SinkAdapterResume = SinkAdapterResume;
    adapter->SinkAdapterPause = SinkAdapterPause;
    adapter->SinkAdapterFlush = SinkAdapterFlush;
    adapter->SinkAdapterReset = SinkAdapterReset;
    adapter->SinkAdapterRenderFrame = SinkAdapterRenderFrame;

    adapter->SinkAdapterSetVolume = SinkAdapterSetVolume;
    adapter->SinkAdapterGetVolume = SinkAdapterGetVolume;

    adapter->SinkAdapterGetLatency = SinkAdapterGetLatency;
    adapter->SinkAdapterGetPresentationPosition = SinkAdapterGetPresentationPosition;

    adapter->SinkAdapterGetAudioScene = SinkAdapterGetAudioScene;

    adapter->SinkAdapterSetPaPower = SinkAdapterSetPaPower;
    adapter->SinkAdapterSetPriPaPower = SinkAdapterSetPriPaPower;

    adapter->SinkAdapterUpdateAppsUid = SinkAdapterUpdateAppsUid;

    // offload extend function
    adapter->SinkAdapterRegistOffloadHdiCallback = SinkAdapterRegistOffloadHdiCallback;
    adapter->SinkAdapterSetBufferSize = SinkAdapterSetBufferSize;
    adapter->SinkAdapterLockOffloadRunningLock = SinkAdapterLockOffloadRunningLock;
    adapter->SinkAdapterUnLockOffloadRunningLock = SinkAdapterUnLockOffloadRunningLock;

    // remote extend function
    adapter->SinkAdapterSplitRenderFrame = SinkAdapterSplitRenderFrame;

    // primary extend function
    adapter->SinkSetDeviceConnectedFlag = SinkSetDeviceConnectedFlag;
}

struct SinkAdapter *GetSinkAdapter(const char *deviceClass, const char *info)
{
    CHECK_AND_RETURN_RET_LOG(deviceClass != NULL, NULL, "deviceClass is nullptr");
    struct SinkAdapter *adapter = (struct SinkAdapter *)calloc(1, sizeof(*adapter));
    CHECK_AND_RETURN_RET_LOG(adapter != NULL, NULL, "alloc sink adapter fail");

    AUDIO_INFO_LOG("deviceClass: %{public}s, networkId: %{public}s", deviceClass, info);
    int32_t ret = InitSinkAdapter(adapter, deviceClass, info);
    if (ret != 0) {
        AUDIO_ERR_LOG("not support, deviceClass: %{public}s, info: %{public}s", deviceClass, info);
        free(adapter);
        return NULL;
    }
    FillAdapterFuncPtr(adapter);
    return adapter;
}

void ReleaseSinkAdapter(struct SinkAdapter *sinkAdapter)
{
    CHECK_AND_RETURN_LOG(sinkAdapter != NULL, "adapter is nullptr");
    DeInitSinkAdapter(sinkAdapter);
    free(sinkAdapter);
}

const char *GetSinkDeviceClass(uint32_t classType)
{
    if (classType >= CLASS_TYPE_NUM) {
        AUDIO_ERR_LOG("invalid param, classType: %{public}u", classType);
        return NULL;
    }
    return g_deviceClassMap[classType];
}

#ifdef __cplusplus
}
#endif
