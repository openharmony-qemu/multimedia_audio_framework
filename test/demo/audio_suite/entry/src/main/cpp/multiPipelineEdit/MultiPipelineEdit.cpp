/*
 * Copyright (c) 2025 Huawei Device Co., Ltd. 2025-2025. ALL rights reserved.
 */
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <climits>
#include <future>
#include <ohaudio/native_audiorenderer.h>
#include <ohaudio/native_audiostreambuilder.h>
#include <string>
#include <map>
#include <thread>
#include <algorithm>
#include <unistd.h>
#include "hilog/log.h"
#include <iomanip>
#include <fstream>
#include <fcntl.h>
#include <multiPipelineEdit/MultiPipelineEdit.h>
#include "./callback/RegisterCallback.h"
#include "./audioEffectNode/Output.h"
#include "./audioEffectNode/Input.h"
#include "../callback/RegisterCallback.h"
#include "../audioEffectNode/Output.h"
#include "../audioEffectNode/Input.h"
#include "NodeManager.h"
#include "PipelineManager.h"
#include "audioSuiteError/AudioSuiteError.h"
#include <./utils/Utils.h>
#include "ohaudio/native_audio_suite_base.h"
#include "ohaudio/native_audio_suite_engine.h"
#include <multimedia/player_framework/native_avdemuxer.h>
#include <multimedia/player_framework/native_avsource.h>
#include <multimedia/player_framework/native_avcodec_base.h>
#include <multimedia/player_framework/native_avformat.h>
#include <multimedia/player_framework/native_avbuffer.h>

const int GLOBAL_RESMGR = 0xFF00;
const int FIRST_ARGV_PARAM = 0;
const int SECOND_ARGV_PARAM = 1;
const int THIRD_ARGV_PARAM = 2;
const int FORTH_ARGV_PARAM = 3;
const unsigned int BITS_PER_SAMPLE_0 = 0;
const unsigned int BITS_PER_SAMPLE_1 = 1;
const unsigned int BITS_PER_SAMPLE_2 = 2;
const unsigned int BITS_PER_SAMPLE_4 = 4;
const unsigned int BITS_PER_SAMPLE_8 = 8;
const unsigned int BITS_PER_SAMPLE_16 = 16;
const unsigned int BITS_PER_SAMPLE_24 = 24;
const unsigned int BITS_PER_SAMPLE_32 = 32;
const unsigned int VB_MODE_CLEAR = 1;
const unsigned int VB_MODE_THEATRE = 2;
const unsigned int VB_MODE_CD = 3;
const unsigned int VB_MODE_RECORDING_STUDIO = 4;
const size_t INPUT_NODE_SIZE_2 = 2;
const size_t MAX_FRAME_SIZE = 20 * 192000 * 2 / 1000 * 32 / 8;
const int MAX_BUFFER_SIZE = 100 * 1024 * 1024;
const int ARRAY_SIZE_2 = 2;
const int AUDIOSUITE_ERROR_SYSTEM_CODE = 3;
const int ERROR_CODE_3 = 3;
const double HUNDRED_NUM = 100;
const int INVALID_FILE_LENGTH = -1;
const int SAMPLINGRATE_MULTI = 20;
const int CHANNELCOUNT_MULTI = 1000;
const int BITSPERSAMPLE_MULTI = 8;
const int CONSTANT_0 = 0;
const char *MULTI_PIPELINE_TAG = "[AudioEditTestApp_multiPipelineEdit_cpp]";

// Multi-thread shared lock
std::mutex g_threadLock;
// Thread private pipelineManager
thread_local std::shared_ptr<PipelineManager> threadPipelineManager;
// Set the maximum number of parallel pipelines to 10
int g_maxPipelineSize = 10;
OH_AudioSuitePipeline **g_multiAudioSuitePipeline =
    (OH_AudioSuitePipeline **)malloc(g_maxPipelineSize * sizeof(OH_AudioSuitePipeline *));
std::atomic<int> g_initedPipelineNum {0};
// The engine is globally unique and cannot be created repeatedly
std::atomic<bool> g_engineInitedFlag {false};
std::atomic<bool> g_startMultiProcess {true};
OH_AudioSuiteEngine *g_multiAudioSuiteEngine;
// Create output builder constructor
OH_AudioNodeBuilder *g_multiBuilderOut;
std::unordered_map<std::string, std::shared_ptr<PipelineManager>> pipelineIdToPipelineManagerMap;
std::vector<std::string> initedPipelineIdArray = {};
std::unordered_map<std::string, double> multiPipelineProcessMap;
std::vector<FILE*> g_activedFileArray;

void MultiStoreTotalBuffToMap(const char *totalBuff, size_t size, const std::string &key)
{
    std::lock_guard<std::mutex> lock(g_threadLock);
    OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, MULTI_PIPELINE_TAG,
                 "audioEditTest storeTotalBuffToMap totalBuff:%{public}p, size:%{public}zu", totalBuff, size);
    OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, MULTI_PIPELINE_TAG, "audioEditTest storeTotalBuffToMap failed");
}

OH_AudioSuite_Result GetMultiRenderFrameOutput(char *&firData, char *&secData, size_t &firDataSize,
    size_t &secDataSize, bool &finishedFlag)
{
    OH_AudioSuitePipeline *threadPipeline = threadPipelineManager->audioSuitePipeline;
    OH_AudioFormat threadAudioFormatOutput = threadPipelineManager->audioFormatOutput;
    int32_t writeSize = 0;
    int32_t bitsPerSample = GetBitsPerSample(threadAudioFormatOutput.sampleFormat);
    int32_t frameSize =
        20 * threadAudioFormatOutput.samplingRate * threadAudioFormatOutput.channelCount / 1000 * bitsPerSample / 8;
    OH_AudioDataArray *ohAudioDataArray = new OH_AudioDataArray();
    ohAudioDataArray->audioDataArray = (void **)malloc(ARRAY_SIZE_2 * sizeof(void *));
    for (int i = 0; i < ARRAY_SIZE_2; i++) {
        ohAudioDataArray->audioDataArray[i] = (void *)malloc(frameSize);
    }
    ohAudioDataArray->arraySize = ARRAY_SIZE_2;
    ohAudioDataArray->requestFrameSize = frameSize;
    OH_AudioSuite_Result result;

    do {
        result =
            OH_AudioSuiteEngine_MultiRenderFrame(threadPipeline, ohAudioDataArray, &writeSize, &finishedFlag);
        OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, MULTI_PIPELINE_TAG,
                     "audioEditTest OH_AudioSuiteEngine_MultiRenderFrame "
                     "frameSize: %{public}d,writeSize:%{public}d,finishedFlag : %{public}s",
                     ohAudioDataArray->requestFrameSize, writeSize, (finishedFlag ? "true" : "false"));
        if (result != OH_AudioSuite_Result::AUDIOSUITE_SUCCESS) {
            OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, MULTI_PIPELINE_TAG,
                         "audioEditTest OH_audioSuiteEngine_RenderFrame result is %{public}d",
                         static_cast<int>(result));
            continue;
        }

        // Save the obtained buffer value each time
        std::copy(static_cast<const char*>(ohAudioDataArray->audioDataArray[0]),
            static_cast<const char*>(ohAudioDataArray->audioDataArray[0]) + writeSize,
            static_cast<char*>(firData) + firDataSize);
        std::copy(static_cast<const char*>(ohAudioDataArray->audioDataArray[1]),
            static_cast<const char*>(ohAudioDataArray->audioDataArray[1]) + writeSize,
            static_cast<char*>(secData) + secDataSize);
        firDataSize += writeSize;
        secDataSize += writeSize;
        OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, MULTI_PIPELINE_TAG,
                     "audioEditTest OH_AudioSuiteEngine_RenderFrame resultTotalSize: %{public}zu, writeSize : "
                     "%{public}d, finished: %{public}s",
                     firDataSize, writeSize, (finishedFlag ? "true" : "false"));
    } while (!finishedFlag);
    for (int i = 0; i < ARRAY_SIZE_2; i++) {
        FreeBuffer((char **)&ohAudioDataArray->audioDataArray[i]);
    }
    return result;
}

