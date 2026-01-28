/*
 * Copyright (c) 2025 Huawei Device Co., Ltd. 2025-2025. ALL rights reserved.
 */

#include "Utils.h"
#include "audioEffectNode/Input.h"
#include "audioEffectNode/Output.h"
#include "audioSuiteError/AudioSuiteError.h"
#include <cstdint>
#include <hilog/log.h>
#include "Constant.h"

const char* STR_0 = "0";
const char* STR_1 = "1";
const char* STR_2 = "2";
const char* STR_3 = "3";
const char* STR_4 = "4";
const char* STR_5 = "5";
const char* STR_6 = "6";

const int CONSTANT_0 = 0;

const int CONSTANT_1 = 1;
const int GLOBAL_RESMGR = 0xFF00;
const char *UTILS_TAG = "[AudioEditTestApp_utils_cpp]";
const int BITSPERSAMPLEMODE_INT = 0;
const int BITSPERSAMPLEMODE_FLOAT = 1;

const int BIT_8 = 8;
const int BIT_16 = 16;
const int BIT_32 = 32;
const int OFFSET_BIT_24 = 3;
const int BIT_DEPTH_TWO = 2;

// Parsing napi string parameters
napi_status ParseNapiString(napi_env env, napi_value value, std::string &result)
{
    size_t size;
    napi_status status = napi_get_value_string_utf8(env, value, nullptr, CONSTANT_0, &size);
    if (status != napi_ok) {
        return status;
    }

    result.resize(size + CONSTANT_1); // Contains ending null characters
    status = napi_get_value_string_utf8(env, value, const_cast<char *>(result.data()), size + CONSTANT_1, nullptr);

    return status;
}

void GetBitsPerSampleAndStreamFormat(const OH_AudioFormat& g_audioFormatOutput,
    int32_t* bitsPerSample, OH_AudioStream_SampleFormat* streamSampleFormat)
{
    if (g_audioFormatOutput.sampleFormat == OH_Audio_SampleFormat::AUDIO_SAMPLE_U8) {
        *bitsPerSample = DemoBitsPerSample::DEMO_BITSPERSAMPLE_8;
        *streamSampleFormat = OH_AudioStream_SampleFormat::AUDIOSTREAM_SAMPLE_U8;
    } else if (g_audioFormatOutput.sampleFormat == OH_Audio_SampleFormat::AUDIO_SAMPLE_S16LE) {
        *bitsPerSample = DemoBitsPerSample::DEMO_BITSPERSAMPLE_16;
        *streamSampleFormat = OH_AudioStream_SampleFormat::AUDIOSTREAM_SAMPLE_S16LE;
    } else if (g_audioFormatOutput.sampleFormat == OH_Audio_SampleFormat::AUDIO_SAMPLE_S24LE) {
        *bitsPerSample = DemoBitsPerSample::DEMO_BITSPERSAMPLE_24;
        *streamSampleFormat = OH_AudioStream_SampleFormat::AUDIOSTREAM_SAMPLE_S24LE;
    }  else if (g_audioFormatOutput.sampleFormat == OH_Audio_SampleFormat::AUDIO_SAMPLE_S32LE) {
        *bitsPerSample = DEMO_BITSPERSAMPLE_32;
        *streamSampleFormat = OH_AudioStream_SampleFormat::AUDIOSTREAM_SAMPLE_S32LE;
    } else if (g_audioFormatOutput.sampleFormat == OH_Audio_SampleFormat::AUDIO_SAMPLE_F32LE) {
        *bitsPerSample = DEMO_BITSPERSAMPLE_32;
        *streamSampleFormat = OH_AudioStream_SampleFormat::AUDIOSTREAM_SAMPLE_F32LE;
    }
}

