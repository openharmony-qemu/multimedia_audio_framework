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
#define LOG_TAG "SourceInterface"
#endif

#include "source/source_intf.h"
#include <stdlib.h>
#include "audio_hdi_log.h"
#include "common/hdi_adapter_info.h"
#include "source/source_adapter.h"

#ifdef __cplusplus
extern "C" {
#endif

static const char *g_deviceClassMap[CLASS_TYPE_NUM] = { "primary", "a2dp", "file_io", "remote", "usb", NULL,
    NULL, NULL };

static void FillAdapterFuncPtr(struct SourceAdapter *adapter)
{
    CHECK_AND_RETURN_LOG(adapter != NULL, "adapter is nullptr");

    adapter->SourceAdapterInit = SourceAdapterInit;
    adapter->SourceAdapterDeInit = SourceAdapterDeInit;

    adapter->SourceAdapterStart = SourceAdapterStart;
    adapter->SourceAdapterStop = SourceAdapterStop;
    adapter->SourceAdapterCaptureFrame = SourceAdapterCaptureFrame;
    adapter->SourceAdapterCaptureFrameWithEc = SourceAdapterCaptureFrameWithEc;

    adapter->SourceAdapterSetVolume = SourceAdapterSetVolume;
    adapter->SourceAdapterGetVolume = SourceAdapterGetVolume;
    adapter->SourceAdapterSetMute = SourceAdapterSetMute;
    adapter->SourceAdapterGetMute = SourceAdapterGetMute;

    adapter->SourceAdapterUpdateAppsUid = SourceAdapterUpdateAppsUid;
}

struct SourceAdapter *GetSourceAdapter(const char *deviceClass, const int32_t sourceType, const char *info)
{
    CHECK_AND_RETURN_RET_LOG(deviceClass != NULL, NULL, "deviceClass is nullptr");
    struct SourceAdapter *adapter = (struct SourceAdapter *)calloc(1, sizeof(*adapter));
    CHECK_AND_RETURN_RET_LOG(adapter != NULL, NULL, "alloc source adapter fail");

    AUDIO_INFO_LOG("deviceClass: %{public}s, sourceType: %{public}d, networkId: %{public}s", deviceClass, sourceType,
        info);
    int32_t ret = InitSourceAdapter(adapter, deviceClass, sourceType, info);
    if (ret != 0) {
        AUDIO_ERR_LOG("not support, deviceClass: %{public}s, info: %{public}s", deviceClass, info);
        free(adapter);
        return NULL;
    }
    FillAdapterFuncPtr(adapter);
    return adapter;
}

void ReleaseSourceAdapter(struct SourceAdapter *sourceAdapter)
{
    CHECK_AND_RETURN_LOG(sourceAdapter != NULL, "adapter is nullptr");
    DeInitSourceAdapter(sourceAdapter);
    free(sourceAdapter);
}

const char *GetSourceDeviceClass(uint32_t classType)
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