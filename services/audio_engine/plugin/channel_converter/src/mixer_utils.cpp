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
#define LOG_TAG "HpaeMixerUtils"
#endif
#include "mixer_utils.h"
#include <algorithm>
#include <cinttypes>
#include <cmath>
#include <map>
#include <mutex>
#include <new>
#include "audio_engine_log.h"
#include "securec.h"
// USE_ARM_NEON for the PcmLimiter NEON paths below is defined by architecture in this
// header (also pulled in by mixer_utils.h; included directly to keep the dependency explicit).
#include "audio_proresampler_process.h"
namespace OHOS {
namespace AudioStandard {
namespace HPAE {
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

static std::map<AudioChannel, AudioChannelLayout> DEFAULT_CHANNEL_MAP = {
    {MONO, CH_LAYOUT_MONO},
    {STEREO, CH_LAYOUT_STEREO},
    {CHANNEL_3, CH_LAYOUT_SURROUND},
    {CHANNEL_4, CH_LAYOUT_3POINT1},
    {CHANNEL_5, CH_LAYOUT_4POINT1},
    {CHANNEL_6, CH_LAYOUT_5POINT1},
    {CHANNEL_7, CH_LAYOUT_6POINT1},
    {CHANNEL_8, CH_LAYOUT_5POINT1POINT2},
    {CHANNEL_9, CH_LAYOUT_HOA_ORDER2_ACN_N3D},
    {CHANNEL_10, CH_LAYOUT_7POINT1POINT2},
    {CHANNEL_12, CH_LAYOUT_7POINT1POINT4},
    {CHANNEL_14, CH_LAYOUT_9POINT1POINT4},
    {CHANNEL_16, CH_LAYOUT_9POINT1POINT6}
};

static std::set<AudioChannelLayout> HOA_SET = {
    CH_LAYOUT_HOA_ORDER1_ACN_N3D, CH_LAYOUT_HOA_ORDER1_ACN_SN3D, CH_LAYOUT_HOA_ORDER1_FUMA,
    CH_LAYOUT_HOA_ORDER2_ACN_N3D, CH_LAYOUT_HOA_ORDER2_ACN_SN3D, CH_LAYOUT_HOA_ORDER2_FUMA,
    CH_LAYOUT_HOA_ORDER3_ACN_N3D, CH_LAYOUT_HOA_ORDER3_ACN_SN3D, CH_LAYOUT_HOA_ORDER3_FUMA,
};

uint32_t BitCounts(uint64_t bits)
{
    uint32_t num = 0;
    for (; bits != 0; bits &= bits - 1) {
        num++;
    }
    return num;
}

static void MixLfe(float (&coeffTable)[MAX_CHANNELS][MAX_CHANNELS], std::pair<uint32_t, uint64_t> posToBit,
    std::pair<uint64_t, uint64_t> chMskPair, std::map<uint64_t, uint32_t> &channelPosMap);
static void MixMidFront(float (&coeffTable)[MAX_CHANNELS][MAX_CHANNELS], std::pair<uint32_t, uint64_t> posToBit,
    uint64_t outChLayout, std::map<uint64_t, uint32_t> &channelPosMap);
static void MixMidRear(float (&coeffTable)[MAX_CHANNELS][MAX_CHANNELS], std::pair<uint32_t, uint64_t> posToBit,
    uint64_t outChLayout, std::map<uint64_t, uint32_t> &channelPosMap);
static void MixTopCenter(float (&coeffTable)[MAX_CHANNELS][MAX_CHANNELS], std::pair<uint32_t, uint64_t> posToBit,
    uint64_t outChLayout, std::map<uint64_t, uint32_t> &channelPosMap);
static void MixTopFront(float (&coeffTable)[MAX_CHANNELS][MAX_CHANNELS], std::pair<uint32_t, uint64_t> posToBit,
    uint64_t outChLayout, std::map<uint64_t, uint32_t> &channelPosMap);
static void MixTopRear(float (&coeffTable)[MAX_CHANNELS][MAX_CHANNELS], std::pair<uint32_t, uint64_t> posToBit,
    uint64_t outChLayout, std::map<uint64_t, uint32_t> &channelPosMap);
static void MixBottom(float (&coeffTable)[MAX_CHANNELS][MAX_CHANNELS], std::pair<uint32_t, uint64_t> posToBit,
    uint64_t outChLayout, std::map<uint64_t, uint32_t> &channelPosMap);

// channel masks for downmixing general output channel layout
/**** helper functions for setting up general mixing table ***/
// inBit: switch for output
static void MixMidFrontInner(float (&coeffTable)[MAX_CHANNELS][MAX_CHANNELS], std::pair<uint32_t, uint64_t> posToBit,
    uint64_t outChLayout, std::map<uint64_t, uint32_t> &channelPosMap)
{
    uint32_t inPos = posToBit.first;
    uint64_t inBit = posToBit.second;
    switch (inBit) {
        case WIDE_LEFT:
            if (outChLayout & WIDE_LEFT) {
                coeffTable[channelPosMap[WIDE_LEFT]][inPos] = COEF_0DB_F;
            } else if (outChLayout & FRONT_LEFT) {
                coeffTable[channelPosMap[FRONT_LEFT]][inPos] = COEF_0DB_F;
            }
            break;
        case WIDE_RIGHT:
            if (outChLayout & WIDE_RIGHT) {
                coeffTable[channelPosMap[WIDE_RIGHT]][inPos] = COEF_0DB_F;
            } else if (outChLayout & FRONT_RIGHT) {
                coeffTable[channelPosMap[FRONT_RIGHT]][inPos] = COEF_0DB_F;
            }
            break;
        default:
            break;
    }
}
static void MixMidFront(float (&coeffTable)[MAX_CHANNELS][MAX_CHANNELS], std::pair<uint32_t, uint64_t> posToBit,
    uint64_t outChLayout, std::map<uint64_t, uint32_t> &channelPosMap)
{
    uint32_t inPos = posToBit.first;
    uint64_t inBit = posToBit.second;
    switch (inBit) {
        // if falls here, the output channel layout must at least contains front left and front right
        case FRONT_LEFT:
            coeffTable[channelPosMap[FRONT_LEFT]][inPos] = COEF_0DB_F;
            break;
        case FRONT_RIGHT:
            coeffTable[channelPosMap[FRONT_RIGHT]][inPos] = COEF_0DB_F;
            break;
        case FRONT_CENTER:
            if (outChLayout & FRONT_CENTER) {
                coeffTable[channelPosMap[FRONT_CENTER]][inPos] = COEF_0DB_F;
            } else if ((outChLayout & FRONT_LEFT_OF_CENTER) && (outChLayout & FRONT_RIGHT_OF_CENTER)) {
                coeffTable[channelPosMap[FRONT_LEFT_OF_CENTER]][inPos] = COEF_M3DB_F;
                coeffTable[channelPosMap[FRONT_RIGHT_OF_CENTER]][inPos] = COEF_M3DB_F;
            } else if ((outChLayout & FRONT_LEFT) && (outChLayout & FRONT_RIGHT)) {
                coeffTable[channelPosMap[FRONT_LEFT]][inPos] = COEF_M3DB_F;
                coeffTable[channelPosMap[FRONT_RIGHT]][inPos] = COEF_M3DB_F;
            }
            break;
        case FRONT_LEFT_OF_CENTER:
            if ((outChLayout & FRONT_CENTER) && (outChLayout & FRONT_LEFT)) {
                coeffTable[channelPosMap[FRONT_CENTER]][inPos] = COEF_M3DB_F;
                coeffTable[channelPosMap[FRONT_LEFT]][inPos] = COEF_M3DB_F;
            } else if (outChLayout & FRONT_LEFT) {
                coeffTable[channelPosMap[FRONT_LEFT]][inPos] = COEF_0DB_F;
            }
            break;
        case FRONT_RIGHT_OF_CENTER:
            if ((outChLayout & FRONT_CENTER) && (outChLayout & FRONT_RIGHT)) {
                coeffTable[channelPosMap[FRONT_CENTER]][inPos] = COEF_M3DB_F;
                coeffTable[channelPosMap[FRONT_RIGHT]][inPos] = COEF_M3DB_F;
            } else if (outChLayout & FRONT_RIGHT) {
                coeffTable[channelPosMap[FRONT_RIGHT]][inPos] = COEF_0DB_F;
            }
            break;
        default:
            MixMidFrontInner(coeffTable, posToBit, outChLayout, channelPosMap);
            break;
    }
}
static void MixMidRearInner(float (&coeffTable)[MAX_CHANNELS][MAX_CHANNELS], std::pair<uint32_t, uint64_t> posToBit,
    uint64_t outChLayout, std::map<uint64_t, uint32_t> &channelPosMap)
{
    uint32_t inPos = posToBit.first;
    uint64_t inBit = posToBit.second;
    switch (inBit) {
        case SIDE_LEFT:
            if (outChLayout & SIDE_LEFT) {
                coeffTable[channelPosMap[SIDE_LEFT]][inPos] = COEF_0DB_F;
            } else if (outChLayout & BACK_LEFT) {
                coeffTable[channelPosMap[BACK_LEFT]][inPos] = COEF_0DB_F;
            } else if (outChLayout & BACK_CENTER) {
                coeffTable[channelPosMap[BACK_CENTER]][inPos] = COEF_0DB_F;
            } else {
                MixMidFront(coeffTable, {inPos, WIDE_LEFT}, outChLayout, channelPosMap);
            }
            break;
        case SIDE_RIGHT:
            if (outChLayout & SIDE_RIGHT) {
                coeffTable[channelPosMap[SIDE_RIGHT]][inPos] = COEF_0DB_F;
            } else if (outChLayout & BACK_RIGHT) {
                coeffTable[channelPosMap[BACK_RIGHT]][inPos] = COEF_0DB_F;
            } else if (outChLayout & BACK_CENTER) {
                coeffTable[channelPosMap[BACK_CENTER]][inPos] = COEF_0DB_F;
            } else {
                MixMidFront(coeffTable, {inPos, WIDE_RIGHT}, outChLayout, channelPosMap);
            }
            break;
        case BACK_RIGHT:
            if (outChLayout & BACK_RIGHT) {
                coeffTable[channelPosMap[BACK_RIGHT]][inPos] = COEF_0DB_F;
            } else if (outChLayout & SIDE_RIGHT) {
                coeffTable[channelPosMap[SIDE_RIGHT]][inPos] = COEF_0DB_F;
            } else if (outChLayout & BACK_CENTER) {
                coeffTable[channelPosMap[BACK_CENTER]][inPos] = COEF_0DB_F;
            } else {
                MixMidFront(coeffTable, {inPos, WIDE_RIGHT}, outChLayout, channelPosMap);
            }
            break;
        default:
            break;
    }
}
static void MixMidRear(float (&coeffTable)[MAX_CHANNELS][MAX_CHANNELS], std::pair<uint32_t, uint64_t> posToBit,
    uint64_t outChLayout, std::map<uint64_t, uint32_t> &channelPosMap)
{
    uint32_t inPos = posToBit.first;
    uint64_t inBit = posToBit.second;
    switch (inBit) {
        case BACK_CENTER:
            if ((outChLayout & BACK_CENTER)) {
                coeffTable[channelPosMap[BACK_CENTER]][inPos] = COEF_0DB_F;
            } else if ((outChLayout & BACK_LEFT) && (outChLayout & BACK_RIGHT)) {
                coeffTable[channelPosMap[BACK_LEFT]][inPos] = COEF_M3DB_F;
                coeffTable[channelPosMap[BACK_RIGHT]][inPos] = COEF_M3DB_F;
            } else if ((outChLayout & SIDE_LEFT) && (outChLayout & SIDE_RIGHT)) {
                coeffTable[channelPosMap[SIDE_LEFT]][inPos] = COEF_M3DB_F;
                coeffTable[channelPosMap[SIDE_RIGHT]][inPos] = COEF_M3DB_F;
            } else if ((outChLayout & WIDE_LEFT) && (outChLayout & WIDE_RIGHT)) {
                coeffTable[channelPosMap[WIDE_LEFT]][inPos] = COEF_M3DB_F;
                coeffTable[channelPosMap[WIDE_RIGHT]][inPos] = COEF_M3DB_F;
            } else if ((outChLayout & FRONT_LEFT) && (outChLayout & FRONT_RIGHT)) {
                coeffTable[channelPosMap[FRONT_LEFT]][inPos] = COEF_M3DB_F;
                coeffTable[channelPosMap[FRONT_RIGHT]][inPos] = COEF_M3DB_F;
            }
            break;
        case BACK_LEFT:
            if (outChLayout & BACK_LEFT) {
                coeffTable[channelPosMap[BACK_LEFT]][inPos] = COEF_0DB_F;
            } else if (outChLayout & SIDE_LEFT) {
                coeffTable[channelPosMap[SIDE_LEFT]][inPos] = COEF_0DB_F;
            } else if (outChLayout & BACK_CENTER) {
                coeffTable[channelPosMap[BACK_CENTER]][inPos] = COEF_0DB_F;
            } else {
                MixMidFront(coeffTable, {inPos, WIDE_LEFT}, outChLayout, channelPosMap);
            }
            break;
        default:
            MixMidRearInner(coeffTable, posToBit, outChLayout, channelPosMap);
            break;
    }
}

static void MixBottom(float (&coeffTable)[MAX_CHANNELS][MAX_CHANNELS], std::pair<uint32_t, uint64_t> posToBit,
    uint64_t outChLayout, std::map<uint64_t, uint32_t> &channelPosMap)
{
    uint32_t inPos = posToBit.first;
    uint64_t inBit = posToBit.second;
    uint64_t existBottomOutput = (outChLayout & MASK_BOTTOM);
    if (existBottomOutput) {
        switch (inBit) {
            case (BOTTOM_FRONT_CENTER):
                if ((outChLayout & BOTTOM_FRONT_LEFT) || (outChLayout & BOTTOM_FRONT_RIGHT)) {
                    coeffTable[channelPosMap[BOTTOM_FRONT_LEFT]][inPos] = COEF_M3DB_F;
                    coeffTable[channelPosMap[BOTTOM_FRONT_RIGHT]][inPos] = COEF_M3DB_F;
                }
                break;
            case (BOTTOM_FRONT_LEFT):
                if (outChLayout & BOTTOM_FRONT_CENTER) {
                    coeffTable[channelPosMap[BOTTOM_FRONT_CENTER]][inPos] = COEF_0DB_F;
                }
                break;
            case (BOTTOM_FRONT_RIGHT):
                if (outChLayout & BOTTOM_FRONT_CENTER) {
                    coeffTable[channelPosMap[BOTTOM_FRONT_CENTER]][inPos] = COEF_0DB_F;
                }
                break;
            default:
                break;
        }
    } else {
        switch (inBit) {
            case (BOTTOM_FRONT_CENTER):
                MixMidFront(coeffTable, {inPos, FRONT_CENTER}, outChLayout, channelPosMap);
                break;
            case (BOTTOM_FRONT_LEFT):
                MixMidFront(coeffTable, {inPos, FRONT_LEFT}, outChLayout, channelPosMap);
                break;
            case (BOTTOM_FRONT_RIGHT):
                MixMidFront(coeffTable, {inPos, FRONT_RIGHT}, outChLayout, channelPosMap);
                break;
            default:
                break;
        }
    }
}

static void MixTopCenter(float (&coeffTable)[MAX_CHANNELS][MAX_CHANNELS], std::pair<uint32_t, uint64_t> posToBit,
    uint64_t outChLayout, std::map<uint64_t, uint32_t> &channelPosMap)
{
    uint32_t inPos = posToBit.first;
    uint64_t existMiddleOuts =  outChLayout & (MASK_MIDDLE_FRONT | MASK_MIDDLE_REAR);
    uint64_t existTopOuts =  outChLayout & (MASK_TOP_FRONT | MASK_TOP_REAR);
    if (existTopOuts) {
        uint64_t numChannels = BitCounts(existTopOuts);
        float coeff = 1.0f / sqrt(static_cast<float>(numChannels));
        uint64_t bitMsk = existTopOuts;
        for (uint32_t i = 0; i < numChannels; i++) {
            uint64_t outBit = bitMsk & (~bitMsk + 1);
            coeffTable[channelPosMap[outBit]][inPos] = coeff;
            bitMsk ^= outBit;
        }
    } else if (existMiddleOuts) {
        uint64_t numChannels = BitCounts(existMiddleOuts);
        float coeff = 1.0f / sqrt(static_cast<float>(numChannels));
        uint64_t bitMsk = existMiddleOuts;
        for (uint32_t i = 0; i < numChannels; i++) {
            uint64_t outBit = bitMsk & (~bitMsk + 1);
            coeffTable[channelPosMap[outBit]][inPos] = coeff;
            bitMsk ^= outBit;
        }
    }
}

static void MixTopFront(float (&coeffTable)[MAX_CHANNELS][MAX_CHANNELS], std::pair<uint32_t, uint64_t> posToBit,
    uint64_t outChLayout, std::map<uint64_t, uint32_t> &channelPosMap)
{
    uint64_t existTopFrontOut = outChLayout & MASK_TOP_FRONT;
    uint32_t inPos = posToBit.first;
    uint64_t inBit = posToBit.second;
    if (existTopFrontOut) {
        switch (inBit) {
            case TOP_FRONT_CENTER:
                if ((outChLayout & TOP_FRONT_LEFT) && (outChLayout & TOP_FRONT_RIGHT)) {
                    coeffTable[channelPosMap[TOP_FRONT_LEFT]][inPos] = COEF_M3DB_F;
                    coeffTable[channelPosMap[TOP_FRONT_RIGHT]][inPos] = COEF_M3DB_F;
                }
                break;
            case TOP_FRONT_LEFT:
                if (outChLayout & TOP_FRONT_CENTER) {
                    coeffTable[channelPosMap[TOP_FRONT_CENTER]][inPos] = COEF_0DB_F;
                }
                break;
            case TOP_FRONT_RIGHT:
                if (outChLayout & TOP_FRONT_CENTER) {
                    coeffTable[channelPosMap[TOP_FRONT_CENTER]][inPos] = COEF_0DB_F;
                }
                break;
            default:
                break;
        }
    } else {
        switch (inBit) {
            case TOP_FRONT_CENTER:
                MixMidFront(coeffTable, {inPos, FRONT_CENTER}, outChLayout, channelPosMap);
                break;
            case TOP_FRONT_LEFT:
                MixMidFront(coeffTable, {inPos, FRONT_LEFT}, outChLayout, channelPosMap);
                break;
            case TOP_FRONT_RIGHT:
                MixMidFront(coeffTable, {inPos, FRONT_RIGHT}, outChLayout, channelPosMap);
                break;
            default:
                break;
        }
    }
}
static void MixTopRearexistTopRearOuts(float (&coeffTable)[MAX_CHANNELS][MAX_CHANNELS],
    std::pair<uint32_t, uint64_t> posToBit, uint64_t outChLayout, std::map<uint64_t, uint32_t> &channelPosMap)
{
    uint32_t inPos = posToBit.first;
    uint64_t inBit = posToBit.second;
    switch (inBit) {
        case TOP_BACK_CENTER:
            if ((outChLayout & TOP_BACK_LEFT) && (outChLayout & TOP_BACK_RIGHT)) {
                coeffTable[channelPosMap[TOP_BACK_LEFT]][inPos] = COEF_M3DB_F;
                coeffTable[channelPosMap[TOP_BACK_RIGHT]][inPos] = COEF_M3DB_F;
            } else if ((outChLayout & TOP_SIDE_LEFT) && (outChLayout & TOP_SIDE_RIGHT)) {
                coeffTable[channelPosMap[TOP_SIDE_LEFT]][inPos] = COEF_M3DB_F;
                coeffTable[channelPosMap[TOP_SIDE_RIGHT]][inPos] = COEF_M3DB_F;
            }
            break;
        case TOP_BACK_LEFT:
            if (outChLayout & TOP_SIDE_LEFT) {
                coeffTable[channelPosMap[TOP_SIDE_LEFT]][inPos] = COEF_0DB_F;
            } else if (outChLayout & TOP_BACK_CENTER) {
                coeffTable[channelPosMap[TOP_BACK_CENTER]][inPos] = COEF_0DB_F;
            }
            break;
        case TOP_BACK_RIGHT:
            if (outChLayout & TOP_SIDE_RIGHT) {
                coeffTable[channelPosMap[TOP_SIDE_RIGHT]][inPos] = COEF_0DB_F;
            } else if (outChLayout & TOP_BACK_CENTER) {
                coeffTable[channelPosMap[TOP_BACK_CENTER]][inPos] = COEF_0DB_F;
            }
            break;
        case TOP_SIDE_LEFT:
            if (outChLayout & TOP_BACK_LEFT) {
                coeffTable[channelPosMap[TOP_BACK_LEFT]][inPos] = COEF_0DB_F;
            } else if (outChLayout & TOP_BACK_CENTER) {
                coeffTable[channelPosMap[TOP_BACK_CENTER]][inPos] = COEF_0DB_F;
            }
            break;
        case TOP_SIDE_RIGHT:
            if (outChLayout & TOP_BACK_RIGHT) {
                coeffTable[channelPosMap[TOP_BACK_RIGHT]][inPos] = COEF_0DB_F;
            } else if (outChLayout & TOP_BACK_CENTER) {
                coeffTable[channelPosMap[TOP_BACK_CENTER]][inPos] = COEF_0DB_F;
            }
            break;
        default:
            break;
    }
}

static void MixTopRearOthers(float (&coeffTable)[MAX_CHANNELS][MAX_CHANNELS], std::pair<uint32_t, uint64_t> posToBit,
    uint64_t outChLayout, std::map<uint64_t, uint32_t> &channelPosMap)
{
    uint32_t inPos = posToBit.first;
    uint64_t inBit = posToBit.second;
    switch (inBit) {
        case TOP_BACK_CENTER:
            MixMidRear(coeffTable, {inPos, BACK_CENTER}, outChLayout, channelPosMap);
            break;
        case TOP_BACK_LEFT:
            MixMidRear(coeffTable, {inPos, BACK_LEFT}, outChLayout, channelPosMap);
            break;
        case TOP_BACK_RIGHT:
            MixMidRear(coeffTable, {inPos, BACK_RIGHT}, outChLayout, channelPosMap);
            break;
        case TOP_SIDE_LEFT:
            MixMidRear(coeffTable, {inPos, SIDE_LEFT}, outChLayout, channelPosMap);
            break;
        case TOP_SIDE_RIGHT:
            MixMidRear(coeffTable, {inPos, SIDE_RIGHT}, outChLayout, channelPosMap);
            break;
        default:
            break;
    }
}
static void MixTopRear(float (&coeffTable)[MAX_CHANNELS][MAX_CHANNELS], std::pair<uint32_t, uint64_t> posToBit,
    uint64_t outChLayout, std::map<uint64_t, uint32_t> &channelPosMap)
{
    uint64_t existTopRearOuts = outChLayout & MASK_TOP_REAR;
    if (existTopRearOuts) {
        MixTopRearexistTopRearOuts(coeffTable, posToBit, outChLayout, channelPosMap);
    } else {
        MixTopRearOthers(coeffTable, posToBit, outChLayout, channelPosMap);
    }
}

static void MixLfe(float (&coeffTable)[MAX_CHANNELS][MAX_CHANNELS], std::pair<uint32_t, uint64_t> posToBit,
    std::pair<uint64_t, uint64_t> chMskPair, std::map<uint64_t, uint32_t> &channelPosMap)
{
    uint64_t outChLayout = chMskPair.second;
    uint64_t existLfeOuts = outChLayout & MASK_LFE;
    uint64_t existLfe2In = chMskPair.first & LOW_FREQUENCY_2;
    uint32_t inPos = posToBit.first;
    uint64_t inBit = posToBit.second;
    if (existLfeOuts) {
        switch (inBit) {
            case LOW_FREQUENCY:
                coeffTable[channelPosMap[LOW_FREQUENCY_2]][inPos] = COEF_0DB_F;
                break;
            case LOW_FREQUENCY_2:
                coeffTable[channelPosMap[LOW_FREQUENCY]][inPos] = COEF_0DB_F;
                break;
            default:
                break;
        }
    } else {
        switch (inBit) {
            case LOW_FREQUENCY_2:
                if ((outChLayout & FRONT_LEFT) && (outChLayout & FRONT_RIGHT)) {
                    coeffTable[channelPosMap[FRONT_LEFT]][inPos] = COEF_ZERO_F;
                    coeffTable[channelPosMap[FRONT_RIGHT]][inPos] = COEF_M6DB_F;
                }
                break;
            case LOW_FREQUENCY:
                if (existLfe2In && (outChLayout & FRONT_LEFT) && (outChLayout & FRONT_RIGHT)) {
                    coeffTable[channelPosMap[FRONT_LEFT]][inPos] = COEF_M6DB_F;
                    coeffTable[channelPosMap[FRONT_RIGHT]][inPos] = COEF_ZERO_F;
                } else if ((outChLayout & FRONT_LEFT) && (outChLayout & FRONT_RIGHT)) {
                    coeffTable[channelPosMap[FRONT_LEFT]][inPos] = COEF_M6DB_F;
                    coeffTable[channelPosMap[FRONT_RIGHT]][inPos] = COEF_M6DB_F;
                }
                break;
            default:
                break;
        }
    }
}

static int32_t SetUpGeneralMixingTableInner(float (&coeffTable)[MAX_CHANNELS][MAX_CHANNELS],
    AudioChannelInfo inChannelInfo, AudioChannelInfo outChannelInfo, bool mixLfe)
{
    AudioChannelInfo dstChannelInfo;
    AudioChannelInfo srcChannelInfo;
    // general table made up rule: if input ch positon is output ch positon, coefficient is 1
    if (inChannelInfo.numChannels > outChannelInfo.numChannels) { // downmix
        dstChannelInfo = outChannelInfo;
        srcChannelInfo = inChannelInfo;
    } else { // upmix
        dstChannelInfo = inChannelInfo;
        srcChannelInfo = outChannelInfo;
    }
    if (dstChannelInfo.numChannels > MAX_CHANNELS) {
        return MIX_ERR_INVALID_ARG;
    }
    // Get position map of dst channels in dstChMsk
    std::map<uint64_t, uint32_t> channelPosMap;
    uint64_t dstChMsk = dstChannelInfo.channelLayout;
    for (uint32_t i = 0; i < dstChannelInfo.numChannels; i++) {
        uint64_t dstBit = dstChMsk & (~dstChMsk + 1);
        channelPosMap[dstBit] = i;
        dstChMsk ^= dstBit;
    }

    dstChMsk = dstChannelInfo.channelLayout;
    uint64_t srcChMsk = srcChannelInfo.channelLayout;
    for (uint32_t i = 0; i < srcChannelInfo.numChannels; i++) {
        uint64_t inBit = srcChMsk & (~srcChMsk + 1);
        if (inBit & dstChMsk) { // if in channel and out channel is the same
            coeffTable[channelPosMap[inBit]][i] = COEF_0DB_F;
        } else if (inBit == TOP_CENTER) {
            // check general downmix table!
            MixTopCenter(coeffTable, {i, inBit}, dstChMsk, channelPosMap);
        } else if ((inBit & MASK_MIDDLE_FRONT) != 0) {
            MixMidFront(coeffTable, {i, inBit}, dstChMsk, channelPosMap);
        } else if ((inBit & MASK_MIDDLE_REAR) != 0) {
            MixMidRear(coeffTable, {i, inBit}, dstChMsk, channelPosMap);
        } else if ((inBit & MASK_BOTTOM) != 0) {
            MixBottom(coeffTable, {i, inBit}, dstChMsk, channelPosMap);
        } else if (mixLfe && (inBit & MASK_LFE) != 0) {
            MixLfe(coeffTable, {i, inBit}, {srcChMsk, dstChMsk}, channelPosMap);
        } else if ((inBit & MASK_TOP_FRONT) != 0) {
            MixTopFront(coeffTable, {i, inBit}, dstChMsk, channelPosMap);
        } else if ((inBit & MASK_TOP_REAR) != 0) {
            MixTopRear(coeffTable, {i, inBit}, dstChMsk, channelPosMap);
        }
        srcChMsk ^= inBit;
    }
    return MIX_ERR_SUCCESS;
}

// coeffTable[outputIndex][inputIndex]
int32_t SetUpGeneralMixingTable(float (&coeffTable)[MAX_CHANNELS][MAX_CHANNELS], AudioChannelInfo inChannelInfo,
    AudioChannelInfo outChannelInfo, bool mixLfe)
{
    CHECK_AND_RETURN_RET_LOG(inChannelInfo.numChannels <= MAX_CHANNELS, MIX_ERR_INVALID_ARG,
        "column size of coeffTable not enough");
    CHECK_AND_RETURN_RET_LOG(outChannelInfo.numChannels <= MAX_CHANNELS, MIX_ERR_INVALID_ARG,
        "row size of coeffTable not enough");
    CHECK_AND_RETURN_RET_LOG(IsValidChLayout(inChannelInfo.channelLayout, inChannelInfo.numChannels),
        MIX_ERR_INVALID_ARG, "invalid input channel info");
    CHECK_AND_RETURN_RET_LOG(IsValidChLayout(outChannelInfo.channelLayout, outChannelInfo.numChannels),
        MIX_ERR_INVALID_ARG, "invalid output channel info");
    
    // for now, genneral mixer does not support HOA output
    CHECK_AND_RETURN_RET_LOG(!CheckIsHOA(outChannelInfo.channelLayout), MIX_ERR_INVALID_ARG,
        "mixer does not support HOA output");
    
    // for HOA intput, use the first channel input for every output channel
    if (CheckIsHOA(inChannelInfo.channelLayout)) {
        for (uint32_t i = 0; i < outChannelInfo.numChannels; i++) {
            coeffTable[i][0] = COEF_0DB_F;
        }
        return MIX_ERR_SUCCESS;
    }

    // when output is Mono, Mono downmix: add up all the intputs and normalize, nomalization will be done in downmixer
    // for downmix, coeffTable is used as coeffTable[out][in]
    if (outChannelInfo.channelLayout == CH_LAYOUT_MONO) {
        for (uint32_t i = 0; i < inChannelInfo.numChannels; i++) {
            coeffTable[0][i] = COEF_0DB_F;
        }
        return MIX_ERR_SUCCESS;
    }
    // when input is Mono, Mono upmix: copy Mono input too all the outputs
    // for upmix, we use coeffTable in transpose like coeffTable[in][out],
    if (inChannelInfo.channelLayout == CH_LAYOUT_MONO) {
        for (uint32_t i = 0; i < outChannelInfo.numChannels; i++) {
            coeffTable[0][i] = COEF_0DB_F;
        }
        return MIX_ERR_SUCCESS;
    }
    // here, input and output channellayout must at least have front left and front right
    CHECK_AND_RETURN_RET_LOG(((inChannelInfo.channelLayout & CH_LAYOUT_STEREO) == CH_LAYOUT_STEREO) &&
        ((outChannelInfo.channelLayout & CH_LAYOUT_STEREO) == CH_LAYOUT_STEREO), MIX_ERR_INVALID_ARG,
        "input channelLayout %{public}" PRIu64 " or output channelLayout %{public}" PRIu64 " is invalid. "
        "Must at least have FRONT_LEFT and FRONT_RIGHT", inChannelInfo.channelLayout, outChannelInfo.channelLayout);

    return SetUpGeneralMixingTableInner(coeffTable, inChannelInfo, outChannelInfo, mixLfe);
}

bool SetDefaultChannelLayout(AudioChannel channels, AudioChannelLayout &channelLayout)
{
    CHECK_AND_RETURN_RET_LOG(DEFAULT_CHANNEL_MAP.find(channels) != DEFAULT_CHANNEL_MAP.end(), false,
        "invalid channel count %{public}d", channels);
    channelLayout = DEFAULT_CHANNEL_MAP[channels];
    return true;
}

bool IsValidChLayout(AudioChannelLayout &chLayout, uint32_t chCounts)
{
    // for HOA, bitCount does not match channelCount
    CHECK_AND_RETURN_RET(!CheckIsHOA(chLayout), true);
    if (chLayout == CH_LAYOUT_UNKNOWN || BitCounts(chLayout) != chCounts) {
        return SetDefaultChannelLayout(static_cast<AudioChannel>(chCounts), chLayout);
    }
    return true;
}

bool CheckIsHOA(AudioChannelLayout layout)
{
    return (HOA_SET.find(layout) != HOA_SET.end());
}

namespace {
// Float-domain limiter ported from audio_service AudioLimiter (algorithm kept
// identical, dump-file logic stripped). Internal linkage: only the C mixer facade
// below uses it, never exported.
const float PCM_NEXT_LEVEL = 0.5f;
const float PCM_THRESHOLD = 0.92f;
const float PCM_LEVEL_ATTACK = 0.3f;
const float PCM_LEVEL_RELEASE = 0.7f;
const float PCM_GAIN_ATTACK = 0.1f;
const float PCM_GAIN_RELEASE = 0.6f;
const int32_t PCM_LMT_PROC_COUNT = 4;          // process 4 times
const int32_t PCM_LMT_ALGO_CHANNEL = 2;        // 2 channel for stereo
const int32_t PCM_LMT_BYTE_PER_SAMPLE = 4;     // sizeof(float)
constexpr float PCM_FLOAT_EPS_NUM = 1e-6f;
constexpr uint32_t PCM_MS_PER_S = 1000;
#if USE_ARM_NEON == 1
const int32_t PCM_NEON_FRAME_PER_ITER = 4; // 4 stereo frame in a NEON process
const int32_t PCM_NEON_SAMPLE_PER_ITER = 8; // 8 sample in 4 stereo frame (LR)
const int32_t PCM_NEON_MONO_FRAME_PER_ITER = 4; // 4 mono sample in a NEON process
#endif

class PcmLimiter {
public:
    PcmLimiter();
    ~PcmLimiter() = default;
    int32_t SetConfig(int32_t inputFrameBytes, int32_t bytePerSample, int32_t sampleRate, int32_t channels);
    int32_t Process(int32_t inputSampleCount, float *inBuffer, float *outBuffer);
    uint32_t GetLatency() { return latency_; }

private:
    float CalculateEnvelopeEnergy(float *inBuffer);
    void ApplyGainToFrame(float *inBuffer, float *outBuffer, float &lastGain, float deltaGain);
    void ProcessAlgo(float *inBuffer, float *outBuffer);
    int32_t ValidateConfigParams(int32_t inputFrameBytes, int32_t bytePerSample, int32_t sampleRate,
        int32_t channels, int32_t &newAlgoFrameLen);
    int32_t AllocateHistoryBuffer(int32_t newAlgoFrameLen);
    void CalculateEnvelopeStereo(float *inBuffer, float &maxEnvelopeLevel);
    void CalculateEnvelopeMono(float *inBuffer, float &maxEnvelopeLevel);
    void ApplyGainStereo(float *inBuffer, float *outBuffer, float &lastGain, float deltaGain);
    void ApplyGainMono(float *inBuffer, float *outBuffer, float &lastGain, float deltaGain);
    uint32_t latency_ = 0;
    int32_t algoFrameLen_ = 0;
    float nextLev_ = 0.0f;
    float curMaxLev_ = 0.0f;
    float threshold_ = 0.0f;
    float gain_ = 0.0f;
    float levelAttack_ = 0.0f;
    float levelRelease_ = 0.0f;
    float gainAttack_ = 0.0f;
    float gainRelease_ = 0.0f;
    std::vector<float> bufHis_;
    uint32_t sampleRate_ = 0;
    int32_t channels_ = 0;
};

PcmLimiter::PcmLimiter()
{
    nextLev_ = PCM_NEXT_LEVEL;
    threshold_ = PCM_THRESHOLD;
    levelAttack_ = PCM_LEVEL_ATTACK;
    levelRelease_ = PCM_LEVEL_RELEASE;
    gainAttack_ = PCM_GAIN_ATTACK;
    gainRelease_ = PCM_GAIN_RELEASE;
    latency_ = 0;
    algoFrameLen_ = 0;
    curMaxLev_ = 0.0f;
    gain_ = 0.0f;
    channels_ = 0;
    sampleRate_ = 0;
    AUDIO_INFO_LOG("PcmLimiter");
}

int32_t PcmLimiter::ValidateConfigParams(int32_t inputFrameBytes, int32_t bytePerSample, int32_t sampleRate,
    int32_t channels, int32_t &newAlgoFrameLen)
{
    CHECK_AND_RETURN_RET_LOG(inputFrameBytes > 0 && sampleRate > 0 &&
                                 (channels == 1 || channels == PCM_LMT_ALGO_CHANNEL) &&
                                 bytePerSample == PCM_LMT_BYTE_PER_SAMPLE,
        MIX_ERR_INVALID_ARG,
        "Invalid input parameters");
    CHECK_AND_RETURN_RET_LOG((inputFrameBytes / bytePerSample) % PCM_LMT_PROC_COUNT == 0,
        MIX_ERR_INVALID_ARG,
        "Invalid inputFrameBytes, frameLen must be an even number, intput frameLen is %{public}d",
        inputFrameBytes / (bytePerSample * channels));

    newAlgoFrameLen = inputFrameBytes / (bytePerSample * PCM_LMT_PROC_COUNT);
    CHECK_AND_RETURN_RET_LOG(newAlgoFrameLen > 0,
        MIX_ERR_INVALID_ARG,
        "Invalid inputFrameBytes, min request frameBytes is %{public}d",
        bytePerSample * PCM_LMT_PROC_COUNT * channels);
    return MIX_ERR_SUCCESS;
}

int32_t PcmLimiter::AllocateHistoryBuffer(int32_t newAlgoFrameLen)
{
    if (bufHis_.capacity() < static_cast<size_t>(newAlgoFrameLen)) {
        bufHis_.assign(newAlgoFrameLen + 1, 0);
    } else {
        memset_s(bufHis_.data(), bufHis_.capacity() * sizeof(float), 0, bufHis_.capacity() * sizeof(float));
    }

    CHECK_AND_RETURN_RET_LOG(bufHis_.capacity() > static_cast<size_t>(newAlgoFrameLen),
        MIX_ERR_ALLOC_FAILED,
        "allocate limit algorithm buffer failed, buffer capacity %{public}zu, requestLen %{public}d",
        bufHis_.capacity(),
        newAlgoFrameLen);
    return MIX_ERR_SUCCESS;
}

int32_t PcmLimiter::SetConfig(int32_t inputFrameBytes, int32_t bytePerSample, int32_t sampleRate, int32_t channels)
{
    // reset
    algoFrameLen_ = 0;

    int32_t newAlgoFrameLen = 0;
    int32_t ret = ValidateConfigParams(inputFrameBytes, bytePerSample, sampleRate, channels, newAlgoFrameLen);
    CHECK_AND_RETURN_RET_LOG(ret == MIX_ERR_SUCCESS, ret, "SetConfig validate params failed");

    latency_ = static_cast<uint32_t>(newAlgoFrameLen * PCM_MS_PER_S / (sampleRate * channels));

    ret = AllocateHistoryBuffer(newAlgoFrameLen);
    CHECK_AND_RETURN_RET_LOG(ret == MIX_ERR_SUCCESS, ret, "SetConfig allocate history buffer failed");
    algoFrameLen_ = newAlgoFrameLen;

    sampleRate_ = static_cast<uint32_t>(sampleRate);
    channels_ = channels;

    AUDIO_INFO_LOG("SetConfig Success, inputFrameBytes = %{public}d, bytePerSample = %{public}d, sampleRate = "
                   "%{public}d, channels = %{public}d, newAlgoFrameLen = %{public}d, latency = %{public}d",
        inputFrameBytes,
        bytePerSample,
        sampleRate,
        channels,
        algoFrameLen_,
        latency_);

    return MIX_ERR_SUCCESS;
}

int32_t PcmLimiter::Process(int32_t inputSampleCount, float *inBuffer, float *outBuffer)
{
    CHECK_AND_RETURN_RET_LOG(
        inBuffer != nullptr && outBuffer != nullptr, MIX_ERR_INVALID_ARG, "PcmLimiter Process Error, buffer is nullptr");

    CHECK_AND_RETURN_RET_LOG(algoFrameLen_ > 0 && bufHis_.capacity() > static_cast<size_t>(algoFrameLen_),
        MIX_ERR_INVALID_ARG,
        "could not do process before SetConfig success");

    CHECK_AND_RETURN_RET_LOG(algoFrameLen_ * PCM_LMT_PROC_COUNT == inputSampleCount,
        MIX_ERR_INVALID_ARG,
        "error, requestSample = %{public}d, inputSample = %{public}d",
        algoFrameLen_ * PCM_LMT_PROC_COUNT,
        inputSampleCount);

    int32_t ptrIndex = 0;
    for (int32_t i = 0; i < PCM_LMT_PROC_COUNT; i++) {
        ProcessAlgo(inBuffer + ptrIndex, outBuffer + ptrIndex);
        ptrIndex += algoFrameLen_;
    }
    return MIX_ERR_SUCCESS;
}

void PcmLimiter::CalculateEnvelopeStereo(float *inBuffer, float &maxEnvelopeLevel)
{
    int32_t i = 0;
#if USE_ARM_NEON == 1
    for (; i <= algoFrameLen_ - PCM_NEON_SAMPLE_PER_ITER; i += PCM_NEON_SAMPLE_PER_ITER) {
        float32x4x2_t lr = vld2q_f32(&inBuffer[i]); // load interleaved stereo samples

        float32x4_t absL = vabsq_f32(lr.val[0]);
        float32x4_t absR = vabsq_f32(lr.val[1]);
        float32x4_t tempLevelVec = vmaxq_f32(absL, absR); // max value for each frame

        float tempLevelArr[PCM_NEON_FRAME_PER_ITER];
        vst1q_f32(tempLevelArr, tempLevelVec);

        // envelope update must be scalar because of data dependency
        for (int j = 0; j < PCM_NEON_FRAME_PER_ITER; ++j) {
            float tempLevel = tempLevelArr[j];
            float coeff = tempLevel > nextLev_ ? levelAttack_ : levelRelease_;
            nextLev_ = coeff * nextLev_ + (1.0f - coeff) * tempLevel;
            maxEnvelopeLevel = std::max(maxEnvelopeLevel, nextLev_);
        }
    }
#endif
    for (; i < algoFrameLen_ - 1; i += PCM_LMT_ALGO_CHANNEL) {
        float tempBufInLeft = inBuffer[i];
        float tempBufInRight = inBuffer[i + 1];
        float tempLevel = std::max(std::abs(tempBufInLeft), std::abs(tempBufInRight));
        float coeff = tempLevel > nextLev_ ? levelAttack_ : levelRelease_;
        nextLev_ = coeff * nextLev_ + (1 - coeff) * tempLevel;
        maxEnvelopeLevel = std::max(maxEnvelopeLevel, nextLev_);
    }
}

void PcmLimiter::CalculateEnvelopeMono(float *inBuffer, float &maxEnvelopeLevel)
{
    int32_t i = 0;
#if USE_ARM_NEON == 1
    for (; i <= algoFrameLen_ - PCM_NEON_MONO_FRAME_PER_ITER; i += PCM_NEON_MONO_FRAME_PER_ITER) {
        float32x4_t samples = vld1q_f32(&inBuffer[i]);
        float32x4_t absSamples = vabsq_f32(samples);

        float tempLevelArr[PCM_NEON_MONO_FRAME_PER_ITER];
        vst1q_f32(tempLevelArr, absSamples);

        for (int j = 0; j < PCM_NEON_MONO_FRAME_PER_ITER; ++j) {
            float tempLevel = tempLevelArr[j];
            float coeff = tempLevel > nextLev_ ? levelAttack_ : levelRelease_;
            nextLev_ = coeff * nextLev_ + (1.0f - coeff) * tempLevel;
            maxEnvelopeLevel = std::max(maxEnvelopeLevel, nextLev_);
        }
    }
#endif
    for (; i < algoFrameLen_; i += 1) {
        float tempLevel = std::abs(inBuffer[i]);
        float coeff = tempLevel > nextLev_ ? levelAttack_ : levelRelease_;
        nextLev_ = coeff * nextLev_ + (1 - coeff) * tempLevel;
        maxEnvelopeLevel = std::max(maxEnvelopeLevel, nextLev_);
    }
}

float PcmLimiter::CalculateEnvelopeEnergy(float *inBuffer)
{
    float maxEnvelopeLevel = 0.0f;
    if (channels_ == PCM_LMT_ALGO_CHANNEL) {
        CalculateEnvelopeStereo(inBuffer, maxEnvelopeLevel);
    } else {
        CalculateEnvelopeMono(inBuffer, maxEnvelopeLevel);
    }
    return maxEnvelopeLevel;
}

void PcmLimiter::ApplyGainStereo(float *inBuffer, float *outBuffer, float &lastGain, float deltaGain)
{
    int32_t i = 0;
#if USE_ARM_NEON == 1
    for (; i <= algoFrameLen_ - PCM_NEON_SAMPLE_PER_ITER; i += PCM_NEON_SAMPLE_PER_ITER) {
        // Load interleaved stereo input and history
        float32x4x2_t in_data  = vld2q_f32(&inBuffer[i]);
        float32x4x2_t his_data = vld2q_f32(&bufHis_[i]);

        // Scalar-equivalent gain progression
        lastGain += deltaGain;
        float g0 = lastGain;
        lastGain += deltaGain;
        float g1 = lastGain;
        lastGain += deltaGain;
        float g2 = lastGain;
        lastGain += deltaGain;
        float g3 = lastGain;

        // Pack gains into vector [g0 g1 g2 g3]
        float32x4_t gainVec = { g0, g1, g2, g3 };

        // Apply gain to history buffer
        float32x4x2_t out_data;
        out_data.val[0] = vmulq_f32(his_data.val[0], gainVec);
        out_data.val[1] = vmulq_f32(his_data.val[1], gainVec);

        // Store output
        vst2q_f32(&outBuffer[i], out_data);

        // Update history buffer with new input
        vst2q_f32(&bufHis_[i], in_data);
    }
#endif
    for (; i < algoFrameLen_ - 1; i += PCM_LMT_ALGO_CHANNEL) {
        lastGain += deltaGain;
        outBuffer[i] = bufHis_[i] * lastGain;
        outBuffer[i + 1] = bufHis_[i + 1] * lastGain;
        bufHis_[i] = inBuffer[i];
        bufHis_[i + 1] = inBuffer[i + 1];
    }
}

void PcmLimiter::ApplyGainMono(float *inBuffer, float *outBuffer, float &lastGain, float deltaGain)
{
    int32_t i = 0;
#if USE_ARM_NEON == 1
    for (; i <= algoFrameLen_ - PCM_NEON_MONO_FRAME_PER_ITER; i += PCM_NEON_MONO_FRAME_PER_ITER) {
        float32x4_t his_data = vld1q_f32(&bufHis_[i]);
        float32x4_t in_data  = vld1q_f32(&inBuffer[i]);

        // Scalar-equivalent gain progression for mono
        lastGain += deltaGain;
        float g0 = lastGain;
        lastGain += deltaGain;
        float g1 = lastGain;
        lastGain += deltaGain;
        float g2 = lastGain;
        lastGain += deltaGain;
        float g3 = lastGain;

        float32x4_t gainVec = { g0, g1, g2, g3 };

        float32x4_t out_data = vmulq_f32(his_data, gainVec);
        vst1q_f32(&outBuffer[i], out_data);
        vst1q_f32(&bufHis_[i], in_data);
    }
#endif
    for (; i < algoFrameLen_; i += 1) {
        lastGain += deltaGain;
        outBuffer[i] = bufHis_[i] * lastGain;
        bufHis_[i] = inBuffer[i];
    }
}

void PcmLimiter::ApplyGainToFrame(float *inBuffer, float *outBuffer, float &lastGain, float deltaGain)
{
    if (channels_ == PCM_LMT_ALGO_CHANNEL) {
        ApplyGainStereo(inBuffer, outBuffer, lastGain, deltaGain);
    } else {
        ApplyGainMono(inBuffer, outBuffer, lastGain, deltaGain);
    }
}

void PcmLimiter::ProcessAlgo(float *inBuffer, float *outBuffer)
{
    // calculate envelope energy
    float maxEnvelopeLevel = CalculateEnvelopeEnergy(inBuffer);

    // calculate gain
    float tempMaxLevel = std::max(maxEnvelopeLevel, curMaxLev_);
    curMaxLev_ = maxEnvelopeLevel;
    float targetGain = 1.0f;
    if (tempMaxLevel != 0 && tempMaxLevel > PCM_FLOAT_EPS_NUM) {
        targetGain = tempMaxLevel > threshold_ ? threshold_ / tempMaxLevel : targetGain;
    }
    float lastGain = gain_;
    float coeff = gain_ > targetGain ? gainAttack_ : gainRelease_;
    gain_ = coeff * gain_ + (1 - coeff) * targetGain;
    float deltaGain = (gain_ - lastGain) * channels_ / algoFrameLen_;

    // apply gain
    if (algoFrameLen_ % channels_ == 0) {
        ApplyGainToFrame(inBuffer, outBuffer, lastGain, deltaGain);
    } else {
        outBuffer[0] = bufHis_[0] * lastGain;
        bufHis_[0] = bufHis_[algoFrameLen_];
        for (int32_t i = 1; i < algoFrameLen_ - 1; i += channels_) {
            lastGain += deltaGain;
            outBuffer[i] = bufHis_[i] * lastGain;
            if (channels_ == PCM_LMT_ALGO_CHANNEL) {
                outBuffer[i + 1] = bufHis_[i + 1] * lastGain;
            }
            bufHis_[i] = inBuffer[i - 1];
            if (channels_ == PCM_LMT_ALGO_CHANNEL) {
                bufHis_[i + 1] = inBuffer[i];
            }
        }
        bufHis_[algoFrameLen_] = inBuffer[algoFrameLen_ - 1];
    }
}
} // namespace
} // HPAE
} // AudioStandard
} // OHOS

