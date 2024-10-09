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
#define LOG_TAG "CapturerSourceAdapter"
#endif

#include "capturer_source_adapter.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "audio_hdi_log.h"
#include "i_audio_capturer_source_intf.h"

#ifdef __cplusplus
extern "C" {
#endif

const int32_t SUCCESS = 0;
const int32_t ERROR = -1;

const int32_t CLASS_TYPE_PRIMARY = 0;
const int32_t CLASS_TYPE_A2DP = 1;
const int32_t CLASS_TYPE_FILE = 2;
const int32_t CLASS_TYPE_REMOTE = 3;
const int32_t CLASS_TYPE_USB = 4;

const char *DEVICE_CLASS_PRIMARY = "primary";
const char *DEVICE_CLASS_USB = "usb";
const char *DEVICE_CLASS_A2DP = "a2dp";
const char *DEVICE_CLASS_FILE = "file_io";
const char *DEVICE_CLASS_REMOTE = "remote";

int32_t LoadSourceAdapter(const char *device, const char *deviceNetworkId, const int32_t sourceType,
    const char *sourceName, struct CapturerSourceAdapter **sourceAdapter)
{
    AUDIO_INFO_LOG("sourceType: %{public}d  device: %{public}s", sourceType, device);
    CHECK_AND_RETURN_RET_LOG((device != NULL) && (deviceNetworkId != NULL) && (sourceAdapter != NULL),
        ERROR, "Invalid parameter");

    struct CapturerSourceAdapter *adapter = (struct CapturerSourceAdapter *)calloc(1, sizeof(*adapter));
    CHECK_AND_RETURN_RET_LOG((adapter != NULL), ERROR, "alloc sink adapter failed");

    if (FillinSourceWapper(device, deviceNetworkId, sourceType, sourceName, &adapter->wapper) != SUCCESS) {
        AUDIO_ERR_LOG(" Device not supported");
        free(adapter);
        return ERROR;
    }
    // fill deviceClass for hdi_source.c
    if (!strcmp(device, DEVICE_CLASS_PRIMARY)) {
        adapter->deviceClass = CLASS_TYPE_PRIMARY;
    }
    if (!strcmp(device, DEVICE_CLASS_USB)) {
        adapter->deviceClass = CLASS_TYPE_USB;
    }
    if (!strcmp(device, DEVICE_CLASS_A2DP)) {
        adapter->deviceClass = CLASS_TYPE_A2DP;
    }
    if (!strcmp(device, DEVICE_CLASS_FILE)) {
        adapter->deviceClass = CLASS_TYPE_FILE;
    }
    if (!strcmp(device, DEVICE_CLASS_REMOTE)) {
        adapter->deviceClass = CLASS_TYPE_REMOTE;
    }
    adapter->CapturerSourceInit = IAudioCapturerSourceInit;
    adapter->CapturerSourceDeInit = IAudioCapturerSourceDeInit;
    adapter->CapturerSourceStart = IAudioCapturerSourceStart;
    adapter->CapturerSourceStop = IAudioCapturerSourceStop;
    adapter->CapturerSourceSetMute = IAudioCapturerSourceSetMute;
    adapter->CapturerSourceIsMuteRequired = IAudioCapturerSourceIsMuteRequired;
    adapter->CapturerSourceFrame = IAudioCapturerSourceFrame;
    adapter->CapturerSourceSetVolume = IAudioCapturerSourceSetVolume;
    adapter->CapturerSourceGetVolume = IAudioCapturerSourceGetVolume;
    adapter->CapturerSourceAppsUid = IAudioCapturerSourceUpdateAppsUid;

    *sourceAdapter = adapter;

    return SUCCESS;
}

int32_t UnLoadSourceAdapter(struct CapturerSourceAdapter *sourceAdapter)
{
    CHECK_AND_RETURN_RET_LOG(sourceAdapter != NULL, ERROR, "Invalid parameter");

    free(sourceAdapter);

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
    } else {
        return "";
    }
}
#ifdef __cplusplus
}
#endif
