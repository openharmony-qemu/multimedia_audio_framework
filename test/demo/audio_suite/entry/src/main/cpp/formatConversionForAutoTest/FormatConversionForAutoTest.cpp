/*
 * Copyright (c) 2026 Huawei Device Co., Ltd. 2025-2026. All rights reserved.
 */

#include "FormatConversionForAutoTest.h"
#include "hilog/log.h"
#include "ohaudiosuite/native_audio_converter.h"
#include <unistd.h>
#include <atomic>
#include <chrono>
#include <cstdio>
#include <cstring>
#include <memory>
#include <sstream>
#include <sys/stat.h>
#include <multimedia/player_framework/native_avdemuxer.h>
#include <multimedia/player_framework/native_avsource.h>
#include <multimedia/player_framework/native_avcodec_base.h>
#include <multimedia/player_framework/native_avformat.h>
#include <multimedia/player_framework/native_avbuffer.h>

#define TAG "AutoTestFormatConverter"

static const int GLOBAL_RESMGR = 0xFF00;

static const int32_t OUTPUT_BUFFER_SIZE = 1024 * 1024;
static const int32_t INPUT_BUFFER_SIZE = 256 * 1024;
static const uint32_t MAX_FILE_PATH_LENGTH = 512;
static const uint32_t ARG_COUNT = 2;

// OH_MD_KEY_AUDIO_SAMPLE_FORMAT 枚举值宏定义
#define SAMPLE_FORMAT_U8 0
#define SAMPLE_FORMAT_S16LE 1
#define SAMPLE_FORMAT_S24LE 2
#define SAMPLE_FORMAT_S32LE 3
#define SAMPLE_FORMAT_F32LE 4

static std::atomic<uint32_t> g_tempFileCounter{0};

struct ConversionContext {
    OH_AVSource* source = nullptr;
    OH_AVDemuxer* demuxer = nullptr;
    OH_AVBuffer* pcmBuffer = nullptr;
    uint32_t trackIndex = 0;
    OH_AudioConverter_Format inputFormat;
    OH_AudioConverter* converter = nullptr;
    uint8_t* accumulatedBuffer = nullptr;
    int32_t accumulatedCapacity = 0;
    uint8_t* leftoverBuffer = nullptr;
    int32_t leftoverSize = 0;
};

static void GenerateTempFilePath(char* path, size_t maxLen)
{
    auto now = std::chrono::system_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()).count();
    uint32_t counter = g_tempFileCounter.fetch_add(1);
    std::ostringstream oss;
    oss << "/data/storage/el2/base/haps/entry/files/converted_audio_" << ms << "_" << counter << ".pcm";
    std::string pathStr = oss.str();
    size_t copyLen = (pathStr.length() < maxLen - 1) ? pathStr.length() : maxLen - 1;
    std::copy(pathStr.begin(), pathStr.begin() + copyLen, path);
    path[copyLen] = '\0';
}

