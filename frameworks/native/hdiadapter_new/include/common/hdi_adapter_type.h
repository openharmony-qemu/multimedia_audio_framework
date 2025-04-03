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

#ifndef HDI_ADAPTER_TYPE_H
#define HDI_ADAPTER_TYPE_H

#include <iostream>
#include <cstring>
#include "audio_info.h"

namespace OHOS {
namespace AudioStandard {
// if attr struct change, please check ipc serialize and deserialize code
typedef struct IAudioSinkAttr {
    std::string adapterName = "";
    uint32_t openMicSpeaker = 0;
    AudioSampleFormat format = AudioSampleFormat::INVALID_WIDTH;
    uint32_t sampleRate = 0;
    uint32_t channel = 0;
    float volume = 0.0f;
    const char *filePath = nullptr;
    const char *deviceNetworkId = nullptr;
    int32_t deviceType = 0;
    uint64_t channelLayout = 0;
    int32_t audioStreamFlag = 0;
    std::string address;
    const char *aux;
} IAudioSinkAttr;

typedef struct IAudioSourceAttr {
    std::string adapterName = "";
    uint32_t openMicSpeaker = 0;
    AudioSampleFormat format = AudioSampleFormat::INVALID_WIDTH;
    uint32_t sampleRate = 0;
    uint32_t channel = 0;
    float volume = 0.0f;
    uint32_t bufferSize = 0;
    bool isBigEndian = false;
    const char *filePath = nullptr;
    const char *deviceNetworkId = nullptr;
    int32_t deviceType = 0;
    int32_t sourceType = 0;
    uint64_t channelLayout = 0;
    int32_t audioStreamFlag = 0;
    bool hasEcConfig = false;
    AudioSampleFormat formatEc = AudioSampleFormat::INVALID_WIDTH;
    uint32_t sampleRateEc = 0;
    uint32_t channelEc = 0;
} IAudioSourceAttr;

typedef struct FrameDesc {
    char *frame;
    uint64_t frameLen;
} FrameDesc;

} // namespace AudioStandard
} // namespace OHOS

#endif // HDI_ADAPTER_TYPE_H
