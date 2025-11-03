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
#ifndef CHANNEL_CONVERTER_H
#define CHANNEL_CONVERTER_H

#include "down_mixer.h"
namespace OHOS {
namespace AudioStandard {
namespace HPAE {

class ChannelConverter {
public:
    ChannelConverter();
    int32_t Process(uint32_t framesize, float* in, uint32_t inLen, float* out, uint32_t outLen);
    // input and output stream format must be workFormat
    int32_t SetParam(AudioChannelInfo inChannelInfo, AudioChannelInfo outChannelInfo,
        AudioSampleFormat workFormat, bool mixLfe);
    void GetMixTable(float (&coeffTable)[MAX_CHANNELS][MAX_CHANNELS]) const;
    AudioChannelInfo GetInChannelInfo() const;
    AudioChannelInfo GetOutChannelInfo() const;
    int32_t SetInChannelInfo(AudioChannelInfo inChannelInfo);
    int32_t SetOutChannelInfo(AudioChannelInfo outChannelInfo);
    void SetDownmixNormalization(bool normalizing);
    void Reset();
private:
    int32_t MixProcess(bool isDmix, uint32_t frameLen, float* in, float* out);
    void UpmixGainAttenuation();
    DownMixer downMixer_;
    float mixTable_[MAX_CHANNELS][MAX_CHANNELS] = {{0}};
    AudioChannelInfo inChannelInfo_;
    AudioChannelInfo outChannelInfo_;
    AudioSampleFormat workFormat_ = INVALID_WIDTH;  // work format, for now only supports float
    uint32_t workSize_ = 0; // work format, for now only supports float
    bool mixLfe_ = true;
    bool isInitialized_ = false;
    bool downmixNormalizing_ = true; // by default, downmixer should do normalization to prevent overflow
};
} // HPAE
} // AudioStandard
} // OHOS
#endif