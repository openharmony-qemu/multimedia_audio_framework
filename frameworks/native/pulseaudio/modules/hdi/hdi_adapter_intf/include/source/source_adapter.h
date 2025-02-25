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

#ifndef SOURCE_ADAPTER_H
#define SOURCE_ADAPTER_H

#include <stdio.h>
#include <stdint.h>
#include "intf_def.h"

#ifdef __cplusplus
extern "C" {
#endif

int32_t InitSourceAdapter(struct SourceAdapter *adapter, const char *deviceClass, const int32_t sourceType,
    const char *info);
void DeInitSourceAdapter(struct SourceAdapter *adapter);

int32_t SourceAdapterInit(struct SourceAdapter *adapter, const struct SourceAdapterAttr *attr);
void SourceAdapterDeInit(struct SourceAdapter *adapter);

int32_t SourceAdapterStart(struct SourceAdapter *adapter);
int32_t SourceAdapterStop(struct SourceAdapter *adapter);
int32_t SourceAdapterCaptureFrame(struct SourceAdapter *adapter, char *frame, uint64_t requestBytes,
    uint64_t *replyBytes);
int32_t SourceAdapterCaptureFrameWithEc(struct SourceAdapter *adapter, struct SourceAdapterFrameDesc *fdesc,
    uint64_t *replyBytes, struct SourceAdapterFrameDesc *fdescEc, uint64_t *replyBytesEc);

int32_t SourceAdapterSetVolume(struct SourceAdapter *adapter, float left, float right);
int32_t SourceAdapterGetVolume(struct SourceAdapter *adapter, float *left, float *right);
int32_t SourceAdapterSetMute(struct SourceAdapter *adapter, bool isMute);
bool SourceAdapterGetMute(struct SourceAdapter *adapter);

int32_t SourceAdapterUpdateAppsUid(struct SourceAdapter *adapter, const int32_t appsUid[PA_MAX_OUTPUTS_PER_SOURCE],
    const size_t size);

#ifdef __cplusplus
}
#endif
#endif // SOURCE_ADAPTER_H