OH_AudioSuite_Result GetRenderFrameOutput(char *&firData, size_t frameSize, size_t &firDataSize, bool &finishedFlag)
{
    OH_AudioSuitePipeline *threadPipeline = threadPipelineManager->audioSuitePipeline;
    OH_AudioSuite_Result result;
    int32_t writeSize = 0;
    if (frameSize <= 0 || frameSize > MAX_FRAME_SIZE) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, MULTI_PIPELINE_TAG,
            "audioEditTest GetRenderFrameOutput frameSize is invalid, cannot allocate memory");
        return AUDIOSUITE_ERROR_INVALID_PARAM;
    }
    char *audioData = (char *)malloc(frameSize);
    // Get pipeline status
    OH_AudioSuite_PipelineState pipeLineState;
    result = OH_AudioSuiteEngine_GetPipelineState(threadPipeline, &pipeLineState);
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, MULTI_PIPELINE_TAG,
        "audioEditTest OH_audioSuiteEngine_GetPipelineState11111 result: %{public}d --- pipeLineState: %{public}d",
        static_cast<int>(result), static_cast<int>(pipeLineState));
    OH_LOG_Print(LOG_APP, LOG_WARN, GLOBAL_RESMGR, MULTI_PIPELINE_TAG,
        "audioEditTest renDerFrame frameSize:%{public}d", frameSize);
    do {
        result = OH_AudioSuiteEngine_RenderFrame(threadPipeline, audioData, frameSize, &writeSize, &finishedFlag);
        OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, MULTI_PIPELINE_TAG,
                     "audioEditTest OH_AudioSuiteEngine_RenderFrame frameSize: %{public}zu,writeSize:%{public}d "
                     "finishedFlag : %{public}s, result: %{public}d",
                     frameSize, writeSize, (finishedFlag ? "true" : "false"), static_cast<int>(result));
        if (result != OH_AudioSuite_Result::AUDIOSUITE_SUCCESS) {
            OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, MULTI_PIPELINE_TAG,
                         "audioEditTest OH_audioSuiteEngine_RenderFrame result is %{public}d",
                         static_cast<int>(result));
            continue;
        }
        // Save the obtained buffer value each time
        std::copy(static_cast<const char*>(audioData),
            static_cast<const char*>(audioData) + writeSize,
            static_cast<char*>(firData) + firDataSize);
        firDataSize += writeSize;
        OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, MULTI_PIPELINE_TAG,
                     "audioEditTest OH_AudioSuiteEngine_RenderFrame resultTotalSize: %{public}zu, writeSize : "
                     "%{public}d, finished: %{public}s",
                     firDataSize, writeSize, (finishedFlag ? "true" : "false"));
    } while (!finishedFlag);
    FreeBuffer(&audioData);
    return result;
}

OH_AudioSuite_Result MultiPipelineRenderFrame()
{
    OH_AudioSuitePipeline *threadPipeline = threadPipelineManager->audioSuitePipeline;
    bool &multiRenderFrameFlag = threadPipelineManager->multiRenderFrameFlag;
    char *&firstAudioBuffer = threadPipelineManager->firstAudioBuffer;
    char *&secondAudioBuffer = threadPipelineManager->secondAudioBuffer;
    size_t &firstBufferSize = threadPipelineManager->firstBufferSize;
    size_t &secondBufferSize = threadPipelineManager->secondBufferSize;
    OH_AudioFormat threadAudioFormatOutput = threadPipelineManager->audioFormatOutput;
    bool &finishedFlag = threadPipelineManager->renderFrameFinishFlag;

    OH_AudioSuite_Result result = StartPipelineAndCheckState(threadPipeline);
    if (result != OH_AudioSuite_Result::AUDIOSUITE_SUCCESS) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, MULTI_PIPELINE_TAG, "StartPipeline ERROR:%{public}d", result);
        return result;
    }
    
    char *firAudioData = (char *)malloc(1024 * 1024 * 100);
    char *secAudioData = (char *)malloc(1024 * 1024 * 100);
    
    int32_t bitsPerSample = GetBitsPerSample(threadAudioFormatOutput.sampleFormat);
    int32_t frameSize =
        20 * threadAudioFormatOutput.samplingRate * threadAudioFormatOutput.channelCount / 1000 * bitsPerSample / 8;
    
    if (multiRenderFrameFlag) {
        result = GetMultiRenderFrameOutput(firAudioData, secAudioData, firstBufferSize, secondBufferSize, finishedFlag);
    } else {
        result = GetRenderFrameOutput(firAudioData, frameSize, firstBufferSize, finishedFlag);
    }
    if (result == OH_AudioSuite_Result::AUDIOSUITE_SUCCESS) {
        FreeBuffer(&firstAudioBuffer);
        firstAudioBuffer = (char *)malloc(firstBufferSize);
        if (firstAudioBuffer == nullptr) {
            return OH_AudioSuite_Result::AUDIOSUITE_ERROR_SYSTEM;
        }
        std::copy(static_cast<const char*>(firAudioData),
            static_cast<const char*>(firAudioData) + firstBufferSize,
            static_cast<char*>(firstAudioBuffer));
        OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, MULTI_PIPELINE_TAG, "audioEditTest renDerFrame std::copy "
                     "firBuff: %{public}p, firstBufferSize:%{public}zu", firstAudioBuffer, firstBufferSize);
        if (multiRenderFrameFlag) {
            FreeBuffer(&secondAudioBuffer);
            secondAudioBuffer = (char *)malloc(secondBufferSize);
            std::copy(static_cast<const char*>(secAudioData),
                static_cast<const char*>(secAudioData) + secondBufferSize,
                static_cast<char*>(secondAudioBuffer));
            OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, MULTI_PIPELINE_TAG, "audioEditTest renDerFrame "
                         "std::copy secBuff: %{public}p, g_totalSize:%{public}zu",
                         secondAudioBuffer, secondBufferSize);
            multiRenderFrameFlag = false;
        }
    }
    return result;
}

void MultiRenderFrameAsync(RenderFrameAsyncParam *param)
{
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, MULTI_PIPELINE_TAG, "RenderFrameAsync start pipelineId:%{public}s",
                 threadPipelineManager->pipelineId.c_str());
    std::future<OH_AudioSuite_Result> futureTask = std::async(std::launch::async, OH_AudioSuiteEngine_MultiRenderFrame,
        param->audioSuitePipeline, param->ohAudioDataArray, &param->responseSize, &param->finishedFlag);
    OH_AudioSuite_Result futureResult = futureTask.get();
    if (futureResult == AUDIOSUITE_SUCCESS) {
        std::copy(static_cast<char *>(param->ohAudioDataArray->audioDataArray[BITS_PER_SAMPLE_0]),
                  static_cast<char *>(param->ohAudioDataArray->audioDataArray[BITS_PER_SAMPLE_0]) + param->responseSize,
                  static_cast<char *>(param->firAudioData) + *param->firstBufferSize);
        std::copy(static_cast<char *>(param->ohAudioDataArray->audioDataArray[BITS_PER_SAMPLE_1]),
                  static_cast<char *>(param->ohAudioDataArray->audioDataArray[BITS_PER_SAMPLE_1]) + param->responseSize,
                  static_cast<char *>(param->secAudioData) + *param->secondBufferSize);
        *param->secondBufferSize += param->responseSize;
        *param->firstBufferSize += param->responseSize;
        OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, MULTI_PIPELINE_TAG,
                     "audioEditTest OH_AudioSuiteEngine_RenderFrame frameSize: %{public}d, resultTotalSize: %{public}zu"
                     ", writeSize : %{public}d, finished: %{public}s",  param->requestFrameSize,
                     *param->firstBufferSize, param-> responseSize, (param->finishedFlag ? "true" : "false"));
        if (!param->finishedFlag) {
            MultiRenderFrameAsync(param);
        } else {
            FreeBuffer(param->firstAudioBuffer);
            *param->firstAudioBuffer = (char *)malloc(*param->firstBufferSize);
            std::copy(param->firAudioData, param->firAudioData + *param->firstBufferSize, *param->firstAudioBuffer);
            OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, MULTI_PIPELINE_TAG, "audioEditTest renDerFrame memcpy "
                         "firBuff: %{public}p, firstBufferSize:%{public}zu",
                         *param->firstAudioBuffer, *param->firstBufferSize);
            FreeBuffer(param->secondAudioBuffer);
            *param->secondAudioBuffer = (char *)malloc(*param->secondBufferSize);
            std::copy(static_cast<char *>(param->secAudioData),
                      static_cast<char *>(param->secAudioData) + *param->secondBufferSize, *param->secondAudioBuffer);
            OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, MULTI_PIPELINE_TAG, "audioEditTest renDerFrame "
                         "memcpy secBuff: %{public}p, g_totalSize:%{public}zu",
                         param->secondAudioBuffer, *param->secondBufferSize);
        }
    }
}

void RenderFrameAsync(RenderFrameAsyncParam *param)
{
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, MULTI_PIPELINE_TAG, "RenderFrameAsync start pipelineId:%{public}s",
                 threadPipelineManager->pipelineId.c_str());
    std::future<OH_AudioSuite_Result> futureTask = std::async(std::launch::async, OH_AudioSuiteEngine_RenderFrame,
        param->audioSuitePipeline, param->audioData, param->requestFrameSize, &param->responseSize,
        &param->finishedFlag);
    OH_AudioSuite_Result futureResult = futureTask.get();
    if (futureResult == AUDIOSUITE_SUCCESS) {
        std::copy(static_cast<char *>(param->audioData), static_cast<char *>(param->audioData) + param->responseSize,
                  static_cast<char *>(param->firAudioData) + *param->firstBufferSize);
        *param->firstBufferSize += param->responseSize;
        OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, MULTI_PIPELINE_TAG,
                     "audioEditTest OH_AudioSuiteEngine_RenderFrame frameSize: %{public}d, resultTotalSize: %{public}zu"
                     ", writeSize : %{public}d, finished: %{public}s", param->requestFrameSize,
                     *param->firstBufferSize, param-> responseSize, (param->finishedFlag ? "true" : "false"));
        if (!param->finishedFlag) {
            RenderFrameAsync(param);
        } else {
            FreeBuffer(param->firstAudioBuffer);
            *param->firstAudioBuffer = (char *)malloc(*param->firstBufferSize);
            std::copy(static_cast<char *>(param->audioData),
                      static_cast<char *>(param->audioData) + *param->firstBufferSize, *param->firstAudioBuffer);
            OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, MULTI_PIPELINE_TAG, "audioEditTest renDerFrame memcpy "
                "firBuff: %{public}p, firstBufferSize:%{public}zu", *param->firstAudioBuffer, *param->firstBufferSize);
            delete param;
        }
    }
}

