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
#define LOG_TAG "AudioPcmProcess"
#endif

#include "audio_pcm_process_impl.h"
#include "audio_errors.h"
#include "audio_suite_log.h"
#include "audio_suite_common.h"
#include <cstring>
#include <securec.h>

using namespace OHOS::AudioStandard;
using namespace OHOS::AudioStandard::AudioSuite;

namespace {
constexpr int32_t ERR_CODE_SUCCESS = 0;
constexpr int32_t ERR_CODE_INVALID_PARAM = -1;
constexpr int32_t ERR_CODE_INVALID_STATE = -2;
constexpr int32_t ERR_CODE_OPERATION_FAILED = -3;
}

AudioSampleFormat ConvertBitDepthToSampleFormat(AudioBitDepth bitDepth)
{
    switch (bitDepth) {
        case AudioBit16Int:
            return AudioSampleFormat::SAMPLE_S16LE;
        case AudioBit24Int:
            return AudioSampleFormat::SAMPLE_S24LE;
        case AudioBit32Int:
            return AudioSampleFormat::SAMPLE_S32LE;
        case AudioBit32Float:
            return AudioSampleFormat::SAMPLE_F32LE;
        default:
            return AudioSampleFormat::INVALID_WIDTH;
    }
}

AudioChannelLayout ConvertChannelToLayout(CAudioChannel channel)
{
    switch (channel) {
        case AudioChMono:
            return AudioChannelLayout::CH_LAYOUT_MONO;
        case AudioChStereo:
            return AudioChannelLayout::CH_LAYOUT_STEREO;
        default:
            return AudioChannelLayout::CH_LAYOUT_UNKNOWN;
    }
}

bool ValidateSampleRate(int sampleRate, AudioSamplingRate& outRate)
{
    for (const auto& rate : AUDIO_SUPPORTED_SAMPLING_RATES) {
        if (static_cast<int>(rate) == sampleRate) {
            outRate = rate;
            return true;
        }
    }
    return false;
}

uint32_t GetSampleSizeFromBitDepth(AudioBitDepth bitDepth)
{
    switch (bitDepth) {
        case AudioBit16Int:
            return 2;
        case AudioBit24Int:
            return 3;
        case AudioBit32Int:
            return 4;
        case AudioBit32Float:
            return 4;
        default:
            return 0;
    }
}

PcmBufferFormat ConvertToPcmBufferFormat(const AudioFormatConfig* cfg)
{
    AudioSamplingRate rate;
    ValidateSampleRate(cfg->sampleRate, rate);

    uint32_t channelCount = static_cast<uint32_t>(cfg->channel);
    AudioChannelLayout layout = ConvertChannelToLayout(cfg->channel);
    AudioSampleFormat format = ConvertBitDepthToSampleFormat(cfg->bitDepth);

    return PcmBufferFormat(rate, channelCount, layout, format);
}

AudioFormat ConvertToAudioFormat(const AudioFormatConfig* cfg)
{
    AudioFormat format;
    AudioSamplingRate rate;
    ValidateSampleRate(cfg->sampleRate, rate);

    format.rate = rate;
    format.format = ConvertBitDepthToSampleFormat(cfg->bitDepth);
    format.audioChannelInfo.channelLayout = ConvertChannelToLayout(cfg->channel);
    format.audioChannelInfo.numChannels = static_cast<uint32_t>(cfg->channel);
    format.outRate = rate;
    format.outFormat = format.format;
    format.outAudioChannelInfo = format.audioChannelInfo;

    return format;
}

