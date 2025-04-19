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
#include <algorithm>
#include <cinttypes>
#include "securec.h"
#include "audio_engine_log.h"
#include "down_mixer.h"

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

static uint32_t g_sl = 6;
static uint32_t g_sr = 7;
static uint32_t g_tfl = 8;
static uint32_t g_tfr = 9;
static uint32_t g_tbl = 10;
static uint32_t g_tbr = 11;
static uint32_t g_tsl = 12;
static uint32_t g_tsr = 13;

static constexpr uint32_t INDEX_SIX = 6;
static constexpr uint32_t INDEX_SEVEN = 7;
static constexpr uint32_t INDEX_EIGHT = 8;
static constexpr uint32_t INDEX_NINE = 9;
static constexpr uint32_t INDEX_TEN = 10;
static constexpr uint32_t INDEX_ELEVEN = 11;

// channel masks for downmixing general output channel layout
static constexpr uint64_t MASK_MIDDLE_FRONT = FRONT_LEFT | FRONT_RIGHT | FRONT_CENTER |
FRONT_LEFT_OF_CENTER | FRONT_RIGHT_OF_CENTER | WIDE_LEFT | WIDE_RIGHT;

static constexpr uint64_t MASK_MIDDLE_REAR = BACK_LEFT | BACK_RIGHT | BACK_CENTER
| SIDE_LEFT
| SIDE_RIGHT;

static constexpr uint64_t MASK_TOP_FRONT = TOP_FRONT_LEFT
| TOP_FRONT_CENTER
| TOP_FRONT_RIGHT;

static constexpr uint64_t MASK_TOP_REAR = TOP_CENTER
| TOP_BACK_LEFT
| TOP_BACK_CENTER
| TOP_BACK_RIGHT
| TOP_SIDE_LEFT
| TOP_SIDE_RIGHT;

static constexpr uint64_t MASK_BOTTOM = BOTTOM_FRONT_CENTER
| BOTTOM_FRONT_LEFT
| BOTTOM_FRONT_RIGHT;

static constexpr uint64_t MASK_LFE = LOW_FREQUENCY
| LOW_FREQUENCY_2;

static uint32_t BitCounts(uint64_t bits);
static bool IsValidChLayout(AudioChannelLayout &chLayout, uint32_t chCounts);
static AudioChannelLayout SetDefaultChannelLayout(AudioChannel channels);

// 改成默认构造
DownMixer::DownMixer()
{
    downMixTable_.resize(MAX_CHANNELS, std::vector<float>(MAX_CHANNELS, 0));
}

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
    
    formatSize_ = formatSize;
    int32_t ret = SetupDownMixTable();
    if (ret == DMIX_ERR_SUCCESS) {
        isInitialized_ = true;
    }
    return ret;
}

int32_t DownMixer::Process(uint32_t frameLen, float* in, uint32_t inLen, float* out, uint32_t outLen)
{
    if ((in == nullptr) || (out == nullptr) || frameLen <= 0) {
        AUDIO_ERR_LOG("invalid input params for downmix process, frameLen %{public}d", frameLen);
        return DMIX_ERR_ALLOC_FAILED;
    }
    if (!isInitialized_) {
        AUDIO_DEBUG_LOG("Downmixe table has not been initialized!");
        return DMIX_ERR_ALLOC_FAILED;
    }
    uint32_t expectInLen = frameLen * inChannels_ * formatSize_;
    uint32_t expectOutLen = frameLen * outChannels_ * formatSize_;
    if ((expectInLen > inLen) || (expectOutLen > outLen)) {
        int32_t ret = memcpy_s(out, outLen, in, inLen);
        if (ret != 0) {
            AUDIO_ERR_LOG("memcpy failed when down-mixer process");
            return DMIX_ERR_ALLOC_FAILED;
        }
        AUDIO_ERR_LOG("expect Input Len: %{public}d, inLen: %{public}d,"
            "expected output len: %{public}d, outLen %{public}d",
            expectInLen, inLen, expectOutLen, outLen);
        return DMIX_ERR_ALLOC_FAILED;
    }
    AUDIO_DEBUG_LOG("Downmixing: frameLen: %{public}d,", frameLen);
    float a;
    for (; frameLen > 0; frameLen--) {
        for (uint32_t i = 0; i < outChannels_; i++) {
            a = 0.0f;
            for (uint32_t j = 0; j < inChannels_; j++) {
                a += in[j] * downMixTable_[i][j];
            }
            *(out++) = a;
        }
        in += inChannels_;
    }
    return DMIX_ERR_SUCCESS;
}

