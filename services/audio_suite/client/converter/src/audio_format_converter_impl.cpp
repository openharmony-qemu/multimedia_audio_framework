/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "audio_format_converter_impl.h"
#include "audio_suite_common.h"
#include "audio_suite_log.h"
#include "audio_log.h"
#include "audio_errors.h"
#include <cmath>
#include <cstring>
#include <securec.h>
#include <numeric>

namespace OHOS {
namespace AudioStandard {
namespace AudioSuite {

// The set of sampling formats supported by format validation
static const std::unordered_set<AudioSampleFormat> SUPPORTED_SAMPLE_FORMATS = {
    AudioSampleFormat::SAMPLE_U8,
    AudioSampleFormat::SAMPLE_S16LE,
    AudioSampleFormat::SAMPLE_S24LE,
    AudioSampleFormat::SAMPLE_S32LE,
    AudioSampleFormat::SAMPLE_F32LE
};

constexpr int32_t MAX_CALLBACK_DATA_SIZE = 400 * 1024; // 400KB
constexpr uint32_t MIN_FRAMES_FOR_20MS = 20;
constexpr uint32_t MILLISECONDS_PER_SECOND = 1000;
constexpr uint32_t MAX_INPUT_DURATION_MS = 1000;

// Sample rate validation constants
constexpr uint32_t MAX_RESAMPLE_QUALITY = 10;
constexpr uint32_t MIN_RESAMPLE_QUALITY = 1;
constexpr uint32_t RESAMPLE_DELAY_MAX_MS = 2;

// Ex mode sample rate validation constants
constexpr int32_t EX_MIN_SAMPLE_RATE = 8000;
constexpr int32_t EX_MAX_SAMPLE_RATE = 5644800;
constexpr int32_t EX_SAMPLE_RATE_RESOLUTION = 10;
constexpr int32_t EX_EXEMPTED_RATE_11025 = 11025;

// Raw format conversion constants
constexpr uint32_t FLOAT_SIZE_BYTES = sizeof(float);
constexpr uint32_t MAX_FRAME_LENGTH = static_cast<uint32_t>(EX_MAX_SAMPLE_RATE) * 10;

AudioFormatConverterImpl::AudioFormatConverterImpl()
{
}

AudioFormatConverterImpl::~AudioFormatConverterImpl()
{
    DestroyResamplerState();
}

std::shared_ptr<AudioFormatConverter> AudioFormatConverter::Create(
    const AudioConverterFormat &inputFormat, const AudioConverterFormat &outputFormat,
    const uint32_t resampleQuality, bool enableDataAlignment)
{
    auto inLayoutIt = LAYOUT_TO_CHANNEL.find(inputFormat.channelLayout);
    CHECK_AND_RETURN_RET_LOG(inLayoutIt != LAYOUT_TO_CHANNEL.end(), nullptr,
        "Create: unsupported input channel layout");
    auto outLayoutIt = LAYOUT_TO_CHANNEL.find(outputFormat.channelLayout);
    CHECK_AND_RETURN_RET_LOG(outLayoutIt != LAYOUT_TO_CHANNEL.end(), nullptr,
        "Create: unsupported output channel layout");

    uint32_t inChannelCount = (inputFormat.channelCount != 0)
        ? inputFormat.channelCount
        : static_cast<uint32_t>(inLayoutIt->second);
    uint32_t outChannelCount = (outputFormat.channelCount != 0)
        ? outputFormat.channelCount
        : static_cast<uint32_t>(outLayoutIt->second);

    AudioConverterFormat inputFmt;
    inputFmt.sampleRate = inputFormat.sampleRate;
    inputFmt.channelCount = inChannelCount;
    inputFmt.channelLayout = inputFormat.channelLayout;
    inputFmt.sampleFormat = inputFormat.sampleFormat;

    AudioConverterFormat outputFmt;
    outputFmt.sampleRate = outputFormat.sampleRate;
    outputFmt.channelCount = outChannelCount;
    outputFmt.channelLayout = outputFormat.channelLayout;
    outputFmt.sampleFormat = outputFormat.sampleFormat;

    auto audioFormatConverter = std::make_shared<AudioFormatConverterImpl>();
    int32_t ret = audioFormatConverter->Init(inputFmt, outputFmt, resampleQuality, enableDataAlignment);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, nullptr, "Create: rawFormatConversion init failed");

    return audioFormatConverter;
}

int32_t AudioFormatConverterImpl::GetMinInOutSizes(uint32_t &minInSize, uint32_t &minOutSize)
{
    std::lock_guard<std::mutex> lock(mutex_);
    CHECK_AND_RETURN_RET_LOG(!destroyed_, ERR_INVALID_PARAM, "GetMinInOutSizes: converter already destroyed");
    CHECK_AND_RETURN_RET_LOG(minInSize_ > 0 && minOutSize_ > 0, ERR_INVALID_PARAM, "GetMinInOutSizes: not initialized");
    minInSize = minInSize_;
    minOutSize = minOutSize_;
    return SUCCESS;
}

int32_t AudioFormatConverterImpl::Init(
    const AudioConverterFormat &inputFormat, const AudioConverterFormat &outputFormat, const uint32_t resampleQuality,
    bool enableDataAlignment)
{
    std::lock_guard<std::mutex> lock(mutex_);

    int32_t ret = ValidateCommonFormat(inputFormat, outputFormat, resampleQuality);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "Init: common format validation failed");

    AUDIO_INFO_LOG("Create: src=[%{public}d %{public}u], dst=[%{public}d %{public}u]",
        inputFormat.sampleRate, LAYOUT_TO_CHANNEL.at(inputFormat.channelLayout),
        outputFormat.sampleRate, LAYOUT_TO_CHANNEL.at(outputFormat.channelLayout));

    inputFormat_ = inputFormat;
    outputFormat_ = outputFormat;
    hasDelay_ = inputFormat_.sampleRate != outputFormat_.sampleRate;
    enableDataAlignment_ = enableDataAlignment;
    inSingleFrameBytes_ = AudioSuiteUtil::GetSampleSize(inputFormat_.sampleFormat) * inputFormat_.channelCount;
    outSingleFrameBytes_ = AudioSuiteUtil::GetSampleSize(outputFormat_.sampleFormat) * outputFormat_.channelCount;