OH_AudioSuite_Result GetMultiRenderFrameOutputAsync(char *&firData, char *&secData, size_t &firDataSize,
    size_t &secDataSize, bool &finishedFlag)
{
    OH_AudioSuitePipeline *threadPipeline = threadPipelineManager->audioSuitePipeline;
    OH_AudioFormat threadAudioFormatOutput = threadPipelineManager->audioFormatOutput;
    int32_t bitsPerSample = GetBitsPerSample(threadAudioFormatOutput.sampleFormat);
    int32_t frameSize =
        20 * threadAudioFormatOutput.samplingRate * threadAudioFormatOutput.channelCount / 1000 * bitsPerSample / 8;
    OH_AudioDataArray *ohAudioDataArray = new OH_AudioDataArray();
    ohAudioDataArray->audioDataArray = (void **)malloc(sizeof(void *) + sizeof(void *));
    for (int i = 0; i < ARRAY_SIZE_2; i++) {
        ohAudioDataArray->audioDataArray[i] = (void *)malloc(frameSize);
    }
    ohAudioDataArray->arraySize = ARRAY_SIZE_2;
    ohAudioDataArray->requestFrameSize = frameSize;
    OH_AudioSuite_Result result = AUDIOSUITE_SUCCESS;
    RenderFrameAsyncParam *param = new RenderFrameAsyncParam();
    param->audioSuitePipeline = threadPipeline;
    param->ohAudioDataArray = ohAudioDataArray;
    param->firAudioData = firData;
    param->firstBufferSize = &firDataSize;
    param->secAudioData = secData;
    param->secondBufferSize = &secDataSize;
    param->requestFrameSize = frameSize;
    param->responseSize = 0;
    param->finishedFlag = finishedFlag;
    param->firstAudioBuffer = &threadPipelineManager->firstAudioBuffer;
    param->secondAudioBuffer = &threadPipelineManager->secondAudioBuffer;
    MultiRenderFrameAsync(param);
    for (int i = 0; i < ARRAY_SIZE_2; i++) {
        FreeBufferOfVoid(&ohAudioDataArray->audioDataArray[i]);
    }
    delete ohAudioDataArray;
    ohAudioDataArray = nullptr;
    delete param;
    param = nullptr;
    return result;
}

OH_AudioSuite_Result GetRenderFrameOutputAsync(char *&firData, size_t frameSize,
                                               size_t &firDataSize, bool &finishedFlag)
{
    OH_AudioSuitePipeline *threadPipeline = threadPipelineManager->audioSuitePipeline;
    OH_AudioSuite_Result result = AUDIOSUITE_SUCCESS;
    if (frameSize <= 0 || frameSize > MAX_FRAME_SIZE) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, MULTI_PIPELINE_TAG,
            "audioEditTest GetRenderFrameOutput frameSize is invalid, cannot allocate memory");
        return AUDIOSUITE_ERROR_INVALID_PARAM;
    }
    char *audioData = (char *)malloc(frameSize);
    if (audioData == nullptr) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, MULTI_PIPELINE_TAG, "audioData malloc failed");
        return OH_AudioSuite_Result::AUDIOSUITE_ERROR_SYSTEM;
    }
    // Get pipeline status
    OH_AudioSuite_PipelineState pipeLineState;
    result = OH_AudioSuiteEngine_GetPipelineState(threadPipeline, &pipeLineState);
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, MULTI_PIPELINE_TAG,
        "audioEditTest OH_audioSuiteEngine_GetPipelineState result: %{public}d --- pipeLineState: %{public}d",
        static_cast<int>(result), static_cast<int>(pipeLineState));
    RenderFrameAsyncParam *param = new RenderFrameAsyncParam();
    param->audioSuitePipeline = threadPipeline;
    param->firAudioData = firData;
    param->firstBufferSize = &threadPipelineManager->firstBufferSize;
    param->requestFrameSize = frameSize;
    param->responseSize = 0;
    param->audioData = audioData;
    param->finishedFlag = finishedFlag;
    param->firstAudioBuffer = &threadPipelineManager->firstAudioBuffer;
    RenderFrameAsync(param);
    delete param;
    param = nullptr;
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, MULTI_PIPELINE_TAG,
                 "RenderFrameAsync done, pipeline:%{public}s", threadPipelineManager->pipelineId.c_str());
    return OH_AudioSuite_Result::AUDIOSUITE_SUCCESS;
}

napi_value MultiPipelineEnvPrepare(napi_env env, napi_callback_info info)
{
    std::lock_guard<std::mutex> lock(g_threadLock);
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, MULTI_PIPELINE_TAG, "audioEditTest multiPipelinePrepare start");
    napi_value napiValue;
    OH_AudioSuite_Result result = AUDIOSUITE_SUCCESS;

    size_t argc = 1;
    napi_value *argv = new napi_value[argc];
    std::string pipelineId;
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    ParseNapiString(env, argv[FIRST_ARGV_PARAM], pipelineId);
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, MULTI_PIPELINE_TAG,
                 "audioEditTest multiPipelinePrepare pipelineId=%{public}s", pipelineId.c_str());

    if (threadPipelineManager != nullptr && threadPipelineManager->pipelineId == pipelineId) {
        OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, MULTI_PIPELINE_TAG,
                     "multiPipelinePrepare direct return, pipeline manager of thread had inited,"
                     "threadPipelineManager=%{public}p",
                     threadPipelineManager.get());
        napi_create_int64(env, static_cast<int>(0), &napiValue);
        return napiValue;
    }
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, MULTI_PIPELINE_TAG, "multiPipelinePrepare thread_id=%{public}d",
                 gettid());
    std::shared_ptr<PipelineManager> pipelineManager = pipelineIdToPipelineManagerMap[pipelineId];
    if (pipelineManager == nullptr) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, MULTI_PIPELINE_TAG,
                     "multiPipelinePrepare pipelineIdToPipelineManagerMap ERROR pipelineId=%{public}s",
                     pipelineId.c_str());
        napi_create_int64(env, 1, &napiValue);
        return napiValue;
    }
    threadPipelineManager = pipelineManager;
    g_nodeManager = pipelineManager->nodeManager;
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, MULTI_PIPELINE_TAG,
                 "multiPipelinePrepare threadPipelineManager=%{public}p", threadPipelineManager.get());
    napi_create_int64(env, static_cast<int>(result), &napiValue);
    return napiValue;
}

napi_value AudioEditNodeInitMultiPipeline(napi_env env, napi_callback_info info)
{
    std::lock_guard<std::mutex> lock(g_threadLock);
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, MULTI_PIPELINE_TAG,
                 "audioEditTest AudioEditNodeInitMultiPipeline start");
    OH_AudioSuite_Result result;
    size_t argc = 1;
    napi_value *argv = new napi_value[argc];
    std::string pipelineId;
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    ParseNapiString(env, argv[FIRST_ARGV_PARAM], pipelineId);
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, MULTI_PIPELINE_TAG,
                 "audioEditTest AudioEditNodeInitMultiPipeline pipelineId:%{public}s", pipelineId.c_str());

    // Create Engine
    if (!g_engineInitedFlag) {
        g_engineInitedFlag = true;
        result = OH_AudioSuiteEngine_Create(&g_multiAudioSuiteEngine);
    }
    OH_AudioSuite_PipelineWorkMode workMode;

    // Create Pipeline
    result = OH_AudioSuiteEngine_CreatePipeline(g_multiAudioSuiteEngine,
        &g_multiAudioSuitePipeline[g_initedPipelineNum], workMode);
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, MULTI_PIPELINE_TAG,
                 "audioEditTest OH_AudioEditEngine_CreatePipeline result: %{public}d", static_cast<int>(result));
    // Instantiate PipelineManager
    std::shared_ptr<NodeManager> nodeManager =
        std::make_shared<NodeManager>(g_multiAudioSuitePipeline[g_initedPipelineNum]);
    std::shared_ptr<PipelineManager> pipelineManager =
        std::make_shared<PipelineManager>(pipelineId, g_multiAudioSuitePipeline[g_initedPipelineNum], nodeManager);
    pipelineIdToPipelineManagerMap[pipelineId] = pipelineManager;
    OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, MULTI_PIPELINE_TAG,
                 "audioEditTest multi pipeline init: g_initedPipelineNum: %{public}d,"
                 "pipelineManager:%{public}p, audioSuitePipeline: %{public}p, nodeManager:%{public}p",
                 g_initedPipelineNum.load(), pipelineManager.get(), pipelineManager->audioSuitePipeline,
                 pipelineManager->nodeManager.get());
    
    for (const auto &pair : pipelineIdToPipelineManagerMap) {
        OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, MULTI_PIPELINE_TAG,
                     "pipelineIdToPipelineManagerMap key=%{public}s,value=%{public}p", pair.first.c_str(),
                     pair.second.get());
    }
    initedPipelineIdArray.push_back(pipelineId);
    g_initedPipelineNum++;
    napi_value napiValue;
    napi_create_int64(env, static_cast<int>(result), &napiValue);
    return napiValue;
}

