/*
 * Copyright (c) 2026-2026 Huawei Device Co., Ltd.
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
#define LOG_TAG "MixerProcessorNode"
#endif

#include <mutex>
#include <vector>
#include <cinttypes>
#include "audio_utils.h"
#include "audio_suite_base.h"
#include "audio_suite_common.h"
#include "audio_suite_mixer_processor_node.h"
#include "hpae_format_convert.h"

namespace OHOS {
namespace AudioStandard {
namespace AudioSuite {

constexpr uint32_t MIXER_PROCESS_DATA_MAX_MS = 60;
constexpr uint32_t SECONDS_TO_MS = 1000;
constexpr float MIXER_STREAM_VOLUME_MIN = 0.0f;
constexpr float MIXER_STREAM_VOLUME_MAX = 1.0f;
constexpr uint32_t SAMPLES_EVEN_MULTIPLE = 2;
constexpr size_t FORMAT_S24_BYTES = 3;
constexpr int BIT_LEN_8 = 8;
constexpr int BIT_LEN_16 = 16;
constexpr size_t BIT_LEN_24 = 24;
constexpr int64_t INT24_MAX = (1 << (BIT_LEN_24 - 1)) - 1;
constexpr int64_t INT24_MIN = -(1 << (BIT_LEN_24 - 1));
constexpr int32_t U8_FORMAT_ZERO_OFFSET = 0x80;
constexpr int64_t BIT_MASK = 0xFF;
constexpr size_t LSB_BYTE_INDEX = 0;
constexpr size_t MID_BYTE_INDEX = 1;
constexpr size_t MSB_BYTE_INDEX = 2;

MixerProcessorNode::MixerProcessorNode()
    : isInitialized_(false)
{
    AUDIO_DEBUG_LOG("MixerProcessor default constructor");
}

MixerProcessorNode::~MixerProcessorNode()
{
    Destroy();
}

void MixerProcessorNode::Destroy()
{
    std::lock_guard<std::mutex> lock(mutex_);
    limiter_.reset();
    isInitialized_ = false;
    AUDIO_DEBUG_LOG("MixerProcessor::Destroy end");
}

int32_t MixerProcessorNode::Init(const AudioFormat &format, bool enableLimiter)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (isInitialized_) {
        AUDIO_ERR_LOG("Already initialized, cannot init again");
        return ERR_ILLEGAL_STATE;
    }

    int32_t ret = ValidateFormat(format);
    if (ret != SUCCESS) {
        AUDIO_ERR_LOG("ValidateFormat failed, ret: %{public}d", ret);
        return ret;
    }

    format_ = format;
    enableLimiter_ = enableLimiter;
    if (enableLimiter_ && limiter_ == nullptr) {
        limiter_ = std::make_unique<AudioLimiter>(0);
    }

    if (format_.audioChannelInfo.channelLayout != CH_LAYOUT_STEREO || format_.audioChannelInfo.numChannels != STEREO) {
        AudioChannelInfo converterChannel = {
            format_.audioChannelInfo.channelLayout, format_.audioChannelInfo.numChannels};
        AudioChannelInfo stereoChannel = {CH_LAYOUT_STEREO, STEREO};

        channelToStereo_.SetUpmixCoef(HPAE::COEF_M6DB_F);
        int32_t ret = channelToStereo_.SetParam(converterChannel, stereoChannel, SAMPLE_F32LE, true);
        CHECK_AND_RETURN_RET_LOG(
            ret == SUCCESS, ERR_OPERATION_FAILED, "Init: channelToStereo SetParam failed, err:%{public}d", ret);

        channelFromStereo_.SetUpmixCoef(HPAE::COEF_M6DB_F);
        ret = channelFromStereo_.SetParam(stereoChannel, converterChannel, SAMPLE_F32LE, true);
        CHECK_AND_RETURN_RET_LOG(
            ret == SUCCESS, ERR_OPERATION_FAILED, "Init: channelFromeStereo SetParam failed, err:%{public}d", ret);
        channelConvert_ = true;
    }

    formatConvert_ = (format_.format != SAMPLE_F32LE);
    bitDepth_ = AudioSuiteUtil::GetSampleSize(format_.format);

    isInitialized_ = true;
    AUDIO_INFO_LOG("MixerProcessor::Init end");
    return SUCCESS;
}

int32_t MixerProcessorNode::ValidateFormat(const AudioFormat &format)
{
    AudioChannelLayout layout = format.audioChannelInfo.channelLayout;
    uint32_t numChannels = format.audioChannelInfo.numChannels;
    AUDIO_INFO_LOG("mixer format layout:%{public}" PRIu64 ", channels:%{public}u, sample:%{public}u, format:%{public}u",
        layout, numChannels, static_cast<uint32_t>(format.rate), static_cast<uint32_t>(format.format));

    auto it = LAYOUT_TO_CHANNEL.find(layout);
    if (it == LAYOUT_TO_CHANNEL.end()) {
        AUDIO_ERR_LOG("Invalid channelLayout.");
        return ERR_INVALID_PARAM;
    }

    if (it->second != static_cast<AudioChannel>(numChannels)) {
        AUDIO_ERR_LOG("numChannels %{public}u mismatch with calc channels %{public}u", numChannels, it->second);
        return ERR_INVALID_PARAM;
    }

    if (NotContain(AUDIO_SUPPORTED_FORMATS, format.format)) {
        AUDIO_ERR_LOG("Invalid format: %{public}d", static_cast<int32_t>(format.format));
        return ERR_INVALID_PARAM;
    }

    if (NotContain(AUDIO_SUPPORTED_SAMPLING_RATES, format.rate)) {
        AUDIO_ERR_LOG("Invalid sampleRate: %{public}u", static_cast<uint32_t>(format.rate));
        return ERR_INVALID_PARAM;
    }

    return SUCCESS;
}

int32_t MixerProcessorNode::ProcessCheck(const std::vector<AudioMixStream> &streams,
    void *outData, uint32_t outCapacity, uint32_t *outSize)
{
    CHECK_AND_RETURN_RET_LOG(isInitialized_, ERR_ILLEGAL_STATE, "limiter Not initialized");
    CHECK_AND_RETURN_RET_LOG(outData != nullptr && outSize != nullptr,
        ERR_INVALID_PARAM, "outData or outSize is nullptr");
    CHECK_AND_RETURN_RET_LOG(!streams.empty(), ERR_INVALID_PARAM, "streams is empty");
    AUDIO_DEBUG_LOG("streams count:%{public}zu, inSize:%{public}u outCapacity:%{public}u",
        streams.size(), streams[0].dataSize, outCapacity);

    uint32_t inputDataSize = streams[0].dataSize;
    for (size_t i = 0; i < streams.size(); ++i) {
        CHECK_AND_RETURN_RET_LOG(streams[i].data != nullptr, ERR_INVALID_PARAM,
            "Stream %{public}zu data is nullptr", i);
        CHECK_AND_RETURN_RET_LOG(streams[i].dataSize == inputDataSize, ERR_INVALID_PARAM,
            "Stream %{public}zu dataSize mismatch", i);
        CHECK_AND_RETURN_RET_LOG(streams[i].volume >= MIXER_STREAM_VOLUME_MIN &&
            streams[i].volume <= MIXER_STREAM_VOLUME_MAX, ERR_INVALID_PARAM, "stream %{public}zu not in[0.0, 1.0]", i);
    }
    CHECK_AND_RETURN_RET_LOG(inputDataSize != 0, ERR_INVALID_PARAM,
        "Stream dataSize:%{public}u must be greater than 0 ", streams[0].dataSize);

    uint32_t channels = format_.audioChannelInfo.numChannels;
    uint32_t sampleRate = static_cast<uint32_t>(format_.rate);
    uint32_t frameCount = inputDataSize / bitDepth_ / channels;
    // Data size must be a multiple of channels and bit depth
    if ((inputDataSize % (channels * bitDepth_)) != 0) {
        AUDIO_ERR_LOG("inputDataSize %{public}u not aligned to sign frame %{public}u",
            inputDataSize, channels * bitDepth_);
        return ERR_INVALID_PARAM;
    }

    if (enableLimiter_) {
        // The number of samples in a single channel must be an even multiple
        CHECK_AND_RETURN_RET_LOG(frameCount % SAMPLES_EVEN_MULTIPLE == 0, ERR_INVALID_PARAM,
            "Single-channel samples:%{public}u must be an even multiple", frameCount);
    }
    // Maximum length cannot exceed 60ms
    uint32_t maxDataLen = channels * bitDepth_ * sampleRate * MIXER_PROCESS_DATA_MAX_MS / SECONDS_TO_MS;
    CHECK_AND_RETURN_RET_LOG(inputDataSize <= maxDataLen, ERR_INVALID_PARAM,
        "inputDataSize %{public}u exceeds max 60ms maxDataLen %{public}u", inputDataSize, maxDataLen);
    // Output capacity cannot be less than input size
    CHECK_AND_RETURN_RET_LOG(outCapacity >= inputDataSize, ERR_INVALID_PARAM,
        "Output buffer capacity %{public}u insufficient for inputDataSize %{public}u", outCapacity, inputDataSize);

    // Initialize only on first time, report error if length changes later
    if (enableLimiter_ && streams[0].dataSize != chunkSize_) {
        if (chunkSize_ == 0) {
            int32_t ret = limiter_->SetConfig(frameCount * STEREO * sizeof(float),
                sizeof(float), static_cast<int32_t>(format_.rate), STEREO);
            CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "limiter SetConfig failed, ret: %{public}d", ret);
            chunkSize_ = streams[0].dataSize;
        } else {
            AUDIO_ERR_LOG("The processing length of the mixer has changed, old size:%{public}u, new size:%{public}u",
                chunkSize_, streams[0].dataSize);
            return ERR_INVALID_PARAM;
        }
    }
    return SUCCESS;
}

int32_t MixerProcessorNode::MixStreams(const std::vector<AudioMixStream> &streams,
    uint32_t sampleCount, uint32_t frameCount, float *mixOut)
{
    for (const auto &stream : streams) {
        CHECK_AND_RETURN_RET_LOG(stream.data != nullptr, ERR_INVALID_PARAM, "mixer Stream data is nullptr");
        float *inBuffer = reinterpret_cast<float *>(stream.data);
        uint32_t inCount = sampleCount;
        if (formatConvert_) {
            bitDepthBuffer_.resize(sampleCount);
            HPAE::ConvertToFloat(format_.format, sampleCount, stream.data, bitDepthBuffer_.data());
            inBuffer = bitDepthBuffer_.data();
            inCount = bitDepthBuffer_.size();
        }

        if (channelConvert_) {
            channelBuffer_.resize(frameCount * STEREO);
            int32_t ret = channelToStereo_.Process(frameCount, inBuffer, inCount * sizeof(float),
                channelBuffer_.data(),  channelBuffer_.size() * sizeof(float));
            CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_OPERATION_FAILED, "convert channel err: %{public}d", ret);
            inBuffer = channelBuffer_.data();
            inCount = channelBuffer_.size();
        }

        for (uint32_t i = 0; i < inCount; ++i) {
            mixOut[i] += inBuffer[i];
        }
    }
    return SUCCESS;
}

int32_t MixerProcessorNode::Process(const std::vector<AudioMixStream> &streams,
    void *outData, uint32_t outCapacity, uint32_t *outSize)
{
    std::lock_guard<std::mutex> lock(mutex_);
    int32_t ret = ProcessCheck(streams, outData, outCapacity, outSize);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "ProcessCheck failed, ret: %{public}d", ret);

    if (enableLimiter_) {
        return MixerLimiter(streams, outData, outCapacity, outSize);
    } else {
        switch (format_.format) {
            case AudioSampleFormat::SAMPLE_U8:
                MixU8Volume(streams, outData, outCapacity, outSize);
                return SUCCESS;
            case AudioSampleFormat::SAMPLE_S16LE:
                MixS16Volume(streams, outData, outCapacity, outSize);
                return SUCCESS;
            case AudioSampleFormat::SAMPLE_S24LE:
                MixS24Volume(streams, outData, outCapacity, outSize);
                return SUCCESS;
            case AudioSampleFormat::SAMPLE_S32LE:
                MixS32Volume(streams, outData, outCapacity, outSize);
                return SUCCESS;
            case AudioSampleFormat::SAMPLE_F32LE:
                MixF32Volume(streams, outData, outCapacity, outSize);
                return SUCCESS;
            default:
                AUDIO_ERR_LOG("mixer error, format:%{public}u invalid", format_.format);
                return ERR_INVALID_PARAM;
        }
    }
}

int32_t MixerProcessorNode::MixerLimiter(const std::vector<AudioMixStream> &streams,
    void *outData, uint32_t outCapacity, uint32_t *outSize)
{
    uint32_t expectedDataSize = streams[0].dataSize;
    uint32_t sampleCount = expectedDataSize / bitDepth_;
    uint32_t frameCount = sampleCount / format_.audioChannelInfo.numChannels;
    mixOutput_.assign(frameCount * STEREO, 0.0f);
    limiterOutput_.resize(frameCount * STEREO);
    float *mixOut = mixOutput_.data();

    int32_t ret = MixStreams(streams, sampleCount, frameCount, mixOut);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "MixStreams failed, ret: %{public}d", ret);

    ret = limiter_->Process(frameCount * STEREO, mixOut, limiterOutput_.data());
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "limiter Process failed, ret: %{public}d", ret);

    float *outBuffer = limiterOutput_.data();
    uint32_t outCount = limiterOutput_.size();
    if (channelConvert_) {
        channelBuffer_.resize(sampleCount);
        ret = channelFromStereo_.Process(frameCount,
            outBuffer, outCount * sizeof(float), channelBuffer_.data(), channelBuffer_.size() * sizeof(float));
        CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_OPERATION_FAILED, "convert stereo channel err: %{public}d", ret);
        outBuffer = channelBuffer_.data();
        outCount = channelBuffer_.size();
    }

    if (formatConvert_) {
        HPAE::ConvertFromFloat(format_.format, sampleCount, outBuffer, outData);
    } else {
        errno_t err = memcpy_s(outData, outCapacity, reinterpret_cast<void *>(outBuffer), outCount * sizeof(float));
        CHECK_AND_RETURN_RET_LOG(err == 0, ERR_OPERATION_FAILED, "memcpy_s execution failed, ret: %{public}d", err);
    }

    *outSize = expectedDataSize;
    return SUCCESS;
}

void MixerProcessorNode::MixU8Volume(const std::vector<AudioMixStream> &streams,
    void *outData, uint32_t outCapacity, uint32_t *outSize)
{
    size_t loopCount = streams[0].dataSize / sizeof(uint8_t);
    uint8_t *dstPtr = reinterpret_cast<uint8_t *>(outData);
    for (size_t offset = 0; loopCount > 0; loopCount--) {
        int32_t sum = 0;
        for (size_t i = 0; i < streams.size(); i++) {
            uint8_t *srcPtr = reinterpret_cast<uint8_t *>(streams[i].data) + offset;
            sum += (static_cast<int32_t>(*srcPtr) - U8_FORMAT_ZERO_OFFSET);
        }
        offset++;
        sum = sum > INT8_MAX ? INT8_MAX : (sum < INT8_MIN ? INT8_MIN : sum);
        *dstPtr++ = sum + U8_FORMAT_ZERO_OFFSET;
    }
    *outSize = streams[0].dataSize;
}

void MixerProcessorNode::MixS16Volume(const std::vector<AudioMixStream> &streams,
    void *outData, uint32_t outCapacity, uint32_t *outSize)
{
    size_t loopCount = streams[0].dataSize / sizeof(int16_t);
    int16_t *dstPtr = reinterpret_cast<int16_t *>(outData);
    for (size_t offset = 0; loopCount > 0; loopCount--) {
        int32_t sum = 0;
        for (size_t i = 0; i < streams.size(); i++) {
            int16_t *srcPtr = reinterpret_cast<int16_t *>(streams[i].data) + offset;
            sum += *srcPtr;
        }
        offset++;
        *dstPtr++ = sum > INT16_MAX ? INT16_MAX : (sum < INT16_MIN ? INT16_MIN : sum);
    }
    *outSize = streams[0].dataSize;
}

void MixerProcessorNode::MixS24Volume(const std::vector<AudioMixStream> &streams,
    void *outData, uint32_t outCapacity, uint32_t *outSize)
{
    size_t loopCount = streams[0].dataSize / FORMAT_S24_BYTES;
    uint8_t *dstPtr = reinterpret_cast<uint8_t *>(outData);
    for (size_t offset = 0; loopCount > 0; loopCount--, offset += FORMAT_S24_BYTES) {
        int64_t sum = 0;
        for (size_t i = 0; i < streams.size(); i++) {
            uint8_t *srcBuffer = reinterpret_cast<uint8_t *>(streams[i].data) + offset;
            uint32_t srcAudioBuffer24Bit = (srcBuffer[MSB_BYTE_INDEX] << BIT_LEN_16) |
                (srcBuffer[MID_BYTE_INDEX] << BIT_LEN_8) | srcBuffer[LSB_BYTE_INDEX];
            int32_t srcAudioData24Bit = (static_cast<int32_t>(srcAudioBuffer24Bit) << BIT_LEN_8) >> BIT_LEN_8;
            sum += srcAudioData24Bit;
        }
        sum = std::min(INT24_MAX, std::max(INT24_MIN, sum));
        *dstPtr++ = static_cast<uint8_t>(sum & BIT_MASK);
        *dstPtr++ = static_cast<uint8_t>((sum >> BIT_LEN_8) & BIT_MASK);
        *dstPtr++ = static_cast<uint8_t>((sum >> BIT_LEN_16) & BIT_MASK);
    }
    *outSize = streams[0].dataSize;
}

void MixerProcessorNode::MixS32Volume(const std::vector<AudioMixStream> &streams,
    void *outData, uint32_t outCapacity, uint32_t *outSize)
{
    size_t loopCount = streams[0].dataSize / sizeof(int32_t);
    int32_t *dstPtr = reinterpret_cast<int32_t *>(outData);
    for (size_t offset = 0; loopCount > 0; loopCount--) {
        int64_t sum = 0;
        for (size_t i = 0; i < streams.size(); i++) {
            int32_t *srcPtr = reinterpret_cast<int32_t *>(streams[i].data) + offset;
            sum += *srcPtr;
        }
        offset++;
        sum = sum > INT32_MAX ? INT32_MAX : (sum < INT32_MIN ? INT32_MIN : sum);
        *dstPtr++ = static_cast<int32_t>(sum);
    }
    *outSize = streams[0].dataSize;
}

void MixerProcessorNode::MixF32Volume(const std::vector<AudioMixStream> &streams,
    void *outData, uint32_t outCapacity, uint32_t *outSize)
{
    size_t loopCount = streams[0].dataSize / sizeof(float);
    float *dstPtr = reinterpret_cast<float *>(outData);
    for (size_t offset = 0; loopCount > 0; loopCount--) {
        float sum = 0.0f;
        for (size_t i = 0; i < streams.size(); i++) {
            float *srcPtr = reinterpret_cast<float *>(streams[i].data) + offset;
            sum += *srcPtr;
        }
        offset++;
        *dstPtr++ = sum;
    }
    *outSize = streams[0].dataSize;
}

} // namespace AudioSuite
} // namespace AudioStandard
} // namespace OHOS