    uint32_t inRate = static_cast<uint32_t>(inputFormat_.sampleRate);
    uint32_t outRate = static_cast<uint32_t>(outputFormat_.sampleRate);
    uint32_t gcdVal = std::gcd(inRate, outRate);
    CHECK_AND_RETURN_RET_LOG(
        gcdVal != 0, ERROR, "The greatest common divisor of the sampling rates must be greater than zero.");
    uint64_t minInFrames = static_cast<uint64_t>(inRate) / gcdVal;
    uint64_t minOutFrames = static_cast<uint64_t>(outRate) / gcdVal;
    minInSize_ = static_cast<uint32_t>(minInFrames) * inSingleFrameBytes_;
    minOutSize_ = static_cast<uint32_t>(minOutFrames) * outSingleFrameBytes_;
    uint32_t gcdSecond = std::gcd(gcdVal, SECONDS_TO_MS);
    CHECK_AND_RETURN_RET_LOG(gcdSecond != 0, ERROR, "The minimum processing time must be greater than zero");
    uint32_t processMinMs = SECONDS_TO_MS / gcdSecond;
    uint32_t inputBytesPerSec = static_cast<uint32_t>(inputFormat_.sampleRate) * inSingleFrameBytes_;
    processMinInBytes_ = static_cast<uint32_t>(inputBytesPerSec * processMinMs / SECONDS_TO_MS);
    AUDIO_INFO_LOG("Create: processMinMs:%{public}u, inSingleFrameBytes:%{public}u, "
        "outSingleFrameBytes:%{public}u, processMinInBytes:%{public}u, "
        "minInBytes:%{public}u, minOutBytes:%{public}u",
        processMinMs, inSingleFrameBytes_, outSingleFrameBytes_, processMinInBytes_, minInSize_, minOutSize_);

    resampleQuality_ = resampleQuality;
    ret = InitRawConversion();
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "Init: raw conversion init failed");
    return SUCCESS;
}

int32_t AudioFormatConverterImpl::ValidateCommonFormat(
    const AudioConverterFormat &inputFormat, const AudioConverterFormat &outputFormat, const uint32_t resampleQuality)
{
    CHECK_AND_RETURN_RET_LOG(inputFormat.sampleRate >= EX_MIN_SAMPLE_RATE &&
        inputFormat.sampleRate <= EX_MAX_SAMPLE_RATE,
        ERR_INVALID_PARAM, "Create: input sampleRate out of range [8000, 5644800], rate=%{public}d",
        inputFormat.sampleRate);
    CHECK_AND_RETURN_RET_LOG(outputFormat.sampleRate >= EX_MIN_SAMPLE_RATE &&
        outputFormat.sampleRate <= EX_MAX_SAMPLE_RATE,
        ERR_INVALID_PARAM, "Create: output sampleRate out of range [8000, 5644800], rate=%{public}d",
        outputFormat.sampleRate);
    CHECK_AND_RETURN_RET_LOG(
        inputFormat.sampleRate % EX_SAMPLE_RATE_RESOLUTION == 0 || inputFormat.sampleRate == EX_EXEMPTED_RATE_11025,
        ERR_INVALID_PARAM, "Create: input sampleRate must be multiple of 10 (11025 exempted), rate=%{public}d",
        inputFormat.sampleRate);
    CHECK_AND_RETURN_RET_LOG(
        outputFormat.sampleRate % EX_SAMPLE_RATE_RESOLUTION == 0 || outputFormat.sampleRate == EX_EXEMPTED_RATE_11025,
        ERR_INVALID_PARAM, "Create: output sampleRate must be multiple of 10 (11025 exempted), rate=%{public}d",
        outputFormat.sampleRate);
    if (LAYOUT_TO_CHANNEL.find(inputFormat.channelLayout) == LAYOUT_TO_CHANNEL.end() ||
        LAYOUT_TO_CHANNEL.find(outputFormat.channelLayout) == LAYOUT_TO_CHANNEL.end()) {
        AUDIO_ERR_LOG("Create: unsupported channel layout");
        return ERR_INVALID_PARAM;
    }
    CHECK_AND_RETURN_RET_LOG(inputFormat.channelCount == LAYOUT_TO_CHANNEL.at(inputFormat.channelLayout),
                             ERR_INVALID_PARAM, "Create: input channelCount/layout mismatch");
    CHECK_AND_RETURN_RET_LOG(outputFormat.channelCount == LAYOUT_TO_CHANNEL.at(outputFormat.channelLayout),
                             ERR_INVALID_PARAM, "Create: output channelCount/layout mismatch");
    if (SUPPORTED_SAMPLE_FORMATS.find(inputFormat.sampleFormat) == SUPPORTED_SAMPLE_FORMATS.end() ||
        SUPPORTED_SAMPLE_FORMATS.find(outputFormat.sampleFormat) == SUPPORTED_SAMPLE_FORMATS.end()) {
        AUDIO_ERR_LOG("Create: unsupported sample format");
        return ERR_INVALID_PARAM;
    }
    CHECK_AND_RETURN_RET_LOG(resampleQuality >= MIN_RESAMPLE_QUALITY &&
        resampleQuality <= MAX_RESAMPLE_QUALITY, ERR_INVALID_PARAM, "Resampling level assignment error");
    return SUCCESS;
}

int32_t AudioFormatConverterImpl::InitRawConversion()
{
    formatsSame_ = (inputFormat_.sampleRate == outputFormat_.sampleRate &&
                    inputFormat_.channelCount == outputFormat_.channelCount &&
                    inputFormat_.channelLayout == outputFormat_.channelLayout &&
                    inputFormat_.sampleFormat == outputFormat_.sampleFormat);

    int32_t ret = InitResampler();
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "InitRawConversion: InitResampler failed");

    ret = InitChannelConverter();
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "InitRawConversion: InitChannelConverter failed");

    AUDIO_INFO_LOG("InitRawConversion: same=%{public}d", formatsSame_);
    return SUCCESS;
}

int32_t AudioFormatConverterImpl::InitResampler()
{
    if (inputFormat_.sampleRate == outputFormat_.sampleRate) {
        return SUCCESS;
    }
    int32_t errRet = RESAMPLER_ERR_SUCCESS;
    resamplerState_ = SingleStagePolyphaseResamplerInit(
        inputFormat_.channelCount, static_cast<uint32_t>(inputFormat_.sampleRate),
        static_cast<uint32_t>(outputFormat_.sampleRate), static_cast<int32_t>(resampleQuality_), &errRet);
    CHECK_AND_RETURN_RET_LOG(resamplerState_ != nullptr && errRet == RESAMPLER_ERR_SUCCESS,
        ERR_NO_MEMORY, "InitResampler: Init failed, err=%{public}d", errRet);
    SingleStagePolyphaseResamplerSkipHalfTaps(resamplerState_);

    uint32_t expectedOutFrameLen =
        static_cast<uint32_t>(outputFormat_.sampleRate) * MIN_FRAMES_FOR_20MS / MILLISECONDS_PER_SECOND;
    resamplerTmpBuf_.resize(static_cast<size_t>(expectedOutFrameLen) * inputFormat_.channelCount, 0.0f);
    return SUCCESS;
}

void AudioFormatConverterImpl::DestroyResamplerState()
{
    if (resamplerState_ != nullptr) {
        SingleStagePolyphaseResamplerFree(resamplerState_);
        resamplerState_ = nullptr;
    }
    resamplerTmpBuf_.clear();
    resamplerLastDelayFrames_ = 0;
}

