/*
 * Copyright (c) 2025 Huawei Device Co., Ltd. 2025-2025. ALL rights reserved.
 */

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <climits>
#include <string>
#include <map>
#include <sys/stat.h>
#include "napi/native_api.h"
#include <unistd.h>
#include "hilog/log.h"
#include "ohaudio/native_audiocapturer.h"
#include "ohaudio/native_audiorenderer.h"
#include "ohaudio/native_audiostreambuilder.h"
#include "ohaudio/native_audiostream_base.h"
#include "ohaudio/native_audio_suite_base.h"
#include "ohaudio/native_audio_suite_engine.h"
#include "NodeManager.h"
#include <iomanip>
#include <fstream>
#include <filemanagement/file_uri/oh_file_uri.h>

const int GLOBAL_RESMGR = 0xFF00;
const char *TAG = "[AudioEditTestApp_AudioEdit_cpp]";
// 写入底层的音频数据缓冲区，目前 - 初始化input、用户保存数据后会释放以前的内存，，调用底层OH_AudioSuiteEngine_RenderFrame后，需要重新给g_totalBuff赋值
const int TOTAL_BUFF = 8 * 1024 * 1024;
char *g_totalBuff = (char *)malloc(TOTAL_BUFF);
char *g_tapTotalBuff = (char *)malloc(TOTAL_BUFF);
bool g_multiRenderFrameFlag = false;
// 需要写入的音频数据大小
size_t g_totalSize = 0;
std::map<std::string, void *> anotherAudioChannel;
std::string g_latestAissNodeId;
ssize_t g_audioDataSize = 0;
// 用于接收处理过后，音频的大小
ssize_t g_frameSize;
std::map<std::string, void *> tapCallbackResultMap;
std::string g_processingAissNode = "";
bool g_couldSetCallbackResult = false;
double g_dataInputProcessing = 0;
int g_writeIndex = 0;

// 定义一个结构体来存储ID和数字
struct UserData {
    std::string id;                      // 根据id去writeDataBufferMap_获取对应的音频数据
    int32_t bufferSize;                  // 音频总数据大小
    ssize_t totalWriteAudioDataSize;     // 已经写入的音频数据大小
    bool isResetTotalWriteAudioDataSize; // 音频是否从头开始写入
};
// 写入音频数据的map
std::map<std::string, std::vector<uint8_t>> writeDataBufferMap_;
// 存储UserData的map
std::map<std::string, UserData *> userDataMap_;
std::shared_ptr<NodeManager> nodeManager;
OH_AudioSuiteEngine *audioSuiteEngine;
OH_AudioSuitePipeline *audioSuitePipeline;
// 创造 output builder 构造器
OH_AudioNodeBuilder *builderOut;

struct CallbackOriginData {
    napi_env env;
    napi_value recv;
    napi_value callback;
};
struct SaveBufferCallbackParam {
    void *pcmBuffer;
    std::string filename;
    std::string audioFormate;
    int sampleRate;
    int channels;
    int bitsPerSample;
};
CallbackOriginData *g_callbackData = nullptr;
void *g_aissTapAudioData = (char *)malloc(1024 * 1024 * 100);
bool g_globalFinishFlag = true;
ssize_t g_tapDataTotalSize = 0;

OH_AudioFormat audioFormatInput;
OH_AudioFormat audioFormatOutput;

enum {
    ARG_1 = 0,
    ARG_2 = 1,
    ARG_3 = 2,
    ARG_4 = 3,
    ARG_5 = 4,
    ARG_6 = 5,
    ARG_7 = 6,
    ARG_8 = 7,
    ARG_9 = 8
};

enum class SampleFormat {
    AUDIO_SAMPLE_U8 = 8,
    AUDIO_SAMPLE_S16LE = 16,
    AUDIO_SAMPLE_S24LE = 24,
    AUDIO_SAMPLE_S32LE = 64,
    AUDIO_SAMPLE_F32LE = 32
};

enum class EqualizerFrequencyBandGains {
    EQUALIZER_PARAM_DEFAULT = 1,
    EQUALIZER_PARAM_BALLADS = 2,
    EQUALIZER_PARAM_CHINESE_STYLE = 3,
    EQUALIZER_PARAM_CLASSICAL = 4,
    EQUALIZER_PARAM_DANCE_MUSIC = 5,
    EQUALIZER_PARAM_JAZZ = 6,
    EQUALIZER_PARAM_POP = 7,
    EQUALIZER_PARAM_RB = 8,
    EQUALIZER_PARAM_ROCK = 9
};

enum class AudioChannelLayout {
    CH_LAYOUT_MONO = 1,
    CH_LAYOUT_STEREO = 2,
    CH_LAYOUT_STEREO_DOWNMIX = 3
};
const int AUDIODATA_ARRAYSIZE = 1024 * 4;
const int TOTALSIZE_MULTI = 100;
const int ERROR_RESULT = -1;
const int SAMPLINGRATE_MULTI = 20;
const int CHANNELCOUNT_MULTI = 1000;
const int BITSPERSAMPLE_MULTI = 8;
const int INPUTNODES_SIZE1 = 1;
const int INPUTNODES_SIZE2 = 2;
const int DATAINPUT_PROCESSING = 100;
const int AUDIODATAARRAY_SIZE = 2 * sizeof(void*);
const double DATAINPUTPROCESSING_SIZE = 100;
const int ACCESSAUDIODATA_ARRAY_NUM = 2;

// 解析 napi 字符串参数
napi_status parseNapiString(napi_env env, napi_value value, std::string &result)
{
    size_t size;
    napi_status status = napi_get_value_string_utf8(env, value, nullptr, 0, &size);
    if (status != napi_ok) {
        return status;
    }

    result.resize(size + 1); // 包含结尾的空字符
    status = napi_get_value_string_utf8(env, value, const_cast<char *>(result.data()), size + 1, nullptr);

    return status;
}

static void StoreTotalBuffToMap(const char *totalBuff, size_t size, const std::string &key)
{
    if (size > 0 && totalBuff != nullptr) {
        std::vector<uint8_t> buffer(totalBuff, totalBuff + size);
        writeDataBufferMap_[key] = buffer;
        return;
    }
    OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, TAG, "audioEditTest StoreTotalBuffToMap failed");
}

static OH_AudioSuite_Result RenDerFrame()
{
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest RenDerFrame start");

    // 启动管线
    OH_AudioSuite_Result result = OH_AudioSuiteEngine_StartPipeline(audioSuitePipeline);
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG,
        "audioEditTest OH_audioSuiteEngine_StartPipeline result: %{public}d", static_cast<int>(result));
    if (result != OH_AudioSuite_Result::AUDIOSUITE_SUCCESS) {
        return result;
    }

    // 获取管线状态
    OH_AudioSuite_PipelineState pipeLineState;
    result = OH_AudioSuiteEngine_GetPipelineState(audioSuitePipeline, &pipeLineState);
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG,
        "audioEditTest OH_audioSuiteEngine_GetPipelineState result: %{public}d --- pipeLineState: %{public}d",
        static_cast<int>(result), static_cast<int>(pipeLineState));
    if (result != OH_AudioSuite_Result::AUDIOSUITE_SUCCESS) {
        return result;
    }

    char *totalAudioData = (char *)malloc(1024 * 1024 * 100);
    char *tapTotalAudioData = (char *)malloc(1024 * 1024 * 100);
    char *audioData = (char *)malloc(1024 * 4);
    int32_t writeSize = 0;
    int32_t frameSize = 1024 * 4;
    bool finishedFlag = false;
    ssize_t resultTotalSize = 0;
    ssize_t tapResultTotalSize = 0;

    OH_AudioDataArray* ohAudioDataArray = new OH_AudioDataArray();
    ohAudioDataArray->audioDataArray = (void**)malloc(AUDIODATAARRAY_SIZE);
    for (int i = 0; i < ACCESSAUDIODATA_ARRAY_NUM; i++) {
        ohAudioDataArray->audioDataArray[i]=(void*)malloc(AUDIODATA_ARRAYSIZE);
    }
    ohAudioDataArray->arraySize=ACCESSAUDIODATA_ARRAY_NUM;
    ohAudioDataArray->requestFrameSize = frameSize;

    do {
        if (g_multiRenderFrameFlag) {
            result =
                OH_AudioSuiteEngine_MultiRenderFrame(audioSuitePipeline, ohAudioDataArray, &writeSize, &finishedFlag);
            OH_LOG_Print(
                LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG,
                "audioEditTest OH_AudioSuiteEngine_MultiRenderFrame frameSize: %{public}d,writeSize:%{public}d "
                "finishedFlag : %{public}s, result: %{public}d",
                ohAudioDataArray->requestFrameSize, writeSize, (finishedFlag ? "true" : "false"),
                static_cast<int>(result));
            if (result != OH_AudioSuite_Result::AUDIOSUITE_SUCCESS) {
                OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG,
                            "audioEditTest OH_audioSuiteEngine_RenderFrame result is %{public}d",
                            static_cast<int>(result));
                break;
            }

            // 每次保存一次获取的buffer值 ...
            memcpy(static_cast<char *>(totalAudioData) + resultTotalSize, ohAudioDataArray->audioDataArray[0],
                   writeSize);
            memcpy(static_cast<char *>(tapTotalAudioData) + tapResultTotalSize, ohAudioDataArray->audioDataArray[1],
                   writeSize);
            resultTotalSize += writeSize;
            tapResultTotalSize += writeSize;
            OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG,
                        "audioEditTest OH_AudioSuiteEngine_RenderFrame resultTotalSize: %{public}d, writeSize : "
                        "%{public}d, finished: %{public}s",
                        resultTotalSize, writeSize, (finishedFlag ? "true" : "false"));
        } else {
            result =
                OH_AudioSuiteEngine_RenderFrame(audioSuitePipeline, audioData, frameSize, &writeSize, &finishedFlag);
            OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG,
                "audioEditTest OH_AudioSuiteEngine_RenderFrame frameSize: %{public}d,writeSize:%{public}d "
                "finishedFlag : %{public}s, result: %{public}d",
                frameSize, writeSize, (finishedFlag ? "true" : "false"), static_cast<int>(result));
            if (result != OH_AudioSuite_Result::AUDIOSUITE_SUCCESS) {
                OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG,
                    "audioEditTest OH_audioSuiteEngine_RenderFrame result is %{public}d",
                    static_cast<int>(result));
                break;
            }
            // 每次保存一次获取的buffer值 ...
            memcpy(static_cast<char *>(totalAudioData) + resultTotalSize, audioData, writeSize);
            resultTotalSize += writeSize;
            OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG,
                "audioEditTest OH_AudioSuiteEngine_RenderFrame resultTotalSize: %{public}d, writeSize : "
                "%{public}d, finished: %{public}s",
                resultTotalSize, writeSize, (finishedFlag ? "true" : "false"));
        }
        if (finishedFlag) {
            g_globalFinishFlag = true;
            break;
        }
    } while (!finishedFlag);

    // 重新将数据写入到 totalBuff_
    if (g_totalBuff != nullptr) {
        free(g_totalBuff);
        g_totalBuff = nullptr;
    }
    g_totalSize = resultTotalSize;
    g_totalBuff = (char *)malloc(g_totalSize);
    memcpy(g_totalBuff, totalAudioData, g_totalSize);
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG,
        "audioEditTest RenDerFrame memcpy sizeof(g_totalBuff): %{public}d, g_totalSize:%{public}d",
        sizeof(g_totalBuff), g_totalSize);
    if (g_multiRenderFrameFlag) {
        g_totalSize = tapResultTotalSize;
        g_tapTotalBuff = (char *)malloc(g_totalSize);
        g_tapDataTotalSize = g_totalSize;
        memcpy(g_tapTotalBuff, tapTotalAudioData, g_totalSize);
        OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG,
            "audioEditTest RenDerFrame memcpy sizeof(g_totalBuff): %{public}d, g_totalSize:%{public}d",
            TOTAL_BUFF, g_totalSize);
        g_multiRenderFrameFlag = false;
    }
    return result;
}