// Set Sampling Rate
OH_Audio_SampleRate SetSamplingRate(int32_t sampleRate)
{
    switch (sampleRate) {
        case static_cast<int32_t>(OH_Audio_SampleRate::SAMPLE_RATE_8000):
            return OH_Audio_SampleRate::SAMPLE_RATE_8000;
        case static_cast<int32_t>(OH_Audio_SampleRate::SAMPLE_RATE_11025):
            return OH_Audio_SampleRate::SAMPLE_RATE_11025;
        case static_cast<int32_t>(OH_Audio_SampleRate::SAMPLE_RATE_12000):
            return OH_Audio_SampleRate::SAMPLE_RATE_12000;
        case static_cast<int32_t>(OH_Audio_SampleRate::SAMPLE_RATE_16000):
            return OH_Audio_SampleRate::SAMPLE_RATE_16000;
        case static_cast<int32_t>(OH_Audio_SampleRate::SAMPLE_RATE_22050):
            return OH_Audio_SampleRate::SAMPLE_RATE_22050;
        case static_cast<int32_t>(OH_Audio_SampleRate::SAMPLE_RATE_24000):
            return OH_Audio_SampleRate::SAMPLE_RATE_24000;
        case static_cast<int32_t>(OH_Audio_SampleRate::SAMPLE_RATE_32000):
            return OH_Audio_SampleRate::SAMPLE_RATE_32000;
        case static_cast<int32_t>(OH_Audio_SampleRate::SAMPLE_RATE_44100):
            return OH_Audio_SampleRate::SAMPLE_RATE_44100;
        case static_cast<int32_t>(OH_Audio_SampleRate::SAMPLE_RATE_48000):
            return OH_Audio_SampleRate::SAMPLE_RATE_48000;
        case static_cast<int32_t>(OH_Audio_SampleRate::SAMPLE_RATE_64000):
            return OH_Audio_SampleRate::SAMPLE_RATE_64000;
        case static_cast<int32_t>(OH_Audio_SampleRate::SAMPLE_RATE_88200):
            return OH_Audio_SampleRate::SAMPLE_RATE_88200;
        case static_cast<int32_t>(OH_Audio_SampleRate::SAMPLE_RATE_96000):
            return OH_Audio_SampleRate::SAMPLE_RATE_96000;
        case static_cast<int32_t>(OH_Audio_SampleRate::SAMPLE_RATE_176400):
            return OH_Audio_SampleRate::SAMPLE_RATE_176400;
        case static_cast<int32_t>(OH_Audio_SampleRate::SAMPLE_RATE_192000):
            return OH_Audio_SampleRate::SAMPLE_RATE_192000;
        default:
            return OH_Audio_SampleRate::SAMPLE_RATE_48000;
    }
}

// Set audio channels
OH_AudioChannelLayout SetChannelLayout(int32_t channels)
{
    OH_AudioChannelLayout audioChannelLayout;
    switch (channels) {
        case DemoChannels::DEMO_CHANNELS_1:
            audioChannelLayout = CH_LAYOUT_MONO;
            break;
        case DemoChannels::DEMO_CHANNELS_2:
            audioChannelLayout = CH_LAYOUT_STEREO;
            break;
        default:
            audioChannelLayout = CH_LAYOUT_STEREO_DOWNMIX;
            break;
    }
    return audioChannelLayout;
}

// Set bit depth
OH_Audio_SampleFormat SetSampleFormat(int32_t bitsPerSample)
{
    OH_Audio_SampleFormat audioSampleFormat;
    switch (bitsPerSample) {
        case DemoBitsPerSample::DEMO_SAMPLE_U8:
            audioSampleFormat = OH_Audio_SampleFormat::AUDIO_SAMPLE_U8;
            break;
        case DemoBitsPerSample::DEMO_SAMPLE_S16LE:
            audioSampleFormat = OH_Audio_SampleFormat::AUDIO_SAMPLE_S16LE;
            break;
        case DemoBitsPerSample::DEMO_SAMPLE_S246E:
            audioSampleFormat = OH_Audio_SampleFormat::AUDIO_SAMPLE_S24LE;
            break;
        case DemoBitsPerSample::DEMO_SAMPLE_S32LE:
            audioSampleFormat = OH_Audio_SampleFormat::AUDIO_SAMPLE_S32LE;
            break;
        case DemoBitsPerSample::DEMO_SAMPLE_F32LE:
            audioSampleFormat = OH_Audio_SampleFormat::AUDIO_SAMPLE_F32LE;
            break;
        default:
            break;
    }
    return audioSampleFormat;
}