int32_t AudioFormatConverterImpl::InitChannelConverter()
{
    if (inputFormat_.channelLayout == outputFormat_.channelLayout) {
        return SUCCESS;
    }
    AudioChannelInfo inChannelInfo = {inputFormat_.channelLayout, inputFormat_.channelCount};
    AudioChannelInfo outChannelInfo = {outputFormat_.channelLayout, outputFormat_.channelCount};
    ChannelConvertSetUpmixCoef(HPAE::COEF_M6DB_F);
    int32_t ret = ChannelConvertSetParam(inChannelInfo, outChannelInfo);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_OPERATION_FAILED,
        "InitChannelConverter: SetParam failed, err:%{public}d", ret);
    return SUCCESS;
}

int32_t AudioFormatConverterImpl::ChannelConvertSetParam(
    AudioChannelInfo inChannelInfo, AudioChannelInfo outChannelInfo)
{
    channelInInfo_.channelLayout = inChannelInfo.channelLayout;
    channelOutInfo_.channelLayout = outChannelInfo.channelLayout;
    channelInInfo_.numChannels = inChannelInfo.numChannels;
    channelOutInfo_.numChannels = outChannelInfo.numChannels;
    CHECK_AND_RETURN_RET_LOG(inChannelInfo.numChannels <= HPAE::MAX_CHANNELS,
        HPAE::MIX_ERR_INVALID_ARG, "invalid input channels");
    CHECK_AND_RETURN_RET_LOG(outChannelInfo.numChannels <= HPAE::MAX_CHANNELS,
        HPAE::MIX_ERR_INVALID_ARG, "invalid output channels");
    // stereo downmix the same as stereo
    if (channelInInfo_.channelLayout == CH_LAYOUT_STEREO_DOWNMIX) {
        channelInInfo_.channelLayout = CH_LAYOUT_STEREO;
    }
    if (channelOutInfo_.channelLayout == CH_LAYOUT_STEREO_DOWNMIX) {
        channelOutInfo_.channelLayout = CH_LAYOUT_STEREO;
    }

    int32_t ret = HPAE::MIX_ERR_SUCCESS;
    if (channelInInfo_.numChannels > channelOutInfo_.numChannels) {
        ret = channelDownMixer_.SetParam(channelInInfo_, channelOutInfo_, FLOAT_SIZE_BYTES, true);
        channelDownMixer_.SetNormalization(true);
        channelDownMixer_.GetDownMixTable(channelMixTable_);
    } else {
        ret = HPAE::SetUpGeneralMixingTable(channelMixTable_, channelInInfo_, channelOutInfo_, true);
        ChannelConvertApplyUpmixGainRatio(channelUpmixCoef_);
    }
    return ret;
}

int32_t AudioFormatConverterImpl::ChannelConvertProcess(uint32_t frameLen, float* in, float* out)
{
    CHECK_AND_RETURN_RET_LOG(in, HPAE::MIX_ERR_INVALID_ARG, "input pointer is nullptr");
    CHECK_AND_RETURN_RET_LOG(out, HPAE::MIX_ERR_INVALID_ARG, "output pointer is nullptr");
    CHECK_AND_RETURN_RET_LOG(frameLen <= MAX_FRAME_LENGTH, HPAE::MIX_ERR_INVALID_ARG, "invalid frameSize oversize");

    return ChannelConvertMixProcess(channelInInfo_.numChannels >= channelOutInfo_.numChannels, frameLen, in, out);
}

void AudioFormatConverterImpl::ChannelConvertSetUpmixCoef(float coef)
{
    if (std::isnan(coef) || coef < 0.0f) {
        AUDIO_ERR_LOG("set upmix coef invalid");
        return;
    }
    CHECK_AND_RETURN(coef != channelUpmixCoef_);
    float oldCoef = channelUpmixCoef_;
    AUDIO_INFO_LOG("set upmix coef from %{public}f to %{public}f", channelUpmixCoef_, coef);
    channelUpmixCoef_ = coef;
    if (channelInInfo_.numChannels <= channelOutInfo_.numChannels && oldCoef != 0.0f) {
        ChannelConvertApplyUpmixGainRatio(coef / oldCoef);
    }
}

void AudioFormatConverterImpl::ChannelConvertReset()
{
    channelDownMixer_.Reset();
    std::fill(&channelMixTable_[0][0], &channelMixTable_[0][0] + HPAE::MAX_CHANNELS * HPAE::MAX_CHANNELS, 0.0f);
    channelUpmixCoef_ = HPAE::COEF_0DB_F;
}

int32_t AudioFormatConverterImpl::ChannelConvertMixProcess(
    bool isDmix, uint32_t frameLen, float* in, float* out)
{
    for (; frameLen > 0; frameLen--) {
        for (uint32_t i = 0; i < channelOutInfo_.numChannels; i++) {
            float mixedSample = 0.0f;
            for (uint32_t j = 0; j < channelInInfo_.numChannels; j++) {
                float coeff = isDmix ? channelMixTable_[i][j] : channelMixTable_[j][i];
                mixedSample += in[j] * coeff;
            }
            *(out++) = mixedSample;
        }
        in += channelInInfo_.numChannels;
    }
    return HPAE::MIX_ERR_SUCCESS;
}

void AudioFormatConverterImpl::ChannelConvertApplyUpmixGainRatio(float ratio)
{
    if (channelInInfo_.numChannels == 1) {
        return;
    }
    uint64_t outChMsk = channelOutInfo_.channelLayout;
    for (uint32_t i = 0; i < channelOutInfo_.numChannels; i++) {
        uint64_t outBit = outChMsk & (~outChMsk + 1);
        uint64_t inChMsk = channelInInfo_.channelLayout;
        for (uint32_t j = 0; j < channelInInfo_.numChannels; j++) {
            uint64_t inBit = inChMsk & (~inChMsk + 1);
            if (inBit != outBit) {
                channelMixTable_[j][i] *= ratio;
            }
            inChMsk ^= inBit;
        }
        outChMsk ^= outBit;
    }
}

int32_t AudioFormatConverterImpl::ProcessRawConversion(
    const uint8_t* inData, uint32_t inDataSize, std::vector<uint8_t>& outputVector)
{
    CHECK_AND_RETURN_RET_LOG(inData != nullptr, ERR_INVALID_PARAM, "ProcessRaw: inData is nullptr");
    CHECK_AND_RETURN_RET_LOG(inDataSize > 0, ERR_INVALID_PARAM, "ProcessRaw: inDataSize is 0");

    if (formatsSame_) {
        outputVector.resize(inDataSize);
        errno_t copyRet = memcpy_s(outputVector.data(), outputVector.size(), inData, inDataSize);
        CHECK_AND_RETURN_RET_LOG(copyRet == 0, ERR_OPERATION_FAILED, "ProcessRaw: memcpy_s failed");
        return SUCCESS;
    }

    float* currentData;
    uint32_t currentSampleCount;
    int32_t ret;
    if (inputFormat_.sampleFormat == SAMPLE_F32LE) {
        currentData = reinterpret_cast<float*>(const_cast<uint8_t*>(inData));
        currentSampleCount = inDataSize / FLOAT_SIZE_BYTES;
    } else {
        ret = ConvertToFloat(inData, inDataSize, floatBuffer_);
        CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_OPERATION_FAILED, "ProcessRaw: ConvertToFloat failed");
        currentData = floatBuffer_.data();
        currentSampleCount = static_cast<uint32_t>(floatBuffer_.size());
    }

    ret = RunResampleStage(currentData, currentSampleCount);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_OPERATION_FAILED, "ProcessRaw: Resample failed");

    ret = RunChannelConvertStage(currentData, currentSampleCount);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_OPERATION_FAILED, "ProcessRaw: ChannelConvert failed");

    ret = ConvertFromFloat(currentData, currentSampleCount, outputVector);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_OPERATION_FAILED, "ProcessRaw: ConvertFromFloat failed");
    return SUCCESS;
}

