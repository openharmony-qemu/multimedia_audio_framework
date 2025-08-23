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
namespace OHOS {
namespace AudioStandard {
namespace HPAE {
constexpr uint32_t MAX_CHANNELS = 16;

constexpr float COEF_ZERO_F = 0.0f;
constexpr float COEF_0DB_F = 1.0f;
constexpr float COEF_M3DB_F = 0.7071f;
constexpr float COEF_M6DB_F = 0.5f;
constexpr float COEF_M435DB_F = 0.6057f;
constexpr float COEF_M45DB_F = 0.5946f;
constexpr float COEF_M9DB_F = 0.3544f;
constexpr float COEF_M899DB_F = 0.3552f;
constexpr float COEF_M12DB_F = 0.2509f;

enum {
    DMIX_ERR_SUCCESS = 0,
    DMIX_ERR_ALLOC_FAILED = -1,
    DMIX_ERR_INVALID_ARG = -2
};

class DownMixer {
public:
    DownMixer();
    int32_t Process(uint32_t framesize, float* in, uint32_t inLen, float* out, uint32_t outLen);
    
    int32_t SetParam(AudioChannelInfo inChannelInfo_, AudioChannelInfo outChannelInfo_,
        uint32_t formatSize, bool mixLfe);
    void Reset();
    static AudioChannelLayout SetDefaultChannelLayout(AudioChannel channels);
private:
    AudioChannelLayout inLayout_ = CH_LAYOUT_UNKNOWN;
    uint32_t inChannels_ = 0;
    AudioChannelLayout outLayout_ = CH_LAYOUT_UNKNOWN;
    uint32_t outChannels_ = 0;
    uint32_t formatSize_ = INVALID_WIDTH; // work format, for now only supports float
    bool isInLayoutHOA_ = false;
    std::vector<std::vector<float>> downMixTable_;
    bool mixLfe_ = true;
    bool isInitialized_ = false;

    uint32_t gSl_ = 6;
    uint32_t gSr_ = 7;
    uint32_t gTfl_ = 8;
    uint32_t gTfr_ = 9;
    uint32_t gTbl_ = 10;
    uint32_t gTbr_ = 11;
    uint32_t gTsl_ = 12;
    uint32_t gTsr_ = 13;
    bool CheckIsHOA(AudioChannelLayout layout);
    void SetupStereoDmixTable();
    void Setup5Point1DmixTable();
    void Setup5Point1Point2DmixTable();
    void Setup5Point1Point4DmixTable();
    void Setup7Point1DmixTable();
    void Setup7Point1Point2DmixTable();
    void Setup7Point1Point4DmixTable();
    void SetupGeneralDmixTable();
    void ResetSelf();
    int32_t SetupDownMixTable();
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
    /**** helper functions for setting up general downmix table ***/
    void DownMixBottom(uint64_t inBit, uint64_t outBit, uint32_t i, uint32_t j);
    void DownMixLfe(uint64_t inBit, uint64_t outBit, uint32_t i, uint32_t j);
    void DownMixMidFront(uint64_t inBit, uint64_t outBit, uint32_t i, uint32_t j);
    void DownMixMidRear(uint64_t inBit, uint64_t outBit, uint32_t i, uint32_t j);
    void DownMixTopCenter(uint64_t inBit, uint64_t outBit, uint32_t i, uint32_t j);
    void DownMixTopFront(uint64_t inBit, uint64_t outBit, uint32_t i, uint32_t j);
    void DownMixTopRear(uint64_t inBit, uint64_t outBit, uint32_t i, uint32_t j);
    void NormalizeDMixTable();
};
} // HPAE
} // AudioStandard
} // OHOS
#endif