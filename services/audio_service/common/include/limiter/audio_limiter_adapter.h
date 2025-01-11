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

#ifndef AUDIO_LIMITER_ADAPTER_H
#define AUDIO_LIMITER_ADAPTER_H

#include <stdio.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

int32_t LimiterManagerCreate(int32_t sinkIndex);
int32_t LimiterManagerSetConfig(int32_t sinkIndex, int32_t maxRequest, int32_t biteSize,
    int32_t sampleRate, int32_t channels);
int32_t LimiterManagerProcess(int32_t sinkIndex, int32_t frameLen, float *inBuffer, float *outBuffer);
int32_t LimiterManagerRelease(int32_t sinkIndex);
#ifdef __cplusplus
}
#endif
#endif // AUDIO_LIMITER_ADAPTER_H