int32_t AudioFormatConverterImpl::RunResampleStage(float*& currentData, uint32_t& currentSampleCount)
{
    if (inputFormat_.sampleRate == outputFormat_.sampleRate) {
        return SUCCESS;
    }
    return ProcessResample(currentData, currentSampleCount, currentData, currentSampleCount);
}

int32_t AudioFormatConverterImpl::RunChannelConvertStage(float*& currentData, uint32_t& currentSampleCount)
{
    if (inputFormat_.channelLayout == outputFormat_.channelLayout) {
        return SUCCESS;
    }
    return ProcessChannelConvert(currentData, currentSampleCount, currentData, currentSampleCount);
}

int32_t AudioFormatConverterImpl::ProcessResamplerInternal(
    const float* inBuffer, uint32_t inFrameLen, float* outBuffer, uint32_t outFrameLen)
{
    uint32_t expectedOutFrameLen = outFrameLen;
    uint32_t channels = inputFormat_.channelCount;

    if (resamplerTmpBuf_.size() < static_cast<size_t>(expectedOutFrameLen) * channels) {
        resamplerTmpBuf_.resize(static_cast<size_t>(expectedOutFrameLen) * channels, 0.0f);
    }

    int32_t ret = SingleStagePolyphaseResamplerProcess(
        resamplerState_, inBuffer, &inFrameLen, resamplerTmpBuf_.data(), &outFrameLen);
    CHECK_AND_RETURN_RET_LOG(ret == EOK, ERR_OPERATION_FAILED, "process failed: %{public}d", ret);

    uint32_t fillSize = expectedOutFrameLen > outFrameLen ? expectedOutFrameLen - outFrameLen : 0;
    resamplerLastDelayFrames_ = fillSize;

    if (outFrameLen == 0) {
        ret = memset_s(outBuffer, expectedOutFrameLen * channels * sizeof(float), 0,
                       expectedOutFrameLen * channels * sizeof(float));
        CHECK_AND_RETURN_RET_LOG(ret == EOK, ERR_OPERATION_FAILED, "memset_s failed: %{public}d", ret);
        return SUCCESS;
    }

    if (fillSize > 0) {
        ret = memset_s(outBuffer, fillSize * channels * sizeof(float), 0,
                       fillSize * channels * sizeof(float));
        CHECK_AND_RETURN_RET_LOG(ret == EOK, ERR_OPERATION_FAILED, "memset_s failed: %{public}d", ret);
    }

    ret = memcpy_s(outBuffer + fillSize * channels, (expectedOutFrameLen - fillSize) * channels * sizeof(float),
                   resamplerTmpBuf_.data(), outFrameLen * channels * sizeof(float));
    CHECK_AND_RETURN_RET_LOG(ret == EOK, ERR_OPERATION_FAILED, "memcpy_s failed: %{public}d", ret);
    return SUCCESS;
}

int32_t AudioFormatConverterImpl::ProcessResample(
    const float* inputData, uint32_t inSampleCount, float*& outData, uint32_t& outSampleCount)
{
    CHECK_AND_RETURN_RET_LOG(resamplerState_ != nullptr, ERR_OPERATION_FAILED,
        "ProcessResample: resamplerState_ is nullptr");

    uint32_t inFrameSize = inSampleCount / inputFormat_.channelCount;
    uint32_t outFrameSize = static_cast<uint32_t>(
        static_cast<uint64_t>(inFrameSize) * static_cast<uint64_t>(outputFormat_.sampleRate)
        / static_cast<uint64_t>(inputFormat_.sampleRate));

    uint32_t additionalFrames = (inputFormat_.sampleRate == EX_EXEMPTED_RATE_11025) ? outFrameSize : 0;
    uint32_t totalOutFrameSize = outFrameSize + additionalFrames;
    uint32_t outSampleCountLocal = totalOutFrameSize * inputFormat_.channelCount;
    resampleBuffer_.resize(outSampleCountLocal);

    uint32_t actualOutFrameSize = outFrameSize;
    int32_t ret = ProcessResamplerInternal(inputData, inFrameSize, resampleBuffer_.data(), actualOutFrameSize);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_OPERATION_FAILED,
        "ProcessResample: failed with error code: %{public}d", ret);

    outData = resampleBuffer_.data();
    outSampleCount = actualOutFrameSize * inputFormat_.channelCount;
    if (isFirstProcess_) {
        delayFrames_ = resamplerLastDelayFrames_;
    }
    return SUCCESS;
}

int32_t AudioFormatConverterImpl::ProcessChannelConvert(
    const float* inputData, uint32_t inSampleCount, float*& outData, uint32_t& outSampleCount)
{
    CHECK_AND_RETURN_RET_LOG(inputFormat_.channelCount > 0, ERR_INVALID_PARAM,
        "ProcessChannelConvert: input channelCount is 0");
    CHECK_AND_RETURN_RET_LOG(outputFormat_.channelCount > 0, ERR_INVALID_PARAM,
        "ProcessChannelConvert: output channelCount is 0");

    uint32_t frameSize = inSampleCount / inputFormat_.channelCount;
    uint32_t outSampleCountLocal = frameSize * outputFormat_.channelCount;
    channelBuffer_.resize(outSampleCountLocal);

    int32_t ret = ChannelConvertProcess(frameSize, const_cast<float*>(inputData), channelBuffer_.data());
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_OPERATION_FAILED,
        "ProcessChannelConvert: failed with error code: %{public}d", ret);

    outData = channelBuffer_.data();
    outSampleCount = outSampleCountLocal;
    return SUCCESS;
}

int32_t AudioFormatConverterImpl::ConvertToFloat(
    const uint8_t* inData, uint32_t inDataSize, std::vector<float>& outBuffer)
{
    uint32_t inputSampleSize = AudioSuiteUtil::GetSampleSize(inputFormat_.sampleFormat);
    CHECK_AND_RETURN_RET_LOG(inputSampleSize != 0, ERR_INVALID_PARAM, "inputSampleSize cannot be zero.");
    uint32_t inSampleCount = inDataSize / inputSampleSize;
    outBuffer.resize(inSampleCount);

    HPAE::ConvertToFloat(inputFormat_.sampleFormat, inSampleCount,
        static_cast<void*>(const_cast<uint8_t*>(inData)), outBuffer.data());
    return SUCCESS;
}

