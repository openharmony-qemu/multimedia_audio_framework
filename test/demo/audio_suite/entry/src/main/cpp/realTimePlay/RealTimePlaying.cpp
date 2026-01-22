/*
 * Copyright (c) 2025 Huawei Device Co., Ltd. 2025-2025. ALL rights reserved.
 */

#include "RealTimePlaying.h"
#include "hilog/log.h"
#include "../audioEffectNode/Input.h"
#include "ohaudiosuite/native_audio_suite_engine.h"
#include "ohaudio/native_audiorenderer.h"
#include "ohaudio/native_audiostreambuilder.h"
#include "./callback/RegisterCallback.h"
#include "../audioSuiteError/AudioSuiteError.h"
#include "../audioEffectNode/Output.h"
#include "../utils/Utils.h"
#include "timeline/Timeline.h"
#include "utils/Constant.h"

const int GLOBAL_RESMGR = 0xFF00;
const char *REAL_TIME_PLAYING_TAG = "[AudioEditTestApp_RealTimePlaying_cpp]";

const int CONSTANT_0 = 0;

OH_AudioRenderer *audioRenderer = nullptr;

OH_AudioStreamBuilder *rendererBuilder = nullptr;

bool g_playFinishedFlag = false;

char *g_playAudioData = (char *)malloc(g_playDataSize * 5);

int32_t g_playDataSize = 0;

bool g_isRecord = false;

char *g_playTotalAudioData = (char *)malloc(1024 * 1024 * 100);

int32_t g_playResultTotalSize = 0;

OH_AudioDataArray *g_playOhAudioDataArray = new OH_AudioDataArray();

uint32_t g_separationMode = -1;

OH_AudioSuite_Result ProcessPipeline() {
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, REAL_TIME_PLAYING_TAG, "audioEditTest ProcessPipeline start");
    // Get pipeline status
    OH_AudioSuite_PipelineState pipeLineState;
    OH_AudioSuite_Result result = OH_AudioSuiteEngine_GetPipelineState(g_audioSuitePipeline, &pipeLineState);
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, REAL_TIME_PLAYING_TAG,
                 "audioEditTest OH_AudioSuiteEngine_GetPipelineState result: %{public}d --- pipeLineState: %{public}d",
                 static_cast<int>(result), static_cast<int>(pipeLineState));
    if (result != OH_AudioSuite_Result::AUDIOSUITE_SUCCESS) {
        return result;
    }

    // Start the pipeline
    if (pipeLineState != OH_AudioSuite_PipelineState::AUDIOSUITE_PIPELINE_RUNNING) {
        result = OH_AudioSuiteEngine_StartPipeline(g_audioSuitePipeline);
        OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, REAL_TIME_PLAYING_TAG,
                     "audioEditTest OH_AudioSuiteEngine_StartPipeline result: %{public}d", static_cast<int>(result));
        if (result != OH_AudioSuite_Result::AUDIOSUITE_SUCCESS) {
            return result;
        }
    }
    return result;
}

OH_AudioSuite_Result OneRenDerFrame(int32_t audioDataSize, int32_t *writeSize) {
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, REAL_TIME_PLAYING_TAG, "audioEditTest OneRenDerFrame start");
    ProcessPipeline();
    if (audioDataSize <= CONSTANT_0) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, REAL_TIME_PLAYING_TAG,
                     "audioEditTest OH_AudioSuiteEngine_RenderFrame audioDataSize is %{public}d",
                     static_cast<int>(audioDataSize));
        return OH_AudioSuite_Result::AUDIOSUITE_ERROR_SYSTEM;
    }
    char *audioData = (char *)malloc(audioDataSize);
    if (audioData == nullptr) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, REAL_TIME_PLAYING_TAG,
                     "audioEditTest OneRenDerFrame malloc audioData failed, audioDataSize: %{public}d", audioDataSize);
        return static_cast<OH_AudioSuite_Result>(AudioSuiteResult::DEMO_ERROR_FAILD);
    }
    OH_AudioSuite_Result result =
        OH_AudioSuiteEngine_RenderFrame(g_audioSuitePipeline, audioData, audioDataSize, writeSize, &g_playFinishedFlag);
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, REAL_TIME_PLAYING_TAG,
                 "audioEditTest OH_AudioSuiteEngine_RenderFrame audioDataSize: %{public}d, writeSize:%{public}d "
                 "g_playFinishedFlag : %{public}s, result: %{public}d",
                 audioDataSize, *writeSize, (g_playFinishedFlag ? "true" : "false"), static_cast<int>(result));
    if (result != OH_AudioSuite_Result::AUDIOSUITE_SUCCESS) {
        OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, REAL_TIME_PLAYING_TAG,
                     "audioEditTest OH_AudioSuiteEngine_RenderFrame result is %{public}d", static_cast<int>(result));
        return result;
    }
    // Save the obtained buffer value each time
    g_playAudioData = (char *)malloc(*writeSize);
    if (g_playAudioData == nullptr) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, REAL_TIME_PLAYING_TAG,
                     "audioEditTest OneRenDerFrame malloc g_playAudioData failed, writeSize: %{public}d", *writeSize);
        free(audioData);
        audioData = nullptr;
        return static_cast<OH_AudioSuite_Result>(AudioSuiteResult::DEMO_ERROR_FAILD);
    }
    std::copy(audioData, audioData + *writeSize, static_cast<char *>(g_playAudioData));
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, REAL_TIME_PLAYING_TAG,
                 "audioEditTest OH_AudioSuiteEngine_RenderFrame writeSize : %{public}d, g_playFinishedFlag: %{public}s",
                 *writeSize, (g_playFinishedFlag ? "true" : "false"));
    FreeBuffer(&audioData);
    return result;
}