OH_AudioStream_SampleFormat ConvertInt2AudioStream(const int32_t sampleFormat)
{
    OH_AudioStream_SampleFormat audioSampleFormat;
    switch (sampleFormat) {
        case DemoBitsPerSample::DEMO_SAMPLE_U8:
            audioSampleFormat = OH_AudioStream_SampleFormat::AUDIOSTREAM_SAMPLE_U8;
            break;
        case DemoBitsPerSample::DEMO_SAMPLE_S16LE:
            audioSampleFormat = OH_AudioStream_SampleFormat::AUDIOSTREAM_SAMPLE_S16LE;
            break;
        case DemoBitsPerSample::DEMO_SAMPLE_S246E:
            audioSampleFormat = OH_AudioStream_SampleFormat::AUDIOSTREAM_SAMPLE_S24LE;
            break;
        case DemoBitsPerSample::DEMO_SAMPLE_S32LE:
            audioSampleFormat = OH_AudioStream_SampleFormat::AUDIOSTREAM_SAMPLE_S32LE;
            break;
        case DemoBitsPerSample::DEMO_SAMPLE_F32LE:
            audioSampleFormat = OH_AudioStream_SampleFormat::AUDIOSTREAM_SAMPLE_F32LE;
            break;
        default:
            break;
    }
    return audioSampleFormat;
}

// Bit depth conversion
void ConvertBitsPerSample(unsigned int& bitsPerSample, const unsigned int& bitsPerSampleMode)
{
    switch (bitsPerSample) {
        case DemoBitsPerSample::DEMO_BITSPERSAMPLE_8:
            bitsPerSample = DemoBitsPerSample::DEMO_SAMPLE_U8;
            break;
        case DemoBitsPerSample::DEMO_BITSPERSAMPLE_16:
            bitsPerSample = DemoBitsPerSample::DEMO_SAMPLE_S16LE;
            break;
        case DemoBitsPerSample::DEMO_BITSPERSAMPLE_24:
            bitsPerSample = DemoBitsPerSample::DEMO_SAMPLE_S246E;
            break;
        case DemoBitsPerSample::DEMO_BITSPERSAMPLE_32:
            if (bitsPerSampleMode == BITSPERSAMPLEMODE_INT) {
                bitsPerSample = DemoBitsPerSample::DEMO_SAMPLE_S32LE;
            } else {
                bitsPerSample = DemoBitsPerSample::DEMO_SAMPLE_F32LE;
            }
            break;
        default:
            break;
    }
}

int32_t GetBitsPerSample(OH_Audio_SampleFormat sampleFormat)
{
    switch (sampleFormat) {
        case OH_Audio_SampleFormat::AUDIO_SAMPLE_U8:
            return DemoBitsPerSample::DEMO_BITSPERSAMPLE_8;
        case OH_Audio_SampleFormat::AUDIO_SAMPLE_S16LE:
            return DemoBitsPerSample::DEMO_BITSPERSAMPLE_16;
        case OH_Audio_SampleFormat::AUDIO_SAMPLE_S24LE:
            return DemoBitsPerSample::DEMO_BITSPERSAMPLE_24;
        default:
            return DemoBitsPerSample::DEMO_BITSPERSAMPLE_32;
    }
}

OH_EnvironmentType GetEnvEnumByNumber(int num)
{
    OH_EnvironmentType type;
    switch (num) {
        case ARG_1:
            type = ENVIRONMENT_TYPE_BROADCAST;
            break;
        case ARG_2:
            type = ENVIRONMENT_TYPE_EARPIECE;
            break;
        case ARG_3:
            type = ENVIRONMENT_TYPE_UNDERWATER;
            break;
        case ARG_4:
            type = ENVIRONMENT_TYPE_GRAMOPHONE;
            break;
        default:
            break;
    }
    return type;
}

napi_value ReturnResult(napi_env env, AudioSuiteResult result)
{
    std::string resultMessage = GetErrorMessage(result);
    napi_value sum;
    if (result != AudioSuiteResult::AUDIOSUITE_SUCCESS) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, UTILS_TAG,
            "result: %{public}d, resultMessage: %{public}s", result, resultMessage.c_str());
    } else {
        OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, UTILS_TAG,
            "result: %{public}d, resultMessage: %{public}s", result, resultMessage.c_str());
    }
    napi_create_int64(env, static_cast<int>(result), &sum);
    return sum;
}

void FreeBufferOfVoid(void **buffer)
{
    if (buffer != nullptr && *buffer != nullptr) {
        free(*buffer);
        *buffer = nullptr;
    }
}