static napi_value AudioEditNodeInit(napi_env env, napi_callback_info info)
{
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest AudioEditNodeInit start");
    size_t argc = 1;
    napi_value argv[1] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    // 解析工作模式

    // 创建引擎
    OH_AudioSuite_Result result = OH_AudioSuiteEngine_Create(&audioSuiteEngine);
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest OH_AudioEditEngine_Create result: %{public}d",
        static_cast<int>(result));
    // 根据入参判断当前的workmode
    unsigned int mode = -1;
    napi_get_value_uint32(env, argv[0], &mode);
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest---AudioEditNodeInit--workMode==%{public}d",
        mode);
    OH_AudioSuite_PipelineWorkMode workMode;
    if (mode == OH_AudioSuite_PipelineWorkMode::AUDIOSUITE_PIPELINE_EDIT_MODE) {
        workMode = OH_AudioSuite_PipelineWorkMode::AUDIOSUITE_PIPELINE_EDIT_MODE;
    } else if (mode == OH_AudioSuite_PipelineWorkMode::AUDIOSUITE_PIPELINE_REALTIME_MODE) {
        workMode = OH_AudioSuite_PipelineWorkMode::AUDIOSUITE_PIPELINE_REALTIME_MODE;
    } else {
        OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, TAG,
            "audioEditTest OH_AudioEditEngine_CreatePipeline workMode error: %{public}d", workMode);
        return nullptr;
    }
    // 创建管线
    result = OH_AudioSuiteEngine_CreatePipeline(audioSuiteEngine, &audioSuitePipeline, workMode);
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG,
        "audioEditTest OH_AudioEditEngine_CreatePipeline result: %{public}d", static_cast<int>(result));
    // 实例化NodeManager
    nodeManager = std::make_shared<NodeManager>(audioSuitePipeline);
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest createNodeManager result: %{public}d",
        static_cast<int>(nodeManager->getAllNodes().size()));

    napi_value napiValue;
    napi_create_int64(env, static_cast<int>(result), &napiValue);
    return napiValue;
}

// 释放内存
static void Clear()
{
    // 释放map内存
    writeDataBufferMap_.clear();
    for (auto &pair : userDataMap_) {
        delete pair.second; // 删除指针指向的对象
    }
    userDataMap_.clear();
}

static napi_value AudioEditDestory(napi_env env, napi_callback_info info)
{
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest AudioEditDestory start");
    Clear();
    OH_AudioSuite_Result result = OH_AudioSuiteEngine_DestroyPipeline(audioSuitePipeline);
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG,
        "audioEditTest OH_audioSuiteEngine_DestroyPipeline result: %{public}d", static_cast<int>(result));
    result = OH_AudioSuiteEngine_Destroy(audioSuiteEngine);
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest OH_audioSuiteEngine_Destroy result: %{public}d",
        static_cast<int>(result));
    napi_value napiValue;
    napi_create_int64(env, static_cast<int>(result), &napiValue);
    return napiValue;
}

// 设置采样率
static OH_Audio_SampleRate SetSamplingRate(int32_t sampleRate)
{
    OH_Audio_SampleRate audioSampleRate;
    switch (sampleRate) {
        case OH_Audio_SampleRate::SAMPLE_RATE_8000:
            audioSampleRate = OH_Audio_SampleRate::SAMPLE_RATE_8000;
            break;
        case OH_Audio_SampleRate::SAMPLE_RATE_11025:
            audioSampleRate = OH_Audio_SampleRate::SAMPLE_RATE_11025;
            break;
        case OH_Audio_SampleRate::SAMPLE_RATE_12000:
            audioSampleRate = OH_Audio_SampleRate::SAMPLE_RATE_12000;
            break;
        case OH_Audio_SampleRate::SAMPLE_RATE_16000:
            audioSampleRate = OH_Audio_SampleRate::SAMPLE_RATE_16000;
            break;
        case OH_Audio_SampleRate::SAMPLE_RATE_22050:
            audioSampleRate = OH_Audio_SampleRate::SAMPLE_RATE_22050;
            break;
        case OH_Audio_SampleRate::SAMPLE_RATE_24000:
            audioSampleRate = OH_Audio_SampleRate::SAMPLE_RATE_24000;
            break;
        case OH_Audio_SampleRate::SAMPLE_RATE_32000:
            audioSampleRate = OH_Audio_SampleRate::SAMPLE_RATE_32000;
            break;
        case OH_Audio_SampleRate::SAMPLE_RATE_44100:
            audioSampleRate = OH_Audio_SampleRate::SAMPLE_RATE_44100;
            break;
        case OH_Audio_SampleRate::SAMPLE_RATE_48000:
            audioSampleRate = OH_Audio_SampleRate::SAMPLE_RATE_48000;
            break;
        case OH_Audio_SampleRate::SAMPLE_RATE_64000:
            audioSampleRate = OH_Audio_SampleRate::SAMPLE_RATE_64000;
            break;
        case OH_Audio_SampleRate::SAMPLE_RATE_88200:
            audioSampleRate = OH_Audio_SampleRate::SAMPLE_RATE_88200;
            break;
        case OH_Audio_SampleRate::SAMPLE_RATE_96000:
            audioSampleRate = OH_Audio_SampleRate::SAMPLE_RATE_96000;
            break;
        case OH_Audio_SampleRate::SAMPLE_RATE_176400:
            audioSampleRate = OH_Audio_SampleRate::SAMPLE_RATE_176400;
            break;
        case OH_Audio_SampleRate::SAMPLE_RATE_192000:
            audioSampleRate = OH_Audio_SampleRate::SAMPLE_RATE_192000;
            break;
        default:
            audioSampleRate = OH_Audio_SampleRate::SAMPLE_RATE_48000;
            break;
    }
    return audioSampleRate;
}

// 设置声道
static OH_AudioChannelLayout SetChannelLayout(int32_t channels)
{
    OH_AudioChannelLayout audioChannelLayout;
    switch (channels) {
        case static_cast<int>(AudioChannelLayout::CH_LAYOUT_MONO):
            audioChannelLayout = CH_LAYOUT_MONO;
            break;
        case static_cast<int>(AudioChannelLayout::CH_LAYOUT_STEREO):
            audioChannelLayout = CH_LAYOUT_STEREO;
            break;
        default:
            audioChannelLayout = CH_LAYOUT_STEREO_DOWNMIX;
            break;
    }
    return audioChannelLayout;
}

// 设置位深
static OH_Audio_SampleFormat SetSampleFormat(int32_t bitsPerSample)
{
    OH_Audio_SampleFormat audioSampleFormat;
    switch (bitsPerSample) {
        case static_cast<int>(SampleFormat::AUDIO_SAMPLE_U8):
            audioSampleFormat = OH_Audio_SampleFormat::AUDIO_SAMPLE_U8;
            break;
        case static_cast<int>(SampleFormat::AUDIO_SAMPLE_S16LE):
            audioSampleFormat = OH_Audio_SampleFormat::AUDIO_SAMPLE_S16LE;
            break;
        case static_cast<int>(SampleFormat::AUDIO_SAMPLE_S24LE):
            audioSampleFormat = OH_Audio_SampleFormat::AUDIO_SAMPLE_S24LE;
            break;
        case static_cast<int>(SampleFormat::AUDIO_SAMPLE_F32LE):
            audioSampleFormat = OH_Audio_SampleFormat::AUDIO_SAMPLE_F32LE;
            break;
        default:
            audioSampleFormat = OH_Audio_SampleFormat::AUDIO_SAMPLE_S32LE;
            break;
    }
    return audioSampleFormat;
}

static napi_value SetFormat(napi_env env, napi_callback_info info)
{
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest SetFormat start");
    size_t argc = 3;
    napi_value *argv = new napi_value[argc];
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    // 获取通道数
    unsigned int channels;
    napi_get_value_uint32(env, argv[0], &channels);
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest SetFormat channels is %{public}d", channels);
    // 获取采样率
    unsigned int sampleRate;
    napi_get_value_uint32(env, argv[1], &sampleRate);
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest SetFormat sampleRate is %{public}d", sampleRate);
    // 获取位深
    unsigned int bitsPerSample;
    napi_get_value_uint32(env, argv[ARG_3], &bitsPerSample);
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest SetFormat bitsPerSample is %{public}d",
        bitsPerSample);

    // 设置采样率
    audioFormatOutput.samplingRate = SetSamplingRate(sampleRate);
    // 设置声道
    audioFormatOutput.channelCount = channels;
    audioFormatOutput.channelLayout = SetChannelLayout(channels);
    // 设置位深
    audioFormatOutput.sampleFormat = SetSampleFormat(bitsPerSample);
    // 设置编码格式
    audioFormatOutput.encodingType = OH_Audio_EncodingType::AUDIO_ENCODING_TYPE_RAW;
    const std::vector<Node> outPutNodes = nodeManager->getNodesByType(OH_AudioNode_Type::OUT_NODE_TYPE_DEFAULT);
    OH_AudioSuite_Result result = OH_AudioSuiteEngine_SetAudioFormat(outPutNodes[0].physicalNode, &audioFormatOutput);
    napi_value napiValue;
    napi_create_int64(env, static_cast<int>(result), &napiValue);
    return napiValue;
}

int32_t WriteDataCallBack(OH_AudioNode *audioNode, void *userData, void *audioData, int32_t audioDataSize,
                          bool *finished)
{
    // 检查audioNode参数，底层接口问题
    if (audioNode == nullptr) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, TAG, "audioEditTest WriteDataCallBack audioNode is nullptr");
        *finished = true;
        return 0;
    }
    // audioData，底层接口问题
    if (audioData == nullptr) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, TAG, "audioEditTest WriteDataCallBack audioData is nullptr");
        *finished = true;
        return 0;
    }
    // 检查finished参数，底层接口问题
    if (finished == nullptr) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, TAG, "audioEditTest WriteDataCallBack finished is nullptr");
        *finished = true;
        return 0;
    }
    // 处理音频数据 此处如果是nullptr，是demo获取音频数据的问题，非底层接口问题
    std::string inputId = static_cast<UserData *>(userData)->id;
    auto usetDataIt = userDataMap_.find(inputId);
    if (usetDataIt->second->isResetTotalWriteAudioDataSize) {
        usetDataIt->second->isResetTotalWriteAudioDataSize = false;
        static_cast<UserData *>(userData)->totalWriteAudioDataSize = 0;
    }
    int32_t totalSize = usetDataIt->second->bufferSize;
    ssize_t totalWriteAudioDataSize = usetDataIt->second->totalWriteAudioDataSize;
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest WriteDataCallBack inputId: %{public}s",
        inputId.c_str());
    auto it = writeDataBufferMap_.find(inputId);
    if (it == writeDataBufferMap_.end()) {
        // map没有找到对应的音频buffer
        OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, TAG,
            "audioEditTest WriteDataCallBack writeDataBufferMap_ is end");
        *finished = true;
        g_dataInputProcessing = DATAINPUTPROCESSING_SIZE;
        return 0;
    }
    // 计算剩余数据量
    int32_t remainingDataSize = totalSize - totalWriteAudioDataSize;
    // 确定本次写入的实际数据量
    int32_t actualDataSize = std::min(audioDataSize, remainingDataSize);
    g_dataInputProcessing += (static_cast<double>(actualDataSize) / static_cast<double>(totalSize) * TOTALSIZE_MULTI);
    // 将数据从totalBuff_复制到audioData
    memcpy(static_cast<char *>(audioData), it->second.data() + totalWriteAudioDataSize, actualDataSize);
    g_writeIndex++;
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest WriteDataCallBack g_writeIndex=%{public}d",
        g_writeIndex);
    // 跟新已写入的数据量
    totalWriteAudioDataSize += actualDataSize;
    usetDataIt->second->totalWriteAudioDataSize = totalWriteAudioDataSize;
    // 如果不够，则补0
    int32_t padSize = audioDataSize - remainingDataSize;
    if (padSize > 0) {
        OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest WriteDataCallBack padSize: %{public}d",
            padSize);
        memset(static_cast<char *>(audioData) + actualDataSize, 0, padSize);
    }

    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG,
        "audioEditTest WriteDataCallBack totalSize: %{public}d, totalWriteAudioDataSize: %{public}d, "
        "audioDataSize: %{public}d, actualDataSize:%{public}d, padSize: %{public}d",
        totalSize, totalWriteAudioDataSize, audioDataSize, actualDataSize, padSize);
    // 如果所有数据都写入完毕
    if (totalWriteAudioDataSize >= totalSize) {
        OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest WriteDataCallBack is finished");
        g_totalSize = 0;
        totalWriteAudioDataSize = 0;
        g_writeIndex = 0;
        *finished = true;
        g_dataInputProcessing = DATAINPUT_PROCESSING;
    }
    // 返回写入的数据数据量
    return actualDataSize;
}