int32_t AudioFormatConverterImpl::ConvertFromFloat(
    const float* inData, uint32_t inSampleCount, std::vector<uint8_t>& outBuffer)
{
    uint32_t outputSampleSize = (outputFormat_.sampleFormat != SAMPLE_F32LE)
        ? AudioSuiteUtil::GetSampleSize(outputFormat_.sampleFormat) : sizeof(float);
    uint32_t outputBytes = inSampleCount * outputSampleSize;
    outBuffer.resize(outputBytes);

    if (outputFormat_.sampleFormat != SAMPLE_F32LE) {
        HPAE::ConvertFromFloat(outputFormat_.sampleFormat, inSampleCount,
            const_cast<float*>(inData), outBuffer.data());
    } else {
        errno_t copyRet = memcpy_s(outBuffer.data(), outBuffer.size(), inData, outputBytes);
        CHECK_AND_RETURN_RET_LOG(copyRet == 0, ERR_OPERATION_FAILED, "ConvertFromFloat: memcpy_s failed");
    }
    return SUCCESS;
}

void AudioFormatConverterImpl::ResetResamplerState()
{
    if (resamplerState_ == nullptr) {
        return;
    }
    SingleStagePolyphaseResamplerResetMem(resamplerState_);
    SingleStagePolyphaseResamplerSkipHalfTaps(resamplerState_);
    resamplerLastDelayFrames_ = 0;
}

void AudioFormatConverterImpl::ResetRawConversion()
{
    ResetResamplerState();
    ChannelConvertReset();
    formatsSame_ = false;
    floatBuffer_.clear();
    resampleBuffer_.clear();
    channelBuffer_.clear();
}

int32_t AudioFormatConverterImpl::SetInputCallback(std::shared_ptr<FormatConverterDataCallback> callback)
{
    std::lock_guard<std::mutex> lock(mutex_);
    CHECK_AND_RETURN_RET_LOG(!destroyed_, ERR_ILLEGAL_STATE, "SetInputCallback: converter already destroyed");
    CHECK_AND_RETURN_RET_LOG(!directProcessMode_, ERR_NOT_SUPPORTED,
        "SetInputCallback: already in direct process mode, not available");
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, ERR_CALLBACK_NOT_FOUND, "Callback not set");
    dataCallback_ = std::move(callback);
    return SUCCESS;
}

int32_t AudioFormatConverterImpl::ReadAndCacheInputData(bool* shouldStopProcessing)
{
    const void* inputDataPtr = nullptr;
    InputDataStatus status = InputDataStatus::HAVE_DATA;
    CHECK_AND_RETURN_RET_LOG(dataCallback_ != nullptr, ERR_CALLBACK_NOT_FOUND, "Callback function not set.");
    int32_t inputSize = dataCallback_->OnRequestData(&inputDataPtr, &status);
    if (inputSize > MAX_CALLBACK_DATA_SIZE || inputSize < 0) {
        AUDIO_ERR_LOG("The callback function returned an error indicating the amount of data was incorrect: %{public}d",
            inputSize);
        return ERR_INVALID_WRITE;
    }

    if (status == InputDataStatus::DATA_FINISHED) {
        inputFinished_ = true;
    }

    if (status == InputDataStatus::NO_AVAILABLE_DATA) {
        *shouldStopProcessing = true;
        return SUCCESS;
    }

    if (inputSize == 0 && status != InputDataStatus::DATA_FINISHED) {
        *shouldStopProcessing = true;
        return SUCCESS;
    }

    if (inputSize > 0 && inputDataPtr != nullptr) {
        cachedInputData_.insert(cachedInputData_.end(),
            static_cast<const uint8_t*>(inputDataPtr),
            static_cast<const uint8_t*>(inputDataPtr) + inputSize);
    }
    *shouldStopProcessing = false;
    return SUCCESS;
}

int32_t AudioFormatConverterImpl::CalculateMinBytesForIntMsInternal(
    uint32_t& alignedCachedSize, bool& shouldStopProcessing)
{
    uint32_t cachedSize = static_cast<uint32_t>(cachedInputData_.size());
    CHECK_AND_RETURN_RET_LOG(processMinInBytes_ != 0, ERR_INVALID_PARAM, "Minimum process input byptes cannot be 0.");
    alignedCachedSize = (cachedSize / processMinInBytes_) * processMinInBytes_;
    
    if (alignedCachedSize == 0) {
        if (inputFinished_ && !isPadded_) {
            paddedInputBytes_ = CalculatePaddingInputLen(cachedSize);
            cachedInputData_.insert(cachedInputData_.end(), paddedInputBytes_, 0);
            alignedCachedSize = cachedInputData_.size() / processMinInBytes_ * processMinInBytes_;
            AUDIO_INFO_LOG("Process: padding %{public}u bytes for final data to meet int ms", paddedInputBytes_);
            isPadded_ = true;
            shouldStopProcessing = false;
        } else {
            AUDIO_DEBUG_LOG("Process: waiting for more data, cachedSize=%{public}d, processMinInBytes=%{public}d",
                cachedSize, processMinInBytes_);
            shouldStopProcessing = true;
            return SUCCESS;
        }
    }

    shouldStopProcessing = false;
    return SUCCESS;
}

uint32_t AudioFormatConverterImpl::CalculatePaddingInputLen(uint32_t inputSize) const
{
    if (!inputFinished_) {
        return 0;
    }
    if (inputSize == 0 && (!enableDataAlignment_ || !hasDelay_)) {
        return 0;
    }
    if (inputSize > processMinInBytes_) {
        return 0;
    }

    uint32_t paddingLen = processMinInBytes_ - inputSize;
    if (enableDataAlignment_ && hasDelay_) {
        uint32_t minPaddingLen = RESAMPLE_DELAY_MAX_MS * inSingleFrameBytes_ *
            static_cast<uint32_t>(inputFormat_.sampleRate) / MILLISECONDS_PER_SECOND;
        if (paddingLen < minPaddingLen) {
            uint32_t additionalPadding = minPaddingLen - paddingLen;
            uint32_t additionalUnits = (additionalPadding + processMinInBytes_ - 1) / processMinInBytes_;
            paddingLen += additionalUnits * processMinInBytes_;
        }
    }
    return paddingLen;
}