void FreeBuffer(char **buffer)
{
    if (buffer != nullptr && *buffer != nullptr) {
        free(*buffer);
        *buffer = nullptr;
    }
}

void ConvertToFloat(int format, unsigned inputSampleCount, void *src, float *dst)
{
    if (!src || !dst) {
        return;
    }
    switch (format) {
        case UINT_0:
            ConvertFromU8ToFloat(inputSampleCount, (const uint8_t *)src, dst);
            break;
        case UINT_1:
            ConvertFrom16BitToFloat(inputSampleCount, (const int16_t *)src, dst);
            break;
        case UINT_2:
            ConvertFrom24BitToFloat(inputSampleCount, (const uint8_t *)src, dst);
            break;
        case UINT_3:
            ConvertFrom32BitToFloat(inputSampleCount, (const int32_t *)src, dst);
            break;
        default:
            std::copy(static_cast<const float *>(src), static_cast<const float *>(src) + inputSampleCount, dst);
            break;
    }
}

void ConvertFromU8ToFloat(unsigned n, const uint8_t *a, float *b)
{
    for (; n > 0; n--, a++, b++) {
//        *b = (float)(*a - (uint8_t)0x80U) * (1.0 / 0x80U);
         *b = static_cast<float>(*a - 0x80U) * (1.0f / 128.0f);
    }
}

void ConvertFrom16BitToFloat(unsigned n, const int16_t *a, float *b)
{
    for (; n > 0; n--) {
        *(b++) = *(a++) * (1.0f / (1 << (BIT_16 - 1)));
    }
}

void ConvertFrom24BitToFloat(unsigned n, const uint8_t *a, float *b)
{
    for (; n > 0; n--) {
        int32_t s = Read24Bit(a) << BIT_8;
        *b = s * (1.0f / (1U << (BIT_32 - 1)));
        a += OFFSET_BIT_24;
        b++;
    }
}

void ConvertFrom32BitToFloat(unsigned n, const int32_t *a, float *b)
{
    for (; n > 0; n--) {
        *(b++) = *(a++) * (1.0f / (1U << (BIT_32 - 1)));
    }
}

uint32_t Read24Bit(const uint8_t *p)
{
    return (static_cast<uint32_t>(p[BIT_DEPTH_TWO]) << BIT_16) | (static_cast<uint32_t>(p[1]) << BIT_8) |
           (static_cast<uint32_t>(p[0]));
}

void SetAudioFormat(const int sampleRate, const int channels, const int bitsPerSample)
{
    // Set the sampling rate.
    g_audioFormatInput.samplingRate = SetSamplingRate(sampleRate);
    // Set the audio channel
    g_audioFormatInput.channelCount = channels;
    g_audioFormatInput.channelLayout = SetChannelLayout(channels);
    // Set bit depth
    g_audioFormatInput.sampleFormat = SetSampleFormat(bitsPerSample);
    // Set the encoding format
    g_audioFormatInput.encodingType = OH_Audio_EncodingType::AUDIO_ENCODING_TYPE_RAW;
    
    g_audioFormatOutput.samplingRate = g_audioFormatInput.samplingRate;
    g_audioFormatOutput.channelCount = g_audioFormatInput.channelCount;
    g_audioFormatOutput.channelLayout = g_audioFormatInput.channelLayout;
    g_audioFormatOutput.sampleFormat = g_audioFormatInput.sampleFormat;
    g_audioFormatOutput.encodingType = g_audioFormatInput.encodingType;
}

// Obtain the audio duration(ms)
long GetAudioDuration(long pcmDataLength, int sampleRate, int channels, int bitsPerSample)
{
    int bits = UINT_1;
    switch (bitsPerSample) {
        case UINT_0:
            bits = UINT_1;
            break;
        case UINT_1:
            bits = UINT_2;
            break;
        case UINT_2:
            bits = UINT_3;
            break;
        case UINT_3:
            bits = UINT_4;
            break;
        case UINT_4:
            bits = UINT_4;
            break;
        default:
            bits = UINT_1;
    }
    if (channels != 0 && bits != 0 && sampleRate != 0) {
        return pcmDataLength * UINT_1000 / channels / bits / sampleRate;
    }
    return UINT_0;
}