napi_value DestroyMultiPipeline(napi_env env, napi_callback_info info)
{
    std::lock_guard<std::mutex> lock(g_threadLock);
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, MULTI_PIPELINE_TAG, "audioEditTest AudioEditDestory start");
    OH_AudioSuite_Result result = OH_AudioSuite_Result::AUDIOSUITE_SUCCESS;
    for (const auto &pair : pipelineIdToPipelineManagerMap) {
        result = OH_AudioSuiteEngine_DestroyPipeline(pair.second->audioSuitePipeline);
        OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, MULTI_PIPELINE_TAG,
                     "audioEditTest OH_audioSuiteEngine_DestroyPipeline result: %{public}d, pipeline:%{public}p",
                     static_cast<int>(result), pair.second->audioSuitePipeline);
        if (result != OH_AudioSuite_Result::AUDIOSUITE_SUCCESS) {
            OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, MULTI_PIPELINE_TAG, "destoryMultiPipeline ERROR");
        }
    }
    g_initedPipelineNum = 0;
    pipelineIdToPipelineManagerMap.clear();
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, MULTI_PIPELINE_TAG, "audioEditTest clear all map of multi pipeline");

    result = OH_AudioSuiteEngine_Destroy(g_multiAudioSuiteEngine);
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, MULTI_PIPELINE_TAG,
                 "audioEditTest OH_audioSuiteEngine_Destroy result: %{public}d", static_cast<int>(result));
    g_engineInitedFlag = false;
    g_startMultiProcess = true;
    g_nodeManager = g_singlePipelineNodeManager;
    multiPipelineProcessMap.clear();
    initedPipelineIdArray.clear();
    napi_value napiValue;
    for (FILE* file : g_activedFileArray) {
        int ret = fclose(file);
        if (ret != 0) {
            OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, MULTI_PIPELINE_TAG, "Failed to close file");
        }
    }
    napi_create_int64(env, static_cast<int>(result), &napiValue);
    return napiValue;
}

napi_value MultiSetFormat(napi_env env, napi_callback_info info)
{
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, MULTI_PIPELINE_TAG, "audioEditTest SetFormat start");
    size_t argc = 3;
    napi_value *argv = new napi_value[argc];
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    // Get Number of Channels
    unsigned int channels;
    unsigned int sampleRate;
    unsigned int bitsPerSample;
    napi_get_value_uint32(env, argv[FIRST_ARGV_PARAM], &channels);
    napi_get_value_uint32(env, argv[SECOND_ARGV_PARAM], &sampleRate);
    napi_get_value_uint32(env, argv[THIRD_ARGV_PARAM], &bitsPerSample);
    switch (bitsPerSample) {
        case BITS_PER_SAMPLE_8:
            bitsPerSample = BITS_PER_SAMPLE_0;
            break;
        case BITS_PER_SAMPLE_16:
            bitsPerSample = BITS_PER_SAMPLE_1;
            break;
        case BITS_PER_SAMPLE_24:
            bitsPerSample = BITS_PER_SAMPLE_2;
            break;
        case BITS_PER_SAMPLE_32:
            bitsPerSample = BITS_PER_SAMPLE_4;
            break;
        default:
            bitsPerSample = BITS_PER_SAMPLE_0;
            break;
    }
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, MULTI_PIPELINE_TAG,
                 "audioEditTest SetFormat channels: %{public}d, sampleRate: %{public}d, bitsPerSample: %{public}d",
                 channels, sampleRate, bitsPerSample);
    OH_AudioFormat &threadAudioFormatOutput = threadPipelineManager->audioFormatOutput;
    const std::shared_ptr<NodeManager> &threadNodeManager = threadPipelineManager->nodeManager;
    // Set Sampling Rate
    threadAudioFormatOutput.samplingRate = SetSamplingRate(sampleRate);
    // Set audio channels
    threadAudioFormatOutput.channelCount = channels;
    threadAudioFormatOutput.channelLayout = SetChannelLayout(channels);
    // Set bit depth
    threadAudioFormatOutput.sampleFormat = SetSampleFormat(bitsPerSample);
    // Set the encoding format
    threadAudioFormatOutput.encodingType = OH_Audio_EncodingType::AUDIO_ENCODING_TYPE_RAW;
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, MULTI_PIPELINE_TAG,
                 "audioEditTest SetFormat threadAudioFormatOutput is %{public}p", &threadAudioFormatOutput);
    const std::vector<Node> outPutNodes =
        threadNodeManager->getNodesByType(OH_AudioNode_Type::OUTPUT_NODE_TYPE_DEFAULT);
    OH_AudioSuite_Result result =
        OH_AudioSuiteEngine_SetAudioFormat(outPutNodes[0].physicalNode, &threadAudioFormatOutput);
    napi_value napiValue;
    napi_create_int64(env, static_cast<int>(result), &napiValue);
    return napiValue;
}

int32_t MultiWriteDataCallBack(OH_AudioNode *audioNode, void *userData, void *audioData, int32_t audioDataSize,
                               bool *finished)
{
    std::lock_guard<std::mutex> lock(g_threadLock);
    if (audioNode == nullptr || audioData == nullptr || finished == nullptr) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, MULTI_PIPELINE_TAG, "WriteDataCallBack audioNode is nullptr");
        *finished = true;
        return 0;
    }
    OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, MULTI_PIPELINE_TAG, "multiWriteDataCallBack start");
    MultiUserData *curMultiUserData = static_cast<MultiUserData *>(userData);
    std::string inputId = curMultiUserData->inputId;
    std::string pipelineId = curMultiUserData->pipelineId;
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, MULTI_PIPELINE_TAG,"WriteDataCallBack inputId: %{public}s,"
                  "pipelineId:%{public}s", inputId.c_str(), pipelineId.c_str());
    if (pipelineIdToPipelineManagerMap.find(pipelineId) == pipelineIdToPipelineManagerMap.end()) {
        return 0;
    }
    std::shared_ptr<PipelineManager> threadPipelineManager = pipelineIdToPipelineManagerMap[pipelineId];
    std::map<std::string, FILE*> writeDataFileMap = threadPipelineManager->writeDataFileMap;
    float &inputDataProgress = threadPipelineManager->inputDataProgress;
    int32_t totalSize = curMultiUserData->bufferSize;
    size_t &totalWriteAudioDataSize = curMultiUserData->totalWriteAudioDataSize;
    auto it = writeDataFileMap.find(inputId);
    int32_t remainingDataSize = totalSize - totalWriteAudioDataSize;
    int32_t actualDataSize = std::min(audioDataSize, remainingDataSize);
    inputDataProgress += (static_cast<double>(actualDataSize) / static_cast<double>(totalSize) * HUNDRED_NUM);
    size_t bytesRead = fread(audioData, 1, actualDataSize, it->second);
    if (bytesRead < static_cast<size_t>(actualDataSize)) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, MULTI_PIPELINE_TAG, "bytesRead less than actualDataSize");
    }
    totalWriteAudioDataSize += actualDataSize;
    int32_t padSize = audioDataSize - remainingDataSize;
    if (padSize > 0) {
        OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, MULTI_PIPELINE_TAG, "padSize: %{public}d", padSize);
        std::fill(static_cast<char*>(audioData) + actualDataSize,
            static_cast<char*>(audioData) + actualDataSize + padSize, 0);
    }

    double process = static_cast<double >(totalWriteAudioDataSize) / totalSize;
    if (multiPipelineProcessMap.find(pipelineId) != multiPipelineProcessMap.end()) {
        multiPipelineProcessMap[pipelineId] = process;
    }
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, MULTI_PIPELINE_TAG, "process: %{public}f", process);
    if (totalWriteAudioDataSize >= totalSize) {
        OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, MULTI_PIPELINE_TAG, "WriteDataCallBack is finished");
        *finished = true;
        inputDataProgress = 100.f;
        multiPipelineProcessMap[pipelineId] = 1;
    }
    return actualDataSize;
}

OH_AudioSuite_Result MultiSetParamsAndWriteData(OH_AudioNodeBuilder *builder, std::string inputId,
                                                OH_AudioNode_Type type)
{
    OH_AudioSuite_Result result = OH_AudioSuiteNodeBuilder_SetFormat(builder, threadPipelineManager->audioFormatInput);
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, MULTI_PIPELINE_TAG,
                 "audioEditTest OH_AudioNodeBuilder_SetFormat result is %{public}d", static_cast<int>(result));
    if (result != OH_AudioSuite_Result::AUDIOSUITE_SUCCESS) {
        return result;
    }
    if (type != OH_AudioNode_Type::INPUT_NODE_TYPE_DEFAULT) {
        return result;
    }
    std::shared_ptr<MultiUserData> userData =
        std::make_shared<MultiUserData>(threadPipelineManager->pipelineId, inputId);
    // Later, we can consider removing totalInputDataSize and pass it as an input parameter
    userData->inputId = inputId;
    userData->pipelineId = threadPipelineManager->pipelineId;
    userData->bufferSize = threadPipelineManager->totalInputDataSize;
    userData->totalWriteAudioDataSize = 0;
    userData->isResetTotalWriteAudioDataSize = false;
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, MULTI_PIPELINE_TAG,
                 "audioEditTest OH_AudioSuiteNodeBuilder_SetRequestDataCallback"
                 "data address is %{public}p",
                 userData.get());
    OH_LOG_Print(
        LOG_APP, LOG_INFO, GLOBAL_RESMGR, MULTI_PIPELINE_TAG,
        "audioEditTest OH_AudioNodeBuilder_SetFormat MultiUserData inputId is %{public}s, pipelineId is %{public}s",
        userData->inputId.c_str(), userData->pipelineId.c_str());
    // Set the OH_AudioSuiteNodeBuilder_SetRequestDataCallback callback before creating the node
    result = OH_AudioSuiteNodeBuilder_SetRequestDataCallback(builder, MultiWriteDataCallBack, (void *)userData.get());
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, MULTI_PIPELINE_TAG,
                 "audioEditTest OH_AudioSuiteNodeBuilder_SetRequestDataCallback result is %{public}d",
                 static_cast<int>(result));
    
    if (result != OH_AudioSuite_Result::AUDIOSUITE_SUCCESS) {
        return result;
    }
    // Store the MultiUserData instance into the mapping table
    threadPipelineManager->userDataMap[inputId] = userData;
    return result;
}

