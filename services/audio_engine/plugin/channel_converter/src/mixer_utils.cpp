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
#include <cinttypes>
#include <map>
#include "audio_engine_log.h"
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
        "Must at least have FRONR_LEFT and FRONT_RIGHT", inChannelInfo.channelLayout, outChannelInfo.channelLayout);

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
} // HPAE
} // AudioStandard
} // OHOS