// Obtain the size of an audio file of one second.
long GetAudioSize(int sampleRate, int channels, int bitsPerSample)
{
    int bits = UINT_1;
    switch (bitsPerSample) {
        case UINT_0:
            bits = UINT_1;
            break;
        case UINT_1:
            bits = UINT_2;
            break;
        case UINT_2:
            bits = UINT_3;
            break;
        case UINT_3:
            bits = UINT_4;
            break;
        case UINT_4:
            bits = UINT_4;
            break;
        default:
            bits = UINT_1;
    }
    return sampleRate * channels * bits;
}

int GetBit(int bitsPerSample)
{
    int bits = UINT_1;
    switch (bitsPerSample) {
        case UINT_0:
            bits = UINT_1;
            break;
        case UINT_1:
            bits = UINT_2;
            break;
        case UINT_2:
            bits = UINT_3;
            break;
        case UINT_3:
            bits = UINT_4;
            break;
        case UINT_4:
            bits = UINT_4;
            break;
        default:
            bits = UINT_1;
    }
    return bits;
}

bool AddWriteDataBuffer(const std::string inputId, const long oldStartTime, const long newStartTime,
                        std::vector<long> indexs, bool isCopyMultiple)
{
    long startIndex = indexs[ARG_0];
    long endIndex = indexs[ARG_1];
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, UTILS_TAG,
                 "addWriteDataBuffer start addWriteDataBuffer.size: %{public}d", g_writeDataBufferMap.size());
    std::string oldKey = inputId;
    if (oldStartTime != 0) {
        oldKey = inputId.c_str() + std::to_string(oldStartTime);
    }
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, UTILS_TAG, "addWriteDataBuffer key: %{public}s", oldKey.c_str());
    auto it = g_writeDataBufferMap.find(oldKey);
    if (it == g_writeDataBufferMap.end()) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, UTILS_TAG,
                     "addWriteDataBuffer g_writeDataBufferMap failed, oldKey is not exist");
        return false;
    }
    if (startIndex < 0 || startIndex > endIndex) {
        OH_LOG_Print(
            LOG_APP, LOG_ERROR, GLOBAL_RESMGR, UTILS_TAG,
            "addWriteDataBuffer invalid index range: start=%{public}ld, end=%{public}ld, oldPcmdataSize: %{public}d",
            startIndex, endIndex, it->second.size());
        return false;
    }
    std::vector<uint8_t> newPcmData;
    // Extract data within a specified range
    if (endIndex >= it->second.size()) {
        if (isCopyMultiple) {
            size_t totalLength = endIndex - startIndex;
            size_t available = it->second.size() - startIndex;
            newPcmData.reserve(totalLength);
            // Copy the first segment of valid data
            if (available > 0) {
                newPcmData.insert(newPcmData.end(), it->second.begin() + startIndex, it->second.end());
            }
        } else {
            OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, UTILS_TAG, "addWriteDataBuffer invalid");
            newPcmData = std::vector<uint8_t>(it->second.begin() + startIndex, it->second.end());
        }
    } else {
        newPcmData = std::vector<uint8_t>(it->second.begin() + startIndex, it->second.begin() + endIndex);
    }
    std::string newKey = inputId;
    if (newStartTime != 0) {
        newKey = inputId.c_str() + std::to_string(newStartTime);
    }
    g_writeDataBufferMap[newKey] = newPcmData;
    OH_LOG_Print(
        LOG_APP, LOG_INFO, GLOBAL_RESMGR, UTILS_TAG,
        "addWriteDataBuffer end addWriteDataBuffer.size: %{public}d, newStartTime: %{public}d, newKey: %{public}s",
        g_writeDataBufferMap.size(), newStartTime, newKey.c_str());
    return true;
}