// C mixer facade declared in mixer_utils.h: mixes same-format streams in the float
// domain (per-stream volume applied before accumulation) and runs the limiter on the
// accumulated signal in the native channel count.
namespace {
constexpr int32_t MIXER_MAX_STREAM_COUNT = 1024;
constexpr int32_t MIXER_SAMPLE_RATE_MIN = 8000;
constexpr int32_t MIXER_SAMPLE_RATE_MAX = 384000;
constexpr int32_t MIXER_CHANNEL_MONO = 1;
constexpr int32_t MIXER_CHANNEL_STEREO = 2;
constexpr int32_t MIXER_DATA_MAX_MS = 60;
constexpr int32_t MIXER_MS_PER_SECOND = 1000;
constexpr int32_t MIXER_LIMITER_DIVISOR = 4; // limiter processes input in 4 chunks
constexpr float MIXER_VOLUME_MIN = 0.0f;
constexpr float MIXER_VOLUME_MAX = 1.0f;

int32_t GetMixerFormatBytes(AudioPcmFormat format)
{
    switch (format) {
        case AUDIO_PCM_S16_LE:
            return 2;
        case AUDIO_PCM_S24_LE:
            return 3;
        case AUDIO_PCM_S32_LE:
        case AUDIO_PCM_F32_LE:
            return 4;
        default:
            return 0;
    }
}

struct MixerContext {
    std::mutex mutex;
    int32_t sampleRate = 0;
    int32_t channels = 0;
    int32_t bytesPerSample = 0;
    AudioPcmFormat format = AUDIO_PCM_S16_LE;
    bool blockLocked = false;
    int32_t lockedSampleCount = 0;
    std::vector<float> mixBuf;     // accumulated mix, frames * channels
    std::vector<float> streamBuf;  // per-stream float conversion
    std::vector<float> limitBuf;   // limiter output, frames * channels
    std::vector<uint8_t> outBuf;   // interleaved pcm output
    OHOS::AudioStandard::HPAE::PcmLimiter limiter;
};

AudioMixResult MakeMixResult(uint8_t *data, int32_t count, int32_t errCode)
{
    AudioMixResult result = {};
    result.outData = data;
    result.outSampleCount = count;
    result.errCode = errCode;
    return result;
}

// Validates mixer params and per-stream fields (data, equal sampleCount, volume range);
// outputs the first stream's sampleCount. Runs before the context lock is taken.
int32_t ValidateMixInputs(AudioMixHandle handle, const AudioMixStream *streams, int32_t streamCount,
    int32_t &firstCount)
{
    if (handle == nullptr || streams == nullptr || streamCount <= 0 || streamCount > MIXER_MAX_STREAM_COUNT) {
        AUDIO_ERR_LOG("AudioMixerProcess invalid params, streamCount: %{public}d", streamCount);
        return AUDIO_PCM_ALGO_ERR_INVALID_PARAM;
    }
    firstCount = streams[0].sampleCount;
    if (firstCount <= 0) {
        AUDIO_ERR_LOG("AudioMixerProcess invalid sampleCount: %{public}d", firstCount);
        return AUDIO_PCM_ALGO_ERR_INVALID_PARAM;
    }
    for (int32_t i = 0; i < streamCount; ++i) {
        if (streams[i].data == nullptr || streams[i].sampleCount != firstCount) {
            AUDIO_ERR_LOG("AudioMixerProcess stream[%{public}d] invalid", i);
            return AUDIO_PCM_ALGO_ERR_INVALID_PARAM;
        }
        // inverted comparison: NaN fails both ordered comparisons, so !(...) rejects NaN too
        if (!(streams[i].volume >= MIXER_VOLUME_MIN && streams[i].volume <= MIXER_VOLUME_MAX)) {
            AUDIO_ERR_LOG("AudioMixerProcess stream[%{public}d] volume out of range", i);
            return AUDIO_PCM_ALGO_ERR_INVALID_PARAM;
        }
    }
    return AUDIO_PCM_ALGO_SUCCESS;
}

// Validates block geometry (alignment, 60ms cap, locked sampleCount) and prepares the
// limiter and work buffers on the first successful call. Caller must hold ctx->mutex.
int32_t EnsureMixBlock(MixerContext *ctx, int32_t firstCount, int32_t &totalSamples)
{
    totalSamples = firstCount * ctx->channels;
    if (totalSamples % MIXER_LIMITER_DIVISOR != 0) {
        AUDIO_ERR_LOG("AudioMixerProcess sampleCount*channel must be divisible by 4, total: %{public}d", totalSamples);
        return AUDIO_PCM_ALGO_ERR_INVALID_PARAM;
    }
    if (firstCount > ctx->sampleRate * MIXER_DATA_MAX_MS / MIXER_MS_PER_SECOND) {
        AUDIO_ERR_LOG("AudioMixerProcess sampleCount exceeds 60ms: %{public}d", firstCount);
        return AUDIO_PCM_ALGO_ERR_INVALID_PARAM;
    }
    if (ctx->blockLocked && firstCount != ctx->lockedSampleCount) {
        AUDIO_ERR_LOG("AudioMixerProcess sampleCount changed, locked: %{public}d, new: %{public}d",
            ctx->lockedSampleCount, firstCount);
        return AUDIO_PCM_ALGO_ERR_INVALID_PARAM;
    }
    if (!ctx->blockLocked) {
        int32_t ret = ctx->limiter.SetConfig(totalSamples * static_cast<int32_t>(sizeof(float)),
            static_cast<int32_t>(sizeof(float)), ctx->sampleRate, ctx->channels);
        if (ret != 0) { // PcmLimiter returns HPAE MIX_ERR_* codes, 0 on success
            AUDIO_ERR_LOG("AudioMixerProcess limiter SetConfig failed, ret: %{public}d", ret);
            return AUDIO_PCM_ALGO_ERR_OPERATION_FAILED;
        }
        ctx->streamBuf.resize(totalSamples);
        ctx->limitBuf.resize(totalSamples);
        ctx->outBuf.resize(static_cast<size_t>(totalSamples) * static_cast<size_t>(ctx->bytesPerSample));
        ctx->blockLocked = true;
        ctx->lockedSampleCount = firstCount;
    }
    return AUDIO_PCM_ALGO_SUCCESS;
}

// Converts each stream to float and accumulates it into mixBuf with its per-stream volume.
// Caller must hold ctx->mutex and zero mixBuf beforehand.
int32_t AccumulateMixStreams(MixerContext *ctx, const AudioMixStream *streams, int32_t streamCount,
    int32_t totalSamples)
{
    for (int32_t i = 0; i < streamCount; ++i) {
        int32_t ret = ConvertPcmToFloat(ctx->format, streams[i].data, totalSamples, ctx->streamBuf.data());
        if (ret != AUDIO_PCM_ALGO_SUCCESS) {
            AUDIO_ERR_LOG("AudioMixerProcess convert stream[%{public}d] failed, ret: %{public}d", i, ret);
            return ret;
        }
        float volume = streams[i].volume;
        const float *in = ctx->streamBuf.data();
        float *mix = ctx->mixBuf.data();
        if (volume == 1.0f) {
            for (int32_t j = 0; j < totalSamples; ++j) {
                mix[j] += in[j];
            }
        } else {
            for (int32_t j = 0; j < totalSamples; ++j) {
                mix[j] += in[j] * volume;
            }
        }
    }
    return AUDIO_PCM_ALGO_SUCCESS;
}
} // namespace

