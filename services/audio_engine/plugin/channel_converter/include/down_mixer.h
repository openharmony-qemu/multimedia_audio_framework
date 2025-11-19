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
#ifndef DOWN_MIXER_H
#define DOWN_MIXER_H
#include <vector>
#include "audio_stream_info.h"
#include "mixer_utils.h"
namespace OHOS {
namespace AudioStandard {
namespace HPAE {

class DownMixer {
public:
    DownMixer();
    void GetDownMixTable(float (&coeffTable)[MAX_CHANNELS][MAX_CHANNELS]) const;
    int32_t SetParam(AudioChannelInfo inChannelInfo, AudioChannelInfo outChannelInfo,
        uint32_t formatSize, bool mixLfe);
    void SetNormalization(bool normalizing);
    void Reset();
private:
    void SetupStereoDmixTable();
    void Setup5Point1DmixTable();
    void Setup5Point1Point2DmixTable();
    void Setup5Point1Point4DmixTable();
    void Setup7Point1DmixTable();
    void Setup7Point1Point2DmixTable();
    void Setup7Point1Point4DmixTable();
    void ResetSelf();
    int32_t SetupDownMixTable();
    int32_t SetupDownMixTableInner();
    /**** helper functions for settiing up specific downmix table ***/
    void SetupStereoDmixTablePart1(uint64_t bit_t, uint32_t i);
    void SetupStereoDmixTablePart2(uint64_t bit_t, uint32_t i);
    void Setup5Point1DmixTablePart1(uint64_t bit_t, uint32_t i);
    void Setup5Point1DmixTablePart2(uint64_t bit_t, uint32_t i);
    void Setup5Point1Point2DmixTablePart1(uint64_t bit_t, uint32_t i);
    void Setup5Point1Point2DmixTablePart2(uint64_t bit_t, uint32_t i);
    void Setup5Point1Point4DmixTablePart1(uint64_t bit_t, uint32_t i);
    void Setup5Point1Point4DmixTablePart2(uint64_t bit_t, uint32_t i);
    void Setup7Point1DmixTablePart1(uint64_t bit_t, uint32_t i);
    void Setup7Point1DmixTablePart2(uint64_t bit_t, uint32_t i);
    void Setup7Point1Point2DmixTablePart1(uint64_t bit_t, uint32_t i);
    void Setup7Point1Point2DmixTablePart2(uint64_t bit_t, uint32_t i);
    void Setup7Point1Point4DmixTablePart1(uint64_t bit_t, uint32_t i);
    void Setup7Point1Point4DmixTablePart2(uint64_t bit_t, uint32_t i);
    void NormalizeDMixTable();

    AudioChannelLayout inLayout_ = CH_LAYOUT_UNKNOWN;
    uint32_t inChannels_ = 0;
    AudioChannelLayout outLayout_ = CH_LAYOUT_UNKNOWN;
    uint32_t outChannels_ = 0;
    uint32_t formatSize_ = INVALID_WIDTH; // work format, for now only supports float
    float downMixTable_[MAX_CHANNELS][MAX_CHANNELS] = {{0}};
    bool mixLfe_ = true;
    bool isInitialized_ = false;
    bool normalizing_ = true; // prevent downmix overflow, if there is a limiter this can be set to false

    uint32_t gSl_ = 6;
    uint32_t gSr_ = 7;
    uint32_t gTfl_ = 8;
    uint32_t gTfr_ = 9;
    uint32_t gTbl_ = 10;
    uint32_t gTbr_ = 11;
    uint32_t gTsl_ = 12;
    uint32_t gTsr_ = 13;
};
} // HPAE
} // AudioStandard
} // OHOS
#endif