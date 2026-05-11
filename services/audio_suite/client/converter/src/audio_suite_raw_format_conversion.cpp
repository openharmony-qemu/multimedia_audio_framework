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

#ifndef LOG_TAG
#define LOG_TAG "AudioSuiteRawFormatConversion"
#endif

#include "audio_suite_raw_format_conversion.h"
#include "audio_errors.h"
#include "audio_suite_log.h"
#include "audio_suite_common.h"
#include <cmath>
#include <cstring>
#include <securec.h>

namespace OHOS {
namespace AudioStandard {
namespace AudioSuite {

constexpr uint32_t MILLISECONDS_PER_SECOND = 1000;
constexpr uint32_t FLOAT_SIZE_BYTES = sizeof(float);

AudioSuiteRawFormatConversion::AudioSuiteRawFormatConversion(const uint32_t resampleQuality)
    : initialized_(false), formatsSame_(false), resampleQuality_(resampleQuality)
{}

AudioSuiteRawFormatConversion::~AudioSuiteRawFormatConversion()
{
}

int32_t AudioSuiteRawFormatConversion::Init(const PcmBufferFormat &inputFormat, const PcmBufferFormat &outputFormat)
{
    inputFormat_ = inputFormat;
    outputFormat_ = outputFormat;
    initialized_ = true;

    // Check if input and output formats are identical
    formatsSame_ = (inputFormat_.sampleRate == outputFormat_.sampleRate &&
                    inputFormat_.channelCount == outputFormat_.channelCount &&
                    inputFormat_.channelLayout == outputFormat_.channelLayout &&
                    inputFormat_.sampleFormat == outputFormat_.sampleFormat);

    // Initialize resampler if sample rates differ
    if (inputFormat_.sampleRate != outputFormat_.sampleRate) {
        proResampler_ = std::make_unique<HPAE::ProResampler>(
            inputFormat_.sampleRate, outputFormat_.sampleRate, inputFormat_.channelCount, resampleQuality_);
        CHECK_AND_RETURN_RET_LOG(proResampler_ != nullptr, ERR_NO_MEMORY, "Init: Failed to create resampler");
    }

    // Initialize channel converter if channel layouts differ
    if (inputFormat_.channelLayout != outputFormat_.channelLayout) {
        AudioChannelInfo inChannelInfo = {inputFormat_.channelLayout, inputFormat_.channelCount};
        AudioChannelInfo outChannelInfo = {outputFormat_.channelLayout, outputFormat_.channelCount};

        channelConverter_.SetUpmixCoef(HPAE::COEF_M6DB_F);
        int32_t ret = channelConverter_.SetParam(inChannelInfo, outChannelInfo, SAMPLE_F32LE, true);
        CHECK_AND_RETURN_RET_LOG(
            ret == SUCCESS, ERR_OPERATION_FAILED, "Init: ChannelConverter SetParam failed, err:%{public}d", ret);
    }

    AUDIO_INFO_LOG("Init: src=[%{public}u %{public}u], dst=[%{public}u %{public}u], same=%{public}d",
        inputFormat_.sampleRate,
        inputFormat_.channelCount,
        outputFormat_.sampleRate,
        outputFormat_.channelCount,
        formatsSame_);

    return SUCCESS;
}

void AudioSuiteRawFormatConversion::Reset()
{
    if (proResampler_ != nullptr) {
        proResampler_->Reset();
    }

    channelConverter_.Reset();
    initialized_ = false;
    formatsSame_ = false;

    floatBuffer_.clear();
    resampleBuffer_.clear();
    channelBuffer_.clear();
    outputBuffer_.clear();
}

int32_t AudioSuiteRawFormatConversion::ProcessResample(
    const float *inputData, uint32_t inSampleCount, float *&outData, uint32_t &outSampleCount)
{
    CHECK_AND_RETURN_RET_LOG(
        proResampler_ != nullptr, ERR_OPERATION_FAILED, "ProcessResample: proResampler_ is nullptr");

    uint32_t inFrameSize = inSampleCount / inputFormat_.channelCount;
    double samplesPerOneMs = static_cast<double>(inputFormat_.sampleRate) / MILLISECONDS_PER_SECOND;
    double samplesPerMs = static_cast<double>(inFrameSize) / samplesPerOneMs;
    uint32_t outFrameSize = static_cast<double>(outputFormat_.sampleRate) / MILLISECONDS_PER_SECOND * samplesPerMs;

    uint32_t additionalFrames = (inputFormat_.sampleRate == SAMPLE_RATE_11025) ? outFrameSize : 0;
    uint32_t totalOutFrameSize = outFrameSize + additionalFrames;
    uint32_t outSampleCountLocal = totalOutFrameSize * inputFormat_.channelCount;
    resampleBuffer_.resize(outSampleCountLocal);

    uint32_t actualOutFrameSize = outFrameSize;
    int32_t ret = proResampler_->Process(inputData, inFrameSize, resampleBuffer_.data(), actualOutFrameSize);
    CHECK_AND_RETURN_RET_LOG(
        ret == SUCCESS, ERR_OPERATION_FAILED, "ProcessResample: failed with error code: %{public}d", ret);

    outData = resampleBuffer_.data();
    outSampleCount = actualOutFrameSize * inputFormat_.channelCount;
    return SUCCESS;
}

int32_t AudioSuiteRawFormatConversion::ProcessChannelConvert(
    const float *inputData, uint32_t inSampleCount, float *&outData, uint32_t &outSampleCount)
{
    CHECK_AND_RETURN_RET_LOG(
        inputFormat_.channelCount > 0, ERR_INVALID_PARAM, "ProcessChannelConvert: inputFormat_.channelCount is 0");
    CHECK_AND_RETURN_RET_LOG(
        outputFormat_.channelCount > 0, ERR_INVALID_PARAM, "ProcessChannelConvert: outputFormat_.channelCount is 0");

    // Note: inputData comes from resampleBuffer_, which has inputFormat_.channelCount channels
    // (resampling does not change channel count)
    uint32_t frameSize = inSampleCount / inputFormat_.channelCount;
    uint32_t outSampleCountLocal = frameSize * outputFormat_.channelCount;

    channelBuffer_.resize(outSampleCountLocal);

    uint32_t inLen = inSampleCount * FLOAT_SIZE_BYTES;
    uint32_t outLen = outSampleCountLocal * FLOAT_SIZE_BYTES;

    int32_t ret =
        channelConverter_.Process(frameSize, const_cast<float *>(inputData), inLen, channelBuffer_.data(), outLen);
    CHECK_AND_RETURN_RET_LOG(
        ret == SUCCESS, ERR_OPERATION_FAILED, "ProcessChannelConvert: failed with error code: %{public}d", ret);

    outData = channelBuffer_.data();
    outSampleCount = outSampleCountLocal;
    return SUCCESS;
}

int32_t AudioSuiteRawFormatConversion::ConvertToFloat(
    const uint8_t *inData, uint32_t inDataSize, std::vector<float> &outBuffer)
{
    uint32_t inputSampleSize = AudioSuiteUtil::GetSampleSize(inputFormat_.sampleFormat);
    CHECK_AND_RETURN_RET_LOG(inputSampleSize != 0, ERR_INVALID_PARAM, "inputSampleSize cannot be zero.");
    uint32_t inSampleCount = inDataSize / inputSampleSize;

    outBuffer.resize(inSampleCount);

    if (inputFormat_.sampleFormat != SAMPLE_F32LE) {
        HPAE::ConvertToFloat(inputFormat_.sampleFormat,
            inSampleCount,
            static_cast<void *>(const_cast<uint8_t *>(inData)),
            outBuffer.data());
    } else {
        errno_t copyRet = memcpy_s(outBuffer.data(), outBuffer.size() * sizeof(float), inData, inDataSize);
        CHECK_AND_RETURN_RET_LOG(copyRet == 0, ERR_OPERATION_FAILED, "ConvertToFloat: memcpy_s failed");
    }
    return SUCCESS;
}

int32_t AudioSuiteRawFormatConversion::ConvertFromFloat(
    const float *inData, uint32_t inSampleCount, std::vector<uint8_t> &outBuffer)
{
    uint32_t outputSampleSize = (outputFormat_.sampleFormat != SAMPLE_F32LE)
                                    ? AudioSuiteUtil::GetSampleSize(outputFormat_.sampleFormat)
                                    : sizeof(float);
    uint32_t outputBytes = inSampleCount * outputSampleSize;

    outBuffer.resize(outputBytes);

    if (outputFormat_.sampleFormat != SAMPLE_F32LE) {
        HPAE::ConvertFromFloat(
            outputFormat_.sampleFormat, inSampleCount, const_cast<float *>(inData), outBuffer.data());
    } else {
        errno_t copyRet = memcpy_s(outBuffer.data(), outBuffer.size(), inData, outputBytes);
        CHECK_AND_RETURN_RET_LOG(copyRet == 0, ERR_OPERATION_FAILED, "ConvertFromFloat: memcpy_s failed");
    }
    return SUCCESS;
}

int32_t AudioSuiteRawFormatConversion::Process(
    const uint8_t *inData, uint32_t inDataSize, std::vector<uint8_t> &outputVector)
{
    std::lock_guard<std::mutex> lock(mutex_);

    CHECK_AND_RETURN_RET_LOG(initialized_, ERR_INVALID_PARAM, "Process: not initialized");
    CHECK_AND_RETURN_RET_LOG(inData != nullptr, ERR_INVALID_PARAM, "Process: inData is nullptr");
    CHECK_AND_RETURN_RET_LOG(inDataSize > 0, ERR_INVALID_PARAM, "Process: inDataSize is 0");

    // 1. If input and output formats are the same, just copy and return
    if (formatsSame_) {
        outputVector.resize(inDataSize);
        errno_t copyRet = memcpy_s(outputVector.data(), outputVector.size(), inData, inDataSize);
        CHECK_AND_RETURN_RET_LOG(copyRet == 0, ERR_OPERATION_FAILED, "Process: memcpy_s failed");
        return SUCCESS;
    }

    // 2. Convert to float
    int32_t ret = ConvertToFloat(inData, inDataSize, floatBuffer_);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_OPERATION_FAILED, "Process: ConvertToFloat failed");

    float *currentData = floatBuffer_.data();
    uint32_t currentSampleCount = floatBuffer_.size();

    // 3. Resample if needed
    if (inputFormat_.sampleRate != outputFormat_.sampleRate) {
        ret = ProcessResample(currentData, currentSampleCount, currentData, currentSampleCount);
        CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_OPERATION_FAILED, "Process: Resample failed");
    }

    // 4. Channel convert if needed
    if (inputFormat_.channelLayout != outputFormat_.channelLayout) {
        ret = ProcessChannelConvert(currentData, currentSampleCount, currentData, currentSampleCount);
        CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_OPERATION_FAILED, "Process: ChannelConvert failed");
    }

    // 5. Convert to output format
    ret = ConvertFromFloat(currentData, currentSampleCount, outputVector);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_OPERATION_FAILED, "Process: ConvertFromFloat failed");

    return SUCCESS;
}

}  // namespace AudioSuite
}  // namespace AudioStandard
}  // namespace OHOS