OH_AudioSuite_Result SetParamsAndWriteData(OH_AudioNodeBuilder *builder, std::string inputId, int channels,
                                           int sampleRate, int bitsPerSample, int formatCategory,
                                           OH_AudioNode_Type type) {
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG,
        "audioEditTest SetParamsAndWriteData channels : %{public}d --- sampleRate: %{public}d --- "
        "PerSample: %{public}d --- formatCategory: %{public}d",
        channels, sampleRate, bitsPerSample, formatCategory);
    // 设置采样率
    audioFormatInput.samplingRate = SetSamplingRate(sampleRate);
    // 设置声道
    audioFormatInput.channelCount = channels;
    audioFormatInput.channelLayout = SetChannelLayout(channels);
    // 设置位深
    audioFormatInput.sampleFormat = SetSampleFormat(bitsPerSample);
    // 设置编码格式
    audioFormatInput.encodingType = OH_Audio_EncodingType::AUDIO_ENCODING_TYPE_RAW;

    // audioFormatOutput 初始化和 audioFormatInput 相同
    audioFormatOutput.sampleFormat = audioFormatInput.sampleFormat;
    audioFormatOutput.samplingRate = audioFormatInput.samplingRate;
    audioFormatOutput.channelLayout = audioFormatInput.channelLayout;
    audioFormatOutput.encodingType = audioFormatInput.encodingType;
    audioFormatOutput.channelCount = audioFormatInput.channelCount;

    OH_AudioSuite_Result result = OH_AudioSuiteNodeBuilder_SetFormat(builder, audioFormatInput);
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG,
        "audioEditTest OH_AudioNodeBuilder_SetFormat result is %{public}d", static_cast<int>(result));
    if (result != OH_AudioSuite_Result::AUDIOSUITE_SUCCESS) {
        return result;
    }
    if (type != OH_AudioNode_Type::INPUT_NODE_TYPE_DEFAULT) {
        return result;
    }
    UserData *data = new UserData();
    data->id = inputId;
    // 后面可以考虑去掉g_totalSize, 用入参形式传入
    data->bufferSize = g_totalSize;
    data->totalWriteAudioDataSize = 0;
    data->isResetTotalWriteAudioDataSize = false;
    void *userData = data;
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG,
        "audioEditTest OH_AudioSuiteNodeBuilder_SetRequestDataCallback data address is %{public}p", &data);
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG,
        "audioEditTest OH_AudioNodeBuilder_SetFormat userData inputId is %{public}s",
        static_cast<UserData *>(userData)->id.c_str());
    // 设置OH_AudioSuiteNodeBuilder_SetRequestDataCallback回调，创建节点之前
    result = OH_AudioSuiteNodeBuilder_SetRequestDataCallback(builder, WriteDataCallBack, userData);
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG,
        "audioEditTest OH_AudioSuiteNodeBuilder_SetRequestDataCallback result is %{public}d",
        static_cast<int>(result));
    if (result != OH_AudioSuite_Result::AUDIOSUITE_SUCCESS) {
        return result;
    }
    // 将UserData实例存入映射表中
    userDataMap_[inputId] = data;
    return result;
}

static void ParseArguments(napi_env env, napi_value *argv, std::string &inputId, std::string &outputId,
                           std::string &mixerId, unsigned int &channels, unsigned int &sampleRate,
                           unsigned int &bitsPerSample, unsigned int &formatCategory, unsigned int &pcmLength,
                           void *&buffer, size_t &bufferLength)
{
    napi_status status = parseNapiString(env, argv[ARG_1], inputId);
    status = parseNapiString(env, argv[ARG_2], outputId);
    status = parseNapiString(env, argv[ARG_3], mixerId);
    napi_get_value_uint32(env, argv[ARG_4], &channels);
    napi_get_value_uint32(env, argv[ARG_5], &sampleRate);
    napi_get_value_uint32(env, argv[ARG_6], &bitsPerSample);
    napi_get_value_uint32(env, argv[ARG_7], &formatCategory);
    napi_get_value_uint32(env, argv[ARG_8], &pcmLength);
    napi_get_arraybuffer_info(env, argv[ARG_9], &buffer, &bufferLength);
}

static void CreateInputNode(napi_env env, const std::string &inputId, unsigned int channels, unsigned int sampleRate,
                            unsigned int bitsPerSample, unsigned int formatCategory, void *buffer, size_t bufferLength,
                            napi_value &napiValue, OH_AudioSuite_Result &result)
{
    // 添加音频，将音频的buffer出存储到map中，，上一行中的memcpy可以考虑删除了
    StoreTotalBuffToMap(g_totalBuff, g_totalSize, inputId);
    auto it = writeDataBufferMap_.find(inputId);
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG,
        "audioEditTest AudioInAndOutInit writeDataBufferMap_[inputId] length: %{public}d", it->second.size());
    // 创造 builder 构造器
    OH_AudioNodeBuilder *builderIn;
    result = OH_AudioSuiteNodeBuilder_Create(&builderIn);
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG,
        "audioEditTest OH_AudioSuiteNodeBuilder_Create result: %{public}d", static_cast<int>(result));
    if (result != OH_AudioSuite_Result::AUDIOSUITE_SUCCESS) {
        napi_create_int64(env, static_cast<int>(result), &napiValue);
        return;
    }
    // 透传节点类型
    result = OH_AudioSuiteNodeBuilder_SetNodeType(builderIn, OH_AudioNode_Type::INPUT_NODE_TYPE_DEFAULT);
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG,
        "NodeManagerTest createNode OH_AudioSuiteNodeBuilder_SetNodeType result: %{public}d",
        static_cast<int>(result));
    if (result != OH_AudioSuite_Result::AUDIOSUITE_SUCCESS) {
        napi_create_int64(env, static_cast<int>(result), &napiValue);
        return;
    }

    // 封装方法，设置 音频文件的 参数 以及 写入音频文件到缓冲区
    result = SetParamsAndWriteData(builderIn, inputId, channels, sampleRate, bitsPerSample, formatCategory,
                                   OH_AudioNode_Type::INPUT_NODE_TYPE_DEFAULT);
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest SetParamsAndWriteData result: %{public}d",
        static_cast<int>(result));
    if (result != OH_AudioSuite_Result::AUDIOSUITE_SUCCESS) {
        napi_create_int64(env, static_cast<int>(result), &napiValue);
        return;
    }

    // 创建input节点
    nodeManager->createNode(inputId, OH_AudioNode_Type::INPUT_NODE_TYPE_DEFAULT, builderIn);
}

static void UpdateInputNode(napi_env env, const std::string &inputId, unsigned int channels, unsigned int sampleRate,
                            unsigned int bitsPerSample, napi_value &napiValue, OH_AudioSuite_Result &result)
{
    // 设置采样率
    audioFormatInput.samplingRate = SetSamplingRate(sampleRate);
    // 设置声道
    audioFormatInput.channelCount = channels;
    audioFormatInput.channelLayout = SetChannelLayout(channels);
    // 设置位深
    audioFormatInput.sampleFormat = SetSampleFormat(bitsPerSample);
    // 设置编码格式
    audioFormatInput.encodingType = OH_Audio_EncodingType::AUDIO_ENCODING_TYPE_RAW;

    audioFormatOutput.sampleFormat = audioFormatInput.sampleFormat;
    audioFormatOutput.samplingRate = audioFormatInput.samplingRate;
    audioFormatOutput.channelLayout = audioFormatInput.channelLayout;
    audioFormatOutput.encodingType = audioFormatInput.encodingType;
    audioFormatOutput.channelCount = audioFormatInput.channelCount;

    const std::vector<Node> inPutNodes = nodeManager->getNodesByType(OH_AudioNode_Type::INPUT_NODE_TYPE_DEFAULT);
    result = OH_AudioSuiteEngine_SetAudioFormat(inPutNodes[0].physicalNode, &audioFormatInput);
    const std::vector<Node> outPutNodes = nodeManager->getNodesByType(OH_AudioNode_Type::OUT_NODE_TYPE_DEFAULT);
    result = OH_AudioSuiteEngine_SetAudioFormat(outPutNodes[0].physicalNode, &audioFormatOutput);
    // 添加音频，将音频的buffer出存储到map中，，上一行中的memcpy可以考虑删除了
    if (writeDataBufferMap_.find(inputId) != writeDataBufferMap_.end()) {
        // 键存在，执行删除操作
        writeDataBufferMap_.erase(inputId);
    }
    StoreTotalBuffToMap(g_totalBuff, g_totalSize, inputId);
    auto it = writeDataBufferMap_.find(inputId);
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG,
        "audioEditTest AudioInAndOutInit writeDataBufferMap_[inputId] length: %{public}d", it->second.size());
    UserData *data = new UserData();
    data->id = inputId;
    // 后面可以考虑去掉g_totalSize，用入参形式传入
    data->bufferSize = g_totalSize;
    data->totalWriteAudioDataSize = 0;
    data->isResetTotalWriteAudioDataSize = false;
    // 将UserData实例存入映射表中
    if (userDataMap_.find(inputId) != userDataMap_.end()) {
        // 键存在，执行删除操作
        userDataMap_.erase(inputId);
    }
    userDataMap_[inputId] = data;
}

// 导入音频调用
static napi_value AudioInAndOutInit(napi_env env, napi_callback_info info)
{
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest AudioInAndOutInit start");
    napi_value napiValue;
    OH_AudioSuite_Result result;

    size_t argc = 9;
    napi_value *argv = new napi_value[argc];
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);

    std::string inputId;
    std::string outputId;
    std::string mixerId;
    unsigned int channels;
    unsigned int sampleRate;
    unsigned int bitsPerSample;
    unsigned int formatCategory;
    unsigned int pcmLength;
    void *buffer = nullptr;
    size_t bufferLength = 0;

    ParseArguments(env, argv, inputId, outputId, mixerId, channels, sampleRate, bitsPerSample, formatCategory,
                   pcmLength, buffer, bufferLength);
    if (g_totalBuff != nullptr) {
        free(g_totalBuff);
        g_totalBuff = nullptr;
    }
    g_totalSize = pcmLength;
    g_totalBuff = (char *)malloc(bufferLength);
    memcpy(g_totalBuff, buffer, bufferLength);
    Node inputNode = nodeManager->getNodeById(inputId);
    if (inputNode.id.empty()) {
        CreateInputNode(env, inputId, channels, sampleRate, bitsPerSample, formatCategory, buffer, bufferLength,
                        napiValue, result);
    } else {
        UpdateInputNode(env, inputId, channels, sampleRate, bitsPerSample, napiValue, result);
    }

    const std::vector<Node> outPutNodes = nodeManager->getNodesByType(OH_AudioNode_Type::OUT_NODE_TYPE_DEFAULT);
    // 判断当前有无output节点，没有output节点则创建output节点并连接input节点和output节点
    if (outPutNodes.size() > 0) {
        const std::vector<Node> mixerNodes =
            nodeManager->getNodesByType(OH_AudioNode_Type::EFFECT_NODE_TYPE_AUDIO_MIXER);
        // 判断有混音节点，则直接将input节点连接到混音节点之前；无混音节点则需要创建混音节点，再将混音节点插入到output节点之前
        if (mixerNodes.size() > 0) {
            result = nodeManager->connect(inputId, mixerNodes[0].id);
            OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG,
                "audioEditTest connect input and mixer result: %{public}d", static_cast<int>(result));
        } else {
            result = nodeManager->createNode(mixerId, OH_AudioNode_Type::EFFECT_NODE_TYPE_AUDIO_MIXER);
            OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG,
                "audioEditTest nodeManagerCreateMixerNode result: %{public}d", static_cast<int>(result));
            // 解开outPut节点与它之前得节点连接，将混音节点插入其中，再将input节点连接到混音节点上
            result = nodeManager->insertNode(mixerId, outPutNodes[0].id, Direction::BEFORE);
            OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest insertMixerNode result: %{public}d",
                static_cast<int>(result));
            result = nodeManager->connect(inputId, mixerId);
            OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG,
                "audioEditTest connect inputId and mixerId result: %{public}d", static_cast<int>(result));
        }
    } else {
        OH_AudioSuite_Result result = OH_AudioSuiteNodeBuilder_Create(&builderOut);
        OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG,
            "audioEditTest OH_AudioSuiteNodeBuilder_Create output builder result: %{public}d",
            static_cast<int>(result));
        if (result != OH_AudioSuite_Result::AUDIOSUITE_SUCCESS) {
            napi_create_int64(env, static_cast<int>(result), &napiValue);
            return napiValue;
        }
        result = OH_AudioSuiteNodeBuilder_SetNodeType(builderOut, OH_AudioNode_Type::OUT_NODE_TYPE_DEFAULT);
        OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG,
            "NodeManagerTest createNode OH_AudioSuiteNodeBuilder_SetNodeType result: %{public}d",
            static_cast<int>(result));
        if (result != OH_AudioSuite_Result::AUDIOSUITE_SUCCESS) {
            napi_create_int64(env, static_cast<int>(result), &napiValue);
            return napiValue;
        }
        // 封装方法，设置 音频文件的 参数 以及 写入音频文件到缓冲区
        result = SetParamsAndWriteData(builderOut, inputId, channels, sampleRate, bitsPerSample, formatCategory,
                                       OH_AudioNode_Type::OUT_NODE_TYPE_DEFAULT);
        OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest SetParamsAndWriteData result: %{public}d",
            static_cast<int>(result));
        if (result != OH_AudioSuite_Result::AUDIOSUITE_SUCCESS) {
            napi_create_int64(env, static_cast<int>(result), &napiValue);
            return napiValue;
        }

        result = nodeManager->createNode(outputId, OH_AudioNode_Type::OUT_NODE_TYPE_DEFAULT, builderOut);
        OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG,
            "audioEditTest nodeManagerCreateOutputNode result: %{public}d", static_cast<int>(result));
        result = nodeManager->connect(inputId, outputId);
        OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG,
            "audioEditTest nodeManagerConnectInputAndOutput result: %{public}d", static_cast<int>(result));
    }

    napi_create_int64(env, static_cast<int>(result), &napiValue);
    return napiValue;
}

