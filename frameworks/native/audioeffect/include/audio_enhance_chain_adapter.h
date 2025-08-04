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
#ifdef SUPPORT_OLD_ENGINE
#include <stdio.h>
#include <stdint.h>
#include <pulse/pulseaudio.h>
#include <pulse/sample.h>

#ifdef __cplusplus
extern "C" {
#endif

struct DeviceAttrAdapter {
    uint32_t micRate;
    uint32_t micChannels;
    uint32_t micFormat;
    bool needEc;
    uint32_t ecRate;
    uint32_t ecChannels;
    uint32_t ecFormat;
    bool needMicRef;
    uint32_t micRefRate;
    uint32_t micRefChannels;
    uint32_t micRefFormat;
};

int32_t EnhanceChainManagerCreateCb(const uint64_t sceneKeyCode, const struct DeviceAttrAdapter *adapter);
int32_t EnhanceChainManagerReleaseCb(const uint64_t sceneKeyCode);
bool EnhanceChainManagerExist(const uint64_t sceneKeyCode);
int32_t EnhanceChainManagerGetAlgoConfig(const uint64_t sceneKeyCode, pa_sample_spec *micSpec,
    pa_sample_spec *ecSpec, pa_sample_spec *micRefSpec);
bool EnhanceChainManagerIsEmptyEnhanceChain(void);
int32_t EnhanceChainManagerInitEnhanceBuffer(void);
int32_t CopyToEnhanceBufferAdapter(void *data, uint32_t length);
int32_t CopyEcdataToEnhanceBufferAdapter(void *data, uint32_t length);
int32_t CopyMicRefdataToEnhanceBufferAdapter(void *data, uint32_t length);
int32_t CopyFromEnhanceBufferAdapter(void *data, uint32_t length);
int32_t EnhanceChainManagerProcess(const uint64_t sceneKeyCode, uint32_t length);
int32_t GetSceneTypeCode(const char *sceneType, uint64_t *sceneTypeCode);
int32_t EnhanceChainManagerProcessDefault(const uint32_t captureId, uint32_t length);
int32_t EnhanceChainManagerSendInitCommand(void);

#ifdef __cplusplus
}
#endif
#endif // SUPPORT_OLD_ENGINE
#endif // AUDIO_ENHANCE_CHAIN_ADAPTER_H
