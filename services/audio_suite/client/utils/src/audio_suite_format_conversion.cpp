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
#define LOG_TAG "AudioSuiteFormatConversion"
#endif

#include "audio_errors.h"
#include "audio_suite_log.h"
#include "audio_suite_pcm_buffer.h"
#include "audio_suite_format_conversion.h"

namespace OHOS {
namespace AudioStandard {
namespace AudioSuite {

static constexpr uint32_t RESAMPLE_QUAILTY = 5;
static constexpr uint32_t DOUBLE_FRAME = 2;

int32_t AudioSuiteFormatConversion::ChannelConvert(AudioSuitePcmBuffer *in, AudioSuitePcmBuffer *out)
{
    uint32_t inChannelCount = in->GetChannelCount();
    uint32_t outChannelCount = out->GetChannelCount();
    AUDIO_DEBUG_LOG(
        "Do ChannelConvert: inChannelCount: %{public}u, outChannelCount: %{public}u", inChannelCount, outChannelCount);

    CHECK_AND_RETURN_RET_LOG((inChannelCount != 0) && (outChannelCount != 0) && (inChannelCount != outChannelCount),
        ERROR, "Do ChannelConvert error: invalid input, inChannelCount: %{public}u outChannelCount: %{public}u",
        inChannelCount, outChannelCount);

    AudioChannelInfo inChannelInfo = {in->GetChannelLayout(), inChannelCount};
    AudioChannelInfo outChannelInfo = {out->GetChannelLayout(), outChannelCount};
    int32_t ret = channelConverter_.SetParam(inChannelInfo, outChannelInfo, SAMPLE_F32LE, true);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "SetParam fail with error code: %{public}d", ret);

    uint32_t frameSize = in->GetFrameLen();
    float *inputData =  reinterpret_cast<float *>(in->GetPcmData());
    uint32_t inLen = in->GetDataSize();
    float *outputData =  reinterpret_cast<float *>(out->GetPcmData());
    uint32_t outLen = out->GetDataSize();

    ret = channelConverter_.Process(frameSize, inputData, inLen, outputData, outLen);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "Process fail with error code: %{public}d", ret);

    return SUCCESS;
}

int32_t AudioSuiteFormatConversion::Resample(AudioSuitePcmBuffer *in, AudioSuitePcmBuffer *out)
{
    uint32_t inRate = in->GetSampleRate();
    uint32_t outRate = out->GetSampleRate();
    uint32_t inChannelCount = in->GetChannelCount();

    AUDIO_INFO_LOG("DoResample: inSampleRate: %{public}u, outSampleRate: %{public}u ", inRate, outRate);
    if ((inRate != resampleCfg_.inRate) ||
        (outRate != resampleCfg_.outRate) ||
        (inChannelCount != resampleCfg_.channels)) {
        resampleCfg_.outRate = outRate;
        resampleCfg_.channels = inChannelCount;
        resampleCfg_.inRate = inRate;
        if (proResampler_ == nullptr) {
            proResampler_ = std::make_unique<HPAE::ProResampler>(inRate, outRate, inChannelCount, RESAMPLE_QUAILTY);
        } else {
            proResampler_->Reset();
            int32_t ret = proResampler_->UpdateRates(inRate, outRate);
            CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "ProResampler update rate failed, err:%{public}d", ret);
            ret = proResampler_->UpdateChannels(inChannelCount);
            CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "ProResampler update Channels failed, err:%{public}d", ret);
        }
    }

    float *inputData = reinterpret_cast<float *>(in->GetPcmData());
    uint32_t inFrameSize = in->GetFrameLen();
    float *outputData = reinterpret_cast<float *>(out->GetPcmData());
    uint32_t outFrameSize = out->GetFrameLen();
    AUDIO_INFO_LOG("DoResample: inFrameSize: %{public}u, outFrameSize: %{public}u ", inFrameSize, outFrameSize);

    CHECK_AND_RETURN_RET_LOG(proResampler_ != nullptr, ERROR, "ProResampler_ is nullptr");
    int32_t ret = proResampler_->Process(inputData, inFrameSize, outputData, outFrameSize);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "DoResampleProcess fail with error code: %{public}d", ret);

    if (inRate == SAMPLE_RATE_11025) {
        ret = proResampler_->Process(inputData, 0, outputData +
            (outFrameSize / DOUBLE_FRAME * inChannelCount), outFrameSize / DOUBLE_FRAME);
        CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "DoResampleProcess fail with error code: %{public}d", ret);
    }

    return SUCCESS;
}