// 创建效果节点后调用该方法将效果节点加入到nodeManager中
int AddEffectNodeToNodeManager(std::string &inputNodeId, std::string &effectNodeId)
{
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG,
        "audioEditTest AddEffectNodeToNodeManager start and inputNodeId is : %{public}s, effectNodeId is %{public}s",
        inputNodeId.c_str(), effectNodeId.c_str());
    // 添加效果节点，检查是否有混音节点，没有混音节点就将效果节点添加到output节点之前；有混音节点，获取到对应input节点id，按序插入到混音节点之前
    const std::vector<Node> mixerNodes = nodeManager->getNodesByType(OH_AudioNode_Type::EFFECT_NODE_TYPE_AUDIO_MIXER);
    OH_AudioSuite_Result result;
    Node node = nodeManager->getNodeById(effectNodeId);
    if (node.id.empty()) {
        return ERROR_RESULT;
    }

    if (mixerNodes.size() > 0) {
        OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest AddEffectNodeToNodeManager has mixerNodes");
        Node currentNode = nodeManager->getNodeById(inputNodeId);
        if (currentNode.nextNodeId.empty()) {
            return ERROR_RESULT;
        }
        while (nodeManager->getNodeById(currentNode.nextNodeId).type !=
               OH_AudioNode_Type::EFFECT_NODE_TYPE_AUDIO_MIXER) {
            currentNode = nodeManager->getNodeById(currentNode.nextNodeId);
            OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG,
                "audioEditTest AddEffectNodeToNodeManager has mixerNodes and nextNode : %{public}s",
                currentNode.id.c_str());
        }
        result = nodeManager->insertNode(effectNodeId, currentNode.id, Direction::LATER);
    } else {
        const std::vector<Node> outPutNodes = nodeManager->getNodesByType(OH_AudioNode_Type::OUT_NODE_TYPE_DEFAULT);
        result = nodeManager->insertNode(effectNodeId, outPutNodes[0].id, Direction::BEFORE);
    }

    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG,
        "audioEditTest AddEffectNodeToNodeManager end and result is: %{public}d", static_cast<int>(result));

    return result;
}

// 删除音频
static napi_value DeleteSong(napi_env env, napi_callback_info info)
{
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest DeleteSong start");

    OH_AudioSuite_Result result;
    napi_value napiValue;
    size_t argc = 1;
    napi_value *argv = new napi_value[argc];
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);

    // 获取inputId参数
    std::string inputId;
    napi_status status = parseNapiString(env, argv[0], inputId);
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest DeleteSong inputId is %{public}s",
        inputId.c_str());
    
    const std::vector<Node> inputNodes = nodeManager->getNodesByType(OH_AudioNode_Type::INPUT_NODE_TYPE_DEFAULT);
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest DeleteSong inputNodes length is %{public}d",
        static_cast<int>(inputNodes.size()));
    if (inputNodes.size() > INPUTNODES_SIZE2) {
        Node node = nodeManager->getNodeById(inputId);
        Node nextNode;
        if (node.id.empty()) {
            napi_create_int64(env, static_cast<int>(result), &napiValue);
            return napiValue;
        }
        while (node.type != OH_AudioNode_Type::EFFECT_NODE_TYPE_AUDIO_MIXER) {
            nextNode = nodeManager->getNodeById(node.nextNodeId);
            result = nodeManager->removeNode(node.id);
            if (result != OH_AudioSuite_Result::AUDIOSUITE_SUCCESS) {
                napi_create_int64(env, static_cast<int>(result), &napiValue);
                return napiValue;
            }
            node = nextNode;
        }
        OH_LOG_Print(
            LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG,
            "audioEditTest deleteSong preNodes of mixerNode and inputNodes number greater than 2 : %{public}d",
            static_cast<int>(
                nodeManager->getNodesByType(OH_AudioNode_Type::EFFECT_NODE_TYPE_AUDIO_MIXER)[0].preNodeIds.size()));
    } else if (inputNodes.size() == INPUTNODES_SIZE2) {
        Node node = nodeManager->getNodeById(inputId);
        Node nextNode;
        if (node.id.empty()) {
            napi_create_int64(env, static_cast<int>(result), &napiValue);
            return napiValue;
        }
        while (node.type != OH_AudioNode_Type::OUT_NODE_TYPE_DEFAULT) {
            nextNode = nodeManager->getNodeById(node.nextNodeId);
            result = nodeManager->removeNode(node.id);
            if (result != OH_AudioSuite_Result::AUDIOSUITE_SUCCESS) {
                napi_create_int64(env, static_cast<int>(result), &napiValue);
                return napiValue;
            }
            node = nextNode;
        }
        OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest deleteSong number of mixerNode : %{public}d",
            static_cast<int>(
                nodeManager->getNodesByType(OH_AudioNode_Type::OUT_NODE_TYPE_DEFAULT)[0].preNodeIds.size()));
    } else if (inputNodes.size() == INPUTNODES_SIZE1) {
        Node node = nodeManager->getNodeById(inputId);
        Node nextNode;
        OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG,
            "audioEditTest deleteSong inputNode nextNodeId : %{public}s", node.nextNodeId.c_str());
        while (!node.id.empty()) {
            OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest deleteSong inputNodes is 1 : %{public}s",
                node.id.c_str());
            nextNode = nodeManager->getNodeById(node.nextNodeId);
            result = nodeManager->removeNode(node.id);
            if (result != OH_AudioSuite_Result::AUDIOSUITE_SUCCESS) {
                napi_create_int64(env, static_cast<int>(result), &napiValue);
                return napiValue;
            }
            node = nextNode;
        }
        OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest deleteSong nodes : %{public}d",
            static_cast<int>(nodeManager->getAllNodes().size()));
    } else {
        napi_create_int64(env, static_cast<int>(-1), &napiValue);
        OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest deleteSong inputNodes less than 1");
        return napiValue;
    }
    napi_create_int64(env, static_cast<int>(result), &napiValue);
    return napiValue;
}

// 删除节点
static napi_value DeleteNode(napi_env env, napi_callback_info info)
{
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest DeleteNode start");

    OH_AudioSuite_Result result;
    napi_value napiValue;
    size_t argc = 1;
    napi_value *argv = new napi_value[argc];
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);

    // 获取nodeId参数
    std::string nodeId;
    napi_status status = parseNapiString(env, argv[0], nodeId);
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest DeleteNode nodeId is %{public}s",
        nodeId.c_str());
    
    result = nodeManager->removeNode(nodeId);

    napi_create_int64(env, static_cast<int>(result), &napiValue);
    return napiValue;
}

// 拖拽效果节点
static napi_value DragEffectNode(napi_env env, napi_callback_info info)
{
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest DeleteSong start");

    OH_AudioSuite_Result result;
    napi_value napiValue;
    size_t argc = 1;
    napi_value *argv = new napi_value[argc];
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);

    std::string inputId;
    std::string sourceId;
    std::string targetId;

    napi_status status = parseNapiString(env, argv[0], inputId);
    status = parseNapiString(env, argv[ARG_2], sourceId);
    status = parseNapiString(env, argv[ARG_3], targetId);
    
    if (targetId.empty()) {
        OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest DeleteSong targetId is empty");
        Node node = nodeManager->getNodeById(inputId);
        if (node.nextNodeId.empty()) {
            napi_create_int64(env, static_cast<int>(-1), &napiValue);
            return napiValue;
        }
        while (nodeManager->getNodeById(node.nextNodeId).type != OH_AudioNode_Type::EFFECT_NODE_TYPE_AUDIO_MIXER &&
               nodeManager->getNodeById(node.nextNodeId).type != OH_AudioNode_Type::OUT_NODE_TYPE_DEFAULT) {
            node = nodeManager->getNodeById(node.nextNodeId);
            OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG,
                "audioEditTest DragEffectNode targetId is empty : %{public}s", node.id.c_str());
        }
        result = nodeManager->moveNode(sourceId, node.id, Direction::LATER);
    } else {
        result = nodeManager->moveNode(sourceId, targetId, Direction::BEFORE);
    }
    
    return napiValue;
}

// 封装入参 OH_EqualizerMode
static OH_EqualizerFrequencyBandGains SetEqualizerMode(int32_t equailizerMode)
{
    OH_EqualizerFrequencyBandGains eqMode;
    switch (equailizerMode) {
        case static_cast<int>(EqualizerFrequencyBandGains::EQUALIZER_PARAM_DEFAULT):
            eqMode = OH_EQUALIZER_PARAM_DEFAULT;
            break;
        case static_cast<int>(EqualizerFrequencyBandGains::EQUALIZER_PARAM_BALLADS):
            eqMode = OH_EQUALIZER_PARAM_BALLADS;
            break;
        case static_cast<int>(EqualizerFrequencyBandGains::EQUALIZER_PARAM_CHINESE_STYLE):
            eqMode = OH_EQUALIZER_PARAM_CHINESE_STYLE;
            break;
        case static_cast<int>(EqualizerFrequencyBandGains::EQUALIZER_PARAM_CLASSICAL):
            eqMode = OH_EQUALIZER_PARAM_CLASSICAL;
            break;
        case static_cast<int>(EqualizerFrequencyBandGains::EQUALIZER_PARAM_DANCE_MUSIC):
            eqMode = OH_EQUALIZER_PARAM_DANCE_MUSIC;
            break;
        case static_cast<int>(EqualizerFrequencyBandGains::EQUALIZER_PARAM_JAZZ):
            eqMode = OH_EQUALIZER_PARAM_JAZZ;
            break;
        case static_cast<int>(EqualizerFrequencyBandGains::EQUALIZER_PARAM_POP):
            eqMode = OH_EQUALIZER_PARAM_POP;
            break;
        case static_cast<int>(EqualizerFrequencyBandGains::EQUALIZER_PARAM_RB):
            eqMode = OH_EQUALIZER_PARAM_RB;
            break;
        case static_cast<int>(EqualizerFrequencyBandGains::EQUALIZER_PARAM_ROCK):
            eqMode = OH_EQUALIZER_PARAM_ROCK;
            break;
        default:
            eqMode = OH_EQUALIZER_PARAM_DEFAULT;
            break;
    }
    return eqMode;
}

// 设置均衡器模式方法
static napi_value SetEquailizerMode(napi_env env, napi_callback_info info)
{
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest SetEquailizerMode start");
    napi_value napiValue;
    size_t argc = 3;
    napi_value *argv = new napi_value[argc];
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    // 获取效果节点的效果参数
    unsigned int equailizerMode = -1;
    napi_get_value_uint32(env, argv[ARG_1], &equailizerMode);
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest SetEquailizerMode equailizerMode is %{public}d",
        equailizerMode);
    // 获取效果节点的id
    std::string equalizerId;
    napi_status status = parseNapiString(env, argv[ARG_2], equalizerId);
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest SetEquailizerMode equalizerId is %{public}s",
        equalizerId.c_str());
    
    // 获取input节点的id
    std::string inputId;
    status = parseNapiString(env, argv[ARG_3], inputId);
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest SetEquailizerMode inputId is %{public}s",
        inputId.c_str());

    // 创建均衡器效果节点
    Node eqNode;
    // 获取效果节点
    eqNode = nodeManager->getNodeById(equalizerId);
    if (eqNode.physicalNode) {
        OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest SetEquailizerMode equalizer is exist");
    } else {
        // 创建均衡器节点
        OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest SetEquailizerMode create EQUALIZER node");
        eqNode.id = equalizerId;
        eqNode.type = OH_AudioNode_Type::EFFECT_NODE_TYPE_EQUALIZER;
        nodeManager->createNode(equalizerId, OH_AudioNode_Type::EFFECT_NODE_TYPE_EQUALIZER);
        // 获取效果节点
        eqNode = nodeManager->getNodeById(equalizerId);
        int resultInt = AddEffectNodeToNodeManager(inputId, equalizerId);
        OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG,
            "audioEditTest AddEffectNodeToNodeManager result: %{public}d", resultInt);
        if (resultInt != 0) {
            napi_create_int64(env, resultInt, &napiValue);
            return napiValue;
        }
    }

    OH_AudioSuite_Result result =
        OH_AudioSuiteEngine_SetEqualizerFrequencyBandGains(eqNode.physicalNode, SetEqualizerMode(equailizerMode));
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG,
        "audioEditTest OH_AudioSuiteEngine_SetEqualizerMode result: %{public}d", static_cast<int>(result));
    if (result != OH_AudioSuite_Result::AUDIOSUITE_SUCCESS) {
        napi_create_int64(env, static_cast<int>(result), &napiValue);
        return napiValue;
    }

    napi_create_int64(env, static_cast<int>(result), &napiValue);
    return napiValue;
}