static bool ParseTrackFormat(OH_AVFormat* trackFormat, OH_AudioConverter_Format* inputFormat)
{
    int32_t sampleRate = 0;
    int32_t channels = 0;
    int32_t bitsPerSample = 0;
    if (!OH_AVFormat_GetIntValue(trackFormat, OH_MD_KEY_AUD_SAMPLE_RATE, &sampleRate)) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, TAG, "DetectInputFormat: get sample rate failed");
        return false;
    }
    if (!OH_AVFormat_GetIntValue(trackFormat, OH_MD_KEY_AUD_CHANNEL_COUNT, &channels)) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, TAG, "DetectInputFormat: get channel count failed");
        return false;
    }
    if (!OH_AVFormat_GetIntValue(trackFormat, OH_MD_KEY_AUDIO_SAMPLE_FORMAT, &bitsPerSample)) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, TAG, "DetectInputFormat: get bits per sample failed");
        return false;
    }

    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG,
                 "DetectInputFormat: sampleRate=%{public}d, channels=%{public}d, bitsPerSample=%{public}d",
                 sampleRate, channels, bitsPerSample);

    inputFormat->encodingType = OH_Audio_EncodingType::AUDIO_ENCODING_TYPE_RAW;
    inputFormat->samplingRate = static_cast<OH_Audio_SampleRate>(sampleRate);
    inputFormat->channelLayout = (channels == 1) ? OH_AudioChannelLayout::CH_LAYOUT_MONO
                                                  : OH_AudioChannelLayout::CH_LAYOUT_STEREO;
    switch (bitsPerSample) {
        case SAMPLE_FORMAT_U8:
            inputFormat->sampleFormat = OH_Audio_SampleFormat::AUDIO_SAMPLE_U8;
            break;
        case SAMPLE_FORMAT_S16LE:
            inputFormat->sampleFormat = OH_Audio_SampleFormat::AUDIO_SAMPLE_S16LE;
            break;
        case SAMPLE_FORMAT_S24LE:
            inputFormat->sampleFormat = OH_Audio_SampleFormat::AUDIO_SAMPLE_S24LE;
            break;
        case SAMPLE_FORMAT_S32LE:
            inputFormat->sampleFormat = OH_Audio_SampleFormat::AUDIO_SAMPLE_S32LE;
            break;
        case SAMPLE_FORMAT_F32LE:
            inputFormat->sampleFormat = OH_Audio_SampleFormat::AUDIO_SAMPLE_F32LE;
            break;
        default:
            OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, TAG,
                         "DetectInputFormat: unknown sample format enum value=%{public}d", bitsPerSample);
            return false;
    }
    return true;
}

static bool DetectInputFormat(int fd, int64_t fileLength,
                              OH_AudioConverter_Format* inputFormat,
                              OH_AVSource** outSource,
                              OH_AVDemuxer** outDemuxer,
                              uint32_t* outTrackIndex)
{
    *outSource = OH_AVSource_CreateWithFD(fd, 0, fileLength);
    if (*outSource == nullptr) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, TAG, "DetectInputFormat: OH_AVSource_CreateWithFD failed");
        return false;
    }
    OH_AVFormat* trackFormat = OH_AVSource_GetTrackFormat(*outSource, 0);
    if (trackFormat == nullptr) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, TAG, "DetectInputFormat: OH_AVSource_GetTrackFormat failed");
        return false;
    }
    bool parseOk = ParseTrackFormat(trackFormat, inputFormat);
    OH_AVFormat_Destroy(trackFormat);
    if (!parseOk) {
        return false;
    }

    *outDemuxer = OH_AVDemuxer_CreateWithSource(*outSource);
    if (*outDemuxer == nullptr) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, TAG, "DetectInputFormat: OH_AVDemuxer_CreateWithSource failed");
        return false;
    }
    *outTrackIndex = 0;
    if (OH_AVDemuxer_SelectTrackByID(*outDemuxer, *outTrackIndex) != AV_ERR_OK) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, TAG, "DetectInputFormat: OH_AVDemuxer_SelectTrackByID failed");
        return false;
    }
    return true;
}

static bool ParseOutputFormat(napi_env env, napi_value formatObj,
                              OH_AudioConverter_Format* format)
{
    napi_value encodingTypeValue;
    napi_value channelLayoutValue;
    napi_value samplingRateValue;
    napi_value sampleFormatValue;

    napi_get_named_property(env, formatObj, "encodingType", &encodingTypeValue);
    napi_get_named_property(env, formatObj, "samplingRate", &samplingRateValue);
    napi_get_named_property(env, formatObj, "channelLayout", &channelLayoutValue);
    napi_get_named_property(env, formatObj, "sampleFormat", &sampleFormatValue);

    uint32_t encodingType = 0;
    uint32_t samplingRate = 0;
    uint32_t sampleFormat = 0;
    int64_t channelLayout = 0;
    if (napi_get_value_uint32(env, encodingTypeValue, &encodingType) != napi_ok) {
        return false;
    }
    if (napi_get_value_uint32(env, samplingRateValue, &samplingRate) != napi_ok) {
        return false;
    }
    if (napi_get_value_int64(env, channelLayoutValue, &channelLayout) != napi_ok) {
        return false;
    }
    if (napi_get_value_uint32(env, sampleFormatValue, &sampleFormat) != napi_ok) {
        return false;
    }

    format->encodingType = static_cast<OH_Audio_EncodingType>(encodingType);
    format->samplingRate = static_cast<OH_Audio_SampleRate>(samplingRate);
    format->channelLayout = static_cast<OH_AudioChannelLayout>(channelLayout);
    format->sampleFormat = static_cast<OH_Audio_SampleFormat>(sampleFormat);
    return true;
}