void MultiCreateInputNode(napi_env env, const std::string &inputId, napi_value &napiValue,
                          OH_AudioSuite_Result &result)
{
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, MULTI_PIPELINE_TAG, "audioEditTest createInputNode start");
    char *threadinputBuffer = threadPipelineManager->inputBuffer;
    size_t threadtotalInputDataSize = threadPipelineManager->totalInputDataSize;
    MultiStoreTotalBuffToMap(threadinputBuffer, threadtotalInputDataSize, inputId);
    // Creating a builder constructor
    OH_AudioNodeBuilder *builderIn;
    result = OH_AudioSuiteNodeBuilder_Create(&builderIn);
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, MULTI_PIPELINE_TAG,
                 "audioEditTest OH_AudioSuiteNodeBuilder_Create result: %{public}d", static_cast<int>(result));
    if (result != OH_AudioSuite_Result::AUDIOSUITE_SUCCESS) {
        napi_create_int64(env, static_cast<int>(result), &napiValue);
        return;
    }
    // Transparent transmission node type
    result = OH_AudioSuiteNodeBuilder_SetNodeType(builderIn, OH_AudioNode_Type::INPUT_NODE_TYPE_DEFAULT);
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, MULTI_PIPELINE_TAG,
                 "NodeManagerTest createNode OH_AudioSuiteNodeBuilder_SetNodeType result: %{public}d",
                 static_cast<int>(result));
    if (result != OH_AudioSuite_Result::AUDIOSUITE_SUCCESS) {
        napi_create_int64(env, static_cast<int>(result), &napiValue);
        return;
    }

    // Packaging method, setting parameters for audio files, and writing audio files to a buffer
    result = MultiSetParamsAndWriteData(builderIn, inputId, OH_AudioNode_Type::INPUT_NODE_TYPE_DEFAULT);
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, MULTI_PIPELINE_TAG,
                 "audioEditTest SetParamsAndWriteData result: %{public}d", static_cast<int>(result));
    if (result != OH_AudioSuite_Result::AUDIOSUITE_SUCCESS) {
        napi_create_int64(env, static_cast<int>(result), &napiValue);
        return;
    }

    // Creating an input node
    std::shared_ptr<NodeManager> &threadNodeManager = threadPipelineManager->nodeManager;
    OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, MULTI_PIPELINE_TAG,
                 "audioEditTest CreateInputNode threadNodeManager: %{public}p , inputId:%{public}s",
                 threadNodeManager.get(), inputId.c_str());
    threadNodeManager->createNode(inputId, OH_AudioNode_Type::INPUT_NODE_TYPE_DEFAULT, builderIn);
}

void MultiUpdateInputNode(OH_AudioSuite_Result &result, UpdateInputNodeParams &params)
{
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, MULTI_PIPELINE_TAG, "audioEditTest MultiUpdateInputNode start");
    OH_AudioFormat &audioFormatInput = threadPipelineManager->audioFormatInput;
    OH_AudioFormat &audioFormatOutput = threadPipelineManager->audioFormatOutput;
    // Set Sampling Rate
    audioFormatInput.samplingRate = SetSamplingRate(params.sampleRate);
    // Set audio channels
    audioFormatInput.channelCount = params.channels;
    audioFormatInput.channelLayout = SetChannelLayout(params.channels);
    // Set bit depth
    audioFormatInput.sampleFormat = SetSampleFormat(params.bitsPerSample);
    // Set the encoding format
    audioFormatInput.encodingType = OH_Audio_EncodingType::AUDIO_ENCODING_TYPE_RAW;
    
    audioFormatOutput.samplingRate = audioFormatInput.samplingRate;
    audioFormatOutput.channelCount = params.channels;
    audioFormatOutput.channelLayout = audioFormatInput.channelLayout;
    audioFormatOutput.sampleFormat = audioFormatInput.sampleFormat;
    audioFormatOutput.encodingType = OH_Audio_EncodingType::AUDIO_ENCODING_TYPE_RAW;

    const std::vector<Node> inPutNodes = g_nodeManager->getNodesByType(OH_AudioNode_Type::INPUT_NODE_TYPE_DEFAULT);
    result = OH_AudioSuiteEngine_SetAudioFormat(inPutNodes[0].physicalNode, &audioFormatInput);
    const std::vector<Node> outPutNodes = g_nodeManager->getNodesByType(OH_AudioNode_Type::OUTPUT_NODE_TYPE_DEFAULT);
    result = OH_AudioSuiteEngine_SetAudioFormat(outPutNodes[0].physicalNode, &audioFormatOutput);
    MultiStoreTotalBuffToMap(threadPipelineManager->inputBuffer, threadPipelineManager->totalInputDataSize,
                             params.inputId);
    auto it = g_writeDataBufferMap.find(params.inputId);
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, MULTI_PIPELINE_TAG,
                 "audioEditTest AudioInAndOutInit g_writeDataBufferMap[inputId] length: %{public}d", it->second.size());
}

bool MultiGetAudioProperties(OH_AVFormat *trackFormat, int32_t &sampleRate, int32_t &channels, int32_t &bitsPerSample)
{
    if (!OH_AVFormat_GetIntValue(trackFormat, OH_MD_KEY_AUD_SAMPLE_RATE, &sampleRate)) {
        OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, MULTI_PIPELINE_TAG, "get sample rate failed");
        return false;
    }
    if (!OH_AVFormat_GetIntValue(trackFormat, OH_MD_KEY_AUD_CHANNEL_COUNT, &channels)) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, MULTI_PIPELINE_TAG, "get channel count failed");
        return false;
    }
    if (!OH_AVFormat_GetIntValue(trackFormat, OH_MD_KEY_AUDIO_SAMPLE_FORMAT, &bitsPerSample)) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, MULTI_PIPELINE_TAG, "get bits per sample failed");
        return false;
    }
    OH_AudioFormat &threadAudioFormatInput = threadPipelineManager->audioFormatInput;
    OH_AudioFormat &threadAudioFormatOutput = threadPipelineManager->audioFormatOutput;
    // Set Sampling Rate
    threadAudioFormatInput.samplingRate = SetSamplingRate(sampleRate);
    // Set audio channels
    threadAudioFormatInput.channelCount = channels;
    threadAudioFormatInput.channelLayout = SetChannelLayout(channels);
    // Set bit depth
    threadAudioFormatInput.sampleFormat = SetSampleFormat(bitsPerSample);
    // Set the encoding format
    threadAudioFormatInput.encodingType = OH_Audio_EncodingType::AUDIO_ENCODING_TYPE_RAW;

    threadAudioFormatOutput.samplingRate = threadAudioFormatInput.samplingRate;
    threadAudioFormatOutput.channelCount = channels;
    threadAudioFormatOutput.channelLayout = threadAudioFormatInput.channelLayout;
    threadAudioFormatOutput.sampleFormat = threadAudioFormatInput.sampleFormat;
    threadAudioFormatOutput.encodingType = OH_Audio_EncodingType::AUDIO_ENCODING_TYPE_RAW;
    
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, MULTI_PIPELINE_TAG, "sampleRate: %{public}d, channels: %{public}d,"
                 "bitsPerSample: %{public}d", sampleRate, channels, bitsPerSample);
    return true;
}

void MultiManageExistingOutputNodes(const std::string &inputId, const std::string &mixerId,
                                    OH_AudioSuite_Result &result, std::vector<Node> outPutNodes)
{
    std::shared_ptr<NodeManager> &threadNodeManager = threadPipelineManager->nodeManager;
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, MULTI_PIPELINE_TAG,
                 "audioEditTest AddEffectNodeToNodeManager start, threadNodeManager: %{public}p",
                 threadNodeManager.get());
    const std::vector<Node> mixerNodes =
        threadNodeManager->getNodesByType(OH_AudioNode_Type::EFFECT_NODE_TYPE_AUDIO_MIXER);
    if (mixerNodes.size() > 0) {
        result = threadNodeManager->connect(inputId, mixerNodes[0].id);
        OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, MULTI_PIPELINE_TAG,
                     "audioEditTest connect input and mixer result: %{public}d", static_cast<int>(result));
    } else {
        result = threadNodeManager->createNode(mixerId, OH_AudioNode_Type::EFFECT_NODE_TYPE_AUDIO_MIXER);
        OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, MULTI_PIPELINE_TAG,
                     "audioEditTest nodeManagerCreateMixerNode result: %{public}d", static_cast<int>(result));

        result = threadNodeManager->insertNode(mixerId, outPutNodes[0].id, Direction::BEFORE);
        OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, MULTI_PIPELINE_TAG,
                     "audioEditTest insertMixerNode result: %{public}d", static_cast<int>(result));

        result = threadNodeManager->connect(inputId, mixerId);
        OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, MULTI_PIPELINE_TAG,
                     "audioEditTest connect inputId and mixerId result: %{public}d", static_cast<int>(result));
    }
}

