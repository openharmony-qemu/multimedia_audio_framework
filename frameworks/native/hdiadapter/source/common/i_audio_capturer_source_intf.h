/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#ifndef I_AUDIO_CAPTURER_SINK_INTF_H
#define I_AUDIO_CAPTURER_SINK_INTF_H

#include <stdbool.h>
#include "audio_hdiadapter_info.h"

#ifdef __cplusplus
extern "C" {
#endif

// should be same with IAudioSourceAttr in i_audio_capturer_source.h
typedef struct {
    const char *adapterName;
    uint32_t openMicSpeaker;
    enum HdiAdapterFormat format;
    uint32_t sampleRate;
    uint32_t channel;
    float volume;
    uint32_t bufferSize;
    bool isBigEndian;
    const char *filePath;
    const char *deviceNetworkId;
    int32_t deviceType;
    int32_t sourceType;
    uint64_t channelLayout;
} SourceAttr;

struct CapturerSourceAdapter {
    int32_t deviceClass;
    void *wapper;
    int32_t (*CapturerSourceInit)(void *wapper, const SourceAttr *attr);
    void (*CapturerSourceDeInit)(void *wapper);
    int32_t (*CapturerSourceStart)(void *wapper);
    int32_t (*CapturerSourceSetMute)(void *wapper, bool isMute);
    bool (*CapturerSourceIsMuteRequired)(void *wapper);
    int32_t (*CapturerSourceStop)(void *wapper);
    int32_t (*CapturerSourceFrame)(void *wapper, char *frame, uint64_t requestBytes, uint64_t *replyBytes);
    int32_t (*CapturerSourceSetVolume)(void *wapper, float left, float right);
    int32_t (*CapturerSourceGetVolume)(void *wapper, float *left, float *right);
    int32_t (*CapturerSourceAppsUid) (void *wapper, const int32_t appsUid[PA_MAX_OUTPUTS_PER_SOURCE],
        const size_t size);
};

int32_t FillinSourceWapper(const char *deviceClass, const char *deviceNetworkId,
    const int32_t sourceType, const char *sourceName, void **wapper);
int32_t IAudioCapturerSourceInit(void *wapper, const SourceAttr *attr);
void IAudioCapturerSourceDeInit(void *wapper);
int32_t IAudioCapturerSourceStart(void *wapper);
int32_t IAudioCapturerSourceStop(void *wapper);
int32_t IAudioCapturerSourceFrame(void *wapper, char *frame, uint64_t requestBytes, uint64_t *replyBytes);
int32_t IAudioCapturerSourceSetVolume(void *wapper, float left, float right);
bool IAudioCapturerSourceIsMuteRequired(void *wapper);
int32_t IAudioCapturerSourceSetMute(void *wapper, bool isMute);
int32_t IAudioCapturerSourceGetVolume(void *wapper, float *left, float *right);
int32_t IAudioCapturerSourceUpdateAppsUid(void *wapper, const int32_t appsUid[MAX_MIX_CHANNELS],
    const size_t size);

#ifdef __cplusplus
}
#endif

#endif  // AUDIO_CAPTURER_SINK_INTF_H
