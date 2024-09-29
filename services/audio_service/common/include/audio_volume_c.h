/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef AUDIO_VOLUME_C_H
#define AUDIO_VOLUME_C_H

#include <inttypes.h>

#ifdef __cplusplus
extern "C" {
#endif

float GetCurVolume(uint32_t sessionId, const char *streamType, const char *deviceClass);

float GetPreVolume(uint32_t sessionId);

void SetPreVolume(uint32_t sessionId, float volume);

void GetStreamVolumeFade(uint32_t sessionId, float *fadeBegin, float *fadeEnd);

void SetStreamVolumeFade(uint32_t sessionId, float fadeBegin, float fadeEnd);

bool IsSameVolume(float volumeA, float volumeB);

void MonitorVolume(uint32_t sessionId, bool isOutput);

#ifdef __cplusplus
}
#endif

#endif // AUDIO_VOLUME_C_H