extern "C" AudioMixHandle AudioMixerCreate(int32_t sampleRate, int32_t channel, AudioPcmFormat format)
{
    if (sampleRate < MIXER_SAMPLE_RATE_MIN || sampleRate > MIXER_SAMPLE_RATE_MAX) {
        AUDIO_ERR_LOG("AudioMixerCreate unsupported sampleRate: %{public}d", sampleRate);
        return nullptr;
    }
    if (channel != MIXER_CHANNEL_MONO && channel != MIXER_CHANNEL_STEREO) {
        AUDIO_ERR_LOG("AudioMixerCreate unsupported channel: %{public}d", channel);
        return nullptr;
    }
    if (GetMixerFormatBytes(format) == 0) {
        AUDIO_ERR_LOG("AudioMixerCreate unsupported format: %{public}d", static_cast<int32_t>(format));
        return nullptr;
    }

    MixerContext *ctx = new (std::nothrow) MixerContext();
    if (ctx == nullptr) {
        AUDIO_ERR_LOG("AudioMixerCreate allocate context failed");
        return nullptr;
    }
    ctx->sampleRate = sampleRate;
    ctx->channels = channel;
    ctx->bytesPerSample = GetMixerFormatBytes(format);
    ctx->format = format;
    AUDIO_INFO_LOG("AudioMixerCreate success, rate: %{public}d, channels: %{public}d, bytesPerSample: %{public}d",
        ctx->sampleRate, ctx->channels, ctx->bytesPerSample);
    return static_cast<AudioMixHandle>(ctx);
}

