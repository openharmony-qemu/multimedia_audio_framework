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
#ifndef LOG_TAG
#define LOG_TAG "HpaeDownMixer"
#endif
#include "down_mixer.h"
#include <algorithm>
#include <cinttypes>
#include "securec.h"
#include "audio_engine_log.h"

namespace OHOS {
namespace AudioStandard {
namespace HPAE {
// Initial output channel index
enum OutChannelIndex : uint32_t {
    FL = 0,
    FR,
    FC,
    SW,
// Back channel should be placed before side channel
    BL,
    BR
};

static constexpr uint32_t INDEX_SIX = 6;
static constexpr uint32_t INDEX_SEVEN = 7;
static constexpr uint32_t INDEX_EIGHT = 8;
static constexpr uint32_t INDEX_NINE = 9;
static constexpr uint32_t INDEX_TEN = 10;
static constexpr uint32_t INDEX_ELEVEN = 11;

DownMixer::DownMixer() {}

// setParam
int32_t DownMixer::SetParam(AudioChannelInfo inChannelInfo, AudioChannelInfo outChannelInfo,
    uint32_t formatSize, bool mixLfe)
{
    ResetSelf();
    inLayout_ = inChannelInfo.channelLayout;
    outLayout_ = outChannelInfo.channelLayout;
    inChannels_ = inChannelInfo.numChannels;
    outChannels_ = outChannelInfo.numChannels;
    mixLfe_ = mixLfe;

    isInitialized_ = false;
    CHECK_AND_RETURN_RET_LOG((inChannels_ >= 0) && (inChannels_ <= MAX_CHANNELS), MIX_ERR_INVALID_ARG,
        "invalid input channels");
    CHECK_AND_RETURN_RET_LOG((outChannels_ >= 0) && (outChannels_ <= MAX_CHANNELS), MIX_ERR_INVALID_ARG,
        "invalid output channels");
    
    formatSize_ = formatSize;
    int32_t ret = SetupDownMixTable();
    isInitialized_ = (ret == MIX_ERR_SUCCESS);
    return ret;
}

int32_t DownMixer::SetupDownMixTableInner()
{
    int32_t ret = MIX_ERR_SUCCESS;
    switch (outLayout_) {
        case CH_LAYOUT_STEREO: {
            SetupStereoDmixTable();
            break;
        }
        case CH_LAYOUT_5POINT1: {
            Setup5Point1DmixTable();
            break;
        }
        case CH_LAYOUT_5POINT1POINT2: {
            Setup5Point1Point2DmixTable();
            break;
        }
        case CH_LAYOUT_5POINT1POINT4: {
            Setup5Point1Point4DmixTable();
            break;
        }
        case CH_LAYOUT_7POINT1: {
            Setup7Point1DmixTable();
            break;
        }
        case CH_LAYOUT_7POINT1POINT2: {
            Setup7Point1Point2DmixTable();
            break;
        }
        case CH_LAYOUT_7POINT1POINT4: {
            Setup7Point1Point4DmixTable();
            break;
        }
        default: {
            AudioChannelInfo inChannelInfo = {inLayout_, inChannels_};
            AudioChannelInfo outChannelInfo = {outLayout_, outChannels_};
            ret = SetUpGeneralMixingTable(downMixTable_, inChannelInfo, outChannelInfo, mixLfe_);
            break;
        }
    }
    return ret;
}

int32_t DownMixer::SetupDownMixTable()
{
    CHECK_AND_RETURN_RET_LOG(IsValidChLayout(inLayout_, inChannels_) && IsValidChLayout(outLayout_, outChannels_) &&
        (inLayout_ != outLayout_) && (inChannels_ > outChannels_), MIX_ERR_INVALID_ARG,
        "input channel count %{public}d, inLayout_ %{public}" PRIu64 " or output channel count %{public}d, "
        "outLayout_ %{public}" PRIu64 "is invalid", inChannels_, inLayout_, outChannels_, outLayout_);

    // for HOA intput, use the first channel input for every output channel
    if (CheckIsHOA(inLayout_)) {
        for (uint32_t i = 0; i < outChannels_; i++) {
            downMixTable_[i][0] = COEF_0DB_F;
        }
        return MIX_ERR_SUCCESS;
    }

    int32_t ret = SetupDownMixTableInner();
    isInitialized_ = (ret == MIX_ERR_SUCCESS);
    // no need for normalization, can return directly
    CHECK_AND_RETURN_RET_LOG(normalizing_, ret, "downmix normalization is disabled");
    // normalizing_ is true do normalization then return
    NormalizeDMixTable();
    return ret;
}

void DownMixer::SetNormalization(bool normalizing)
{
    CHECK_AND_RETURN_LOG(normalizing != normalizing_, "no need to update downmix normalizing state");
    normalizing_ = normalizing;
    SetupDownMixTable();
}

void DownMixer::NormalizeDMixTable()
{
    // guard PCM data overflow for downmix
    float maxx = 0.0f;
    for (uint32_t i = 0; i < outChannels_; i++) {
        float summ = 0.0f;
        for (uint32_t j = 0; j < inChannels_; j++) {
            summ += downMixTable_[i][j];
        }
        maxx = std::max(maxx, summ);
    }

    if (maxx < 1e-6) {
        AUDIO_ERR_LOG("invalid channel num: in_ch = %{public}u, out_ch = %{public}u",
            inChannels_, outChannels_);
        maxx = 1.0f;
    } else {
        maxx = 1.0f / maxx;
    }

    for (uint32_t i = 0; i < outChannels_; i++) {
        for (uint32_t j = 0; j < inChannels_; j++) {
            downMixTable_[i][j] *= maxx;
        }
    }
}

void DownMixer::ResetSelf()
{
    isInitialized_ = false;
    inChannels_ = 0;
    outChannels_ = 0;
    inLayout_ = CH_LAYOUT_UNKNOWN;
    outLayout_ = CH_LAYOUT_UNKNOWN;
    std::fill(&downMixTable_[0][0], &downMixTable_[0][0] + MAX_CHANNELS * MAX_CHANNELS, 0.0f);
}

void DownMixer::Reset()
{
    ResetSelf();
}

void DownMixer::SetupStereoDmixTable()
{
    uint64_t inChMsk = inLayout_;
    for (uint32_t i = 0; i < inChannels_; i++) {
        uint64_t bit = inChMsk & (~inChMsk + 1);
        downMixTable_[FL][i] = 0.f;
        downMixTable_[FR][i] = 0.f;
        SetupStereoDmixTablePart1(bit, i);
        SetupStereoDmixTablePart2(bit, i);
        inChMsk ^= bit;
    }
}

void DownMixer::Setup5Point1DmixTable()
{
    uint64_t inChMsk = inLayout_;
    for (uint32_t i = 0; i < inChannels_; i++) {
        downMixTable_[FL][i] = 0.f;
        downMixTable_[FR][i] = 0.f;
        downMixTable_[FC][i] = 0.f;
        downMixTable_[SW][i] = 0.f;
        downMixTable_[BL][i] = 0.f;
        downMixTable_[BR][i] = 0.f;
        uint64_t bit = inChMsk & (~inChMsk + 1);
        Setup5Point1DmixTablePart1(bit, i);
        Setup5Point1DmixTablePart2(bit, i);
        inChMsk ^= bit;
    }
}
void DownMixer::Setup5Point1Point2DmixTable()
{
    gTsl_ = INDEX_SIX;
    gTsr_ = INDEX_SEVEN;
    uint64_t inChMsk = inLayout_;
    for (unsigned i = 0; i < inChannels_; i++) {
        uint64_t bit = inChMsk & (~inChMsk + 1);
        downMixTable_[FL][i] = downMixTable_[FR][i] = downMixTable_[FC][i] = 0.f;
        downMixTable_[SW][i] = downMixTable_[BL][i] = downMixTable_[BR][i] = 0.f;
        downMixTable_[gTsl_][i] = downMixTable_[gTsr_][i] = 0.f;
        Setup5Point1Point2DmixTablePart1(bit, i);
        Setup5Point1Point2DmixTablePart2(bit, i);
        inChMsk ^= bit;
    }
}
void DownMixer::Setup5Point1Point4DmixTable()
{
    gTfl_ = INDEX_SIX;
    gTfr_ = INDEX_SEVEN;
    gTbl_ = INDEX_EIGHT;
    gTbr_ = INDEX_NINE;
    uint64_t inChMsk = inLayout_;
    for (unsigned i = 0; i < inChannels_; i++) {
        uint64_t bit = inChMsk & (~inChMsk + 1);
        downMixTable_[FL][i] = downMixTable_[FR][i] = downMixTable_[FC][i] = 0.f;
        downMixTable_[SW][i] = downMixTable_[BL][i] = downMixTable_[BR][i] = 0.f;
        downMixTable_[gTfl_][i] = downMixTable_[gTfr_][i] = 0.f;
        downMixTable_[gTbl_][i] = downMixTable_[gTbr_][i] = 0.f;
        Setup5Point1Point4DmixTablePart1(bit, i);
        Setup5Point1Point4DmixTablePart2(bit, i);
        inChMsk ^= bit;
    }
}
void DownMixer::Setup7Point1DmixTable()
{
    gSl_ = INDEX_SIX;
    gSr_ = INDEX_SEVEN;
    uint64_t inChMsk = inLayout_;
    for (unsigned i = 0; i < inChannels_; i++) {
        uint64_t bit = inChMsk & (~inChMsk + 1);
        downMixTable_[FL][i] = downMixTable_[FR][i] = downMixTable_[FC][i] = 0.f;
        downMixTable_[SW][i] = downMixTable_[gSl_][i] = downMixTable_[gSr_][i] = 0.f;
        downMixTable_[BL][i] = downMixTable_[BR][i] = 0.f;
        Setup7Point1DmixTablePart1(bit, i);
        Setup7Point1DmixTablePart2(bit, i);
        inChMsk ^= bit;
    }
}
void DownMixer::Setup7Point1Point2DmixTable()
{
    gSl_ = INDEX_SIX;
    gSr_ = INDEX_SEVEN;
    gTsl_ = INDEX_EIGHT;
    gTsr_ = INDEX_NINE;
    uint64_t inChMsk = inLayout_;
    for (unsigned i = 0; i < inChannels_; i++) {
        uint64_t bit = inChMsk & (~inChMsk + 1);
        downMixTable_[FL][i] = downMixTable_[FR][i] = downMixTable_[FC][i] = 0.f;
        downMixTable_[SW][i] = downMixTable_[gSl_][i] = downMixTable_[gSr_][i] = 0.f;
        downMixTable_[BL][i] = downMixTable_[BR][i] = 0.f;
        downMixTable_[gTsl_][i] = downMixTable_[gTsr_][i] = 0.f;
        Setup7Point1Point2DmixTablePart1(bit, i);
        Setup7Point1Point2DmixTablePart2(bit, i);
        inChMsk ^= bit;
    }
}
void DownMixer::Setup7Point1Point4DmixTable()
{
    gSl_ = INDEX_SIX;
    gSr_ = INDEX_SEVEN;
    gTfl_ = INDEX_EIGHT;
    gTfr_ = INDEX_NINE;
    gTbl_ = INDEX_TEN;
    gTbr_ = INDEX_ELEVEN;
    uint64_t inChMsk = inLayout_;
    for (unsigned i = 0; i < inChannels_; i++) {
        uint64_t bit = inChMsk & (~inChMsk + 1);
        downMixTable_[FL][i] = downMixTable_[FR][i] = downMixTable_[FC][i] = 0.f;
        downMixTable_[SW][i] = downMixTable_[gSl_][i] = downMixTable_[gSr_][i] = 0.f;
        downMixTable_[BL][i] = downMixTable_[BR][i] = 0.f;
        downMixTable_[gTfl_][i] = downMixTable_[gTfr_][i] = 0.f;
        downMixTable_[gTbl_][i] = downMixTable_[gTbr_][i] = 0.f;
        Setup7Point1Point4DmixTablePart1(bit, i);
        Setup7Point1Point4DmixTablePart2(bit, i);
        inChMsk ^= bit;
    }
}

void DownMixer::SetupStereoDmixTablePart1(uint64_t bit, uint32_t i)
{
    switch (bit) {
        case FRONT_LEFT:
        case TOP_FRONT_LEFT:
        case BOTTOM_FRONT_LEFT:
            downMixTable_[FL][i] = COEF_0DB_F;
            downMixTable_[FR][i] = COEF_ZERO_F;
            break;
        case FRONT_RIGHT:
        case TOP_FRONT_RIGHT:
        case BOTTOM_FRONT_RIGHT:
            downMixTable_[FL][i] = COEF_ZERO_F;
            downMixTable_[FR][i] = COEF_0DB_F;
            break;
        case FRONT_CENTER:
        case TOP_FRONT_CENTER:
        case BOTTOM_FRONT_CENTER:
            downMixTable_[FL][i] = COEF_M3DB_F;
            downMixTable_[FR][i] = COEF_M3DB_F;
            break;
        case BACK_LEFT:
        case SIDE_LEFT:
        case TOP_BACK_LEFT:
        case TOP_SIDE_LEFT:
        case WIDE_LEFT:
            downMixTable_[FL][i] = COEF_M3DB_F;
            downMixTable_[FR][i] = COEF_ZERO_F;
            break;
        case BACK_RIGHT:
        case SIDE_RIGHT:
        case TOP_BACK_RIGHT:
        case TOP_SIDE_RIGHT:
        case WIDE_RIGHT:
            downMixTable_[FL][i] = COEF_ZERO_F;
            downMixTable_[FR][i] = COEF_M3DB_F;
            break;
        case BACK_CENTER:
            downMixTable_[FL][i] = COEF_M6DB_F;
            downMixTable_[FR][i] = COEF_M6DB_F;
            break;
        default:
            break;
    }
}

void DownMixer::SetupStereoDmixTablePart2(uint64_t bit, uint32_t i)
{
    switch (bit) {
        case FRONT_LEFT_OF_CENTER:
            downMixTable_[FL][i] = COEF_M435DB_F;
            downMixTable_[FR][i] = COEF_M12DB_F;
            break;
        case FRONT_RIGHT_OF_CENTER:
            downMixTable_[FL][i] = COEF_M12DB_F;
            downMixTable_[FR][i] = COEF_M435DB_F;
            break;
        case TOP_BACK_CENTER:
            downMixTable_[FL][i] = COEF_M9DB_F;
            downMixTable_[FR][i] = COEF_M9DB_F;
            break;
        case TOP_CENTER:
            downMixTable_[FL][i] = COEF_M899DB_F;
            downMixTable_[FR][i] = COEF_M899DB_F;
            break;
        case LOW_FREQUENCY_2:
            if (mixLfe_) {
                downMixTable_[FL][i] = COEF_ZERO_F;
                downMixTable_[FR][i] = COEF_M6DB_F;
            }
            break;
        case LOW_FREQUENCY:
            if ((mixLfe_) && ((inLayout_ & LOW_FREQUENCY_2) != 0ULL)) {
                downMixTable_[FL][i] = COEF_M6DB_F;
                downMixTable_[FR][i] = COEF_ZERO_F;
            } else if (mixLfe_) {
                downMixTable_[FL][i] = COEF_M6DB_F;
                downMixTable_[FR][i] = COEF_M6DB_F;
            }
            break;
        default:
            break;
    }
}

void DownMixer::Setup5Point1DmixTablePart1(uint64_t bit, uint32_t i)
{
    switch (bit) {
        case FRONT_LEFT:
        case TOP_FRONT_LEFT:
        case BOTTOM_FRONT_LEFT:
        case WIDE_LEFT:
            downMixTable_[FL][i] = COEF_0DB_F;
            break;
        case FRONT_RIGHT:
        case TOP_FRONT_RIGHT:
        case BOTTOM_FRONT_RIGHT:
        case WIDE_RIGHT:
            downMixTable_[FR][i] = COEF_0DB_F;
            break;
        case FRONT_CENTER:
        case TOP_FRONT_CENTER:
        case BOTTOM_FRONT_CENTER:
            downMixTable_[FC][i] = COEF_0DB_F;
            break;
        case SIDE_LEFT:
        case BACK_LEFT:
        case TOP_BACK_LEFT:
        case TOP_SIDE_LEFT:
            downMixTable_[BL][i] = COEF_0DB_F;
            break;
        case (SIDE_RIGHT):
        case (BACK_RIGHT):
        case (TOP_BACK_RIGHT):
        case (TOP_SIDE_RIGHT):
            downMixTable_[BR][i] = COEF_0DB_F;
            break;
        default:
            break;
    }
}

void DownMixer::Setup5Point1DmixTablePart2(uint64_t bit, uint32_t i)
{
    switch (bit) {
        case (LOW_FREQUENCY):
        case (LOW_FREQUENCY_2):
            if (mixLfe_) {
                downMixTable_[SW][i] = COEF_0DB_F;
            }
            break;
        case (FRONT_LEFT_OF_CENTER):
            downMixTable_[FL][i] = COEF_M45DB_F;
            downMixTable_[FC][i] = COEF_M3DB_F;
            break;
        case (FRONT_RIGHT_OF_CENTER):
            downMixTable_[FR][i] = COEF_M45DB_F;
            downMixTable_[FC][i] = COEF_M3DB_F;
            break;
        case (BACK_CENTER):
        case (TOP_BACK_CENTER):
            downMixTable_[BL][i] = COEF_M3DB_F;
            downMixTable_[BR][i] = COEF_M3DB_F;
            break;
        case (TOP_CENTER):
            downMixTable_[FC][i] = COEF_M6DB_F;
            downMixTable_[BL][i] = COEF_M6DB_F;
            downMixTable_[BR][i] = COEF_M6DB_F;
            break;
        default:
            break;
    }
}

void DownMixer::Setup5Point1Point2DmixTablePart1(uint64_t bit, uint32_t i)
{
    switch (bit) {
        case (FRONT_LEFT):
        case (TOP_FRONT_LEFT):
        case (BOTTOM_FRONT_LEFT):
        case (WIDE_LEFT):
            downMixTable_[FL][i] = COEF_0DB_F;
            break;
        case (FRONT_RIGHT):
        case (TOP_FRONT_RIGHT):
        case (BOTTOM_FRONT_RIGHT):
        case (WIDE_RIGHT):
            downMixTable_[FR][i] = COEF_0DB_F;
            break;
        case (FRONT_CENTER):
        case (TOP_FRONT_CENTER):
        case (BOTTOM_FRONT_CENTER):
            downMixTable_[FC][i] = COEF_0DB_F;
            break;
        case (SIDE_LEFT):
        case (BACK_LEFT):
            downMixTable_[BL][i] = COEF_0DB_F;
            break;
        case (SIDE_RIGHT):
        case (BACK_RIGHT):
            downMixTable_[BR][i] = COEF_0DB_F;
            break;
        default:
            break;
    }
}

void DownMixer::Setup5Point1Point2DmixTablePart2(uint64_t bit, uint32_t i)
{
    switch (bit) {
        case (TOP_BACK_LEFT):
        case (TOP_SIDE_LEFT):
            downMixTable_[gTsl_][i] = COEF_0DB_F;
            break;
        case (TOP_BACK_RIGHT):
        case (TOP_SIDE_RIGHT):
            downMixTable_[gTsr_][i] = COEF_0DB_F;
            break;
        case (LOW_FREQUENCY):
        case (LOW_FREQUENCY_2):
            if (mixLfe_) {
                downMixTable_[SW][i] = COEF_0DB_F;
            }
            break;
        case (FRONT_LEFT_OF_CENTER):
            downMixTable_[FL][i] = COEF_M45DB_F;
            downMixTable_[FC][i] = COEF_M3DB_F;
            break;
        case (FRONT_RIGHT_OF_CENTER):
            downMixTable_[FR][i] = COEF_M45DB_F;
            downMixTable_[FC][i] = COEF_M3DB_F;
            break;
        case (BACK_CENTER):
            downMixTable_[BL][i] = COEF_M3DB_F;
            downMixTable_[BR][i] = COEF_M3DB_F;
            break;
        case (TOP_BACK_CENTER):
        case (TOP_CENTER):
            downMixTable_[gTsl_][i] = COEF_M3DB_F;
            downMixTable_[gTsr_][i] = COEF_M3DB_F;
            break;
        default:
            break;
    }
}

void DownMixer::Setup5Point1Point4DmixTablePart1(uint64_t bit, uint32_t i)
{
    switch (bit) {
        case (FRONT_LEFT):
        case (BOTTOM_FRONT_LEFT):
        case (WIDE_LEFT):
            downMixTable_[FL][i] = COEF_0DB_F;
            break;
        case (FRONT_RIGHT):
        case (BOTTOM_FRONT_RIGHT):
        case (WIDE_RIGHT):
            downMixTable_[FR][i] = COEF_0DB_F;
            break;
        case (FRONT_CENTER):
        case (BOTTOM_FRONT_CENTER):
            downMixTable_[FC][i] = COEF_0DB_F;
            break;
        case (SIDE_LEFT):
        case (BACK_LEFT):
            downMixTable_[BL][i] = COEF_0DB_F;
            break;
        case (SIDE_RIGHT):
        case (BACK_RIGHT):
            downMixTable_[BR][i] = COEF_0DB_F;
            break;
        case (TOP_FRONT_LEFT):
            downMixTable_[gTfl_][i] = COEF_0DB_F;
            break;
        case (TOP_FRONT_RIGHT):
            downMixTable_[gTfr_][i] = COEF_0DB_F;
            break;
        case (TOP_BACK_LEFT):
        case (TOP_SIDE_LEFT):
            downMixTable_[gTbl_][i] = COEF_0DB_F;
            break;
        case (TOP_BACK_RIGHT):
        case (TOP_SIDE_RIGHT):
            downMixTable_[gTbr_][i] = COEF_0DB_F;
            break;
        case (LOW_FREQUENCY):
        case (LOW_FREQUENCY_2):
            if (mixLfe_) {
                downMixTable_[SW][i] = COEF_0DB_F;
            }
            break;
        default:
            break;
    }
}

void DownMixer::Setup5Point1Point4DmixTablePart2(uint64_t bit, uint32_t i)
{
    switch (bit) {
        case (FRONT_LEFT_OF_CENTER):
            downMixTable_[FL][i] = COEF_M45DB_F;
            downMixTable_[FC][i] = COEF_M3DB_F;
            break;
        case (FRONT_RIGHT_OF_CENTER):
            downMixTable_[FR][i] = COEF_M45DB_F;
            downMixTable_[FC][i] = COEF_M3DB_F;
            break;
        case (BACK_CENTER):
            downMixTable_[BL][i] = COEF_M3DB_F;
            downMixTable_[BR][i] = COEF_M3DB_F;
            break;
        case (TOP_FRONT_CENTER):
            downMixTable_[gTfl_][i] = COEF_M3DB_F;
            downMixTable_[gTfr_][i] = COEF_M3DB_F;
            break;
        case (TOP_BACK_CENTER):
            downMixTable_[gTbl_][i] = COEF_M3DB_F;
            downMixTable_[gTbr_][i] = COEF_M3DB_F;
            break;
        case (TOP_CENTER):
            downMixTable_[gTfl_][i] = COEF_M6DB_F;
            downMixTable_[gTfr_][i] = COEF_M6DB_F;
            downMixTable_[gTbl_][i] = COEF_M6DB_F;
            downMixTable_[gTbr_][i] = COEF_M6DB_F;
            break;
        default:
            break;
    }
}

void DownMixer::Setup7Point1DmixTablePart1(uint64_t bit, uint32_t i)
{
    switch (bit) {
        case (FRONT_LEFT):
        case (TOP_FRONT_LEFT):
        case (BOTTOM_FRONT_LEFT):
        case (WIDE_LEFT):
            downMixTable_[FL][i] = COEF_0DB_F;
            break;
        case (FRONT_RIGHT):
        case (TOP_FRONT_RIGHT):
        case (BOTTOM_FRONT_RIGHT):
        case (WIDE_RIGHT):
            downMixTable_[FR][i] = COEF_0DB_F;
            break;
        case (FRONT_CENTER):
        case (TOP_FRONT_CENTER):
        case (BOTTOM_FRONT_CENTER):
            downMixTable_[FC][i] = COEF_0DB_F;
            break;
        case (SIDE_LEFT):
        case (TOP_SIDE_LEFT):
            downMixTable_[gSl_][i] = COEF_0DB_F;
            break;
        case (SIDE_RIGHT):
        case (TOP_SIDE_RIGHT):
            downMixTable_[gSr_][i] = COEF_0DB_F;
            break;
        case (BACK_LEFT):
        case (TOP_BACK_LEFT):
            downMixTable_[BL][i] = COEF_0DB_F;
            break;
        case (BACK_RIGHT):
        case (TOP_BACK_RIGHT):
            downMixTable_[BR][i] = COEF_0DB_F;
            break;
        default:
            break;
    }
}

void DownMixer::Setup7Point1DmixTablePart2(uint64_t bit, uint32_t i)
{
    switch (bit) {
        case (LOW_FREQUENCY):
        case (LOW_FREQUENCY_2):
            if (mixLfe_) {
                downMixTable_[SW][i] = COEF_0DB_F;
            }
            break;
        case (FRONT_LEFT_OF_CENTER):
            downMixTable_[FL][i] = COEF_M45DB_F;
            downMixTable_[FC][i] = COEF_M3DB_F;
            break;
        case (FRONT_RIGHT_OF_CENTER):
            downMixTable_[FR][i] = COEF_M45DB_F;
            downMixTable_[FC][i] = COEF_M3DB_F;
            break;
        case (BACK_CENTER):
        case (TOP_BACK_CENTER):
            downMixTable_[BL][i] = COEF_M3DB_F;
            downMixTable_[BR][i] = COEF_M3DB_F;
            break;
        case (TOP_CENTER):
            downMixTable_[FC][i] = COEF_M6DB_F;
            downMixTable_[gSl_][i] = COEF_M6DB_F;
            downMixTable_[gSr_][i] = COEF_M6DB_F;
            break;
        default:
            break;
    }
}

void DownMixer::Setup7Point1Point2DmixTablePart1(uint64_t bit, uint32_t i)
{
    switch (bit) {
        case (FRONT_LEFT):
        case (TOP_FRONT_LEFT):
        case (BOTTOM_FRONT_LEFT):
        case (WIDE_LEFT):
            downMixTable_[FL][i] = COEF_0DB_F;
            break;
        case (FRONT_RIGHT):
        case (TOP_FRONT_RIGHT):
        case (BOTTOM_FRONT_RIGHT):
        case (WIDE_RIGHT):
            downMixTable_[FR][i] = COEF_0DB_F;
            break;
        case (FRONT_CENTER):
        case (TOP_FRONT_CENTER):
        case (BOTTOM_FRONT_CENTER):
            downMixTable_[FC][i] = COEF_0DB_F;
            break;
        case (SIDE_LEFT):
            downMixTable_[gSl_][i] = COEF_0DB_F;
            break;
        case (SIDE_RIGHT):
            downMixTable_[gSr_][i] = COEF_0DB_F;
            break;
        case (BACK_LEFT):
            downMixTable_[BL][i] = COEF_0DB_F;
            break;
        case (BACK_RIGHT):
            downMixTable_[BR][i] = COEF_0DB_F;
            break;
        case (TOP_BACK_LEFT):
        case (TOP_SIDE_LEFT):
            downMixTable_[gTsl_][i] = COEF_0DB_F;
            break;
        case (TOP_BACK_RIGHT):
        case (TOP_SIDE_RIGHT):
            downMixTable_[gTsr_][i] = COEF_0DB_F;
            break;
        case (LOW_FREQUENCY):
        case (LOW_FREQUENCY_2):
            if (mixLfe_) {
                downMixTable_[SW][i] = COEF_0DB_F;
            }
            break;
        default:
            break;
    }
}

void DownMixer::Setup7Point1Point2DmixTablePart2(uint64_t bit, uint32_t i)
{
    switch (bit) {
        case (FRONT_LEFT_OF_CENTER):
            downMixTable_[FL][i] = COEF_M45DB_F;
            downMixTable_[FC][i] = COEF_M3DB_F;
            break;
        case (FRONT_RIGHT_OF_CENTER):
            downMixTable_[FR][i] = COEF_M45DB_F;
            downMixTable_[FC][i] = COEF_M3DB_F;
            break;
        case (BACK_CENTER):
            downMixTable_[BL][i] = COEF_M3DB_F;
            downMixTable_[BR][i] = COEF_M3DB_F;
            break;
        case (TOP_BACK_CENTER):
        case (TOP_CENTER):
            downMixTable_[gTsl_][i] = COEF_M3DB_F;
            downMixTable_[gTsr_][i] = COEF_M3DB_F;
            break;
        default:
            break;
    }
}

void DownMixer::Setup7Point1Point4DmixTablePart1(uint64_t bit, uint32_t i)
{
    switch (bit) {
        case (FRONT_LEFT):
        case (BOTTOM_FRONT_LEFT):
        case (WIDE_LEFT):
            downMixTable_[FL][i] = COEF_0DB_F;
            break;
        case (FRONT_RIGHT):
        case (BOTTOM_FRONT_RIGHT):
        case (WIDE_RIGHT):
            downMixTable_[FR][i] = COEF_0DB_F;
            break;
        case (FRONT_CENTER):
        case (BOTTOM_FRONT_CENTER):
            downMixTable_[FC][i] = COEF_0DB_F;
            break;
        case (SIDE_LEFT):
            downMixTable_[gSl_][i] = COEF_0DB_F;
            break;
        case (SIDE_RIGHT):
            downMixTable_[gSr_][i] = COEF_0DB_F;
            break;
        case (BACK_LEFT):
            downMixTable_[BL][i] = COEF_0DB_F;
            break;
        case (BACK_RIGHT):
            downMixTable_[BR][i] = COEF_0DB_F;
            break;
        case (TOP_FRONT_LEFT):
            downMixTable_[gTfl_][i] = COEF_0DB_F;
            break;
        case (TOP_FRONT_RIGHT):
            downMixTable_[gTfr_][i] = COEF_0DB_F;
            break;
        case (TOP_BACK_LEFT):
        case (TOP_SIDE_LEFT):
            downMixTable_[gTbl_][i] = COEF_0DB_F;
            break;
        case (TOP_BACK_RIGHT):
        case (TOP_SIDE_RIGHT):
            downMixTable_[gTbr_][i] = COEF_0DB_F;
            break;
        default:
            break;
    }
}

void DownMixer::Setup7Point1Point4DmixTablePart2(uint64_t bit, uint32_t i)
{
    switch (bit) {
        case (LOW_FREQUENCY):
        case (LOW_FREQUENCY_2):
            if (mixLfe_) {
                downMixTable_[SW][i] = COEF_0DB_F;
            }
            break;
        case (FRONT_LEFT_OF_CENTER):
            downMixTable_[FL][i] = COEF_M45DB_F;
            downMixTable_[FC][i] = COEF_M3DB_F;
            break;
        case (FRONT_RIGHT_OF_CENTER):
            downMixTable_[FR][i] = COEF_M45DB_F;
            downMixTable_[FC][i] = COEF_M3DB_F;
            break;
        case (BACK_CENTER):
            downMixTable_[BL][i] = COEF_M3DB_F;
            downMixTable_[BR][i] = COEF_M3DB_F;
            break;
        case (TOP_FRONT_CENTER):
            downMixTable_[gTfl_][i] = COEF_M3DB_F;
            downMixTable_[gTfr_][i] = COEF_M3DB_F;
            break;
        case (TOP_BACK_CENTER):
            downMixTable_[gTbl_][i] = COEF_M3DB_F;
            downMixTable_[gTbr_][i] = COEF_M3DB_F;
            break;
        case (TOP_CENTER):
            downMixTable_[gTfl_][i] = COEF_M6DB_F;
            downMixTable_[gTfr_][i] = COEF_M6DB_F;
            downMixTable_[gTbl_][i] = COEF_M6DB_F;
            downMixTable_[gTbr_][i] = COEF_M6DB_F;
            break;
        default:
            break;
    }
}

void DownMixer::GetDownMixTable(float (&coeffTable)[MAX_CHANNELS][MAX_CHANNELS]) const
{
    CHECK_AND_RETURN_LOG(isInitialized_, "downmix table is not initialized!");
    for (uint32_t i = 0; i < MAX_CHANNELS; i++) {
        for (uint32_t j = 0; j < MAX_CHANNELS; j++) {
            coeffTable[i][j] = downMixTable_[i][j];
        }
    }
}
} // HPAE
} // AudioStandard
} // OHOS