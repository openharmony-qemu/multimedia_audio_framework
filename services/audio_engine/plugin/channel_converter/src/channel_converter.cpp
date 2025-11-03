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
#define LOG_TAG "HpaeChannelConverter"
#endif
#include "channel_converter.h"
#include "audio_engine_log.h"
namespace OHOS {
namespace AudioStandard {
namespace HPAE {
static constexpr uint32_t MAX_FRAME_LENGTH = SAMPLE_RATE_192000 * 10; // max framelength is sample rate 192000, 10s

static uint32_t GetFormatSize(AudioSampleFormat format)
{
    uint32_t sampleSize = 0;
    switch (format) {
        case SAMPLE_U8:
            sampleSize = sizeof(uint8_t);
            break;
        case SAMPLE_S16LE:
            sampleSize = sizeof(int16_t);
            break;
        case SAMPLE_S24LE:
            sampleSize = SAMPLE_S24LE + 1;
            break;
        case SAMPLE_S32LE:
            sampleSize = sizeof(int32_t);
            break;
        case SAMPLE_F32LE:
            sampleSize = sizeof(float);
            break;
        default:
            AUDIO_ERR_LOG("unsupported format %{public}d", format);
            break;
    }
    return sampleSize;
}

ChannelConverter::ChannelConverter() {}

int32_t ChannelConverter::SetParam(AudioChannelInfo inChannelInfo, AudioChannelInfo outChannelInfo,
    AudioSampleFormat format, bool mixLfe)
{
    isInitialized_ = false;
    inChannelInfo_.channelLayout = inChannelInfo.channelLayout;
    outChannelInfo_.channelLayout = outChannelInfo.channelLayout;
    inChannelInfo_.numChannels = inChannelInfo.numChannels;
    outChannelInfo_.numChannels = outChannelInfo.numChannels;
    CHECK_AND_RETURN_RET_LOG((inChannelInfo.numChannels >= 0) && (inChannelInfo.numChannels <= MAX_CHANNELS),
        MIX_ERR_INVALID_ARG, "invalid input channels");
    CHECK_AND_RETURN_RET_LOG((outChannelInfo.numChannels >= 0) && (outChannelInfo.numChannels <= MAX_CHANNELS),
        MIX_ERR_INVALID_ARG, "invalid output channels");
    workFormat_ = format;
    workSize_ = GetFormatSize(format);
    mixLfe_ = mixLfe;
    int32_t ret = MIX_ERR_SUCCESS;
    if (inChannelInfo_.numChannels > outChannelInfo_.numChannels) {
        ret = downMixer_.SetParam(inChannelInfo, outChannelInfo, workSize_, mixLfe);
        downMixer_.SetNormalization(downmixNormalizing_);
        downMixer_.GetDownMixTable(mixTable_);
    } else {
        ret = SetUpGeneralMixingTable(mixTable_, inChannelInfo_, outChannelInfo_, mixLfe_);
        UpmixGainAttenuation();
    }
    isInitialized_ = (ret == MIX_ERR_SUCCESS);
    return ret;
}

int32_t ChannelConverter::SetInChannelInfo(AudioChannelInfo inChannelInfo)
{
    inChannelInfo_.channelLayout = inChannelInfo.channelLayout;
    inChannelInfo_.numChannels = inChannelInfo.numChannels;
    std::fill(&mixTable_[0][0], &mixTable_[0][0] + MAX_CHANNELS * MAX_CHANNELS, 0.0f);
    int32_t ret = MIX_ERR_SUCCESS;
    if (inChannelInfo_.numChannels > outChannelInfo_.numChannels) {
        ret = downMixer_.SetParam(inChannelInfo_, outChannelInfo_, workSize_, mixLfe_);
        downMixer_.SetNormalization(downmixNormalizing_);
        downMixer_.GetDownMixTable(mixTable_);
    } else {
        ret = SetUpGeneralMixingTable(mixTable_, inChannelInfo_, outChannelInfo_, mixLfe_);
        UpmixGainAttenuation();
    }
    isInitialized_ = (ret == MIX_ERR_SUCCESS);
    return ret;
}
 
 
int32_t ChannelConverter::SetOutChannelInfo(AudioChannelInfo outChannelInfo)
{
    outChannelInfo_.channelLayout = outChannelInfo.channelLayout;
    outChannelInfo_.numChannels = outChannelInfo.numChannels;
    std::fill(&mixTable_[0][0], &mixTable_[0][0] + MAX_CHANNELS * MAX_CHANNELS, 0.0f);
    int32_t ret = MIX_ERR_SUCCESS;
    if (inChannelInfo_.numChannels > outChannelInfo_.numChannels) {
        ret = downMixer_.SetParam(inChannelInfo_, outChannelInfo_, workSize_, mixLfe_);
        downMixer_.SetNormalization(downmixNormalizing_);
        downMixer_.GetDownMixTable(mixTable_);
    } else {
        ret = SetUpGeneralMixingTable(mixTable_, inChannelInfo_, outChannelInfo_, mixLfe_);
        UpmixGainAttenuation();
    }
    isInitialized_ = (ret == MIX_ERR_SUCCESS);
    return ret;
}

AudioChannelInfo ChannelConverter::GetInChannelInfo() const
{
    return inChannelInfo_;
}
 
AudioChannelInfo ChannelConverter::GetOutChannelInfo() const
{
    return outChannelInfo_;
}

void ChannelConverter::GetMixTable(float (&coeffTable)[MAX_CHANNELS][MAX_CHANNELS]) const
{
    CHECK_AND_RETURN_LOG(isInitialized_, "mix table is not initialized!");
    for (uint32_t i = 0; i < MAX_CHANNELS; i++) {
        for (uint32_t j = 0; j < MAX_CHANNELS; j++) {
            coeffTable[i][j] = mixTable_[i][j];
        }
    }
}

void ChannelConverter::SetDownmixNormalization(bool normalizing)
{
    downmixNormalizing_ = normalizing;
    CHECK_AND_RETURN_LOG(inChannelInfo_.numChannels > outChannelInfo_.numChannels,
        "channelConverter is at upmix state, no need to set normalization for upmix");
    downMixer_.SetNormalization(downmixNormalizing_);
}
 
int32_t ChannelConverter::Process(uint32_t frameLen, float* in, uint32_t inByteSize, float* out, uint32_t outByteSize)
{
    CHECK_AND_RETURN_RET_LOG(isInitialized_, MIX_ERR_ALLOC_FAILED, "ChannelConverter is not initialized_");
    CHECK_AND_RETURN_RET_LOG(in, MIX_ERR_INVALID_ARG, "input pointer is nullptr");
    CHECK_AND_RETURN_RET_LOG(out, MIX_ERR_INVALID_ARG, "output pointer is nullptr");
    CHECK_AND_RETURN_RET_LOG(frameLen >= 0, MIX_ERR_INVALID_ARG, "invalid negative frameSize");
    CHECK_AND_RETURN_RET_LOG(frameLen <= MAX_FRAME_LENGTH, MIX_ERR_INVALID_ARG, "invalid frameSize oversize");

    uint32_t expectInByteSize = frameLen * inChannelInfo_.numChannels * workSize_;
    uint32_t expectOutByteSize = frameLen * outChannelInfo_.numChannels * workSize_;
    CHECK_AND_RETURN_RET_LOG(expectInByteSize <= inByteSize, MIX_ERR_INVALID_ARG, "expected byte size %{public}d "
        "smaller than input byte size %{public}d, cannot process", expectInByteSize, inByteSize);
    CHECK_AND_RETURN_RET_LOG(expectOutByteSize <= outByteSize, MIX_ERR_INVALID_ARG, "expected byte size %{public}d"
        "samller than output byte size %{public}d, cannot process", expectOutByteSize, outByteSize);

    // upmix
    if (inChannelInfo_.numChannels < outChannelInfo_.numChannels) {
        return MixProcess(false, frameLen, in, out);
    }
    // downmix
    return MixProcess(true, frameLen, in, out);
}

void ChannelConverter::Reset()
{
    isInitialized_ = false;
    downMixer_.Reset();
    std::fill(&mixTable_[0][0], &mixTable_[0][0] + MAX_CHANNELS * MAX_CHANNELS, 0.0f);
}

int32_t ChannelConverter::MixProcess(bool isDmix, uint32_t frameLen, float* in, float* out)
{
    float a;
    for (; frameLen > 0; frameLen--) {
        for (uint32_t i = 0; i < outChannelInfo_.numChannels; i++) {
            a = 0.0f;
            // if upmix, use mixTable_ in transpose because we have reverted input and output channel info
            // when setting up mixTable_ for upmix
            for (uint32_t j = 0; j < inChannelInfo_.numChannels; j++) {
                float coeff = isDmix ? mixTable_[i][j] : mixTable_[j][i];
                a += in[j] * coeff;
            }
            *(out++) = a;
        }
        in += inChannelInfo_.numChannels;
    }
    return MIX_ERR_SUCCESS;
}

void ChannelConverter::UpmixGainAttenuation()
{
    uint64_t outChMsk = outChannelInfo_.channelLayout;
    for (uint32_t i = 0; i < outChannelInfo_.numChannels; i++) {
        uint64_t outBit = outChMsk & (~outChMsk + 1);
        uint64_t inChMsk = inChannelInfo_.channelLayout;
        for (uint32_t j = 0; j < inChannelInfo_.numChannels; j++) {
            uint64_t inBit = inChMsk & (~inChMsk + 1);
            if (inBit != outBit) {
                mixTable_[j][i] *= COEF_M6DB_F;
            }
            inChMsk ^= inBit;
        }
        outChMsk ^= outBit;
    }
}

} // HPAE
} // AudioStandard
} // OHOS