OH_AudioSuite_Result OneMulRenDerFrame(int32_t audioDataSize, int32_t *writeSize) {
    g_playOhAudioDataArray->audioDataArray = (void **)malloc(ARG_2 * sizeof(void *));
    if (g_playOhAudioDataArray->audioDataArray == nullptr) {
        OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, REAL_TIME_PLAYING_TAG,
                     "OH_AudioSuiteEngine_MultiRenderFrame g_playOhAudioDataArray is nullptr");
        return static_cast<OH_AudioSuite_Result>(AudioSuiteResult::DEMO_ERROR_FAILD);
    }
    for (int i = ARG_0; i < ARG_2; i++) {
        if (audioDataSize <= ARG_0) {
            return OH_AudioSuite_Result::AUDIOSUITE_ERROR_INVALID_PARAM;
        }
        g_playOhAudioDataArray->audioDataArray[i] = (void *)malloc(audioDataSize);
    }
    g_playOhAudioDataArray->arraySize = ARG_2;
    g_playOhAudioDataArray->requestFrameSize = audioDataSize;
    OH_AudioSuite_Result result = OH_AudioSuiteEngine_MultiRenderFrame(g_audioSuitePipeline, g_playOhAudioDataArray,
                                                                       writeSize, &g_playFinishedFlag);
    if (result != OH_AudioSuite_Result::AUDIOSUITE_SUCCESS) {
        OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, REAL_TIME_PLAYING_TAG,
                     "audioEditTest OH_AudioSuiteEngine_MultiRenderFrame result is %{public}d",
                     static_cast<int>(result));
        return result;
    }
    // Save the obtained buffer value each time
    g_playAudioData = (char *)malloc(*writeSize);
    if (g_separationMode == ARG_0) {
        std::copy(static_cast<char *>(g_playOhAudioDataArray->audioDataArray[ARG_0]),
                  static_cast<char *>(g_playOhAudioDataArray->audioDataArray[ARG_0]) + *writeSize,
                  static_cast<char *>(g_playAudioData));
    } else if (g_separationMode == ARG_1) {
        std::copy(static_cast<char *>(g_playOhAudioDataArray->audioDataArray[ARG_1]),
                  static_cast<char *>(g_playOhAudioDataArray->audioDataArray[ARG_1]) + *writeSize,
                  static_cast<char *>(g_playAudioData));
    }
    OH_LOG_Print(
        LOG_APP, LOG_INFO, GLOBAL_RESMGR, REAL_TIME_PLAYING_TAG,
        "audioEditTest OH_AudioSuiteEngine_MultiRenderFrame writeSize : %{public}d, g_playFinishedFlag: %{public}s",
        *writeSize, (g_playFinishedFlag ? "true" : "false"));
    return result;
}

