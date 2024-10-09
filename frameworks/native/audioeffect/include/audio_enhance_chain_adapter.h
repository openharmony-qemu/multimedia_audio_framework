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

#ifndef AUDIO_ENHANCE_CHAIN_ADAPTER_H
#define AUDIO_ENHANCE_CHAIN_ADAPTER_H

#include <stdio.h>
#include <stdint.h>
#include <pulse/pulseaudio.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct EnhanceBufferAttr {
    uint8_t *input;
    uint8_t *output;
    uint32_t refNum;
    uint32_t micNum;
    uint32_t outNum;
    uint32_t sampleRate;
    uint32_t frameLength;
    uint32_t dataFormat;
    uint32_t bitDepth;
    uint32_t batchLen;
    uint32_t byteLenPerFrame;
} EnhanceBufferAttr;

int32_t EnhanceChainManagerProcess(const char *sceneType, EnhanceBufferAttr *enhanceBufferAttr);
int32_t EnhanceChainManagerCreateCb(const char *sceneType, const char *enhanceMode, const char *upAndDownDevice);
int32_t EnhanceChainManagerReleaseCb(const char *sceneType, const char *enhanceMode, const char *upAndDownDevice);
bool EnhanceChainManagerExist(const char *sceneType);

#ifdef __cplusplus
}
#endif
#endif // AUDIO_ENHANCE_CHAIN_ADAPTER_H