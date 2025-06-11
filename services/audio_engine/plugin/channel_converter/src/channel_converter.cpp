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

static inline uint32_t Min(uint32_t a, uint32_t b)
{
    return a < b ? a : b;
}

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
    }
    return sampleSize;
}

int32_t ChannelConverter::SetParam(AudioChannelInfo inChannelInfo, AudioChannelInfo outChannelInfo,
    AudioSampleFormat format, bool mixLfe)
{
    inChannelInfo_.channelLayout = inChannelInfo.channelLayout;
    outChannelInfo_.channelLayout = outChannelInfo.channelLayout;
    inChannelInfo_.numChannels = inChannelInfo.numChannels;
    outChannelInfo_.numChannels = outChannelInfo.numChannels;
    workFormat_ = format;
    workSize_ = GetFormatSize(format);
    mixLfe_ = mixLfe;
    int32_t ret = DMIX_ERR_SUCCESS;
    if (inChannelInfo_.numChannels > outChannelInfo_.numChannels) {
        ret = downMixer_.SetParam(inChannelInfo, outChannelInfo, workSize_, mixLfe);
    }
    return ret;
}

int32_t ChannelConverter::SetInChannelInfo(AudioChannelInfo inChannelInfo)
{
    inChannelInfo_.channelLayout = inChannelInfo.channelLayout;
    inChannelInfo_.numChannels = inChannelInfo.numChannels;
    if (inChannelInfo_.numChannels > outChannelInfo_.numChannels) {
        return downMixer_.SetParam(inChannelInfo_, outChannelInfo_, workSize_, mixLfe_);
    }
    return DMIX_ERR_SUCCESS;
}
 
 
int32_t ChannelConverter::SetOutChannelInfo(AudioChannelInfo outChannelInfo)
{
    outChannelInfo_.channelLayout = outChannelInfo.channelLayout;
    outChannelInfo_.numChannels = outChannelInfo.numChannels;
    if (inChannelInfo_.numChannels > outChannelInfo_.numChannels) {
        return downMixer_.SetParam(inChannelInfo_, outChannelInfo_, workSize_, mixLfe_);
    }
    return DMIX_ERR_SUCCESS;
}

AudioChannelInfo ChannelConverter::GetInChannelInfo() const
{
    return inChannelInfo_;
}
 
AudioChannelInfo ChannelConverter::GetOutChannelInfo() const
{
    return outChannelInfo_;
}
 
int32_t ChannelConverter::Process(uint32_t frameSize, float* in, uint32_t inLen, float* out, uint32_t outLen)
{
    CHECK_AND_RETURN_RET_LOG(in, DMIX_ERR_INVALID_ARG, "input pointer is nullptr");
    CHECK_AND_RETURN_RET_LOG(out, DMIX_ERR_INVALID_ARG, "output pointer is nullptr");
    CHECK_AND_RETURN_RET_LOG(frameSize >= 0, DMIX_ERR_INVALID_ARG, "invalid frameSize");
    if (inChannelInfo_.numChannels < outChannelInfo_.numChannels) {
        return Upmix(frameSize, in, inLen, out, outLen);
    }
    return downMixer_.Process(frameSize, in, inLen, out, outLen);
}

void ChannelConverter::Reset()
{
    downMixer_.Reset();
}

int32_t ChannelConverter::Upmix(uint32_t frameSize, float* in, uint32_t inLen, float* out, uint32_t outLen)
{
    uint32_t expectInLen = frameSize * inChannelInfo_.numChannels * workSize_; // to be added size of other formats
    uint32_t expectOutLen = frameSize * outChannelInfo_.numChannels * workSize_;
    if ((expectInLen > inLen) || (expectOutLen > outLen)) {
        AUDIO_ERR_LOG("expect Input Len: %{public}d, inLen: %{public}d,"
            "expected output len: %{public}d, outLen %{public}d",
            expectInLen, inLen, expectOutLen, outLen);
        return DMIX_ERR_ALLOC_FAILED;
    }
    for (uint32_t i = 0; i < frameSize; ++i) {
        for (uint32_t ch = 0; ch < outChannelInfo_.numChannels; ++ch) {
            uint32_t leftChIndex = Min(ch, inChannelInfo_.numChannels - 1);
            out[i * outChannelInfo_.numChannels + ch] = in[i * inChannelInfo_.numChannels + leftChIndex];
        }
    }
    return DMIX_ERR_SUCCESS;
}

} // HPAE
} // AudioStandard
} // OHOS