int32_t AudioFormatConverterImpl::CalculateProcessableBytesInternal(
    const ProcessableBytesCalculationParams& params, uint32_t& alignedProcessableBytes, bool& shouldStopProcessing)
{
    double sampleRateRatio =
        static_cast<double>(inputFormat_.sampleRate) / static_cast<double>(outputFormat_.sampleRate);
    uint32_t estimatedInputFrames = static_cast<uint32_t>(params.maxOutputFrames * sampleRateRatio);
    if (estimatedInputFrames == 0 && params.maxOutputFrames > 0) {
        estimatedInputFrames = 1;
    }
    uint32_t maxProcessableBytes = std::min(params.alignedCachedSize,
        static_cast<uint32_t>(estimatedInputFrames * params.inputBytesPerFrame));

    bool is11025SampleRate = (inputFormat_.sampleRate == 11025 || outputFormat_.sampleRate == 11025);

    if (!is11025SampleRate) {
        uint32_t maxFramesFor20ms = (static_cast<uint32_t>(inputFormat_.sampleRate) * MIN_FRAMES_FOR_20MS) /
            MILLISECONDS_PER_SECOND;
        uint32_t maxBytesFor20ms = maxFramesFor20ms * params.inputBytesPerFrame;
        maxProcessableBytes = std::min(maxProcessableBytes, maxBytesFor20ms);
    }
    CHECK_AND_RETURN_RET_LOG(params.minBytesForIntMs > 0,
        ERR_INVALID_PARAM,
        "The minimum number of bytes of input data to be processed cannot be zero.");
    if (maxProcessableBytes >= params.minBytesForIntMs) {
        if (is11025SampleRate) {
            alignedProcessableBytes = params.minBytesForIntMs;
        } else {
            alignedProcessableBytes = (maxProcessableBytes / params.minBytesForIntMs) * params.minBytesForIntMs;
        }
    } else if (params.alignedCachedSize >= params.minBytesForIntMs) {
        alignedProcessableBytes = params.minBytesForIntMs;
        AUDIO_DEBUG_LOG("Process: output capacity small, processing min unit and caching excess");
    } else {
        alignedProcessableBytes = 0;
    }

    if (alignedProcessableBytes == 0) {
        AUDIO_DEBUG_LOG("Process: insufficient data, maxProcessableBytes=%{public}d, minBytesForIntMs=%{public}d",
            maxProcessableBytes, params.minBytesForIntMs);
        shouldStopProcessing = true;
    }
    return SUCCESS;
}

uint32_t AudioFormatConverterImpl::CalculatePaddedOutputFrames(uint32_t paddedInputBytes) const
{
    if (paddedInputBytes == 0) {
        return 0;
    }
    
    uint32_t paddedInputFrames = paddedInputBytes / inSingleFrameBytes_;
    double sampleRateRatio = static_cast<double>(outputFormat_.sampleRate) /
        static_cast<double>(inputFormat_.sampleRate);
    double estimatedOutput = std::floor(paddedInputFrames * sampleRateRatio);
    uint32_t paddedOutputFrames = (estimatedOutput >= static_cast<double>(delayFrames_)) ?
        static_cast<uint32_t>(estimatedOutput) - delayFrames_ : 0;
    AUDIO_INFO_LOG("paddedOutputFrames:%{public}u.", paddedOutputFrames);
    return paddedOutputFrames;
}

int32_t AudioFormatConverterImpl::WrapAndConvertInputData(
    uint32_t processableSize, AudioSuitePcmBuffer* outConvertedBuffer)
{
    if (inputFinished_ && processableSize == 0) {
        return SUCCESS;
    }
    std::vector<uint8_t> rawOutput;
    int32_t ret = ProcessRawConversion(cachedInputData_.data(), processableSize, rawOutput);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_OPERATION_FAILED,
        "WrapAndConvertInputData: ProcessRawConversion failed, ret=%{public}d", ret);

    PcmBufferFormat outputPcmFmt(static_cast<AudioSamplingRate>(outputFormat_.sampleRate),
        outputFormat_.channelCount, outputFormat_.channelLayout, outputFormat_.sampleFormat);
    uint32_t outDurationMs = static_cast<uint32_t>(static_cast<double>(rawOutput.size()) * 1000.0 /
        (static_cast<double>(outputFormat_.sampleRate) * outSingleFrameBytes_));
    int32_t result = outConvertedBuffer->ResizePcmBuffer(outputPcmFmt, outDurationMs);
    CHECK_AND_RETURN_RET_LOG(result == 0, ERR_MEMORY_ALLOC_FAILED,
        "WrapAndConvertInputData: output buffer alloc failed");
    CHECK_AND_RETURN_RET_LOG(outConvertedBuffer->GetPcmData() != nullptr,
        ERR_MEMORY_ALLOC_FAILED, "WrapAndConvertInputData: output pcm data is nullptr");
    uint32_t outBufferSize = outConvertedBuffer->GetDataSize();
    CHECK_AND_RETURN_RET_LOG(outBufferSize >= rawOutput.size(),
        ERR_OPERATION_FAILED, "WrapAndConvertInputData: output buffer too small");
    errno_t copyRet = memcpy_s(outConvertedBuffer->GetPcmData(), outBufferSize, rawOutput.data(), rawOutput.size());
    CHECK_AND_RETURN_RET_LOG(copyRet == 0, ERR_OPERATION_FAILED,
        "WrapAndConvertInputData: memcpy_s failed");

    cachedInputData_.erase(cachedInputData_.begin(), cachedInputData_.begin() + processableSize);
    return SUCCESS;
}

int32_t AudioFormatConverterImpl::CheckAndProcessInputData(
    const InputProcessParams& params, uint32_t& alignedCachedSize,
    uint32_t& alignedProcessableBytes, bool& shouldStopProcessing)
{
    int32_t ret = CalculateMinBytesForIntMsInternal(alignedCachedSize, shouldStopProcessing);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "Failed to calculate minimum number of bytes to process");
    if (shouldStopProcessing) {
        return SUCCESS;
    }

    ProcessableBytesCalculationParams processableParams = {
        params.inputBytesPerFrame, params.maxOutputFrames,
        params.outputBytesPerFrame, processMinInBytes_, alignedCachedSize
    };
    ret = CalculateProcessableBytesInternal(processableParams, alignedProcessableBytes, shouldStopProcessing);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "Insufficient output capacity");

    return SUCCESS;
}

int32_t AudioFormatConverterImpl::InitializeProcessingContext(bool& shouldStopProcessing)
{
    uint32_t cachedSize = static_cast<uint32_t>(cachedInputData_.size());
    CHECK_AND_RETURN_RET_LOG(processMinInBytes_ != 0, ERR_INVALID_PARAM, "Minimum process input byptes cannot be 0.");
    uint32_t tempAlignedCachedSize = (cachedSize / processMinInBytes_) * processMinInBytes_;

    if (tempAlignedCachedSize == 0 && !inputFinished_) {
        int32_t ret = ReadAndCacheInputData(&shouldStopProcessing);
        if (ret != 0) {
            return ret;
        }
        if (shouldStopProcessing) {
            return SUCCESS;
        }
    }
    shouldStopProcessing = false;

    return SUCCESS;
}