AudioSuitePcmBuffer *AudioSuiteFormatConversion::Process(AudioSuitePcmBuffer *inPcmBuffer, PcmBufferFormat &outFormat)
{
    CHECK_AND_RETURN_RET_LOG(inPcmBuffer != nullptr, nullptr, "nullptr");
    if (inPcmBuffer->IsSameFormat(outFormat)) {
        return inPcmBuffer;
    }

    PcmDataDuration duration = PCM_DATA_DEFAULT_DURATION_20_MS;
    if ((inPcmBuffer->GetSampleRate() == AudioSamplingRate::SAMPLE_RATE_11025) ||
        (outFormat.sampleRate == AudioSamplingRate::SAMPLE_RATE_11025)) {
        duration = PCM_DATA_DURATION_40_MS;
    }

    AudioSuitePcmBuffer *in = inPcmBuffer;
    if (in->GetSampleFormat() != SAMPLE_F32LE) {
        AUDIO_INFO_LOG("ConvertToFloat, informat:%{public}u", in->GetSampleFormat());
        formatFloatOut_.ResizePcmBuffer(PcmBufferFormat(in->GetSampleRate(),
            in->GetChannelCount(), in->GetChannelLayout(), SAMPLE_F32LE), duration);
        HPAE::ConvertToFloat(in->GetSampleFormat(), formatFloatOut_.GetSampleCount(),
            static_cast<void *>(in->GetPcmData()),  reinterpret_cast<float *>(formatFloatOut_.GetPcmData()));
        in = &formatFloatOut_;
    }

    int32_t ret = SUCCESS;
    if (in->GetSampleRate() != outFormat.sampleRate) {
        AUDIO_INFO_LOG("ConvertRate, inrate:%{public}u, outrate: %{public}u ",
            in->GetSampleRate(), outFormat.sampleRate);
        rateOut_.ResizePcmBuffer(PcmBufferFormat(outFormat.sampleRate, in->GetChannelCount(),
            in->GetChannelLayout(), SAMPLE_F32LE), duration);
        ret = Resample(in, &rateOut_);
        CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, nullptr, "Do Resample error");
        in = &rateOut_;
    }

    if (in->GetChannelCount() != outFormat.channelCount) {
        AUDIO_INFO_LOG("ConvertChannel in:%{public}u, out: %{public}u ", in->GetChannelCount(), outFormat.channelCount);
        channelOut_.ResizePcmBuffer(PcmBufferFormat(in->GetSampleRate(),
            outFormat.channelCount, outFormat.channelLayout, SAMPLE_F32LE), duration);
        ret = ChannelConvert(in, &channelOut_);
        CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, nullptr, "Channel Conversion error");
        in = &channelOut_;
    }

    if (outFormat.sampleFormat != SAMPLE_F32LE) {
        AUDIO_INFO_LOG("ConvertFromFloat, in: %{public}u, out: %{public}u.",
            in->GetSampleFormat(), outFormat.sampleFormat);
        formatOut_.ResizePcmBuffer(PcmBufferFormat(in->GetSampleRate(),
            in->GetChannelCount(), in->GetChannelLayout(), outFormat.sampleFormat), duration);
        HPAE::ConvertFromFloat(outFormat.sampleFormat, in->GetSampleCount(),
            reinterpret_cast<float *>(in->GetPcmData()), static_cast<void *>(formatOut_.GetPcmData()));
        in = &formatOut_;
    }

    return in;
}

void AudioSuiteFormatConversion::Reset()
{
    resampleCfg_.inRate = 0;
    resampleCfg_.outRate = 0;
    resampleCfg_.channels = 0;
}

}
}
}