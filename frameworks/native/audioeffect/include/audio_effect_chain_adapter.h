/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef AUDIO_EFFECT_CHAIN_ADAPTER_H
#define AUDIO_EFFECT_CHAIN_ADAPTER_H

#include <stdio.h>
#include <stdint.h>
#include <pulse/pulseaudio.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct BufferAttr {
    float *bufIn;
    float *bufOut;
    int samplingRate;
    int numChanIn;
    int numChanOut;
    int frameLen;
    float *tempBufIn;
    float *tempBufOut;
    bool bufOutUsed;
} BufferAttr;

typedef struct SessionInfoPack {
    const uint32_t channels;
    const char *channelLayout;
    const char *sceneMode;
    const char *spatializationEnabled;
} SessionInfoPack;

int32_t EffectChainManagerProcess(char *sceneType, BufferAttr *bufferAttr);
bool EffectChainManagerExist(const char *sceneType, const char *effectMode, const char *spatializationEnabled);
int32_t EffectChainManagerCreateCb(const char *sceneType, const char *sessionID);
int32_t EffectChainManagerReleaseCb(const char *sceneType, const char *sessionID);
int32_t EffectChainManagerMultichannelUpdate(const char *sceneType);
uint32_t ConvertChLayoutToPaChMap(const uint64_t channelLayout, pa_channel_map *paMap);
int32_t EffectChainManagerAddSessionInfo(const char *sceneType, const char *sessionID, SessionInfoPack pack);
int32_t EffectChainManagerInitCb(const char *sceneType);
int32_t EffectChainManagerSetHdiParam(const char *sceneType, const char *effectMode, bool enabled);
bool EffectChainManagerCheckEffectOffload();
int32_t EffectChainManagerDeleteSessionInfo(const char *sceneType, const char *sessionID);
int32_t EffectChainManagerReturnEffectChannelInfo(const char *sceneType, uint32_t *channels, uint64_t *channelLayout);
int32_t EffectChainManagerReturnMultiChannelInfo(uint32_t *channels, uint64_t *channelLayout);
int32_t EffectChainManagerVolumeUpdate(const char *sessionID);
bool EffectChainManagerGetSpatializationEnabled();
void EffectChainManagerFlush(void);
void EffectChainManagerEffectUpdate(void);
bool EffectChainManagerSceneCheck(const char *sinkSceneType, const char *sceneType);

#ifdef __cplusplus
}
#endif
#endif // AUDIO_EFFECT_CHAIN_ADAPTER_H