void MultiCreateAndConnectOutputNodes(const std::string &inputId, const std::string &outputId,
                                      OH_AudioSuite_Result &result)
{
    std::shared_ptr<NodeManager> &threadNodeManager = threadPipelineManager->nodeManager;
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, MULTI_PIPELINE_TAG,
                 "audioEditTest CreateAndConnectOutputNodes start, threadNodeManager: %{public}p",
                 threadNodeManager.get());
    result = OH_AudioSuiteNodeBuilder_Create(&g_multiBuilderOut);
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, MULTI_PIPELINE_TAG,
                 "audioEditTest CreateAndConnectOutputNodes output builder result: %{public}d",
                 static_cast<int>(result));
    if (result != OH_AudioSuite_Result::AUDIOSUITE_SUCCESS) {
        return;
    }

    result = OH_AudioSuiteNodeBuilder_SetNodeType(g_multiBuilderOut, OH_AudioNode_Type::OUTPUT_NODE_TYPE_DEFAULT);
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, MULTI_PIPELINE_TAG,
                 "NodeManagerTest createNode OH_AudioSuiteNodeBuilder_SetNodeType result: %{public}d",
                 static_cast<int>(result));
    if (result != OH_AudioSuite_Result::AUDIOSUITE_SUCCESS) {
        return;
    }

    result = MultiSetParamsAndWriteData(g_multiBuilderOut, inputId, OH_AudioNode_Type::OUTPUT_NODE_TYPE_DEFAULT);
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, MULTI_PIPELINE_TAG,
                 "audioEditTest SetParamsAndWriteData result: %{public}d", static_cast<int>(result));
    if (result != OH_AudioSuite_Result::AUDIOSUITE_SUCCESS) {
        return;
    }

    result = threadNodeManager->createNode(outputId, OH_AudioNode_Type::OUTPUT_NODE_TYPE_DEFAULT, g_multiBuilderOut);
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, MULTI_PIPELINE_TAG,
                 "audioEditTest nodeManagerCreateOutputNode result: %{public}d", static_cast<int>(result));

    result = threadNodeManager->connect(inputId, outputId);
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, MULTI_PIPELINE_TAG,
                 "audioEditTest nodeManagerConnectInputAndOutput result: %{public}d", static_cast<int>(result));
}

void MultiManageOutputNodes(napi_env env, const std::string &inputId, const std::string &outputId,
                            const std::string &mixerId, OH_AudioSuite_Result &result)
{
    std::shared_ptr<NodeManager> &threadNodeManager = threadPipelineManager->nodeManager;
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, MULTI_PIPELINE_TAG,
                 "audioEditTest ManageOutputNodes start, threadNodeManager: %{public}p", threadNodeManager.get());
    const std::vector<Node> outPutNodes =
        threadNodeManager->getNodesByType(OH_AudioNode_Type::OUTPUT_NODE_TYPE_DEFAULT);
    if (outPutNodes.size() > 0) {
        MultiManageExistingOutputNodes(inputId, mixerId, result, outPutNodes);
    } else {
        MultiCreateAndConnectOutputNodes(inputId, outputId, result);
    }
}

napi_status ParseInputArguments(napi_env env, napi_callback_info info, InputAudioParams &params)
{
    size_t argc = 4;
    napi_value *argv = new napi_value[argc];
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    napi_status status = ParseNapiString(env, argv[ARG_0], params.inputId);
    status = ParseNapiString(env, argv[ARG_1], params.outputId);
    status = ParseNapiString(env, argv[ARG_2], params.mixerId);
    status = ParseNapiString(env, argv[ARG_3], params.fileName);
    OH_LOG_Print(
        LOG_APP, LOG_ERROR, GLOBAL_RESMGR, MULTI_PIPELINE_TAG,
        "inputId: %{public}s, outputId: %{public}s, mixerId: %{public}s, fileName: %{public}s",
        params.inputId.c_str(), params.outputId.c_str(),
        params.mixerId.c_str(), params.fileName.c_str());
    return status;
}

int GetFileLength(FILE *inputFile)
{
    if (fseek(inputFile, 0, SEEK_END) != 0) {
        fclose(inputFile);
        OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, MULTI_PIPELINE_TAG, "fseek ERROR");
        return INVALID_FILE_LENGTH;
    }
    int fileLength = ftell(inputFile);
    return fileLength;
}

napi_value MultiAudioInAndOutInit(napi_env env, napi_callback_info info)
{
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, MULTI_PIPELINE_TAG, "MultiAudioInAndOutInit start");
    InputAudioParams params;
    napi_status status = ParseInputArguments(env, info, params);
    if (status != napi_ok) {
        return ReturnResult(env, static_cast<AudioSuiteResult>(status));
    }
    FILE *inputFile = fopen(params.fileName.c_str(), "r");
    int fd = fileno(inputFile);
    int fileLength = GetFileLength(inputFile);
    OH_AVSource *source = OH_AVSource_CreateWithFD(fd, 0, fileLength);
    OH_AVFormat *trackFormat = OH_AVSource_GetTrackFormat(source, 0);
    if (trackFormat == nullptr) {
        return ReturnResult(env, AudioSuiteResult::DEMO_ERROR_FAILD);
    }
    int32_t sampleRate;
    int32_t channels;
    int32_t bitsPerSample;
    if (!MultiGetAudioProperties(trackFormat, sampleRate, channels, bitsPerSample)) {
        return ReturnResult(env, AudioSuiteResult::DEMO_ERROR_FAILD);
    }
    OH_AVDemuxer *demuxer = OH_AVDemuxer_CreateWithSource(source);
    if (demuxer == nullptr) {
        return ReturnResult(env, AudioSuiteResult::DEMO_ERROR_FAILD);
    }
    std::shared_ptr<NodeManager> &threadNodeManager = threadPipelineManager->nodeManager;
    std::map<std::string, FILE*> &writeDataFileMap = threadPipelineManager->writeDataFileMap;
    writeDataFileMap[params.inputId] = inputFile;
    threadPipelineManager->totalInputDataSize = fileLength;
    g_activedFileArray.push_back(inputFile);
    napi_value napiValue;
    OH_AudioSuite_Result result = AUDIOSUITE_SUCCESS;
    Node inputNode = threadNodeManager->GetNodeById(params.inputId);
    if (inputNode.id.empty()) {
        MultiCreateInputNode(env, params.inputId, napiValue, result);
    } else {
        UpdateInputNodeParams *updateParams =
            new UpdateInputNodeParams(params.inputId, channels, sampleRate, bitsPerSample);
        MultiUpdateInputNode(result, *updateParams);
        delete updateParams;
        updateParams = nullptr;
        return ReturnResult(env, static_cast<AudioSuiteResult>(result));
    }
    MultiManageOutputNodes(env, params.inputId, params.outputId, params.mixerId, result);
    std::vector<std::string> audioFormat = {std::to_string(sampleRate), std::to_string(channels),
                                            std::to_string(bitsPerSample)};
    CallStringArrayCallback(audioFormat);
    return ReturnResult(env, static_cast<AudioSuiteResult>(result));
}

OH_AudioSuite_Result RemoveNodeMoreThanTwoSize(std::shared_ptr<NodeManager> &threadNodeManager, std::string inputId)
{
    Node node = threadNodeManager->GetNodeById(inputId);
    Node nextNode;
    if (node.id.empty()) {
        return AUDIOSUITE_ERROR_SYSTEM;
    }
    while (node.type != OH_AudioNode_Type::EFFECT_NODE_TYPE_AUDIO_MIXER) {
        nextNode = threadNodeManager->GetNodeById(node.nextNodeId);
        OH_AudioSuite_Result result = threadNodeManager->removeNode(node.id);
        if (result != OH_AudioSuite_Result::AUDIOSUITE_SUCCESS) {
            return result;
        }
        node = nextNode;
    }
    OH_LOG_Print(
        LOG_APP, LOG_INFO, GLOBAL_RESMGR, MULTI_PIPELINE_TAG,
        "audioEditTest deleteSong preNodes of mixerNode and inputNodes number greater than 2 : %{public}d",
        static_cast<int>(threadNodeManager->getNodesByType(OH_AudioNode_Type::EFFECT_NODE_TYPE_AUDIO_MIXER)[0]
                             .preNodeIds.size()));
    return AUDIOSUITE_SUCCESS;
}

OH_AudioSuite_Result RemoveNodeEqualTwoSize(std::shared_ptr<NodeManager> &threadNodeManager, std::string inputId)
{
    Node node = threadNodeManager->GetNodeById(inputId);
    Node nextNode;
    if (node.id.empty()) {
        return AUDIOSUITE_ERROR_SYSTEM;
    }
    while (node.type != OH_AudioNode_Type::OUTPUT_NODE_TYPE_DEFAULT) {
        nextNode = threadNodeManager->GetNodeById(node.nextNodeId);
        OH_AudioSuite_Result result = threadNodeManager->removeNode(node.id);
        if (result != OH_AudioSuite_Result::AUDIOSUITE_SUCCESS) {
            return result;
        }
        node = nextNode;
    }
    OH_LOG_Print(
        LOG_APP, LOG_INFO, GLOBAL_RESMGR, MULTI_PIPELINE_TAG,
        "audioEditTest deleteSong number of mixerNode : %{public}d",
        static_cast<int>(
            threadNodeManager->getNodesByType(OH_AudioNode_Type::OUTPUT_NODE_TYPE_DEFAULT)[0].preNodeIds.size()));
    return AUDIOSUITE_SUCCESS;
}