AudioConverterHandle AudioConverterCreate(const AudioFormatConfig* inCfg, const AudioFormatConfig* outCfg)
{
    if (inCfg == nullptr || outCfg == nullptr) {
        AUDIO_ERR_LOG("invalid params, inCfg or outCfg is nullptr");
        return nullptr;
    }

    AudioSamplingRate inputRate;
    AudioSamplingRate outputRate;
    if (!ValidateSampleRate(inCfg->sampleRate, inputRate)) {
        AUDIO_ERR_LOG("unsupported input sample rate: %{public}d", inCfg->sampleRate);
        return nullptr;
    }
    if (!ValidateSampleRate(outCfg->sampleRate, outputRate)) {
        AUDIO_ERR_LOG("unsupported output sample rate: %{public}d", outCfg->sampleRate);
        return nullptr;
    }

    AudioChannelLayout inputLayout = ConvertChannelToLayout(inCfg->channel);
    AudioChannelLayout outputLayout = ConvertChannelToLayout(outCfg->channel);
    if (inputLayout == AudioChannelLayout::CH_LAYOUT_UNKNOWN ||
        outputLayout == AudioChannelLayout::CH_LAYOUT_UNKNOWN) {
        AUDIO_ERR_LOG("unsupported channel layout");
        return nullptr;
    }

    AudioSampleFormat inputSampleFormat = ConvertBitDepthToSampleFormat(inCfg->bitDepth);
    AudioSampleFormat outputSampleFormat = ConvertBitDepthToSampleFormat(outCfg->bitDepth);
    if (inputSampleFormat == AudioSampleFormat::INVALID_WIDTH ||
        outputSampleFormat == AudioSampleFormat::INVALID_WIDTH) {
        AUDIO_ERR_LOG("unsupported bit depth");
        return nullptr;
    }

    uint32_t inputChannelCount = static_cast<uint32_t>(inCfg->channel);
    uint32_t outputChannelCount = static_cast<uint32_t>(outCfg->channel);

    PcmBufferFormat inputFormat(inputRate, inputChannelCount, inputLayout, inputSampleFormat);
    PcmBufferFormat outputFormat(outputRate, outputChannelCount, outputLayout, outputSampleFormat);

    std::shared_ptr<AudioFormatConverter> converter = AudioFormatConverter::Create(inputFormat, outputFormat);
    if (converter == nullptr) {
        AUDIO_ERR_LOG("failed to create AudioFormatConverter");
        return nullptr;
    }

    ConverterHandleWrapper* wrapper = new (std::nothrow) ConverterHandleWrapper();
    if (wrapper == nullptr) {
        AUDIO_ERR_LOG("failed to allocate wrapper");
        return nullptr;
    }

    wrapper->converter = converter;
    wrapper->inputFormat = inputFormat;
    wrapper->outputFormat = outputFormat;

    AUDIO_INFO_LOG("success");
    return static_cast<AudioConverterHandle>(wrapper);
}

CAudioConvertResult AudioConverterProcess(AudioConverterHandle handle, const uint8_t* inData, int inSampleCount)
{
    CAudioConvertResult result = {nullptr, 0, ERR_CODE_INVALID_PARAM};

    if (handle == nullptr) {
        AUDIO_ERR_LOG("handle is nullptr");
        result.errCode = ERR_CODE_INVALID_PARAM;
        return result;
    }

    if (inData == nullptr || inSampleCount <= 0) {
        AUDIO_ERR_LOG("invalid input data");
        result.errCode = ERR_CODE_INVALID_PARAM;
        return result;
    }

    ConverterHandleWrapper* wrapper = static_cast<ConverterHandleWrapper*>(handle);
    if (wrapper->converter == nullptr) {
        AUDIO_ERR_LOG("converter is nullptr");
        result.errCode = ERR_CODE_INVALID_STATE;
        return result;
    }

    AudioSuite::AudioConvertResult innerResult =
        wrapper->converter->Process(inData, static_cast<uint32_t>(inSampleCount));

    result.outData = innerResult.outData;
    result.outSampleCount = static_cast<int>(innerResult.outSampleCount);
    result.errCode = innerResult.errCode == SUCCESS ? ERR_CODE_SUCCESS : ERR_CODE_OPERATION_FAILED;

    AUDIO_DEBUG_LOG("outSampleCount=%{public}d, errCode=%{public}d", result.outSampleCount, result.errCode);
    return result;
}

void AudioConverterDestroy(AudioConverterHandle handle)
{
    if (handle == nullptr) {
        AUDIO_ERR_LOG("handle is nullptr");
        return;
    }

    ConverterHandleWrapper* wrapper = static_cast<ConverterHandleWrapper*>(handle);
    if (wrapper->converter != nullptr) {
        wrapper->converter->Destroy();
        wrapper->converter.reset();
    }

    delete wrapper;
    AUDIO_INFO_LOG("success");
}

AudioMixHandle AudioMixerCreate(const AudioFormatConfig* inCfg)
{
    if (inCfg == nullptr) {
        AUDIO_ERR_LOG("inCfg is nullptr");
        return nullptr;
    }

    AudioSamplingRate rate;
    if (!ValidateSampleRate(inCfg->sampleRate, rate)) {
        AUDIO_ERR_LOG("unsupported sample rate: %{public}d", inCfg->sampleRate);
        return nullptr;
    }

    AudioChannelLayout layout = ConvertChannelToLayout(inCfg->channel);
    if (layout == AudioChannelLayout::CH_LAYOUT_UNKNOWN) {
        AUDIO_ERR_LOG("unsupported channel layout");
        return nullptr;
    }

    AudioSampleFormat sampleFormat = ConvertBitDepthToSampleFormat(inCfg->bitDepth);
    if (sampleFormat == AudioSampleFormat::INVALID_WIDTH) {
        AUDIO_ERR_LOG("unsupported bit depth");
        return nullptr;
    }

    AudioFormat format = ConvertToAudioFormat(inCfg);
    std::unique_ptr<MixerProcessor> mixer = MixerProcessorManager::Create(format);
    if (mixer == nullptr) {
        AUDIO_ERR_LOG("failed to create MixerProcessor");
        return nullptr;
    }

    MixerHandleWrapper* wrapper = new (std::nothrow) MixerHandleWrapper();
    if (wrapper == nullptr) {
        AUDIO_ERR_LOG("failed to allocate wrapper");
        return nullptr;
    }

    wrapper->mixer = std::move(mixer);
    wrapper->format = format;
    wrapper->bitDepth = GetSampleSizeFromBitDepth(inCfg->bitDepth);
    wrapper->bufferAllocated = false;
    wrapper->expectedSampleCount = 0;

    AUDIO_INFO_LOG("success, channels=%{public}u, bitDepth=%{public}u",
        format.audioChannelInfo.numChannels, wrapper->bitDepth);
    return static_cast<AudioMixHandle>(wrapper);
}