// 设置均衡器频带增益
static napi_value SetEqualizerFrequencyBandGains(napi_env env, napi_callback_info info)
{
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest SetEqualizerFrequencyBandGains start");
    napi_value napiValue;
    size_t argc = 4;
    napi_value *argv = new napi_value[argc];
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    // 判断是否是数组
    bool isArray;
    napi_is_array(env, argv[0], &isArray);
    if (!isArray) {
        OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG,
            "audioEditTest SetEqualizerFrequencyBandGains param not array");
        napi_create_int64(env, -1, &napiValue);
        return napiValue;
    }
    uint32_t length;
    napi_get_array_length(env, argv[ARG_1], &length);
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG,
        "audioEditTest SetEqualizerFrequencyBandGains param length is %{public}d", length);

    // 获取效果节点的id
    std::string equalizerId;
    napi_status status = parseNapiString(env, argv[ARG_2], equalizerId);
    if (status == napi_ok) {
        OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG,
            "audioEditTest SetEqualizerFrequencyBandGains equalizerId is %{public}s", equalizerId.c_str());
    } else {
        OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "Failed to get equalizerId");
    }
    
    // 获取input节点的id
    std::string inputId;
    status = parseNapiString(env, argv[ARG_3], inputId);
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG,
        "audioEditTest SetEqualizerFrequencyBandGains inputId is %{public}s", inputId.c_str());
    if (status == napi_ok) {
        OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG,
            "audioEditTest SetEqualizerFrequencyBandGains inputId is %{public}s", inputId.c_str());
    } else {
        OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "Failed to get inputId");
    }
    // 获取 selectNodeId
    std::string selectedNodeId;
    status = parseNapiString(env, argv[ARG_4], selectedNodeId);
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG,
        "audioEditTest SetEqualizerFrequencyBandGains selectedNodeId is %{public}s", selectedNodeId.c_str());

    // 创建均衡器频带节点
    Node eqNode;
    // 获取效果节点
    eqNode = nodeManager->getNodeById(equalizerId);
    if (eqNode.physicalNode) {
        OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG,
            "audioEditTest SetEqualizerFrequencyBandGains equalizer is exist");
    } else {
        // 创建均衡器节点
        OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG,
            "audioEditTest SetEqualizerFrequencyBandGains create EQUALIZER node");
        eqNode.id = equalizerId;
        eqNode.type = OH_AudioNode_Type::EFFECT_NODE_TYPE_EQUALIZER;
        nodeManager->createNode(equalizerId, OH_AudioNode_Type::EFFECT_NODE_TYPE_EQUALIZER);
        // 获取效果节点
        eqNode = nodeManager->getNodeById(equalizerId);
        if (selectedNodeId.empty()) {
            int resultInt = AddEffectNodeToNodeManager(inputId, equalizerId);
            OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG,
                "audioEditTest AddEffectNodeToNodeManager AddEffectNodeToNodeManager result: %{public}d",
                resultInt);
            if (resultInt != 0) {
                napi_create_int64(env, resultInt, &napiValue);
                return napiValue;
            }
        } else {
            int resultInt = nodeManager->insertNode(equalizerId, selectedNodeId, Direction::LATER);
            OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG,
                "audioEditTest AddEffectNodeToNodeManager insertNode result: %{public}d", resultInt);
            if (resultInt != 0) {
                napi_create_int64(env, resultInt, &napiValue);
                return napiValue;
            }
        }
    }
    OH_EqualizerFrequencyBandGains frequencyBandGains;
    // 遍历数组并打印每个元素
    for (uint32_t i = 0; i < length; ++i) {
        napi_value element;
        napi_get_element(env, argv[0], i, &element);
        unsigned int value;
        napi_get_value_uint32(env, element, &value);
        frequencyBandGains.gains[i] = value;
        OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG,
            "audioEditTest SeqEqualizerFrequencyBandGains element at index %{public}d is %{public}d",
            i, frequencyBandGains.gains[i]);
    }

    OH_AudioSuite_Result result =
        OH_AudioSuiteEngine_SetEqualizerFrequencyBandGains(eqNode.physicalNode, frequencyBandGains);
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG,
        "audioEditTest OH_AudioSuiteEngine_SetEquailizerMode result: %{public}d", static_cast<int>(result));

    napi_create_int64(env, static_cast<int>(result), &napiValue);
    return napiValue;
}

void ResetAllIsResetTotalWriteAudioDataSize()
{
    for (auto &pair : userDataMap_) {
        pair.second->isResetTotalWriteAudioDataSize = true;
    }
}

static napi_value SaveFileBuffer(napi_env env, napi_callback_info info)
{
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest SaveFileBuffer start");
    ResetAllIsResetTotalWriteAudioDataSize();
    RenDerFrame();
    Clear();

    napi_value napiValue = nullptr;
    void *arrayBufferData = nullptr;
    napi_status status = napi_create_arraybuffer(env, g_totalSize, &arrayBufferData, &napiValue);
    if (status != napi_ok || arrayBufferData == nullptr) {
        OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG,
            "audioEditTest OH_AudioSuiteEngine_RenderFrame status: %{public}d", static_cast<int>(status));
        if (g_totalBuff != nullptr) {
            free(g_totalBuff);
            g_totalBuff = nullptr;
        }
        // 创建 ArrayBuffer 失败， 返回一个大小为 0 的ArrayBuffer
        napi_create_arraybuffer(env, 0, &arrayBufferData, &napiValue);
        return napiValue;
    } else {
        auto ret = memcpy_s(arrayBufferData, g_totalSize, g_totalBuff, g_totalSize);
        if (ret != 0) {
            OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, TAG,
                "audioEditTest memcpy_s arrayBufferData failed, ret is %{public}zd", ret);
        }
        if (g_totalBuff != nullptr) {
            free(g_totalBuff);
            g_totalBuff = nullptr;
        }
        return napiValue;
    }
}

static Node createNodeByType(std::string uuid, OH_AudioNode_Type nodeType)
{
    OH_AudioSuite_Result result = nodeManager->createNode(uuid, nodeType);
    if (result != AUDIOSUITE_SUCCESS) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, TAG, "audioEditTest---create AudioSeparation Node Failed");
    }
    Node node = nodeManager->getNodeById(uuid);
    return node;
}
static napi_value addNoiseReduction(napi_env env, napi_callback_info info)
{
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest---addNoiseReduction IN");
    size_t argc = 3;
    napi_value *argv = new napi_value[argc];
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    // 获取uuid
    std::string uuidStr;
    napi_status status = parseNapiString(env, argv[ARG_1], uuidStr);
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest---uuid==%{public}s", uuidStr.c_str());

    // 获取二参inputId
    std::string inputIdStr;
    status = parseNapiString(env, argv[ARG_2], inputIdStr);
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest---inputId==%{public}s", inputIdStr.c_str());

    // 获取当前选中的节点id
    std::string selectNodeId;
    status = parseNapiString(env, argv[ARG_3], selectNodeId);
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest---addNoiseReduction---selectNodeId==%{public}s",
        selectNodeId.c_str());

    napi_value ret = nullptr;
    napi_create_int32(env, 1, &ret);
    Node node = createNodeByType(uuidStr, OH_AudioNode_Type::EFFECT_NODE_TYPE_NOISE_REDUCTION);
    if (node.physicalNode == nullptr) {
        return ret;
    }

    int insertRes = -1;
    if (selectNodeId.empty()) {
        insertRes = AddEffectNodeToNodeManager(inputIdStr, uuidStr);
    } else {
        insertRes = nodeManager->insertNode(uuidStr, selectNodeId, Direction::LATER);
    }

    if (insertRes != 0) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, TAG, "audioEditTest---AddEffectNodeToNodeManager ERROR!");
        return ret;
    }
    napi_create_int32(env, 0, &ret);
    return ret;
}
static napi_value stopNoiseReduction(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    unsigned int fd = -1;
    // Convert the incoming file descriptor into a C-side variable.
    napi_get_value_uint32(env, argv[0], &fd);
    napi_value sum;
    napi_create_int64(env, 0, &sum);
    return sum;
}

static napi_value deleteNoiseReduction(napi_env env, napi_callback_info info)
{
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest---deleteNoiseReduction IN");
    size_t argc = 1;
    napi_value argv[1] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    // 获取uuidStr
    std::string uuidStr;
    napi_status status = parseNapiString(env, argv[0], uuidStr);
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest---uuid==%{public}s", uuidStr.c_str());

    OH_AudioSuite_Result result;
    napi_value napiValue = nullptr;
    result = nodeManager->removeNode(uuidStr);
    if (result != AUDIOSUITE_SUCCESS) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, TAG, "audioEditTest---removeNode ERROR---%{public}zd", result);
    }
    napi_create_int64(env, result, &napiValue);
    return napiValue;
}

OH_AudioSuite_Result OH_AudioEditEngine_SetSoundFiledType(OH_AudioNode *audioNode, OH_SoundFieldType soundFieldType)
{
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest---OH_AudioEditEngine_SetSoundFiledType---IN");
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest---soundFieldType==%{public}zd", soundFieldType);
    return AUDIOSUITE_SUCCESS;
}