OH_AudioSuite_Result RemoveNodeEqualOneSize(std::shared_ptr<NodeManager> &threadNodeManager, std::string inputId)
{
    Node node = threadNodeManager->GetNodeById(inputId);
    Node nextNode;
    while (!node.id.empty()) {
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, MULTI_PIPELINE_TAG,
                 "deleteSong inputNodes is 1 : %{public}s", node.id.c_str());
    nextNode = threadNodeManager->GetNodeById(node.nextNodeId);
    OH_AudioSuite_Result result = threadNodeManager->removeNode(node.id);
    if (result != OH_AudioSuite_Result::AUDIOSUITE_SUCCESS) {
        return result;
    }
    node = nextNode;
    }
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, MULTI_PIPELINE_TAG,
        "audioEditTest deleteSong nodes : %{public}zu", threadNodeManager->getAllNodes().size());
    return AUDIOSUITE_SUCCESS;
}

napi_value MultiDeleteSong(napi_env env, napi_callback_info info)
{
    OH_AudioSuite_Result result = AUDIOSUITE_SUCCESS;
    napi_value napiValue;
    size_t argc = 1;
    napi_value *argv = new napi_value[argc];
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    std::string inputId;
    napi_status status = ParseNapiString(env, argv[0], inputId);
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, MULTI_PIPELINE_TAG,
        "DeleteSong inputId:%{public}s", inputId.c_str());
    if (threadPipelineManager == nullptr) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, MULTI_PIPELINE_TAG, "DeleteSong, threadNodeManager is null");
        napi_create_int64(env, static_cast<int>(AUDIOSUITE_ERROR_SYSTEM), &napiValue);
        return napiValue;
    }
    std::shared_ptr<NodeManager> &threadNodeManager = threadPipelineManager->nodeManager;
    const std::vector<Node> inputNodes = threadNodeManager->getNodesByType(OH_AudioNode_Type::INPUT_NODE_TYPE_DEFAULT);
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, MULTI_PIPELINE_TAG,
                 "audioEditTest DeleteSong inputNodes length is %{public}d", static_cast<int>(inputNodes.size()));
    if (inputNodes.size() > INPUT_NODE_SIZE_2) {
        result = RemoveNodeMoreThanTwoSize(threadNodeManager, inputId);
    } else if (inputNodes.size() == INPUT_NODE_SIZE_2) {
        result = RemoveNodeEqualTwoSize(threadNodeManager, inputId);
    } else if (inputNodes.size() == 1) {
        result = RemoveNodeEqualOneSize(threadNodeManager, inputId);
    } else {
        napi_create_int64(env, static_cast<int>(-1), &napiValue);
        OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, MULTI_PIPELINE_TAG, "deleteSong inputNodes less than 1");
        return napiValue;
    }
    napi_create_int64(env, static_cast<int>(result), &napiValue);
    return napiValue;
}

napi_value MultiSaveFileBuffer(napi_env env, napi_callback_info info)
{
    pthread_t thisId = gettid();
    OH_AudioSuitePipeline *threadPipeline = threadPipelineManager->audioSuitePipeline;
    char *&threadBuffer = threadPipelineManager->firstAudioBuffer;
    size_t &threadBufferSize = threadPipelineManager->firstBufferSize;
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, MULTI_PIPELINE_TAG,
                 "audioEditTest SaveFileBuffer start buffer: %{public}p,"
                 "pipeline: %{public}p, thread_id:%{public}lu, threadBufferSize:%{public}zu",
                 threadBuffer, threadPipeline, thisId, threadBufferSize);
    if (g_startMultiProcess) {
        for (std::string id: initedPipelineIdArray) {
            multiPipelineProcessMap[id] = 0;
        }
        g_startMultiProcess = false;
    }
    multiPipelineProcessMap[threadPipelineManager->pipelineId] = 0;
    MultiPipelineRenderFrame();
    napi_value napiValue = nullptr;
    void *arrayBufferData = nullptr;
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, MULTI_PIPELINE_TAG,
                 "audioEditTest SaveFileBuffer start threadBufferSize:%{public}zu", threadBufferSize);
    napi_status status = napi_create_arraybuffer(env, threadBufferSize, &arrayBufferData, &napiValue);
    if (status != napi_ok || arrayBufferData == nullptr) {
        OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, MULTI_PIPELINE_TAG,
                     "audioEditTest OH_AudioSuiteEngine_RenderFrame status: %{public}d", static_cast<int>(status));
        FreeBuffer(&threadBuffer);
        // Failed to create ArrayBuffer; returned an ArrayBuffer with a size of 0
        napi_create_arraybuffer(env, 0, &arrayBufferData, &napiValue);
        return napiValue;
    } else {
        if ((threadPipeline == nullptr || threadBuffer == nullptr)) {
            napi_create_arraybuffer(env, 0, &arrayBufferData, &napiValue);
            return napiValue;
        }
        std::copy(static_cast<const char*>(threadBuffer),
            static_cast<const char*>(threadBuffer) + threadBufferSize,
            static_cast<char*>(arrayBufferData));
        FreeBuffer(&threadBuffer);
        return napiValue;
    }
}

napi_value MultiGetSecondOutputAudio(napi_env env, napi_callback_info info)
{
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, MULTI_PIPELINE_TAG, "audioEditTest---getAudioOfTap---IN");
    napi_value napiValue = nullptr;
    void *data;
    napi_create_arraybuffer(env, threadPipelineManager->secondBufferSize, &data, &napiValue);
    char *&secData = threadPipelineManager->secondAudioBuffer;
    size_t &threadSecDataSize = threadPipelineManager->secondBufferSize;
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, MULTI_PIPELINE_TAG,
                 "secData:%{public}p, threadSecDataSize:%{public}zu, pipeline:%{public}p",
                 secData, threadSecDataSize, threadPipelineManager->audioSuitePipeline);
    std::copy(static_cast<const char*>(secData),
        static_cast<const char*>(secData) + threadSecDataSize, static_cast<char*>(data));
    std::fill(static_cast<char*>(secData), static_cast<char*>(secData) + threadSecDataSize, 0);
    threadSecDataSize = 0;
    return napiValue;
}

OH_AudioSuite_Result MultiProcessPipeline(OH_AudioSuitePipeline *audioSuitePipeline)
{
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, MULTI_PIPELINE_TAG, "audioEditTest ProcessPipeline start");
    // Obtains pipeline status
    OH_AudioSuite_PipelineState pipeLineState;
    OH_AudioSuite_Result result = OH_AudioSuiteEngine_GetPipelineState(audioSuitePipeline, &pipeLineState);
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, MULTI_PIPELINE_TAG,
                 "audioEditTest OH_audioSuiteEngine_GetPipelineState result: %{public}d --- pipeLineState: %{public}d",
                 static_cast<int>(result), static_cast<int>(pipeLineState));
    if (result != OH_AudioSuite_Result::AUDIOSUITE_SUCCESS) {
        return result;
    }

    // Start the pipeline
    if (pipeLineState != OH_AudioSuite_PipelineState::AUDIOSUITE_PIPELINE_RUNNING) {
        result = OH_AudioSuiteEngine_StartPipeline(audioSuitePipeline);
        OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, MULTI_PIPELINE_TAG,
                     "audioEditTest OH_audioSuiteEngine_StartPipeline result: %{public}d", static_cast<int>(result));
        if (result != OH_AudioSuite_Result::AUDIOSUITE_SUCCESS) {
            return result;
        }
    }
    return result;
}

OH_AudioSuite_Result MultiOneRenDerFrame(int32_t audioDataSize, int32_t *writeSize)
{
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, MULTI_PIPELINE_TAG, "audioEditTest OneRenDerFrame start");
    OH_AudioSuitePipeline *audioSuitePipeline = threadPipelineManager->audioSuitePipeline;
    bool &finishedFlag = threadPipelineManager->renderFrameFinishFlag;
    char *playAudioBuffer = threadPipelineManager->playAudioBuffer;
    MultiProcessPipeline(audioSuitePipeline);
    if (audioDataSize <= CONSTANT_0) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, MULTI_PIPELINE_TAG,
            "audioEditTest OH_AudioSuiteEngine_RenderFrame audioDataSize is %{public}d",
            static_cast<int>(audioDataSize));
        return OH_AudioSuite_Result::AUDIOSUITE_ERROR_SYSTEM;
    }
    char *audioData = (char *)malloc(audioDataSize);
    if (audioData == nullptr) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, MULTI_PIPELINE_TAG, "GetRenderFrameOutput malloc failed");
        return OH_AudioSuite_Result::AUDIOSUITE_ERROR_SYSTEM;
    }
    OH_AudioSuite_Result result =
        OH_AudioSuiteEngine_RenderFrame(g_audioSuitePipeline, audioData, audioDataSize, writeSize, &finishedFlag);
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, MULTI_PIPELINE_TAG,
                 "audioEditTest OH_AudioSuiteEngine_RenderFrame audioDataSize: %{public}d,writeSize:%{public}d "
                 "g_play_finishedFlag : %{public}s, result: %{public}d",
                 audioDataSize, *writeSize, (finishedFlag ? "true" : "false"), static_cast<int>(result));
    if (result != OH_AudioSuite_Result::AUDIOSUITE_SUCCESS) {
        OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, MULTI_PIPELINE_TAG,
                     "audioEditTest OH_audioSuiteEngine_RenderFrame result is %{public}d", static_cast<int>(result));
    }
    FreeBuffer(&playAudioBuffer);
    playAudioBuffer = (char *)malloc(*writeSize);
    std::copy(audioData, audioData + *writeSize, playAudioBuffer);
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, MULTI_PIPELINE_TAG,
                 "audioEditTest OH_AudioSuiteEngine_RenderFrame writeSize: %{public}d, g_play_finishedFlag: %{public}s",
                 *writeSize, (finishedFlag ? "true" : "false"));
    free(audioData);
    audioData = nullptr;
    return result;
}
 