extern "C" struct AudioMixResult AudioMixerProcess(AudioMixHandle handle, const struct AudioMixStream *streams,
    int32_t streamCount)
{
    int32_t firstCount = 0;
    int32_t ret = ValidateMixInputs(handle, streams, streamCount, firstCount);
    if (ret != AUDIO_PCM_ALGO_SUCCESS) {
        return MakeMixResult(nullptr, 0, ret);
    }

    MixerContext *ctx = static_cast<MixerContext *>(handle);
    std::lock_guard<std::mutex> lock(ctx->mutex);

    int32_t totalSamples = 0;
    ret = EnsureMixBlock(ctx, firstCount, totalSamples);
    if (ret != AUDIO_PCM_ALGO_SUCCESS) {
        return MakeMixResult(nullptr, 0, ret);
    }
    ctx->mixBuf.assign(totalSamples, 0.0f);

    ret = AccumulateMixStreams(ctx, streams, streamCount, totalSamples);
    if (ret != AUDIO_PCM_ALGO_SUCCESS) {
        return MakeMixResult(nullptr, 0, ret);
    }

    ret = ctx->limiter.Process(totalSamples, ctx->mixBuf.data(), ctx->limitBuf.data());
    if (ret != 0) { // PcmLimiter returns HPAE MIX_ERR_* codes, 0 on success
        AUDIO_ERR_LOG("AudioMixerProcess limiter failed, ret: %{public}d", ret);
        return MakeMixResult(nullptr, 0, AUDIO_PCM_ALGO_ERR_OPERATION_FAILED);
    }

    ret = ConvertFloatToPcm(ctx->format, ctx->limitBuf.data(), totalSamples, ctx->outBuf.data());
    if (ret != AUDIO_PCM_ALGO_SUCCESS) {
        AUDIO_ERR_LOG("AudioMixerProcess convert output failed, ret: %{public}d", ret);
        return MakeMixResult(nullptr, 0, ret);
    }

    return MakeMixResult(ctx->outBuf.data(), firstCount, AUDIO_PCM_ALGO_SUCCESS);
}

extern "C" void AudioMixerDestroy(AudioMixHandle handle)
{
    if (handle == nullptr) {
        AUDIO_ERR_LOG("AudioMixerDestroy handle is nullptr");
        return;
    }
    MixerContext *ctx = static_cast<MixerContext *>(handle);
    delete ctx;
    AUDIO_INFO_LOG("AudioMixerDestroy success");
}