static napi_value startVBEffect(napi_env env, napi_callback_info info)
{
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest---startVBEffect---IN");
    size_t argc = 4;
    napi_value argv[4] = {nullptr, nullptr, nullptr, nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);

    // inputId
    std::string inputId;
    napi_status status = parseNapiString(env, argv[ARG_1], inputId);
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest---startVBEffect---inputId==%{public}s",
        inputId.c_str());

    // 获取二参、美化类型
    unsigned int mode = -1;
    napi_get_value_uint32(env, argv[ARG_2], &mode);
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest---startVBEffect--mode==%{public}zd", mode);

    // 获取三参、效果节点id
    std::string voiceBeautifierId;
    status = parseNapiString(env, argv[ARG_3], voiceBeautifierId);
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest---uuid==%{public}s", voiceBeautifierId.c_str());

    // 获取当前选中的节点id
    std::string selectNodeId;
    status = parseNapiString(env, argv[ARG_4], selectNodeId);
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest---startVBEffect---selectNodeId==%{public}s",
        selectNodeId.c_str());

    OH_VoiceBeautifierType type;
    switch (mode) {
        case OH_VoiceBeautifierType::VOICE_BEAUTIFIER_TYPE_CLEAR:
            type = OH_VoiceBeautifierType::VOICE_BEAUTIFIER_TYPE_CLEAR;
            break;
        case OH_VoiceBeautifierType::VOICE_BEAUTIFIER_TYPE_THEATRE:
            type = OH_VoiceBeautifierType::VOICE_BEAUTIFIER_TYPE_THEATRE;
            break;
        case OH_VoiceBeautifierType::VOICE_BEAUTIFIER_TYPE_CD:
            type = OH_VoiceBeautifierType::VOICE_BEAUTIFIER_TYPE_CD;
            break;
        case OH_VoiceBeautifierType::VOICE_BEAUTIFIER_TYPE_RECORDING_STUDIO:
            type = OH_VoiceBeautifierType::VOICE_BEAUTIFIER_TYPE_RECORDING_STUDIO;
            break;
        default:
            type = OH_VoiceBeautifierType::VOICE_BEAUTIFIER_TYPE_CLEAR;
            break;
    }
    napi_value ret;
    Node node = createNodeByType(voiceBeautifierId, OH_AudioNode_Type::EFFECT_NODE_TYPE_VOICE_BEAUTIFIER);
    OH_AudioSuite_Result result = OH_AudioSuiteEngine_SetVoiceBeautifierType(node.physicalNode, type);
    if (result != OH_AudioSuite_Result::AUDIOSUITE_SUCCESS) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, TAG,
            "audioEditTest---startVBEffect OH_AudioSuiteEngine_SetVoiceBeautifierType ERROR");
        napi_create_int64(env, result, &ret);
        return ret;
    }
    int res = -1;
    if (selectNodeId.empty()) {
        res = AddEffectNodeToNodeManager(inputId, voiceBeautifierId);
    } else {
        res = nodeManager->insertNode(voiceBeautifierId, selectNodeId, Direction::LATER);
    }
    if (res != 0) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, TAG,
            "audioEditTest---startVBEffect AddEffectNodeToNodeManager ERROR!");
        napi_create_int64(env, res, &ret);
        return ret;
    }

    napi_create_int64(env, result, &ret);
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest---startVBEffect: operation success");
    return ret;
}
static napi_value resetVBEffect(napi_env env, napi_callback_info info)
{
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest---resetVBEffect---IN");
    size_t argc = 3;
    napi_value argv[3] = {nullptr, nullptr, nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);

    // 获取inputId
    std::string inputId;
    napi_status status = parseNapiString(env, argv[ARG_1], inputId);
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest---resetVBEffect---inputId==%{public}s",
        inputId.c_str());

    // 获取二参
    unsigned int mode = -1;
    napi_get_value_uint32(env, argv[ARG_2], &mode);
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest---resetVBEffect--mode==%{public}zd", mode);

    // 获取三参
    std::string voiceBeautifierId;
    status = parseNapiString(env, argv[ARG_3], voiceBeautifierId);
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest---uuid==%{public}s", voiceBeautifierId.c_str());

    OH_VoiceBeautifierType type;
    switch (mode) {
        case OH_VoiceBeautifierType::VOICE_BEAUTIFIER_TYPE_CLEAR:
            type = OH_VoiceBeautifierType::VOICE_BEAUTIFIER_TYPE_CLEAR;
            break;
        case OH_VoiceBeautifierType::VOICE_BEAUTIFIER_TYPE_THEATRE:
            type = OH_VoiceBeautifierType::VOICE_BEAUTIFIER_TYPE_THEATRE;
            break;
        case OH_VoiceBeautifierType::VOICE_BEAUTIFIER_TYPE_CD:
            type = OH_VoiceBeautifierType::VOICE_BEAUTIFIER_TYPE_CD;
            break;
        case OH_VoiceBeautifierType::VOICE_BEAUTIFIER_TYPE_RECORDING_STUDIO:
            type = OH_VoiceBeautifierType::VOICE_BEAUTIFIER_TYPE_RECORDING_STUDIO;
            break;
        default:
            type = OH_VoiceBeautifierType::VOICE_BEAUTIFIER_TYPE_CLEAR;
            break;
    }

    napi_value ret;
    Node node = nodeManager->getNodeById(voiceBeautifierId);
    OH_AudioSuite_Result result = OH_AudioSuiteEngine_SetVoiceBeautifierType(node.physicalNode, type);
    if (result != AUDIOSUITE_SUCCESS) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, TAG,
            "audioEditTest---OH_AudioSuiteEngine_SetVoiceBeautifierType ERROR---%{public}zd", result);
        napi_create_int64(env, result, &ret);
        return ret;
    }

    napi_create_int64(env, result, &ret);
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest---resetVBEffect: operation success");
    return ret;
}
OH_SoundFieldType getSoundFieldTypeByNum(int mode)
{
    OH_SoundFieldType type;
    switch (mode) {
        case OH_SoundFieldType::SOUND_FIELD_FRONT_FACING:
            type = OH_SoundFieldType::SOUND_FIELD_FRONT_FACING;
            break;
        case OH_SoundFieldType::SOUND_FIELD_GRAND:
            type = OH_SoundFieldType::SOUND_FIELD_GRAND;
            break;
        case OH_SoundFieldType::SOUND_FIELD_NEAR:
            type = OH_SoundFieldType::SOUND_FIELD_NEAR;
            break;
        case OH_SoundFieldType::SOUND_FIELD_WIDE:
            type = OH_SoundFieldType::SOUND_FIELD_WIDE;
            break;
        default:
            type = OH_SoundFieldType::SOUND_FIELD_FRONT_FACING;
            break;
    }
    return type;
}
static napi_value startFieldEffect(napi_env env, napi_callback_info info)
{
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest---startFieldEffect start");
    size_t argc = 4;
    napi_value argv[4] = {nullptr, nullptr, nullptr, nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);

    std::string inputId;
    napi_status status = parseNapiString(env, argv[ARG_1], inputId);
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest startFieldEffect inputId is %{public}s",
        inputId.c_str());
    
    // 获取二参
    unsigned int mode = -1;
    napi_get_value_uint32(env, argv[ARG_2], &mode);
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest startFieldEffect mode is %{public}zd", mode);

    // 获取三参
    std::string fieldEffectId;
    status = parseNapiString(env, argv[ARG_3], fieldEffectId);
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest startFieldEffect fieldEffectId is %{public}s",
        fieldEffectId.c_str());

    // 获取四参
    std::string selectedNodeId;
    status = parseNapiString(env, argv[ARG_4], selectedNodeId);
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest startFieldEffect selectedNodeId is %{public}s",
        selectedNodeId.c_str());
    OH_SoundFieldType type = getSoundFieldTypeByNum(mode);
    napi_value ret;
    Node node = createNodeByType(fieldEffectId, OH_AudioNode_Type::EFFECT_NODE_TYPE_SOUND_FIELD);
    OH_AudioSuite_Result result = OH_AudioSuiteEngine_SetSoundFiledType(node.physicalNode, type);
    if (result != OH_AudioSuite_Result::AUDIOSUITE_SUCCESS) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, TAG,
            "audioEditTest startFieldEffect OH_AudioEditEngine_SetSoundFiledType ERROR!");
        napi_create_int64(env, result, &ret);
        return ret;
    }
    if (selectedNodeId.empty()) {
        int res = AddEffectNodeToNodeManager(inputId, fieldEffectId);
        if (res != 0) {
            OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, TAG,
                "audioEditTest startFieldEffect AddEffectNodeToNodeManager ERROR!");
            napi_create_int64(env, res, &ret);
            return ret;
        }
    } else {
        result = nodeManager->insertNode(fieldEffectId, selectedNodeId, Direction::LATER);
        if (result != OH_AudioSuite_Result::AUDIOSUITE_SUCCESS) {
            OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, TAG, "audioEditTest startFieldEffect insertNode ERROR!");
            napi_create_int64(env, result, &ret);
            return ret;
        }
    }

    napi_create_int64(env, result, &ret);
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest startFieldEffect: operation success");
    return ret;
}

static napi_value resetFieldEffect(napi_env env, napi_callback_info info)
{
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest resetFieldEffect start");
    size_t argc = 3;
    napi_value argv[3] = {nullptr, nullptr, nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);

    std::string inputId;
    napi_status status = parseNapiString(env, argv[ARG_1], inputId);
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest resetFieldEffect inputId is %{public}s",
        inputId.c_str());

    // 获取二参
    unsigned int mode = -1;
    napi_get_value_uint32(env, argv[ARG_2], &mode);
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest resetFieldEffect mode is %{public}zd", mode);

    // 获取三参
    std::string fieldEffectId;
    status = parseNapiString(env, argv[ARG_3], fieldEffectId);
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest fieldEffectId is %{public}s",
        fieldEffectId.c_str());

    OH_SoundFieldType type = getSoundFieldTypeByNum(mode);

    napi_value ret;
    Node node = nodeManager->getNodeById(fieldEffectId);
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest get node is %{public}s", node.id.c_str());
    OH_AudioSuite_Result result = OH_AudioSuiteEngine_SetSoundFiledType(node.physicalNode, type);
    if (result != AUDIOSUITE_SUCCESS) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, TAG,
            "audioEditTest OH_AudioSuiteEngine_SetSoundFiledType ERROR %{public}zd", result);
        napi_create_int64(env, result, &ret);
        return ret;
    }

    napi_create_int64(env, result, &ret);
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest resetFieldEffect: operation success");
    return ret;
}
void OnReadTapDataCallback(OH_AudioNode *audioNode, void *userData, void *audioData, int32_t audioDataSize)
{
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest---OnReadTapDataCallback---IN");
    // 检查audioNode参数，底层接口问题
    if (audioNode == nullptr) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, TAG,
            "audioEditTest OnReadTapDataCallback audioNode is nullptr");
        return;
    }
    // audioData，底层接口问题
    if (audioData == nullptr) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, TAG,
            "audioEditTest OnReadTapDataCallback audioData is nullptr");
        return;
    }
    // 处理音频数据  此处如果是nullptr，是demo获取音频数据的问题，非底层接口问题
    if (audioDataSize == 0) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, TAG, "audioEditTest OnReadTapDataCallback audioDataSize is 0");
        return;
    }

    auto ret = memcpy_s(static_cast<char *>(g_aissTapAudioData) + g_tapDataTotalSize,
        audioDataSize, audioData, audioDataSize);
    if (ret != 0) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, TAG,
            "audioEditTest memcpy_s g_aissTapAudioData failed, ret is %{public}zd", ret);
    }
    g_tapDataTotalSize += audioDataSize;
}

static napi_value addAudioSeparation(napi_env env, napi_callback_info info)
{
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest---addAudioSeparation---IN");
    size_t argc = 4;
    napi_value *argv = new napi_value[argc];
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    // 获取一参
    unsigned int arg1 = 0;
    napi_get_value_uint32(env, argv[ARG_1], &arg1);
    // 获取二参uuid
    std::string uuidStr;
    napi_status status = parseNapiString(env, argv[ARG_2], uuidStr);
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest---uuid==%{public}s", uuidStr.c_str());

    // 获取三参inputId
    std::string inputIdStr;
    status = parseNapiString(env, argv[ARG_3], inputIdStr);
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest---inputId==%{public}s", inputIdStr.c_str());

    // 获取四参
    std::string selectedNodeId;
    status = parseNapiString(env, argv[ARG_4], selectedNodeId);
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest addAudioSeparation selectedNodeId is %{public}s",
        selectedNodeId.c_str());

    napi_value ret;
    napi_create_int64(env, ARG_4, &ret);
    Node node = createNodeByType(uuidStr, OH_AudioNode_Type::EFFECT_MULTII_OUTPUT_NODE_TYPE_AUDIO_SEPARATION);
    if (node.physicalNode == nullptr) {
        return ret;
    }
    if (selectedNodeId.empty()) {
        int insertRes = AddEffectNodeToNodeManager(inputIdStr, uuidStr);
        if (insertRes == -1) {
            OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, TAG, "audioEditTest---AddEffectNodeToNodeManager ERROR!");
            return ret;
        }
    } else {
        OH_AudioSuite_Result result = nodeManager->insertNode(uuidStr, selectedNodeId, Direction::LATER);
        if (result != OH_AudioSuite_Result::AUDIOSUITE_SUCCESS) {
            OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, TAG,
                "audioEditTest addAudioSeparation insertNode ERROR %{public}u", result);
        }
    }

    g_multiRenderFrameFlag = true;
    napi_create_int64(env, 0, &ret);
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest---addAudioSeparation: operation success");
    return ret;
}
void getEnvEnumByNumber(int num, OH_EnvironmentType &type)
{
    switch (num) {
        case ENVIRONMENT_TYPE_BROADCAST:
            type = ENVIRONMENT_TYPE_BROADCAST;
            break;
        case ENVIRONMENT_TYPE_EARPIECE:
            type = ENVIRONMENT_TYPE_EARPIECE;
            break;
        case ENVIRONMENT_TYPE_UNDERWATER:
            type = ENVIRONMENT_TYPE_UNDERWATER;
            break;
        case ENVIRONMENT_TYPE_GRAMOPHONE:
            type = ENVIRONMENT_TYPE_GRAMOPHONE;
            break;
        default:
            type = ENVIRONMENT_TYPE_BROADCAST;
            break;
    }
}
static napi_value startEnvEffect(napi_env env, napi_callback_info info)
{
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest---startEnvEffect---IN");
    size_t argc = 4;
    napi_value *argv = new napi_value[argc];
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    napi_status status;
    // 获取一参
    std::string inputIdStr;
    status = parseNapiString(env, argv[ARG_1], inputIdStr);
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest---inputId==%{public}s", inputIdStr.c_str());

    // 获取二参uuid
    std::string uuidStr;
    status = parseNapiString(env, argv[ARG_2], uuidStr);
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest---uuid==%{public}s", uuidStr.c_str());

    // 获取三参
    unsigned int mode = 0;
    napi_get_value_uint32(env, argv[ARG_3], &mode);

    // 获取四参混音台selectedNodeId
    std::string selectedNodeId;
    status = parseNapiString(env, argv[ARG_4], selectedNodeId);
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest selectedNodeId is %{public}s",
        selectedNodeId.c_str());

    OH_EnvironmentType type;
    getEnvEnumByNumber(mode, type);
    napi_value ret;
    Node node = createNodeByType(uuidStr, OH_AudioNode_Type::EFFECT_NODE_TYPE_ENVIRONMENT_EFFECT);
    if (node.physicalNode == nullptr) {
        napi_create_int64(env, ARG_4, &ret);
        return ret;
    }
    OH_AudioSuite_Result result = OH_AudioSuiteEngine_SetEnvironmentType(node.physicalNode, type);
    if (result != AUDIOSUITE_SUCCESS) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, TAG,
            "audioEditTest---OH_AudioSuiteEngine_SetEnvironmentType ERROR---%{public}zd", result);
        napi_create_int64(env, result, &ret);
        return ret;
    }

    if (selectedNodeId.empty()) {
        int insertRes = AddEffectNodeToNodeManager(inputIdStr, uuidStr);
        if (insertRes == -1) {
            OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, TAG, "audioEditTest---AddEffectNodeToNodeManager ERROR!");
            napi_create_int64(env, insertRes, &ret);
            return ret;
        }
    } else {
        result = nodeManager->insertNode(uuidStr, selectedNodeId, Direction::LATER);
        if (result != OH_AudioSuite_Result::AUDIOSUITE_SUCCESS) {
            OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, TAG, "audioEditTest startEnvEffect insertNode ERROR!");
            napi_create_int64(env, result, &ret);
            return ret;
        }
    }

    napi_create_int64(env, 0, &ret);
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest---startEnvEffect: operation success");
    return ret;
}