AudioMixResult AudioMixerProcess(AudioMixHandle handle, const CAudioMixStream* streams, int streamCount)
{
    AudioMixResult result = {nullptr, 0, ERR_CODE_INVALID_PARAM};

    if (handle == nullptr) {
        AUDIO_ERR_LOG("handle is nullptr");
        return result;
    }

    if (streams == nullptr || streamCount <= 0) {
        AUDIO_ERR_LOG("invalid streams params");
        return result;
    }

    MixerHandleWrapper* wrapper = static_cast<MixerHandleWrapper*>(handle);
    if (wrapper->mixer == nullptr) {
        AUDIO_ERR_LOG("mixer is nullptr");
        result.errCode = ERR_CODE_INVALID_STATE;
        return result;
    }

    uint32_t firstSampleCount = static_cast<uint32_t>(streams[0].sampleCount);
    for (int i = 0; i < streamCount; ++i) {
        if (streams[i].data == nullptr) {
            AUDIO_ERR_LOG("stream[%{public}d] data is nullptr", i);
            return result;
        }
        if (static_cast<uint32_t>(streams[i].sampleCount) != firstSampleCount) {
            AUDIO_ERR_LOG("stream[%{public}d] sampleCount mismatch", i);
            return result;
        }
        if (streams[i].volume < 0.0f || streams[i].volume > 1.0f) {
            AUDIO_ERR_LOG("stream[%{public}d] volume out of range", i);
            return result;
        }
    }

    if (wrapper->bufferAllocated) {
        if (firstSampleCount != wrapper->expectedSampleCount) {
            AUDIO_ERR_LOG("sampleCount changed from %{public}u to %{public}u",
                wrapper->expectedSampleCount, firstSampleCount);
            result.errCode = ERR_CODE_INVALID_PARAM;
            return result;
        }
    } else {
        uint32_t tempChannelCount = wrapper->format.audioChannelInfo.numChannels;
        uint32_t dataSize = firstSampleCount * wrapper->bitDepth * tempChannelCount;
        wrapper->outputBuffer.resize(dataSize);
        wrapper->bufferAllocated = true;
        wrapper->expectedSampleCount = firstSampleCount;
        AUDIO_DEBUG_LOG("allocated outputBuffer size=%{public}u", dataSize);
    }

    std::vector<AudioSuite::AudioMixStream> innerStreams;
    innerStreams.reserve(streamCount);
    uint32_t channelCount = wrapper->format.audioChannelInfo.numChannels;
    uint32_t dataSize = firstSampleCount * wrapper->bitDepth * channelCount;

    for (int i = 0; i < streamCount; ++i) {
        AudioSuite::AudioMixStream innerStream;
        innerStream.data = const_cast<void*>(static_cast<const void*>(streams[i].data));
        innerStream.dataSize = dataSize;
        innerStream.volume = streams[i].volume;
        innerStreams.push_back(innerStream);
    }

    uint32_t outSize = 0;
    uint32_t outCapacity = static_cast<uint32_t>(wrapper->outputBuffer.size());
    int32_t ret = wrapper->mixer->Process(innerStreams, wrapper->outputBuffer.data(), outCapacity, &outSize);

    if (ret != SUCCESS) {
        AUDIO_ERR_LOG("mixer Process failed, ret=%{public}d", ret);
        result.errCode = ERR_CODE_OPERATION_FAILED;
        return result;
    }

    result.outData = wrapper->outputBuffer.data();
    result.outSamplerCount = static_cast<int>(firstSampleCount);
    result.errCode = ERR_CODE_SUCCESS;

    AUDIO_DEBUG_LOG("success, outSamplerCount=%{public}d", result.outSamplerCount);
    return result;
}

void AudioMixerDestroy(AudioMixHandle hanlde)
{
    if (hanlde == nullptr) {
        AUDIO_ERR_LOG("handle is nullptr");
        return;
    }

    MixerHandleWrapper* wrapper = static_cast<MixerHandleWrapper*>(hanlde);
    if (wrapper->mixer != nullptr) {
        wrapper->mixer->Destroy();
        wrapper->mixer.reset();
    }

    wrapper->outputBuffer.clear();
    wrapper->bufferAllocated = false;

    delete wrapper;
    AUDIO_INFO_LOG("success");
}