int32_t DownMixer::SetupDownMixTable()
{
    if ((!IsValidChLayout(inLayout_, inChannels_)) || (!IsValidChLayout(outLayout_, outChannels_))
        || inLayout_ == outLayout_ || inChannels_ <= outChannels_) {
        AUDIO_ERR_LOG("invalid input or output channellayout: input channel count %{public}d, "
            "inLayout_ %{public}" PRIu64 "output channel count %{public}d, outLayout_ %{public}" PRIu64 "",
            inChannels_, inLayout_, outChannels_, outLayout_);
        return DMIX_ERR_INVALID_ARG;
    }
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
            SetupGeneralDmixTable();
            break;
        }
    }
    NormalizeDMixTable();
    AUDIO_INFO_LOG("setup downmix table success!");
    isInitialized_ = true;
    return DMIX_ERR_SUCCESS;
}

void DownMixer::NormalizeDMixTable()
{
    float maxx = 0.0f;
    for (uint32_t i = 0; i < outChannels_; i++) {
        float summ = 0.0f;
        for (uint32_t j = 0; j < inChannels_; j++) {
            summ += downMixTable_[i][j];
        }
        maxx = std::max(maxx, summ);
    }

    if (maxx != 0.0f) {
        maxx = 1.0f / maxx;
    } else {
        AUDIO_ERR_LOG("invalid channel num: in_ch = %{public}u, out_ch = %{public}u",
                inChannels_, outChannels_);
        maxx = 1.0f;
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
    for (auto &row : downMixTable_) {
        std::fill(row.begin(), row.end(), 0.0f);
    }
}

void DownMixer::Reset()
{
    ResetSelf();
}

void DownMixer::SetupStereoDmixTable()
{
    uint64_t inChMsk = inLayout_;
    for (uint32_t i = 0; i < inChannels_; i++) {
        uint64_t bit = inChMsk & -(int64_t)inChMsk;
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
        uint64_t bit = inChMsk & -(int64_t)inChMsk;
        Setup5Point1DmixTablePart1(bit, i);
        Setup5Point1DmixTablePart2(bit, i);
        inChMsk ^= bit;
    }
}
void DownMixer::Setup5Point1Point2DmixTable()
{
    g_tsl = INDEX_SIX;
    g_tsr = INDEX_SEVEN;
    uint64_t inChMsk = inLayout_;
    for (unsigned i = 0; i < inChannels_; i++) {
        uint64_t bit = inChMsk & -(int64_t)inChMsk;
        downMixTable_[FL][i] = downMixTable_[FR][i] = downMixTable_[FC][i] = 0.f;
        downMixTable_[SW][i] = downMixTable_[BL][i] = downMixTable_[BR][i] = 0.f;
        downMixTable_[g_tsl][i] = downMixTable_[g_tsr][i] = 0.f;
        Setup5Point1Point2DmixTablePart1(bit, i);
        Setup5Point1Point2DmixTablePart2(bit, i);
        inChMsk ^= bit;
    }
}
void DownMixer::Setup5Point1Point4DmixTable()
{
    g_tfl = INDEX_SIX;
    g_tfr = INDEX_SEVEN;
    g_tbl = INDEX_EIGHT;
    g_tbr = INDEX_NINE;
    uint64_t inChMsk = inLayout_;
    for (unsigned i = 0; i < inChannels_; i++) {
        uint64_t bit = inChMsk & -(int64_t)inChMsk;
        downMixTable_[FL][i] = downMixTable_[FR][i] = downMixTable_[FC][i] = 0.f;
        downMixTable_[SW][i] = downMixTable_[BL][i] = downMixTable_[BR][i] = 0.f;
        downMixTable_[g_tfl][i] = downMixTable_[g_tfr][i] = 0.f;
        downMixTable_[g_tbl][i] = downMixTable_[g_tbr][i] = 0.f;
        Setup5Point1Point4DmixTablePart1(bit, i);
        Setup5Point1Point4DmixTablePart2(bit, i);
        inChMsk ^= bit;
    }
}
void DownMixer::Setup7Point1DmixTable()
{
    g_sl = INDEX_SIX;
    g_sr = INDEX_SEVEN;
    uint64_t inChMsk = inLayout_;
    for (unsigned i = 0; i < inChannels_; i++) {
        uint64_t bit = inChMsk & -(int64_t)inChMsk;
        downMixTable_[FL][i] = downMixTable_[FR][i] = downMixTable_[FC][i] = 0.f;
        downMixTable_[SW][i] = downMixTable_[g_sl][i] = downMixTable_[g_sr][i] = 0.f;
        downMixTable_[BL][i] = downMixTable_[BR][i] = 0.f;
        Setup7Point1DmixTablePart1(bit, i);
        Setup7Point1DmixTablePart2(bit, i);
        inChMsk ^= bit;
    }
}
void DownMixer::Setup7Point1Point2DmixTable()
{
    g_sl = INDEX_SIX;
    g_sr = INDEX_SEVEN;
    g_tsl = INDEX_EIGHT;
    g_tsr = INDEX_NINE;
    uint64_t inChMsk = inLayout_;
    for (unsigned i = 0; i < inChannels_; i++) {
        uint64_t bit = inChMsk & -(int64_t)inChMsk;
        downMixTable_[FL][i] = downMixTable_[FR][i] = downMixTable_[FC][i] = 0.f;
        downMixTable_[SW][i] = downMixTable_[g_sl][i] = downMixTable_[g_sr][i] = 0.f;
        downMixTable_[BL][i] = downMixTable_[BR][i] = 0.f;
        downMixTable_[g_tsl][i] = downMixTable_[g_tsr][i] = 0.f;
        Setup7Point1Point2DmixTablePart1(bit, i);
        Setup7Point1Point2DmixTablePart2(bit, i);
        inChMsk ^= bit;
    }
}
void DownMixer::Setup7Point1Point4DmixTable()
{
    g_sl = INDEX_SIX;
    g_sr = INDEX_SEVEN;
    g_tfl = INDEX_EIGHT;
    g_tfr = INDEX_NINE;
    g_tbl = INDEX_TEN;
    g_tbr = INDEX_ELEVEN;
    uint64_t inChMsk = inLayout_;
    for (unsigned i = 0; i < inChannels_; i++) {
        uint64_t bit = inChMsk & -(int64_t)inChMsk;
        downMixTable_[FL][i] = downMixTable_[FR][i] = downMixTable_[FC][i] = 0.f;
        downMixTable_[SW][i] = downMixTable_[g_sl][i] = downMixTable_[g_sr][i] = 0.f;
        downMixTable_[BL][i] = downMixTable_[BR][i] = 0.f;
        downMixTable_[g_tfl][i] = downMixTable_[g_tfr][i] = 0.f;
        downMixTable_[g_tbl][i] = downMixTable_[g_tbr][i] = 0.f;
        Setup7Point1Point4DmixTablePart1(bit, i);
        Setup7Point1Point4DmixTablePart2(bit, i);
        inChMsk ^= bit;
    }
}

void DownMixer::SetupGeneralDmixTable()
{
    // MONO
    if (outLayout_ == CH_LAYOUT_MONO) {
        for (uint32_t i = 0; i < inChannels_; i++) {
            downMixTable_[0][i] = COEF_0DB_F;
        }
    }
    // check invalid output musk later in init()
    uint64_t outChMsk = outLayout_;

    for (uint32_t i = 0; i < outChannels_; i++) {
        uint64_t outBit = outChMsk & -(int64_t)outChMsk;
        uint64_t inChMsk = inLayout_;
        for (uint32_t j = 0; j < inChannels_; j++) {
            uint64_t inBit = inChMsk & -(int64_t)inChMsk;
            if (inBit & outBit) { // if in channel and out channel is the same
                downMixTable_[i][j] = COEF_0DB_F;
            } else if (inBit == TOP_CENTER) {
                // check general downmix table!
                DownMixTopCenter(inBit, outBit, i, j);
            } else if ((inBit & MASK_MIDDLE_FRONT) != 0) {
                DownMixMidFront(inBit, outBit, i, j);
            } else if ((inBit & MASK_MIDDLE_REAR) != 0) {
                DownMixMidRear(inBit, outBit, i, j);
            } else if ((inBit & MASK_BOTTOM) != 0) {
                DownMixBottom(inBit, outBit, i, j);
            } else if ((inBit & MASK_LFE) != 0) {
                DownMixLfe(inBit, outBit, i, j);
            } else if ((inBit & MASK_TOP_FRONT) != 0) {
                DownMixTopFront(inBit, outBit, i, j);
            } else if ((inBit & MASK_TOP_REAR) != 0) {
                DownMixTopRear(inBit, outBit, i, j);
            }
            inChMsk ^= inBit;
        }
        outChMsk ^= outBit;
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
            if ((mixLfe_) & (inLayout_ & (LOW_FREQUENCY_2))) {
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
            downMixTable_[g_tsl][i] = COEF_0DB_F;
            break;
        case (TOP_BACK_RIGHT):
        case (TOP_SIDE_RIGHT):
            downMixTable_[g_tsr][i] = COEF_0DB_F;
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
            downMixTable_[g_tsl][i] = COEF_M3DB_F;
            downMixTable_[g_tsr][i] = COEF_M3DB_F;
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
            downMixTable_[g_tfl][i] = COEF_0DB_F;
            break;
        case (TOP_FRONT_RIGHT):
            downMixTable_[g_tfr][i] = COEF_0DB_F;
            break;
        case (TOP_BACK_LEFT):
        case (TOP_SIDE_LEFT):
            downMixTable_[g_tbl][i] = COEF_0DB_F;
            break;
        case (TOP_BACK_RIGHT):
        case (TOP_SIDE_RIGHT):
            downMixTable_[g_tbr][i] = COEF_0DB_F;
            break;
        case (LOW_FREQUENCY):
        case (LOW_FREQUENCY_2):
            if (mixLfe_) {
                downMixTable_[SW][i] = COEF_0DB_F;
            }
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
            downMixTable_[g_tfl][i] = COEF_M3DB_F;
            downMixTable_[g_tfr][i] = COEF_M3DB_F;
            break;
        case (TOP_BACK_CENTER):
            downMixTable_[g_tbl][i] = COEF_M3DB_F;
            downMixTable_[g_tbr][i] = COEF_M3DB_F;
            break;
        case (TOP_CENTER):
            downMixTable_[g_tfl][i] = COEF_M6DB_F;
            downMixTable_[g_tfr][i] = COEF_M6DB_F;
            downMixTable_[g_tbl][i] = COEF_M6DB_F;
            downMixTable_[g_tbr][i] = COEF_M6DB_F;
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
            downMixTable_[g_sl][i] = COEF_0DB_F;
            break;
        case (SIDE_RIGHT):
        case (TOP_SIDE_RIGHT):
            downMixTable_[g_sr][i] = COEF_0DB_F;
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
            downMixTable_[g_sl][i] = COEF_M6DB_F;
            downMixTable_[g_sr][i] = COEF_M6DB_F;
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
            downMixTable_[g_sl][i] = COEF_0DB_F;
            break;
        case (SIDE_RIGHT):
            downMixTable_[g_sr][i] = COEF_0DB_F;
            break;
        case (BACK_LEFT):
            downMixTable_[BL][i] = COEF_0DB_F;
            break;
        case (BACK_RIGHT):
            downMixTable_[BR][i] = COEF_0DB_F;
            break;
        case (TOP_BACK_LEFT):
        case (TOP_SIDE_LEFT):
            downMixTable_[g_tsl][i] = COEF_0DB_F;
            break;
        case (TOP_BACK_RIGHT):
        case (TOP_SIDE_RIGHT):
            downMixTable_[g_tsr][i] = COEF_0DB_F;
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
            downMixTable_[g_tsl][i] = COEF_M3DB_F;
            downMixTable_[g_tsr][i] = COEF_M3DB_F;
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
            downMixTable_[g_sl][i] = COEF_0DB_F;
            break;
        case (SIDE_RIGHT):
            downMixTable_[g_sr][i] = COEF_0DB_F;
            break;
        case (BACK_LEFT):
            downMixTable_[BL][i] = COEF_0DB_F;
            break;
        case (BACK_RIGHT):
            downMixTable_[BR][i] = COEF_0DB_F;
            break;
        case (TOP_FRONT_LEFT):
            downMixTable_[g_tfl][i] = COEF_0DB_F;
            break;
        case (TOP_FRONT_RIGHT):
            downMixTable_[g_tfr][i] = COEF_0DB_F;
            break;
        case (TOP_BACK_LEFT):
        case (TOP_SIDE_LEFT):
            downMixTable_[g_tbl][i] = COEF_0DB_F;
            break;
        case (TOP_BACK_RIGHT):
        case (TOP_SIDE_RIGHT):
            downMixTable_[g_tbr][i] = COEF_0DB_F;
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
            downMixTable_[g_tfl][i] = COEF_M3DB_F;
            downMixTable_[g_tfr][i] = COEF_M3DB_F;
            break;
        case (TOP_BACK_CENTER):
            downMixTable_[g_tbl][i] = COEF_M3DB_F;
            downMixTable_[g_tbr][i] = COEF_M3DB_F;
            break;
        case (TOP_CENTER):
            downMixTable_[g_tfl][i] = COEF_M6DB_F;
            downMixTable_[g_tfr][i] = COEF_M6DB_F;
            downMixTable_[g_tbl][i] = COEF_M6DB_F;
            downMixTable_[g_tbr][i] = COEF_M6DB_F;
            break;
        default:
            break;
    }
}

void DownMixer::DownMixBottom(uint64_t inBit, uint64_t outBit, uint32_t i, uint32_t j)
{
    if ((inBit & MASK_BOTTOM) && (outBit & MASK_BOTTOM)) {
        downMixTable_[i][j] = COEF_M3DB_F;
    } else {
        DownMixMidFront(inBit, outBit, i, j);
    }
}

void DownMixer::DownMixMidFront(uint64_t inBit, uint64_t outBit, uint32_t i, uint32_t j)
{
    if ((inBit & MASK_MIDDLE_FRONT) && (outBit & MASK_MIDDLE_FRONT)) {
        downMixTable_[i][j] = COEF_M3DB_F;
    }
}

void DownMixer::DownMixMidRear(uint64_t inBit, uint64_t outBit, uint32_t i, uint32_t j)
{
    // Middle layer
    switch (inBit) {
        case BACK_CENTER:
            if ((outBit == BACK_LEFT) || (outBit == SIDE_LEFT) || (outBit == WIDE_LEFT) || (outBit == FRONT_LEFT) ||
                (outBit == BACK_RIGHT) || outBit == SIDE_RIGHT || outBit == WIDE_RIGHT || outBit == FRONT_RIGHT) {
                downMixTable_[i][j] = COEF_M3DB_F;
            }
            break;
        case BACK_LEFT:
            if (outBit == SIDE_LEFT) {
                downMixTable_[i][j] = COEF_0DB_F;
            } else if (outBit == BACK_CENTER) {
                downMixTable_[i][j] = COEF_0DB_F;
            } else {
                DownMixMidFront(WIDE_LEFT, outBit, i, j);
            }
            break;
        case BACK_RIGHT:
            if (outBit == SIDE_RIGHT) {
                downMixTable_[i][j] = COEF_0DB_F;
            } else if (outBit == BACK_CENTER) {
                downMixTable_[i][j] = COEF_0DB_F;
            } else {
                DownMixMidFront(WIDE_RIGHT, outBit, i, j);
            }
            break;
    }
}

void DownMixer::DownMixLfe(uint64_t inBit, uint64_t outBit, uint32_t i, uint32_t j)
{
    if ((MASK_LFE & inBit) && (MASK_LFE & outBit)) {
        downMixTable_[i][j] = COEF_0DB_F;
    } else {
        if ((inBit == LOW_FREQUENCY) && ((outBit & CH_LAYOUT_STEREO)!= 0)) {
            downMixTable_[i][j] = COEF_M6DB_F;
        } else if ((inBit == LOW_FREQUENCY_2) && ((outBit & CH_LAYOUT_STEREO) != 0)) {
            downMixTable_[i][j] = COEF_M6DB_F;
        }
    }
}

void DownMixer::DownMixTopCenter(uint64_t inBit, uint64_t outBit, uint32_t i, uint32_t j)
{
    uint64_t exitTopOuts = outLayout_ & (MASK_TOP_FRONT | MASK_TOP_REAR);
    uint64_t exitMiddleOuts = outLayout_ & (MASK_MIDDLE_FRONT | MASK_MIDDLE_REAR);
    if (exitTopOuts != 0) { // exist top outs
        uint32_t numChannels = BitCounts(exitTopOuts);
        uint32_t coeff = 1.0f / sqrt((float)numChannels);
        if ((outBit & exitTopOuts) != 0) {
            downMixTable_[i][j] = coeff;
        }
    } else if (exitMiddleOuts != 0) {
        uint32_t numChannels = BitCounts(exitMiddleOuts);
        uint32_t coeff = 1.0f / sqrt((float)numChannels);
        if ((outBit & exitMiddleOuts) != 0) {
            downMixTable_[i][j] = coeff;
        }
    }
}

void DownMixer::DownMixTopFront(uint64_t inBit, uint64_t outBit, uint32_t i, uint32_t j)
{
    uint64_t existTopFrontOuts = outLayout_ & MASK_TOP_FRONT;
    if (existTopFrontOuts != 0) {
        if ((outBit & MASK_TOP_FRONT) != 0) {
            downMixTable_[i][j] = COEF_M3DB_F;
        }
    } else {
        DownMixMidFront(TOP_FRONT_CENTER, outBit, i, j);
    }
}

void DownMixer::DownMixTopRear(uint64_t inBit, uint64_t outBit, uint32_t i, uint32_t j)
{
    uint64_t existTopRearOuts = outLayout_ & MASK_TOP_REAR;
    if (existTopRearOuts != 0) {
        if ((outBit & MASK_TOP_REAR) != 0) {
            downMixTable_[i][j] = COEF_M3DB_F;
        }
    } else {
        DownMixMidRear(BACK_CENTER, outBit, i, j);
    }
}

static uint32_t BitCounts(uint64_t bits)
{
    uint32_t num = 0;
    for (; bits != 0; bits &= bits - 1) {
        num++;
    }
    return num;
}

static bool IsValidChLayout(AudioChannelLayout &chLayout, uint32_t chCounts)
{
    if (chCounts < MONO || chCounts > CHANNEL_16) {
        return false;
    }
    if (chLayout == CH_LAYOUT_UNKNOWN || BitCounts(chLayout) != chCounts) {
        chLayout = SetDefaultChannelLayout((AudioChannel)chCounts);
    }
    return true;
}

static AudioChannelLayout SetDefaultChannelLayout(AudioChannel channels)
{
    if (channels < MONO || channels > CHANNEL_16) {
        return CH_LAYOUT_UNKNOWN;
    }
    switch (channels) {
        case MONO:
            return CH_LAYOUT_MONO;
        case STEREO:
            return CH_LAYOUT_STEREO;
        case CHANNEL_3:
            return CH_LAYOUT_SURROUND;
        case CHANNEL_4:
            return CH_LAYOUT_3POINT1;
        case CHANNEL_5:
            return CH_LAYOUT_4POINT1;
        case CHANNEL_6:
            return CH_LAYOUT_5POINT1;
        case CHANNEL_7:
            return CH_LAYOUT_6POINT1;
        case CHANNEL_8:
            return CH_LAYOUT_5POINT1POINT2;
        case CHANNEL_10:
            return CH_LAYOUT_7POINT1POINT2;
        case CHANNEL_12:
            return CH_LAYOUT_7POINT1POINT4;
        case CHANNEL_14:
            return CH_LAYOUT_9POINT1POINT4;
        case CHANNEL_16:
            return CH_LAYOUT_9POINT1POINT6;
        default:
            return CH_LAYOUT_UNKNOWN;
    }
}

} // HPAE
} // AudioStandard
} // OHOS