OH_AudioData_Callback_Result PlayAudioRendererOnWriteData(OH_AudioRenderer *renderer, void *userData, void *audioData,
                                                          int32_t audioDataSize) {
    (void)userData;
    if (renderer == nullptr || audioData == nullptr) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, REAL_TIME_PLAYING_TAG,
                     "audioEditTest PlayAudioRendererOnWriteData renderer or audioData is nullptr");
        return AUDIO_DATA_CALLBACK_RESULT_INVALID;
    }
    int32_t writeSize = 0;
    if (!g_playFinishedFlag) {
        OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, REAL_TIME_PLAYING_TAG,
                     "OneRenDerFrame g_multiRenderFrameFlag: %{public}s", g_multiRenderFrameFlag ? "true" : "false");
        // If there a source separation node
        if (!g_multiRenderFrameFlag) {
            OneRenDerFrame(audioDataSize, &writeSize);
        } else {
            OneMulRenDerFrame(audioDataSize, &writeSize);
        }
        // Save the obtained buffer value each time
        if (audioDataSize != 0 && g_isRecord == true) {
            int32_t copySize = std::min(audioDataSize, writeSize);
            std::copy(g_playAudioData, g_playAudioData + copySize,
                      static_cast<char *>(g_playTotalAudioData) + g_playResultTotalSize);
            g_playResultTotalSize += copySize;
        }
    }
    // Playing audio data
    int32_t copySize = std::min(audioDataSize, writeSize);
    if (g_playAudioData != nullptr && copySize > 0) {
        std::copy(g_playAudioData, g_playAudioData + copySize, static_cast<char *>(audioData));
    }
    free(g_playAudioData);
    g_playAudioData = nullptr;
    if (g_playFinishedFlag) {
        // Stop playing
        OH_AudioRenderer_Stop(audioRenderer);
        // Stop pipeline
        ResetAllIsResetTotalWriteAudioDataSize();
        OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, REAL_TIME_PLAYING_TAG,
                     "audioEditTest PlayAudioRendererOnWriteData g_playResultTotalSize is %{public}d",
                     g_playResultTotalSize);
        CallBooleanCallback(g_playFinishedFlag);
        g_playFinishedFlag = false;
        if (g_totalBuff != nullptr) {
            free(g_totalBuff);
            g_totalBuff = nullptr;
        }
    }
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, REAL_TIME_PLAYING_TAG,
                 "audioEditTest PlayAudioRendererOnWriteData g_playResultTotalSize: %{public}d, writeSize: %{public}d",
                 g_playResultTotalSize, writeSize);
    return AUDIO_DATA_CALLBACK_RESULT_VALID;
}

void ReleaseExistingResources() {
    if (audioRenderer) {
        // Releasing a Player Instance
        OH_AudioRenderer_Release(audioRenderer);
        audioRenderer = nullptr;
    }
    if (rendererBuilder) {
        // Release Constructor
        OH_AudioStreamBuilder_Destroy(rendererBuilder);
        rendererBuilder = nullptr;
    }
}

std::vector<std::string> ParseStringArray(napi_env env, napi_value arrayValue, uint32_t trackIds_length) {
    std::vector<std::string> trackIds;
    napi_valuetype type;
    for (uint32_t i = 0; i < trackIds_length; i++) {
        napi_value element;
        napi_get_element(env, arrayValue, i, &element);
        napi_typeof(env, element, &type);
        if (type != napi_string) {
            napi_throw_type_error(env, "EINVAL", "nodeIds must contain only strings");
            return {};
        }
        std::string tempString;
        napi_status status = ParseNapiString(env, element, tempString);
        trackIds.push_back(tempString);
    }
    return trackIds;
}

napi_value ModifyRenderTrack(napi_env env, napi_callback_info info) {
    size_t argc = 2;
    napi_value *argv = new napi_value[argc];
    napi_status status = napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (status != napi_ok || argc < UINT_2) {
        napi_throw_error(env, "EINVAL", "Expected exactly 2 arguments");
        return nullptr;
    }
    napi_valuetype type;
    napi_typeof(env, argv[ARG_0], &type);
    napi_typeof(env, argv[ARG_1], &type);
    if (type != napi_object) {
        napi_throw_type_error(env, "EINVAL", "trackIds must be an object");
        delete[] argv;
        return {};
    }
    bool isArray;
    napi_is_array(env, argv[ARG_0], &isArray);
    napi_is_array(env, argv[ARG_1], &isArray);
    if (!isArray) {
        napi_throw_type_error(env, "EINVAL", "trackIds must be an array");
        delete[] argv;
        return {};
    }
    uint32_t trackIdsLength;
    napi_get_array_length(env, argv[ARG_0], &trackIdsLength);
    std::vector<std::string> trackIdsNotRender = ParseStringArray(env, argv[0], trackIdsLength);
    if (trackIdsNotRender.empty()) {
        OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, REAL_TIME_PLAYING_TAG, "Failed to parse NotRenderTrackIds");
    }
    napi_get_array_length(env, argv[ARG_1], &trackIdsLength);
    std::vector<std::string> trackIdsRender = ParseStringArray(env, argv[1], trackIdsLength);
    if (trackIdsRender.empty()) {
        OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, REAL_TIME_PLAYING_TAG, "trackIdsNotRender size %zu",
                     trackIdsNotRender.size());
    }
    const std::vector<bool> isSilents(trackIdsNotRender.size(), true);
    Timeline::getInstance().setAudioTrackSilent(trackIdsNotRender, isSilents);
    const std::vector<bool> isNotSilents(trackIdsRender.size(), false);
    Timeline::getInstance().setAudioTrackSilent(trackIdsRender, isNotSilents);
    napi_value napiValue;
    return napiValue;
}