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

#ifndef RENDERER_SINK_ADAPTER_H
#define RENDERER_SINK_ADAPTER_H

#include <stdio.h>
#include <stdint.h>

#include "i_audio_renderer_sink_intf.h"

#ifdef __cplusplus
extern "C" {
#endif
int32_t LoadSinkAdapter(const char *device, const char *deviceNetworkId, struct RendererSinkAdapter **sinkAdapter);
int32_t UnLoadSinkAdapter(struct RendererSinkAdapter *sinkAdapter);
const char *GetDeviceClass(int32_t deviceClass);
#ifdef __cplusplus
}
#endif
#endif // RENDERER_SINK_ADAPTER_H