int32_t AudioFormatConverterImpl::ProcessSingleBlock(void* audioData,
    uint32_t &totalOutputSize, uint32_t &remainingCapacity, bool& shouldStopProcessing)
{
    shouldStopProcessing = false;
    if (remainingCapacity < outSingleFrameBytes_) {
        shouldStopProcessing = true;
        return SUCCESS;
    }

    uint32_t alignedCachedSize = 0;
    uint32_t alignedProcessableBytes = 0;
    CHECK_AND_RETURN_RET_LOG(outSingleFrameBytes_ != 0, ERR_INVALID_PARAM, "The output frame length cannot be 0.");
    uint32_t currentMaxOutputFrames = remainingCapacity / outSingleFrameBytes_;

    InputProcessParams inputParams = {
        inSingleFrameBytes_, currentMaxOutputFrames, outSingleFrameBytes_, remainingCapacity};
    int32_t ret = CheckAndProcessInputData(
        inputParams, alignedCachedSize, alignedProcessableBytes, shouldStopProcessing);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "Input data processing failed");
    if (shouldStopProcessing) {
        return SUCCESS;
    }

    AudioSuitePcmBuffer convertedBuffer;
    ret = WrapAndConvertInputData(alignedProcessableBytes, &convertedBuffer);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "Data encapsulation and transformation process failed");

    ret = CopyOutputData(convertedBuffer, reinterpret_cast<uint8_t *>(audioData) + totalOutputSize,
        totalOutputSize, remainingCapacity);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "Data copy to output buffer failed.");

    return SUCCESS;
}

int32_t AudioFormatConverterImpl::ProcessCachedOutputData(void* audioData, uint32_t outputCapacity,
    uint32_t &totalOutputSize)
{
    if (cachedOutputData_.empty()) {
        return SUCCESS;
    }
    CHECK_AND_RETURN_RET_LOG(outSingleFrameBytes_ != 0, ERR_INVALID_PARAM, "The output frame length cannot be 0.");
    uint32_t cachedSize = static_cast<uint32_t>(cachedOutputData_.size());
    uint32_t copySize = (outputCapacity / outSingleFrameBytes_) * outSingleFrameBytes_;
    copySize = copySize > cachedSize ? cachedSize : copySize;

    errno_t err = memcpy_s(audioData, outputCapacity, cachedOutputData_.data(), copySize);
    CHECK_AND_RETURN_RET_LOG(err == 0, ERR_OPERATION_FAILED, "memcpy_s failed, err:%{public}u", err);

    totalOutputSize += copySize;

    if (copySize < cachedSize) {
        cachedOutputData_.erase(cachedOutputData_.begin(), cachedOutputData_.begin() + copySize);
    } else {
        cachedOutputData_.clear();
    }

    AUDIO_DEBUG_LOG("copy %{public}u bytes, remaining size: %{public}zu", copySize, cachedOutputData_.size());
    return SUCCESS;
}

int32_t AudioFormatConverterImpl::ProcessDataBlocks(
    void* audioData, uint32_t &totalOutputSize, uint32_t &remainingCapacity)
{
    int32_t ret = 0;
    CHECK_AND_RETURN_RET_LOG(outSingleFrameBytes_ != 0, ERR_INVALID_PARAM, "The output frame length cannot be 0.");
    uint32_t maxOutputFrames = remainingCapacity / outSingleFrameBytes_;
    while (maxOutputFrames > 0) {
        bool shouldStopProcessing = false;
        ret = ProcessSingleBlock(audioData, totalOutputSize, remainingCapacity, shouldStopProcessing);
        if (ret != 0 || shouldStopProcessing) {
            break;
        }
        maxOutputFrames = remainingCapacity / outSingleFrameBytes_;
    }
    return ret;
}

int32_t AudioFormatConverterImpl::ProcessConversion(void* audioData, uint32_t outputCapacity, uint32_t* outputSize)
{
    uint32_t totalOutputSize = 0;
    uint32_t remainingCapacity = outputCapacity;
    int32_t ret = ProcessCachedOutputData(audioData, outputCapacity, totalOutputSize);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "Failed to process output buffer data.");
    remainingCapacity -= totalOutputSize;

    if (remainingCapacity < static_cast<uint32_t>(outSingleFrameBytes_)) {
        *outputSize = totalOutputSize;
        return SUCCESS;
    }

    bool shouldStopProcessing = false;
    ret = InitializeProcessingContext(shouldStopProcessing);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "Input buffer initialization failed.");
    if (shouldStopProcessing) {
        *outputSize = totalOutputSize;
        return SUCCESS;
    }

    ret = ProcessDataBlocks(audioData, totalOutputSize, remainingCapacity);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "Failed to process data in the input buffer.");
    *outputSize = totalOutputSize;
    return SUCCESS;
}

int32_t AudioFormatConverterImpl::Process(void* outputData, uint32_t outputCapacity, uint32_t* outputSize)
{
    CHECK_AND_RETURN_RET_LOG(
        outputData != nullptr && outputSize != nullptr, ERR_INVALID_PARAM, "Process: Invalid parameters.");

    std::lock_guard<std::mutex> lock(mutex_);
    CHECK_AND_RETURN_RET_LOG(!destroyed_, ERR_ILLEGAL_STATE, "Process: converter already destroyed");
    CHECK_AND_RETURN_RET_LOG(outputCapacity >= outSingleFrameBytes_, ERR_BUFFER_TOO_SMALL,
        "Output buffer too small, must greater %{public}u.", outSingleFrameBytes_);
    *outputSize = 0;
    return ProcessConversion(outputData, outputCapacity, outputSize);
}