static int32_t CopyLeftoverToAccumulated(ConversionContext* ctx)
{
    if (ctx->leftoverSize <= 0) {
        return 0;
    }
    int32_t copyFromLeftover = (ctx->leftoverSize <= ctx->accumulatedCapacity)
        ? ctx->leftoverSize : ctx->accumulatedCapacity;
    memcpy(ctx->accumulatedBuffer, ctx->leftoverBuffer, copyFromLeftover);
    if (ctx->leftoverSize > copyFromLeftover) {
        memmove(ctx->leftoverBuffer, ctx->leftoverBuffer + copyFromLeftover,
                ctx->leftoverSize - copyFromLeftover);
        ctx->leftoverSize -= copyFromLeftover;
    } else {
        ctx->leftoverSize = 0;
    }
    return copyFromLeftover;
}

static bool FillAccumulatedFromDemuxer(ConversionContext* ctx, int32_t* totalBytes)
{
    while (*totalBytes < ctx->accumulatedCapacity) {
        int32_t ret = OH_AVDemuxer_ReadSampleBuffer(ctx->demuxer, ctx->trackIndex, ctx->pcmBuffer);
        if (ret != AV_ERR_OK) {
            break;
        }
        OH_AVCodecBufferAttr info;
        OH_AVBuffer_GetBufferAttr(ctx->pcmBuffer, &info);
        if (info.flags == OH_AVCodecBufferFlags::AVCODEC_BUFFER_FLAGS_EOS) {
            break;
        }
        if (info.size <= 0) {
            continue;
        }
        int32_t remaining = ctx->accumulatedCapacity - *totalBytes;
        const uint8_t* srcAddr = static_cast<const uint8_t*>(OH_AVBuffer_GetAddr(ctx->pcmBuffer));
        if (info.size <= remaining) {
            memcpy(ctx->accumulatedBuffer + *totalBytes, srcAddr, info.size);
            *totalBytes += info.size;
        } else {
            memcpy(ctx->accumulatedBuffer + *totalBytes, srcAddr, remaining);
            *totalBytes += remaining;
            int32_t leftoverBytes = info.size - remaining;
            memcpy(ctx->leftoverBuffer, srcAddr + remaining, leftoverBytes);
            ctx->leftoverSize = leftoverBytes;
            break;
        }
    }
    return true;
}

static int32_t ReadDataCallback(void* userData, const void** outInputData,
                                OH_AudioConverter_InputStatus* outStatus)
{
    ConversionContext* ctx = static_cast<ConversionContext*>(userData);
    if (ctx == nullptr || ctx->demuxer == nullptr || ctx->pcmBuffer == nullptr ||
        ctx->accumulatedBuffer == nullptr || ctx->leftoverBuffer == nullptr) {
        *outInputData = nullptr;
        *outStatus = AUDIOCONVERTER_INPUT_DATA_FINISHED;
        return 0;
    }

    int32_t totalBytes = CopyLeftoverToAccumulated(ctx);
    FillAccumulatedFromDemuxer(ctx, &totalBytes);

    if (totalBytes <= 0) {
        *outInputData = nullptr;
        *outStatus = AUDIOCONVERTER_INPUT_DATA_FINISHED;
        return 0;
    }

    *outInputData = ctx->accumulatedBuffer;
    *outStatus = AUDIOCONVERTER_INPUT_HAVE_DATA;
    return totalBytes;
}

static bool ConvertValidateArgs(napi_env env, napi_callback_info info,
                                napi_value argv[ARG_COUNT],
                                OH_AudioConverter_Format* outputFormat,
                                int32_t* fd)
{
    size_t argc = ARG_COUNT;
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < ARG_COUNT) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, TAG, "Convert: invalid argument count");
        return false;
    }
    if (!ParseOutputFormat(env, argv[0], outputFormat)) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, TAG, "Convert: ParseOutputFormat failed");
        return false;
    }
    if (napi_get_value_int32(env, argv[1], fd) != napi_ok) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, TAG, "Convert: get fd failed");
        return false;
    }
    return true;
}