OH_AudioData_Callback_Result MultiPlayAudioRendererOnWriteData(OH_AudioRenderer *renderer,
    void *userData, void *audioData, int32_t audioDataSize)
{
    if (renderer == nullptr) {
        return AUDIO_DATA_CALLBACK_RESULT_INVALID;
    }
    if (audioData == nullptr) {
        return AUDIO_DATA_CALLBACK_RESULT_INVALID;
    }
    bool &finishedFlag = threadPipelineManager->renderFrameFinishFlag;
    char *firstAudioBuffer = threadPipelineManager->firstAudioBuffer;
    char *&playAudioBuffer = threadPipelineManager->playAudioBuffer;
    bool recordFlag = threadPipelineManager->recordFlag;
    size_t &firstBufferSize = threadPipelineManager->firstBufferSize;
    OH_AudioRenderer *&audioRenderer = threadPipelineManager->audioRenderer;
    int32_t writeSize = 0;
    if (!finishedFlag) {
        MultiOneRenDerFrame(audioDataSize, &writeSize);
        OH_LOG_Print(LOG_APP, LOG_WARN, GLOBAL_RESMGR, MULTI_PIPELINE_TAG, "g_isRecord: %{public}s",
            recordFlag ? "true" : "false");
        if (audioDataSize != 0 && recordFlag == true) {
            int32_t copySize = std::min(audioDataSize, writeSize);
            std::copy(playAudioBuffer, playAudioBuffer + copySize,
                static_cast<char *>(firstAudioBuffer) + firstBufferSize);
            firstBufferSize += std::min(audioDataSize, writeSize);
        }
    }
    int32_t copySize = std::min(audioDataSize, writeSize);
    if (firstAudioBuffer != nullptr && copySize > 0) {
        std::copy(firstAudioBuffer, firstAudioBuffer + copySize, static_cast<char *>(audioData));
    }
    if (finishedFlag) {
        // Stop playing
        OH_AudioRenderer_Stop(audioRenderer);
        // Stop pipeline
        OH_AudioSuiteEngine_StopPipeline(g_audioSuitePipeline);
        ResetAllIsResetTotalWriteAudioDataSize();
        OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, MULTI_PIPELINE_TAG, "audioEditTest "
            "playAudioRendererOnWriteData firstBufferSize is %{public}zu", firstBufferSize);
        CallBooleanCallback(finishedFlag);
        finishedFlag = false;
        if (g_totalBuff != nullptr) {
            free(g_totalBuff);
            g_totalBuff = nullptr;
        }
    }
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, MULTI_PIPELINE_TAG, "audioEditTest playAudioRendererOnWriteData "
        "g_play_resultTotalSize: %{public}zu, writeSize: %{public}d", firstBufferSize, writeSize);
    return AUDIO_DATA_CALLBACK_RESULT_VALID;
}
 
napi_value MultiAudioRendererInit(napi_env env, napi_callback_info info)
{
    OH_AudioStream_Type type = OH_AudioStream_Type::AUDIOSTREAM_TYPE_RENDERER;
    OH_AudioStreamBuilder *rendererBuilder = nullptr;
    OH_AudioRenderer *&audioRenderer = threadPipelineManager->audioRenderer;
    int32_t playDataSize = 0;
    OH_AudioStreamBuilder_Create(&rendererBuilder, type);
    int32_t bitsPerSample = 0;
    OH_AudioStream_SampleFormat streamSampleFormat;
 
    GetBitsPerSampleAndStreamFormat(g_audioFormatOutput, &bitsPerSample, &streamSampleFormat);
    
    OH_AudioStreamBuilder_SetSamplingRate(rendererBuilder, g_audioFormatOutput.samplingRate);
    OH_AudioStreamBuilder_SetChannelCount(rendererBuilder, g_audioFormatOutput.channelCount);
    OH_AudioStreamBuilder_SetSampleFormat(rendererBuilder, streamSampleFormat);
    OH_AudioStreamBuilder_SetEncodingType(rendererBuilder, AUDIOSTREAM_ENCODING_TYPE_RAW);
    OH_AudioStreamBuilder_SetRendererInfo(rendererBuilder, AUDIOSTREAM_USAGE_MUSIC);
    playDataSize = SAMPLINGRATE_MULTI * g_audioFormatOutput.samplingRate *
        g_audioFormatOutput.channelCount * bitsPerSample / BITSPERSAMPLE_MULTI / CHANNELCOUNT_MULTI;
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, MULTI_PIPELINE_TAG,
                 "audioEditTest playAudioRendererOnWriteData  playDataSize: %{public}d, samplingRate: %{public}d, "
                 "channelCount: %{public}d, bitsPerSample: %{public}d",
                 playDataSize, g_audioFormatOutput.samplingRate, g_audioFormatOutput.channelCount, bitsPerSample);
    OH_AudioStreamBuilder_SetFrameSizeInCallback(rendererBuilder, playDataSize);
    
    OH_AudioRenderer_OnWriteDataCallback rendererCallbacks = MultiPlayAudioRendererOnWriteData;
    OH_AudioStreamBuilder_SetRendererWriteDataCallback(rendererBuilder, rendererCallbacks, nullptr);
    
    OH_AudioStreamBuilder_GenerateRenderer(rendererBuilder, &audioRenderer);
    return nullptr;
}
 
napi_value MultiAudioRendererStart(napi_env env, napi_callback_info info)
{
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, MULTI_PIPELINE_TAG, "MultiAudioRendererStart start");
    OH_AudioSuitePipeline *audioSuitePipeline = threadPipelineManager->audioSuitePipeline;
    OH_AudioRenderer *&audioRenderer = threadPipelineManager->audioRenderer;
    MultiProcessPipeline(audioSuitePipeline);
    // start
    OH_AudioRenderer_Start(audioRenderer);
    return nullptr;
}
 
napi_value MultiRealTimeSaveFileBuffer(napi_env env, napi_callback_info info)
{
    char *&playAudioBuffer = threadPipelineManager->playAudioBuffer;
    size_t &playAudioBufferSize = threadPipelineManager->playAudioBufferSize;
    threadPipelineManager->recordFlag = false;
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, MULTI_PIPELINE_TAG, "audioEditTest RealTimeSaveFileBuffer start");
    napi_value napiValue = nullptr;
    void *arrayBufferData = nullptr;
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, MULTI_PIPELINE_TAG,
                 "audioEditTest RealTimeSaveFileBuffer g_play_resultTotalSize  is %{public}d", playAudioBufferSize);
    napi_status status = napi_create_arraybuffer(env, playAudioBufferSize, &arrayBufferData, &napiValue);
    if (status != napi_ok || arrayBufferData == nullptr) {
        OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, MULTI_PIPELINE_TAG, "audioEditTest napi_create_arraybuffer "
            "status: %{public}d", static_cast<int>(status));
        playAudioBufferSize = 0;
        if (playAudioBuffer != nullptr) {
            free(playAudioBuffer);
            playAudioBuffer = nullptr;
        }
        // Failed to create ArrayBuffer; returned an ArrayBuffer with a size of 0
        napi_create_arraybuffer(env, 0, &arrayBufferData, &napiValue);
        return napiValue;
    } else {
        std::copy(playAudioBuffer, playAudioBuffer + playAudioBufferSize,
            static_cast<char *>(arrayBufferData));
        if (playAudioBuffer != nullptr) {
            free(playAudioBuffer);
            playAudioBuffer = nullptr;
        }
        playAudioBufferSize = 0;
        return napiValue;
    }
}
 
napi_value GetAutoTestProcess(napi_env env, napi_callback_info info)
{
    std::lock_guard<std::mutex> lock(g_threadLock);
    napi_value jsMap;
    napi_create_object(env, &jsMap);
    bool endFlag = true;
    for (const auto& [key, value] : multiPipelineProcessMap) {
    napi_value keyVal, valVal;
    napi_create_string_utf8(env, key.c_str(), NAPI_AUTO_LENGTH, &keyVal);
    double roundedValue = std::round(value * 1000.0) / 10;
    napi_create_double(env, roundedValue, &valVal);
    if (value != 1.0) {
        endFlag = false;
    }
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, MULTI_PIPELINE_TAG, "roundedValue:%{public}.1f", roundedValue);
    napi_set_property(env, jsMap, keyVal, valVal);
    }
    if (endFlag) {
        OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, MULTI_PIPELINE_TAG, "process is ended");
    }
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, MULTI_PIPELINE_TAG, "ready to return process");
    return jsMap;
}