/*
 * Copyright (c) 2025 Huawei Device Co., Ltd. 2025-2025. ALL rights reserved.
 */

#include "Output.h"
#include "./Input.h"
#include "../utils/Utils.h"
#include "hilog/log.h"

const int GLOBAL_RESMGR = 0xFF00;
const char *OUTPUT_TAG = "[AudioEditTestApp_Output_cpp]";

const int MILLI_SECONDS_20 = 20;
const int MILLISECONDS_PER_SECOND = 1000;
const int BITS_PER_BYTE = 8;
const int AUDIO_DATA_BUFFER_SIZE = 1024 *1024 *1024;

OH_AudioFormat g_audioFormatOutput = {
    .encodingType = OH_Audio_EncodingType::AUDIO_ENCODING_TYPE_RAW
};

bool g_multiRenderFrameFlag = false;

bool g_globalFinishFlag = true;

char *g_tapTotalBuff = (char *)malloc(8 * 1024 * 1024);

int32_t g_tapDataTotalSize = 0;

OH_AudioSuite_Result renDerFrame()
{
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, OUTPUT_TAG, "audioEditTest renDerFrame start");

    OH_AudioSuite_Result result = startPipelineAndCheckState();
    if (result != OH_AudioSuite_Result::AUDIOSUITE_SUCCESS) {
        return result;
    }

    char *totalAudioData = (char *)malloc(AUDIO_DATA_BUFFER_SIZE);
    char *tapTotalAudioData = (char *)malloc(AUDIO_DATA_BUFFER_SIZE);
    // 获取位深
    int32_t bitsPerSample = getBitsPerSample(g_audioFormatOutput.sampleFormat);
    int32_t frameSize = MILLI_SECONDS_20 * g_audioFormatOutput.samplingRate * g_audioFormatOutput.channelCount / MILLISECONDS_PER_SECOND * bitsPerSample / BITS_PER_BYTE;
    bool finishedFlag = false;
    result = audioRenderFrame(totalAudioData, tapTotalAudioData, frameSize, finishedFlag);
    OH_LOG_Print(LOG_APP, LOG_WARN, GLOBAL_RESMGR, OUTPUT_TAG, "audioEditTest renDerFrame result: %{public}d", static_cast<int>(result));
    return result;
}

OH_AudioSuite_Result startPipelineAndCheckState() {
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, OUTPUT_TAG, "audioEditTest startPipelineAndCheckState start");

    // 启动管线
    OH_AudioSuite_Result result = OH_AudioSuiteEngine_StartPipeline(g_audioSuitePipeline);
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, OUTPUT_TAG,
        "audioEditTest OH_audioSuiteEngine_StartPipeline result: %{public}d", static_cast<int>(result));
    if (result != OH_AudioSuite_Result::AUDIOSUITE_SUCCESS) {
        return result;
    }

    // 获取管线状态
    OH_AudioSuite_PipelineState pipeLineState;
    result = OH_AudioSuiteEngine_GetPipelineState(g_audioSuitePipeline, &pipeLineState);
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, OUTPUT_TAG,
        "audioEditTest OH_audioSuiteEngine_GetPipelineState result: %{public}d --- pipeLineState: %{public}d",
        static_cast<int>(result), static_cast<int>(pipeLineState));
    return result;
}

OH_AudioSuite_Result audioRenderFrame(char *totalAudioData, char *tapTotalAudioData, int32_t frameSize, bool &finishedFlag)
{
    OH_AudioDataArray* ohAudioDataArray = new OH_AudioDataArray();
    ohAudioDataArray->audioDataArray = (void**)malloc(2 * sizeof(void*));
    for (int i = 0; i < 2; i++) {
        ohAudioDataArray->audioDataArray[i] = (void*)malloc(frameSize);
    }
    ohAudioDataArray->arraySize = 2;
    ohAudioDataArray->requestFrameSize = frameSize;

    int32_t writeSize = 0;
    int32_t resultTotalSize = 0;
    int32_t tapResultTotalSize = 0;
    OH_AudioSuite_Result result = OH_AudioSuite_Result::AUDIOSUITE_SUCCESS;
    do {
        if (g_multiRenderFrameFlag) {
            result = OH_AudioSuiteEngine_MultiRenderFrame(g_audioSuitePipeline, ohAudioDataArray, &writeSize, &finishedFlag);
            logRenderResult(result, ohAudioDataArray->requestFrameSize, writeSize, finishedFlag, "OH_AudioSuiteEngine_MultiRenderFrame");
            if (result != OH_AudioSuite_Result::AUDIOSUITE_SUCCESS) {
                break;
            }
            saveBuffer(totalAudioData, resultTotalSize, ohAudioDataArray->audioDataArray[0], writeSize);
            saveBuffer(tapTotalAudioData, tapResultTotalSize, ohAudioDataArray->audioDataArray[1], writeSize);
        } else {
            char *audioData = (char *)malloc(frameSize);
            result = OH_AudioSuiteEngine_RenderFrame(g_audioSuitePipeline, audioData, frameSize, &writeSize, &finishedFlag);
            logRenderResult(result, frameSize, writeSize, finishedFlag, "OH_AudioSuiteEngine_RenderFrame");
            if (result != OH_AudioSuite_Result::AUDIOSUITE_SUCCESS) {
                free(audioData);
                break;
            }
            saveBuffer(totalAudioData, resultTotalSize, audioData, writeSize);
            free(audioData);
        }
        if (finishedFlag) {
            g_globalFinishFlag = true;
            break;
        }
    } while (!finishedFlag);
    delete ohAudioDataArray;
    AudioRenderContext context = {totalAudioData, tapTotalAudioData, frameSize, finishedFlag, resultTotalSize, tapResultTotalSize};
    updateGlobalBuffers(context);
    return result;
}

void saveBuffer(char *totalData, int32_t &totalSize, void *buffer, int32_t bufferSize)
{
    memcpy(static_cast<char *>(totalData) + totalSize, buffer, bufferSize);
    totalSize += bufferSize;
}

void logRenderResult(OH_AudioSuite_Result result, int32_t requestFrameSize, int32_t writeSize, bool finishedFlag, std::string logType)
{
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, OUTPUT_TAG,
        "audioEditTest %{public}s frameSize: %{public}d, writeSize: %{public}d"
        "finishedFlag: %{public}s, result: %{public}d",
        logType.c_str(), requestFrameSize, writeSize, (finishedFlag ? "true" : "false"), static_cast<int>(result));
}

void updateGlobalBuffers(AudioRenderContext &context)
{
    if (g_totalBuff != nullptr) {
        free(g_totalBuff);
        g_totalBuff = nullptr;
    }
    g_totalSize = context.resultTotalSize;
    g_totalBuff = (char *)malloc(g_totalSize);
    memcpy(g_totalBuff, context.totalAudioData, g_totalSize);

    if (g_multiRenderFrameFlag) {
        g_totalSize = context.tapResultTotalSize;
        g_tapTotalBuff = (char *)malloc(g_totalSize);
        g_tapDataTotalSize = g_totalSize;
        memcpy(g_tapTotalBuff, context.tapTotalAudioData, g_totalSize);
        g_multiRenderFrameFlag = false;
    }

    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, OUTPUT_TAG, "audioEditTest updateGlobalBuffers g_totalSize: %{public}d", g_totalSize);
}