int32_t AudioFormatConverterImpl::CopyOutputData(
    AudioSuitePcmBuffer& buffer, void* outputBuffer, uint32_t &totalOutputSize, uint32_t &remainingCapacity)
{
    CHECK_AND_RETURN_RET_LOG(outputBuffer != nullptr, ERR_INVALID_PARAM, "outputBuffer is nullptr");
    uint32_t delayFrames = 0;
    uint32_t delaySize = 0;
    if (isFirstProcess_ && hasDelay_) {
        if (enableDataAlignment_) {
            delayFrames = delayFrames_ > buffer.GetFrameLen() ? buffer.GetFrameLen() : delayFrames_;
            delaySize = delayFrames * outSingleFrameBytes_;
        }
        isFirstProcess_ = false;
    }

    uint32_t paddedOutputFrames = 0;
    if (enableDataAlignment_ && paddedInputBytes_ > 0 && cachedInputData_.empty()) {
        paddedOutputFrames = CalculatePaddedOutputFrames(paddedInputBytes_);
    }

    uint32_t availableFrames = buffer.GetFrameLen() - delayFrames;
    if (availableFrames > paddedOutputFrames) {
        availableFrames -= paddedOutputFrames;
    } else {
        availableFrames = 0;
    }
    
    CHECK_AND_RETURN_RET_LOG(outSingleFrameBytes_ != 0, ERR_INVALID_PARAM, "The output frame length cannot be 0.");
    uint32_t maxFramesByCapacity = remainingCapacity / outSingleFrameBytes_;
    uint32_t framesToWrite = (availableFrames < maxFramesByCapacity) ? availableFrames : maxFramesByCapacity;
    uint32_t outputBytes = framesToWrite * outSingleFrameBytes_;

    CHECK_AND_RETURN_RET_LOG(buffer.GetPcmData() != nullptr, ERR_INVALID_PARAM, "pcm data is nullptr");
    int32_t ret = memcpy_s(outputBuffer, remainingCapacity, buffer.GetPcmData() + delaySize, outputBytes);
    CHECK_AND_RETURN_RET_LOG(ret == 0, ERR_OPERATION_FAILED, "memcpy_s failed: memcpy_s for cached data failed");

    if (framesToWrite < availableFrames) {
        uint32_t remainingBytes = availableFrames * outSingleFrameBytes_ - outputBytes;
        uint32_t existingSize = static_cast<uint32_t>(cachedOutputData_.size());

        cachedOutputData_.resize(existingSize + remainingBytes);
        errno_t ret = memcpy_s(cachedOutputData_.data() + existingSize, remainingBytes,
            buffer.GetPcmData() + delaySize + outputBytes, remainingBytes);
        CHECK_AND_RETURN_RET_LOG(ret == 0, ERR_OPERATION_FAILED, "memcpy_s fail, ret:%{public}d.", ret);

        AUDIO_DEBUG_LOG("CopyOutputData: cached %{public}u bytes of uncopied output data", remainingBytes);
    }

    totalOutputSize += outputBytes;
    remainingCapacity -= outputBytes;
    return SUCCESS;
}

size_t AudioFormatConverterImpl::GetDelayDataSize()
{
    return delayFrames_ * outSingleFrameBytes_;
}

int32_t AudioFormatConverterImpl::GetDelaySize(uint32_t &delaySize)
{
    std::lock_guard<std::mutex> lock(mutex_);
    CHECK_AND_RETURN_RET_LOG(!destroyed_, ERR_INVALID_PARAM, "GetDelaySize: converter already destroyed");
    delaySize = static_cast<uint32_t>(GetDelayDataSize());
    return SUCCESS;
}

int32_t AudioFormatConverterImpl::DirectProcess(const uint8_t* inData, uint32_t inSize,
    void* outData, uint32_t outCapacity, uint32_t &outSize)
{
    std::lock_guard<std::mutex> lock(mutex_);
    CHECK_AND_RETURN_RET_LOG(!destroyed_, ERR_INVALID_PARAM, "DirectProcess: converter already destroyed");
    CHECK_AND_RETURN_RET_LOG(dataCallback_ == nullptr, ERR_NOT_SUPPORTED,
        "DirectProcess: already in callback mode, mutually exclusive");
    CHECK_AND_RETURN_RET_LOG(inData != nullptr && inSize > 0, ERR_INVALID_PARAM, "DirectProcess: invalid input data");
    CHECK_AND_RETURN_RET_LOG(outData != nullptr, ERR_INVALID_PARAM, "DirectProcess: invalid output params");
    CHECK_AND_RETURN_RET_LOG(minInSize_ > 0 && inSize % minInSize_ == 0,
        ERR_INVALID_PARAM, "DirectProcess: inSize must be multiple of minInBytes");

    uint32_t inFrames = inSize / inSingleFrameBytes_;
    uint64_t maxInFrames = static_cast<uint64_t>(inputFormat_.sampleRate) * MAX_INPUT_DURATION_MS
                           / MILLISECONDS_PER_SECOND;
    CHECK_AND_RETURN_RET_LOG(inFrames <= maxInFrames, ERR_INVALID_PARAM,
        "DirectProcess: input duration exceeds %{public}ums, inFrames=%{public}u", MAX_INPUT_DURATION_MS, inFrames);
    uint64_t estOutBytes = static_cast<uint64_t>(inFrames) * outSingleFrameBytes_
                          * static_cast<uint64_t>(outputFormat_.sampleRate)
                          / static_cast<uint64_t>(inputFormat_.sampleRate);
    CHECK_AND_RETURN_RET_LOG(static_cast<uint64_t>(outCapacity) >= estOutBytes, ERR_BUFFER_TOO_SMALL,
        "DirectProcess: output buffer too small, need≥%{public}llu have=%{public}u",
        static_cast<unsigned long long>(estOutBytes), outCapacity);
    directProcessMode_ = true;

    std::vector<uint8_t> blockOutput;
    int32_t ret = ProcessRawConversion(inData, inSize, blockOutput);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "DirectProcess: ProcessRawConversion failed, ret=%{public}d", ret);
    if (isFirstProcess_ && hasDelay_) {
        isFirstProcess_ = false;
    }
    outSize = static_cast<uint32_t>(blockOutput.size());
    CHECK_AND_RETURN_RET_LOG(outSize <= outCapacity, ERR_BUFFER_TOO_SMALL,
        "DirectProcess: output exceeds capacity, need=%{public}u have=%{public}u", outSize, outCapacity);
    errno_t err = memcpy_s(outData, outCapacity, blockOutput.data(), outSize);
    CHECK_AND_RETURN_RET_LOG(err == 0, ERR_OPERATION_FAILED, "DirectProcess: memcpy_s failed");
    return SUCCESS;
}

int32_t AudioFormatConverterImpl::Flush()
{
    std::lock_guard<std::mutex> lock(mutex_);
    CHECK_AND_RETURN_RET_LOG(!destroyed_, ERR_INVALID_PARAM, "Flush: converter already destroyed");
    ResetResamplerState();
    ChannelConvertReset();
    int32_t ret = InitChannelConverter();
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_OPERATION_FAILED,
        "Flush: InitChannelConverter failed, err:%{public}d", ret);
    isFirstProcess_ = true;
    delayFrames_ = 0;
    floatBuffer_.clear();
    resampleBuffer_.clear();
    channelBuffer_.clear();
    cachedInputData_.clear();
    cachedOutputData_.clear();
    inputFinished_ = false;
    isPadded_ = false;
    paddedInputBytes_ = 0;
    return SUCCESS;
}

void AudioFormatConverterImpl::Destroy()
{
    std::lock_guard<std::mutex> lock(mutex_);
    CHECK_AND_RETURN_LOG(!destroyed_, "Destroy: already destroyed, skipping");
    ResetRawConversion();
    DestroyResamplerState();
    inputFinished_ = false;
    cachedInputData_.clear();
    cachedOutputData_.clear();
    dataCallback_ = nullptr;
    hasDelay_ = false;
    isFirstProcess_ = true;
    isPadded_ = false;
    destroyed_ = true;
    paddedInputBytes_ = 0;
    delayFrames_ = 0;
    enableDataAlignment_ = false;
    directProcessMode_ = false;
    minInSize_ = 0;
    minOutSize_ = 0;
    AUDIO_INFO_LOG("Destroy: success");
}

} // namespace AudioSuite
} // namespace AudioStandard
} // namespace OHOS