static napi_value resetEnvEffect(napi_env env, napi_callback_info info)
{
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest---resetEnvEffect---IN");
    size_t argc = 3;
    napi_value *argv = new napi_value[argc];
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    // 获取一参
    std::string inputIdStr;
    parseNapiString(env, argv[ARG_1], inputIdStr);
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest---inputId==%{public}s", inputIdStr.c_str());

    // 获取二参uuid
    std::string effectNodeId;
    parseNapiString(env, argv[ARG_2], effectNodeId);
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest---uuid==%{public}s", effectNodeId.c_str());

    // 获取三参
    unsigned int mode = 0;
    napi_get_value_uint32(env, argv[ARG_3], &mode);
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest---mode==%{public}d", mode);

    OH_EnvironmentType type;
    getEnvEnumByNumber(mode, type);
    napi_value ret;
    Node node = nodeManager->getNodeById(effectNodeId);
    OH_AudioSuite_Result result = OH_AudioSuiteEngine_SetEnvironmentType(node.physicalNode, type);
    if (result != AUDIOSUITE_SUCCESS) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, TAG,
            "audioEditTest---OH_AudioSuiteEngine_SetEnvironmentType ERROR==%{public}zd", result);
        napi_create_int64(env, result, &ret);
        return ret;
    }
    napi_create_int64(env, 0, &ret);
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest---resetEnvEffect: operation success");
    return ret;
}

int32_t CheckFilePath(std::string &filePath)
{
    if (filePath.size() >= PATH_MAX) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, TAG,
            "audioEditTest---File path size is too large---%{public}zd", filePath.size());
        return 1;
    }
    char buffer[PATH_MAX] = {0};
    char *path = realpath(filePath.c_str(), buffer);
    if (path == nullptr) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, TAG, "audioEditTest---Invalid file path---%{public}s",
            filePath.c_str());
        return 1;
    }
    filePath = buffer;
    return 0;
}

static napi_value compareTwoFilesBinary(napi_env env, napi_callback_info info)
{
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest---compareTwoFilesBinary---IN");
    napi_value ret;
    napi_create_int32(env, 1, &ret);
    size_t argc = 2;
    napi_value argv[2] = {nullptr, nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);

    // 获取inputFilePath1
    std::string inputFilePath1;
    parseNapiString(env, argv[0], inputFilePath1);
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest---inputFilePath1==%{public}s",
        inputFilePath1.c_str());
    // 获取inputFilePath2;
    std::string inputFilePath2;
    parseNapiString(env, argv[1], inputFilePath2);
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest---inputFilePath2==%{public}s",
        inputFilePath2.c_str());
    
    char *downloadPath = nullptr;
    FileManagement_ErrCode status =
        OH_FileUri_GetUriFromPath(inputFilePath2.c_str(), sizeof(inputFilePath2), &downloadPath);
    if (status ==0) {
        printf("Download Path=%s", downloadPath);
    } else {
        printf("GetDownloadPath failed, error code is %d", status);
    }

    std::ifstream file1(inputFilePath1, std::ios::binary);
    std::ifstream file2("/data/storage/el2/100/base/haps/entry/files/test.txt", std::ios::binary);

    if (!file1.is_open() || !file2.is_open()) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, TAG, "audioEditTest---open file failed");
        return ret;
    }

    file1.seekg(0, std::ios::end);
    file2.seekg(0, std::ios::end);

    if (file1.tellg() != file2.tellg()) {
        OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest---file length is not equal");
        return ret;
    }

    file1.seekg(0, std::ios::beg);
    file2.seekg(0, std::ios::beg);

    const size_t bufferSize = 4096; // 4KB 缓冲区
    char buffer1[bufferSize];
    char buffer2[bufferSize];

    while (file1.good() && file2.good()) {
        file1.read(buffer1, bufferSize);
        file2.read(buffer2, bufferSize);

        if (file1.gcount() != file2.gcount() ||
            std::memcmp(buffer1, buffer2, static_cast<size_t>(file1.gcount())) != 0) {
                OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest---files binary is not equal");
                return ret;
        }
    }

    if (file1.bad() || file2.bad()) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, TAG, "audioEditTest---file read error");
        return ret;
    }

    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest---files binary is equal");
    napi_create_int32(env, 0, &ret);
    return ret;
}

static napi_value resetAudioSeparation(napi_env env, napi_callback_info info)
{
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest---resetAudioSeparation---IN");
    size_t argc = 2; 
    napi_value argv[2] = {nullptr, nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    // 获取一参分离mode
    unsigned int arg1 = 0;
    napi_get_value_uint32(env, argv[0], &arg1);
    // 获取二参aissNodeId
    std::string aissNodeId;
    napi_status status = parseNapiString(env, argv[1], aissNodeId);
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest---uuid==%{public}s, size==%{public}zd",
        aissNodeId.c_str(), aissNodeId.size());

    napi_value ret;
    Node node = nodeManager->getNodeById(aissNodeId);

    OH_AudioSuite_Result result;

    result = nodeManager->disconnect(aissNodeId, node.nextNodeId);
    if (result != AUDIOSUITE_SUCCESS) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, TAG,
            "audioEditTest---resetAudioSeparation: disconnect ERROR---%{public}u", result);
        napi_create_int32(env, result, &ret);
        return ret;
    }
    result = nodeManager->connectByPort(aissNodeId, node.nextNodeId);
    if (result != AUDIOSUITE_SUCCESS) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, TAG,
            "audioEditTest---resetAudioSeparation: connectByPort ERROR---%{public}u", result);
        napi_create_int32(env, result, &ret);
        return ret;
    }

    napi_create_int32(env, 0, &ret);
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest---resetAudioSeparation: operation success");
    return ret;
}

static napi_value deleteAudioSeparation(napi_env env, napi_callback_info info)
{
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest---deleteAudioSeparation IN");
    size_t argc = 1;
    napi_value argv[1] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    // 获取uuid
    std::string uuidStr;
    napi_status status = parseNapiString(env, argv[0], uuidStr);
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest---uuid==%{public}s", uuidStr.c_str());

    OH_AudioSuite_Result result;
    napi_value napiValue = nullptr;
    result = nodeManager->removeNode(uuidStr);
    if (result != AUDIOSUITE_SUCCESS) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, TAG, "audioEditTest---removeNode ERROR---%{public}zd", result);
    }
    napi_create_int64(env, result, &napiValue);
    return napiValue;
}
static napi_value getAudioOfTap(napi_env env, napi_callback_info info)
{
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest---getAudioOfTap---IN");

    napi_value napiValue = nullptr;
    void *data;
    napi_create_arraybuffer(env, g_tapDataTotalSize, &data, &napiValue);
    auto result = memcpy_s(data, g_tapDataTotalSize, g_aissTapAudioData, g_tapDataTotalSize);
    if (result != 0) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, TAG,
            "audioEditTest---getAudioOfTap memcpy data ERROR---%{public}zd", result);
    }
    result = memset_s(g_aissTapAudioData, g_tapDataTotalSize, 0, g_tapDataTotalSize);
    if (result != 0) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, TAG,
            "audioEditTest---getAudioOfTap memset g_aissTapAudioData ERROR---%{public}zd", result);
    }
    g_tapDataTotalSize = 0;
    return napiValue;
}

// 音频播放 -------------------------------------
static OH_AudioRenderer *audioRenderer;
static OH_AudioStreamBuilder *rendererBuilder;
// 实时播放， 用于保存音频数据，具体大小根据需要保存的文件大小而变化
char *g_mixTotalAudioData = (char *)malloc(1024 * 1024 * 100);
// 实时播放需要保存的音频总大小
int32_t g_mixResultTotalSize = 0;
// 待播放的数据大小
int32_t g_mixDataSize = 0;
bool g_oneFinishedFlag = false;
char *g_audioData = (char *)malloc(g_mixDataSize * 5);
auto lastCallbackTime = std::chrono::steady_clock::now();
// 是否录制
bool g_isRecord = false;
static napi_ref callbackAudioRendererRef = nullptr;
static napi_threadsafe_function tsfnBoolean = nullptr;
// 线程安全函数的调用
static void CallBoolThread(napi_env env, napi_value js_callback, void *context, void *data)
{
    int result = *(bool *)data;
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest CallBoolThread result: %{public}d", result);
    napi_value resultValue;
    napi_get_boolean(env, result, &resultValue);
    napi_call_function(env, NULL, js_callback, 1, &resultValue, NULL);
    free(data);
}
// 注册回调，获取音频播放的finished的值
static napi_value RegisterFinishedCallback(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value args[1];
    napi_value callback;
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    // 创建全局引用
    napi_create_reference(env, args[0], 1, &callbackAudioRendererRef);

    // 创建线程安全函数
    napi_value global;
    napi_get_global(env, &global);
    napi_get_reference_value(env, callbackAudioRendererRef, &callback);
    napi_value name;
    napi_status status = napi_create_string_utf8(env, "CallBooleanCallback", NAPI_AUTO_LENGTH, &name);
    napi_create_threadsafe_function(env, callback, NULL, name, 1, 1, NULL, NULL, NULL, CallBoolThread, &tsfnBoolean);

    napi_value result;
    napi_get_undefined(env, &result);
    return result;
}

// 调用回调函数的方法
void CallBooleanCallback(int result)
{
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest CallBooleanCallback result: %{public}d", result);
    if (tsfnBoolean == nullptr) {
        return;
    }

    int *data = (int *)malloc(sizeof(int));
    if (data == nullptr) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, TAG, "Failed to allocate memory for data");
        return;
    }
    *data = result;

    napi_call_threadsafe_function(tsfnBoolean, data, napi_tsfn_blocking);
}

static napi_value Record(napi_env env, napi_callback_info info)
{
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest Record start");
    g_isRecord = true;
    return nullptr;
}

static OH_AudioSuite_Result ProcessPipeline()
{
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest ProcessPipeline start");
    // 获取管线状态
    OH_AudioSuite_PipelineState pipeLineState;
    OH_AudioSuite_Result result = OH_AudioSuiteEngine_GetPipelineState(audioSuitePipeline, &pipeLineState);
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG,
        "audioEditTest OH_AudioSuiteEngine_GetPipelineState result: %{public}d --- pipeLineState: %{public}d",
        static_cast<int>(result), static_cast<int>(pipeLineState));
    if (result != OH_AudioSuite_Result::AUDIOSUITE_SUCCESS) {
        return result;
    }

    // 启动管线
    if (pipeLineState != OH_AudioSuite_PipelineState::AUDIOSUITE_PIPELINE_RUNNING) {
        result = OH_AudioSuiteEngine_StartPipeline(audioSuitePipeline);
        OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG,
            "audioEditTest OH_AudioSuiteEngine_StartPipeline result: %{public}d", static_cast<int>(result));
        if (result != OH_AudioSuite_Result::AUDIOSUITE_SUCCESS) {
            return result;
        }
    }
    return result;
}

static OH_AudioSuite_Result OneRenDerFrame(int32_t audioDataSize)
{
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest OneRenDerFrame start");
    ProcessPipeline();
    if (audioDataSize <= 0) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, TAG,
            "audioEditTest OH_AudioSuiteEngine_RenderFrame audioDataSize is %{public}d",
            static_cast<int>(audioDataSize));
        return OH_AudioSuite_Result::AUDIOSUITE_ERROR_SYSTEM;
    }
    char *audioData = (char *)malloc(audioDataSize);
    int32_t writeSize = 0;
    OH_AudioSuite_Result result =
        OH_AudioSuiteEngine_RenderFrame(audioSuitePipeline, audioData, audioDataSize, &writeSize, &g_oneFinishedFlag);
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG,
        "audioEditTest OH_AudioSuiteEngine_RenderFrame audioDataSize: %{public}d, writeSize:%{public}d "
        "g_oneFinishedFlag : %{public}s, result: %{public}d",
        audioDataSize, writeSize, (g_oneFinishedFlag ? "true" : "false"), static_cast<int>(result));
    if (result != OH_AudioSuite_Result::AUDIOSUITE_SUCCESS) {
        OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG,
            "audioEditTest OH_AudioSuiteEngine_RenderFrame result is %{public}d", static_cast<int>(result));
    }
    if (writeSize <= 0) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, TAG,
            "audioEditTest OH_AudioSuiteEngine_RenderFrame writeSize is %{public}d", static_cast<int>(writeSize));
        return OH_AudioSuite_Result::AUDIOSUITE_ERROR_SYSTEM;
    }
    // 每次保存一次获取的buffer值
    g_audioData = (char *)malloc(writeSize);
    auto ret = memcpy_s(static_cast<char *>(g_audioData), writeSize, audioData, writeSize);
    if (ret != 0) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, TAG,
            "audioEditTest memcpy g_audioData failed, ret is %{public}zd", ret);
    }
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG,
        "audioEditTest OH_AudioSuiteEngine_RenderFrame writeSize : %{public}d, g_oneFinishedFlag: %{public}s",
        writeSize, (g_oneFinishedFlag ? "true" : "false"));
    return result;
}