bool UpdateWriteDataBuffer(const std::string inputId, const long startTime, long startIndex, long endIndex)
{
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, UTILS_TAG,
                 "updateWriteDataBuffer start addWriteDataBuffer.size: %{public}d", g_writeDataBufferMap.size());
    std::string key = inputId;
    if (startTime != 0) {
        key = inputId.c_str() + std::to_string(startTime);
    }
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, UTILS_TAG, "updateWriteDataBuffer key: %{public}s", key.c_str());
    auto it = g_writeDataBufferMap.find(key);
    if (it == g_writeDataBufferMap.end()) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, UTILS_TAG,
                     "updateWriteDataBuffer g_writeDataBufferMap failed, oldKey is not exist");
        return false;
    }
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, UTILS_TAG,
                 "updateWriteDataBuffer old it size: %{public}ld, startIndex: %{public}ld, endIndex: %{public}ld",
                 it->second.size(), startIndex, endIndex);
    if (startIndex < 0 || startIndex > endIndex) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, UTILS_TAG,
                     "updateWriteDataBuffer invalid index range: start=%{public}ld, end=%{public}ld, oldPcmdataSize: "
                     "%{public}ld",
                     startIndex, endIndex, it->second.size());
        return false;
    }
    std::vector<uint8_t> newPcmData;
    // Extract data within a specified range
    if (endIndex >= it->second.size()) {
        newPcmData = std::vector<uint8_t>(it->second.begin() + startIndex, it->second.end());
    } else {
        newPcmData = std::vector<uint8_t>(it->second.begin() + startIndex, it->second.begin() + endIndex);
    }
    g_writeDataBufferMap[key] = newPcmData;
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, UTILS_TAG,
                 "updateWriteDataBuffer end addWriteDataBuffer.size: %{public}d, startTime: %{public}d",
                 g_writeDataBufferMap.size(), startTime);
    return true;
}

bool DeleteWriteDataBuffer(const std::string inputId, const long startTime)
{
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, UTILS_TAG,
                 "deleteWriteDataBuffer start g_writeDataBufferMap.size: %{public}d", g_writeDataBufferMap.size());
    std::string key = inputId;
    if (startTime != 0) {
        key = inputId.c_str() + std::to_string(startTime);
    }
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, UTILS_TAG, "deleteWriteDataBuffer key: %{public}s", key.c_str());
    auto it = g_writeDataBufferMap.find(key);
    if (it == g_writeDataBufferMap.end()) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, UTILS_TAG,
                     "deleteWriteDataBuffer g_writeDataBufferMap failed, key is not exist");
        return false;
    }
    g_writeDataBufferMap.erase(it);
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, UTILS_TAG,
                 "deleteWriteDataBuffer end g_writeDataBufferMap.size: %{public}d", g_writeDataBufferMap.size());
    return true;
}


bool SetWriteDataBuffer(const std::string inputId, const long originStartTime, const long newStartTime)
{
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, UTILS_TAG,
                 "setWriteDataBuffer start g_writeDataBufferMap.size: %{public}d", g_writeDataBufferMap.size());
    std::string oldKey = inputId;
    if (originStartTime != 0) {
        oldKey = inputId.c_str() + std::to_string(originStartTime);
    }
    std::string newKey = inputId;
    if (newStartTime != 0) {
        newKey = inputId.c_str() + std::to_string(newStartTime);
    }
    // If the old and new keys are the same, no action is required.
    if (oldKey == newKey) {
        OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, UTILS_TAG,
                     "setWriteDataBuffer: oldKey and newKey are identical, skipping update");
        return true;
    }
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, UTILS_TAG, "setWriteDataBuffer key: %{public}s", oldKey.c_str());
    auto it = g_writeDataBufferMap.find(oldKey);
    if (it == g_writeDataBufferMap.end()) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, UTILS_TAG,
                     "setWriteDataBuffer g_writeDataBufferMap failed, key is not exist");
        return false;
    }
    // Extract the value and delete the old key
    auto value = std::move(it->second); // Move semantics avoids copying
    g_writeDataBufferMap.erase(it);     // Delete old key-value pairs
    // Insert a new key-value pair (overwriting if newKey exists)
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, UTILS_TAG, "setWriteDataBuffer newKey: %{public}s", newKey.c_str());
    auto [newIt, inserted] = g_writeDataBufferMap.insert({newKey, std::move(value)});
    if (!inserted) {
        // If newKey already exists, overwrite the existing value.
        newIt->second = std::move(value);
        OH_LOG_Print(LOG_APP, LOG_WARN, GLOBAL_RESMGR, UTILS_TAG,
                     "setWriteDataBuffer: newKey already exists, overwriting value");
    }
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, UTILS_TAG,
                 "setWriteDataBuffer end g_writeDataBufferMap.size: %{public}d", g_writeDataBufferMap.size());
    return true;
}