static bool ConvertInitContext(int32_t fd,
                               ConversionContext* ctx,
                               const OH_AudioConverter_Format* outputFormat,
                               char* tempFilePath)
{
    struct stat fileStat;
    if (fstat(fd, &fileStat) != 0) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, TAG, "Convert: fstat failed");
        return false;
    }
    if (!DetectInputFormat(fd, fileStat.st_size,
                           &ctx->inputFormat,
                           &ctx->source, &ctx->demuxer, &ctx->trackIndex)) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, TAG, "Convert: DetectInputFormat failed");
        return false;
    }
    ctx->pcmBuffer = OH_AVBuffer_Create(INPUT_BUFFER_SIZE);
    if (ctx->pcmBuffer == nullptr) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, TAG, "Convert: OH_AVBuffer_Create failed");
        return false;
    }
    ctx->accumulatedCapacity = INPUT_BUFFER_SIZE;
    ctx->accumulatedBuffer = new (std::nothrow) uint8_t[ctx->accumulatedCapacity];
    if (ctx->accumulatedBuffer == nullptr) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, TAG, "Convert: allocate accumulated buffer failed");
        return false;
    }
    ctx->leftoverBuffer = new (std::nothrow) uint8_t[INPUT_BUFFER_SIZE];
    if (ctx->leftoverBuffer == nullptr) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, TAG, "Convert: allocate leftover buffer failed");
        return false;
    }
    if (OH_AudioConverter_Create(&ctx->inputFormat, outputFormat, &ctx->converter) != AUDIOCONVERTER_SUCCESS) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, TAG, "Convert: OH_AudioConverter_Create failed");
        return false;
    }
    if (OH_AudioConverter_SetInputCallback(ctx->converter, ReadDataCallback, ctx) != AUDIOCONVERTER_SUCCESS) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, TAG, "Convert: OH_AudioConverter_SetInputCallback failed");
        return false;
    }
    GenerateTempFilePath(tempFilePath, MAX_FILE_PATH_LENGTH);
    return true;
}

static bool ConvertProcessToFile(ConversionContext* ctx, FILE* outputFile)
{
    std::unique_ptr<uint8_t[]> outputBufferHeap = std::make_unique<uint8_t[]>(OUTPUT_BUFFER_SIZE);
    if (outputBufferHeap == nullptr) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, TAG, "Convert: allocate output buffer failed");
        return false;
    }
    uint8_t* outputBuffer = outputBufferHeap.get();
    int32_t outputSize = 0;
    OH_AudioConverter_Result resultProcess = OH_AudioConverter_Process(
        ctx->converter, outputBuffer, OUTPUT_BUFFER_SIZE, &outputSize);
    while (resultProcess == AUDIOCONVERTER_SUCCESS && outputSize > 0) {
        size_t written = fwrite(outputBuffer, 1, outputSize, outputFile);
        if (written != static_cast<size_t>(outputSize)) {
            OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, TAG,
                         "Convert: fwrite incomplete, expected=%{public}d, written=%{public}zu",
                         outputSize, written);
            return false;
        }
        resultProcess = OH_AudioConverter_Process(
            ctx->converter, outputBuffer, OUTPUT_BUFFER_SIZE, &outputSize);
    }
    return true;
}

static void ConvertCleanup(ConversionContext* ctx, FILE* outputFile)
{
    if (ctx->converter != nullptr) {
        OH_AudioConverter_Destroy(ctx->converter);
    }
    if (outputFile != nullptr) {
        fclose(outputFile);
    }
    if (ctx->pcmBuffer != nullptr) {
        OH_AVBuffer_Destroy(ctx->pcmBuffer);
    }
    if (ctx->accumulatedBuffer != nullptr) {
        delete[] ctx->accumulatedBuffer;
    }
    if (ctx->leftoverBuffer != nullptr) {
        delete[] ctx->leftoverBuffer;
    }
    if (ctx->demuxer != nullptr) {
        OH_AVDemuxer_Destroy(ctx->demuxer);
    }
    if (ctx->source != nullptr) {
        OH_AVSource_Destroy(ctx->source);
    }
}