static napi_value RealTimeSaveFileBuffer(napi_env env, napi_callback_info info)
{
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest RealTimeSaveFileBuffer start");
    ResetAllIsResetTotalWriteAudioDataSize();
    Clear();
    g_isRecord = false;
    napi_value napiValue = nullptr;
    void *arrayBufferData = nullptr;
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG,
        "audioEditTest RealTimeSaveFileBuffer g_mixResultTotalSize is %{public}d", g_mixResultTotalSize);
    napi_status status = napi_create_arraybuffer(env, g_mixResultTotalSize, &arrayBufferData, &napiValue);
    if (status != napi_ok || arrayBufferData == nullptr) {
        OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest napi_create_arraybuffer status: %{public}d",
            static_cast<int>(status));
        g_mixResultTotalSize = 0;
        if (g_mixTotalAudioData != nullptr) {
            free(g_mixTotalAudioData);
            g_mixTotalAudioData = nullptr;
        }
        // 创建 ArrayBuffer 失败，返回一个大小为 0 的ArrayBuffer
        napi_create_arraybuffer(env, 0, &arrayBufferData, &napiValue);
        return napiValue;
    } else {
        auto ret = memcpy_s(arrayBufferData, g_mixResultTotalSize, g_mixTotalAudioData, g_mixResultTotalSize);
        if (ret != 0) {
            OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, TAG,
                "audioEditTest memcpy_s arrayBufferData failed, ret is %{public}zd", ret);
        }
        if (g_mixTotalAudioData != nullptr) {
            free(g_mixTotalAudioData);
            g_mixTotalAudioData = nullptr;
        }
        g_mixResultTotalSize = 0;
        return napiValue;
    }
}

static OH_AudioData_Callback_Result NewAudioRendererOnWriteData(OH_AudioRenderer * renderer, void *userData,
                                                                void *audioData, int32_t audioDataSize)
                                                                {
    if (renderer == nullptr) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, TAG,
            "audioEditTest NewAudioRendererOnWriteData renderer is nullptr");
        return AUDIO_DATA_CALLBACK_RESULT_INVALID;
    }
    if (audioData == nullptr) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, TAG,
            "audioEditTest NewAudioRendererOnWriteData audioData is nullptr");
        return AUDIO_DATA_CALLBACK_RESULT_INVALID;
    }
    
    if (!g_oneFinishedFlag) {
        OneRenDerFrame(audioDataSize);
        OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, TAG, "g_isRecord: %{public}s", g_isRecord ? "true" : "false");
        // 每次保存一次获取的buffer值
        if (audioDataSize != 0 && g_isRecord == true) {
            auto ret = memcpy_s(static_cast<char *>(g_mixTotalAudioData) + g_mixResultTotalSize,
                audioDataSize, g_audioData, audioDataSize);
            if (ret != 0) {
                OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, TAG,
                    "audioEditTest memcpy_s g_mixTotalAudioData failed, ret is %{public}zd", ret);
            }
            g_mixResultTotalSize += audioDataSize;
        }
    }
    // 播放音频数据
    if (g_audioData != nullptr) {
        auto ret = memcpy_s(static_cast<char *>(audioData), audioDataSize, g_audioData, audioDataSize);
        if (ret != 0) {
            OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, TAG,
                "audioEditTest memcpy_s audioData failed, ret is %{public}zd", ret);
        }
    }
    if (g_oneFinishedFlag) {
        OH_AudioRenderer_Stop(audioRenderer);
        ResetAllIsResetTotalWriteAudioDataSize();
        OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, TAG,
            "audioEditTest NewAudioRendererOnWriteData g_mixResultTotalSize is %{public}d",
            g_mixResultTotalSize);
        CallBooleanCallback(g_oneFinishedFlag);
        g_oneFinishedFlag = false;
        if (g_totalBuff != nullptr) {
            free(g_totalBuff);
            g_totalBuff = nullptr;
        }
        g_writeIndex = 0;
    }
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG,
        "audioEditTest NewAudioRendererOnWriteData g_mixResultTotalSize: %{public}d, audioDataSize: %{public}d",
        g_mixResultTotalSize, audioDataSize);
    return AUDIO_DATA_CALLBACK_RESULT_VALID;
}

static napi_value AudioRendererInit(napi_env env, napi_callback_info info)
{
    if (audioRenderer) {
        // 释放播放实例
        OH_AudioRenderer_Release(audioRenderer);
        // 释放构造器
        OH_AudioStreamBuilder_Destroy(rendererBuilder);

        audioRenderer = nullptr;
        rendererBuilder = nullptr;
    }
    // create builder
    OH_AudioStream_Type type = OH_AudioStream_Type::AUDIOSTREAM_TYPE_RENDERER;
    OH_AudioStreamBuilder_Create(&rendererBuilder, type);

    // 获取位深
    int32_t bitsPerSample = 0;
    OH_AudioStream_SampleFormat streamSampleFormat;
    if (audioFormatOutput.sampleFormat == OH_Audio_SampleFormat::AUDIO_SAMPLE_U8) {
        bitsPerSample = static_cast<int>(SampleFormat::AUDIO_SAMPLE_U8);
        streamSampleFormat = OH_AudioStream_SampleFormat::AUDIOSTREAM_SAMPLE_U8;
    } else if (audioFormatOutput.sampleFormat == OH_Audio_SampleFormat::AUDIO_SAMPLE_S16LE) {
        bitsPerSample = static_cast<int>(SampleFormat::AUDIO_SAMPLE_S16LE);
        streamSampleFormat = OH_AudioStream_SampleFormat::AUDIOSTREAM_SAMPLE_S16LE;
    } else if (audioFormatOutput.sampleFormat == OH_Audio_SampleFormat::AUDIO_SAMPLE_S24LE) {
        bitsPerSample = static_cast<int>(SampleFormat::AUDIO_SAMPLE_S24LE);
        streamSampleFormat = OH_AudioStream_SampleFormat::AUDIOSTREAM_SAMPLE_S24LE;
    }  else {
        bitsPerSample = static_cast<int>(SampleFormat::AUDIO_SAMPLE_F32LE);
        streamSampleFormat = OH_AudioStream_SampleFormat::AUDIOSTREAM_SAMPLE_F32LE;
    }

    // 设置音频采样率。
    OH_AudioStreamBuilder_SetSamplingRate(rendererBuilder, audioFormatOutput.samplingRate);
    // 设置音频声道。
    OH_AudioStreamBuilder_SetChannelCount(rendererBuilder, audioFormatOutput.channelCount);
    // 设置音频采样格式。
    OH_AudioStreamBuilder_SetSampleFormat(rendererBuilder, streamSampleFormat);
    // 设置音频流的编码类型。
    OH_AudioStreamBuilder_SetEncodingType(rendererBuilder, AUDIOSTREAM_ENCODING_TYPE_RAW);
    // 设置输出音频流的工作场景。
    OH_AudioStreamBuilder_SetRendererInfo(rendererBuilder, AUDIOSTREAM_USAGE_MUSIC);
    // 设置 audioDataSize 长度 （待播放的数据大小）
    g_mixDataSize = SAMPLINGRATE_MULTI * audioFormatOutput.samplingRate *
        audioFormatOutput.channelCount / CHANNELCOUNT_MULTI * bitsPerSample / BITSPERSAMPLE_MULTI;
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG,
        "audioEditTest AudioRendererInit g_mixDataSize: %{public}d, samplingRate: %{public}d, "
        "channelCount: %{public}d, bitsPerSample: %{public}d",
        g_mixDataSize, audioFormatOutput.samplingRate, audioFormatOutput.channelCount, bitsPerSample);
    OH_AudioStreamBuilder_SetFrameSizeInCallback(rendererBuilder, g_mixDataSize);

    // 配置写入音频数据回调函数。
    OH_AudioRenderer_OnWriteDataCallback rendererCallbacks = NewAudioRendererOnWriteData;
    OH_AudioStreamBuilder_SetRendererWriteDataCallback(rendererBuilder, rendererCallbacks, nullptr);

    // create OH_AudioRenderer
    OH_AudioStreamBuilder_GenerateRenderer(rendererBuilder, &audioRenderer);
    return nullptr;
}

static napi_value AudioRendererDestory(napi_env env, napi_callback_info info)
{
    napi_value napiValue = nullptr;
    if (audioRenderer) {
        // 释放播放实例
        OH_AudioStream_Result result = OH_AudioRenderer_Release(audioRenderer);
        // 释放构造器
        result = OH_AudioStreamBuilder_Destroy(rendererBuilder);
        napi_create_int64(env, static_cast<int>(result), &napiValue);
        audioRenderer = nullptr;
        rendererBuilder = nullptr;
    }
    return napiValue;
}

// 开始播放
static napi_value AudioRendererStart(napi_env env, napi_callback_info info)
{
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "writeDataBufferMap_ size %{public}d",
        writeDataBufferMap_.size());
    ProcessPipeline();
    // start
    OH_AudioRenderer_Start(audioRenderer);
    return nullptr;
}

// 暂停播放
static napi_value AudioRendererPause(napi_env env, napi_callback_info info)
{
    // pause
    OH_AudioRenderer_Pause(audioRenderer);
    return nullptr;
}

// 停止播放
static napi_value AudioRendererStop(napi_env env, napi_callback_info info)
{
    // stop
    g_isRecord = false;
    OH_AudioRenderer_Stop(audioRenderer);
    return nullptr;
}

// 获取播放状态
static napi_value GetRendererState(napi_env env, napi_callback_info info)
{
    OH_AudioStream_State state;
    OH_AudioRenderer_GetCurrentState(audioRenderer, &state);
    napi_value sum;
    napi_create_int32(env, state, &sum);

    return sum;
}

// 是否重置totalWriteAudioDataSize
static napi_value ResetTotalWriteAudioDataSize(napi_env env, napi_callback_info info)
{
    // 写入音频的buffer，重头开始
    ResetAllIsResetTotalWriteAudioDataSize();
    // 报错音频的也重头开始保存
    g_mixResultTotalSize = 0;
    return nullptr;
}

EXTERN_C_START static napi_value Init(napi_env env, napi_value exports)
{
    napi_property_descriptor desc[] = {
        {"record", nullptr, Record, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"audioRendererInit", nullptr, AudioRendererInit, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"audioRendererDestory", nullptr, AudioRendererDestory, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"audioRendererStart", nullptr, AudioRendererStart, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"audioRendererPause", nullptr, AudioRendererPause, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"audioRendererStop", nullptr, AudioRendererStop, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"getRendererState", nullptr, GetRendererState, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"registerFinishedCallback", nullptr, RegisterFinishedCallback, nullptr, nullptr, nullptr, napi_default,
            nullptr},
        {"resetTotalWriteAudioDataSize", nullptr, ResetTotalWriteAudioDataSize, nullptr, nullptr, nullptr, napi_default,
            nullptr},
        {"realTimeSaveFileBuffer", nullptr, RealTimeSaveFileBuffer, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"audioEditNodeInit", nullptr, AudioEditNodeInit, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"audioInAndOutInit", nullptr, AudioInAndOutInit, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"audioEditDestory", nullptr, AudioEditDestory, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"setFormat", nullptr, SetFormat, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"setEquailizerMode", nullptr, SetEquailizerMode, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"setEqualizerFrequencyBandGains", nullptr, SetEqualizerFrequencyBandGains, nullptr, nullptr, nullptr,
            napi_default, nullptr},
        {"saveFileBuffer", nullptr, SaveFileBuffer, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"startFieldEffect", nullptr, startFieldEffect, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"startVBEffect", nullptr, startVBEffect, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"addAudioSeparation", nullptr, addAudioSeparation, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"addNoiseReduction", nullptr, addNoiseReduction, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"stopNoiseReduction", nullptr, stopNoiseReduction, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"resetFieldEffect", nullptr, resetFieldEffect, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"resetVBEffect", nullptr, resetVBEffect, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"resetAudioSeparation", nullptr, resetAudioSeparation, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"deleteNoiseReduction", nullptr, deleteNoiseReduction, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"deleteSong", nullptr, DeleteSong, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"deleteAudioSeparation", nullptr, deleteAudioSeparation, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"getAudioOfTap", nullptr, getAudioOfTap, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"startEnvEffect", nullptr, startEnvEffect, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"resetEnvEffect", nullptr, resetEnvEffect, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"compareTwoFilesBinary", nullptr, compareTwoFilesBinary, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"deleteNode", nullptr, DeleteNode, nullptr, nullptr, nullptr, napi_default, nullptr}};
    napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc);
    return exports;
}
EXTERN_C_END

static napi_module demoModule = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = Init,
    .nm_modname = "entry",
    .nm_priv = ((void *)0),
    .reserved = {0},
};

extern "C" __attribute__((constructor)) void RegisterEntryModule(void) { napi_module_register(&demoModule); }