static const char* GetSampleFormatStr(OH_Audio_SampleFormat format)
{
    switch (format) {
        case OH_Audio_SampleFormat::AUDIO_SAMPLE_U8:
            return "U8";
        case OH_Audio_SampleFormat::AUDIO_SAMPLE_S16LE:
            return "S16LE";
        case OH_Audio_SampleFormat::AUDIO_SAMPLE_S24LE:
            return "S24LE";
        case OH_Audio_SampleFormat::AUDIO_SAMPLE_S32LE:
            return "S32LE";
        case OH_Audio_SampleFormat::AUDIO_SAMPLE_F32LE:
            return "F32LE";
        default:
            return "UNKNOWN";
    }
}

static uint32_t GetChannelCount(OH_AudioChannelLayout layout)
{
    if (layout == OH_AudioChannelLayout::CH_LAYOUT_MONO) {
        return 1;
    } else if (layout == OH_AudioChannelLayout::CH_LAYOUT_STEREO) {
        return 2;
    }
    return 0;
}

static napi_value BuildConvertResult(napi_env env,
                                     const char* tempFilePath,
                                     const ConversionContext* ctx)
{
    napi_value result;
    napi_create_object(env, &result);
    napi_value tempPathValue;
    napi_create_string_utf8(env, tempFilePath, strlen(tempFilePath), &tempPathValue);
    napi_set_named_property(env, result, "tempPath", tempPathValue);

    napi_value inputSampleRateValue;
    napi_create_int32(env, static_cast<int32_t>(ctx->inputFormat.samplingRate), &inputSampleRateValue);
    napi_set_named_property(env, result, "inputSampleRate", inputSampleRateValue);

    napi_value inputChannelsValue;
    napi_create_uint32(env, GetChannelCount(ctx->inputFormat.channelLayout), &inputChannelsValue);
    napi_set_named_property(env, result, "inputChannels", inputChannelsValue);

    const char* inputSampleFormatStr = GetSampleFormatStr(ctx->inputFormat.sampleFormat);
    napi_value inputSampleFormatValue;
    napi_create_string_utf8(env, inputSampleFormatStr, strlen(inputSampleFormatStr), &inputSampleFormatValue);
    napi_set_named_property(env, result, "inputSampleFormat", inputSampleFormatValue);
    return result;
}

static napi_value Convert(napi_env env, napi_callback_info info)
{
    char tempFilePath[MAX_FILE_PATH_LENGTH] = {0};
    ConversionContext ctx = {};
    FILE* outputFile = nullptr;
    OH_AudioConverter_Format outputFormat;
    int32_t fd = -1;
    napi_value result = nullptr;
    napi_get_null(env, &result);

    napi_value argv[ARG_COUNT];
    if (!ConvertValidateArgs(env, info, argv, &outputFormat, &fd)) {
        goto CLEANUP;
    }
    if (!ConvertInitContext(fd, &ctx, &outputFormat, tempFilePath)) {
        goto CLEANUP;
    }
    outputFile = fopen(tempFilePath, "wb");
    if (outputFile == nullptr) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, TAG, "Convert: fopen temp file failed");
        goto CLEANUP;
    }
    if (!ConvertProcessToFile(&ctx, outputFile)) {
        goto CLEANUP;
    }
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "Convert: completed, tempPath=%{public}s", tempFilePath);

CLEANUP:
    ConvertCleanup(&ctx, outputFile);

    struct stat finalStat;
    if (stat(tempFilePath, &finalStat) != 0 || finalStat.st_size == 0) {
        if (tempFilePath[0] != '\0') {
            unlink(tempFilePath);
        }
        OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, TAG, "Convert: output file empty or not exist");
        napi_get_null(env, &result);
    } else {
        result = BuildConvertResult(env, tempFilePath, &ctx);
    }
    return result;
}

napi_value FormatConversionForAutoTestRegister(napi_env env, napi_value exports)
{
    napi_property_descriptor desc[] = {
        {"convert", nullptr, Convert, nullptr, nullptr, nullptr, napi_default, nullptr},
    };
    napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc);
    return exports;
}
