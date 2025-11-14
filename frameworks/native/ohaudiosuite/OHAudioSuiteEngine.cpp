/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
#define LOG_TAG "OHAudioSuiteEngine"
#endif

#include <string>
#include <thread>
#include "audio_utils.h"
#include "audio_errors.h"
#include "audio_common_log.h"
#include "OHAudioSuiteEngine.h"
#include "audio_suite_manager.h"
#include "OHAudioSuiteNodeBuilder.h"
#include "audio_suite_capabilities.h"

using OHOS::AudioStandard::OHAudioSuiteEngine;
using OHOS::AudioStandard::OHAudioSuitePipeline;
using OHOS::AudioStandard::OHAudioNode;
using OHOS::AudioStandard::OHAudioSuiteNodeBuilder;

const OH_EqualizerFrequencyBandGains OH_EQUALIZER_PARAM_DEFAULT = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
const OH_EqualizerFrequencyBandGains OH_EQUALIZER_PARAM_BALLADS = {3, 5, 2, -4, 1, 2, -3, 1, 4, 5};
const OH_EqualizerFrequencyBandGains OH_EQUALIZER_PARAM_CHINESE_STYLE = {0, 0, 2, 0, 0, 4, 4, 2, 2, 5};
const OH_EqualizerFrequencyBandGains OH_EQUALIZER_PARAM_CLASSICAL = {2, 3, 2, 1, 0, 0, -5, -5, -5, -6};
const OH_EqualizerFrequencyBandGains OH_EQUALIZER_PARAM_DANCE_MUSIC = {4, 3, 2, -3, 0, 0, 5, 4, 2, 0};
const OH_EqualizerFrequencyBandGains OH_EQUALIZER_PARAM_JAZZ = {2, 0, 2, 3, 6, 5, -1, 3, 4, 4};
const OH_EqualizerFrequencyBandGains OH_EQUALIZER_PARAM_POP = {5, 2, 1, -1, -5, -5, -2, 1, 2, 4};
const OH_EqualizerFrequencyBandGains OH_EQUALIZER_PARAM_RB = {1, 4, 5, 3, -2, -2, 2, 3, 5, 5};
const OH_EqualizerFrequencyBandGains OH_EQUALIZER_PARAM_ROCK = {6, 4, 4, 2, 0, 1, 3, 3, 5, 4};

static OHAudioSuiteEngine *ConvertAudioSuiteEngine(OH_AudioSuiteEngine *audioSuiteEngine)
{
    return (OHAudioSuiteEngine *)audioSuiteEngine;
}

static OHAudioSuitePipeline *ConvertAudioSuitePipeline(OH_AudioSuitePipeline *audioSuitePipeline)
{
    return (OHAudioSuitePipeline *)audioSuitePipeline;
}

static OHAudioNode *ConvertAudioNode(OH_AudioNode *audioNode)
{
    return (OHAudioNode *)audioNode;
}

static OHAudioSuiteNodeBuilder *ConvertAudioSuitBuilder(OH_AudioNodeBuilder *builder)
{
    return (OHAudioSuiteNodeBuilder *)builder;
}

static OHOS::AudioStandard::AudioSuite::AudioDataArray *ConvertAudioDataArray(OH_AudioDataArray *audioDataArray)
{
    return (OHOS::AudioStandard::AudioSuite::AudioDataArray *)audioDataArray;
}

static OH_AudioSuite_Result ConvertError(int32_t err)
{
    if (err == OHOS::AudioStandard::SUCCESS) {
        return AUDIOSUITE_SUCCESS;
    } else if (err == OHOS::AudioStandard::ERR_INVALID_PARAM) {
        return AUDIOSUITE_ERROR_INVALID_PARAM;
    } else if (err == OHOS::AudioStandard::ERR_ILLEGAL_STATE) {
        return AUDIOSUITE_ERROR_INVALID_STATE;
    } else if (err == OHOS::AudioStandard::ERR_AUDIO_SUITE_UNSUPPORTED_FORMAT) {
        return AUDIOSUITE_ERROR_UNSUPPORTED_FORMAT;
    } else if (err == OHOS::AudioStandard::ERR_AUDIO_SUITE_ENGINE_NOT_EXIST) {
        return AUDIOSUITE_ERROR_ENGINE_NOT_EXIST;
    } else if (err == OHOS::AudioStandard::ERR_AUDIO_SUITE_PIPELINE_NOT_EXIST) {
        return AUDIOSUITE_ERROR_PIPELINE_NOT_EXIST;
    } else if (err == OHOS::AudioStandard::ERR_AUDIO_SUITE_NODE_NOT_EXIST) {
        return AUDIOSUITE_ERROR_NODE_NOT_EXIST;
    } else if (err == OHOS::AudioStandard::ERR_AUDIO_SUITE_UNSUPPORT_CONNECT) {
        return AUDIOSUITE_ERROR_UNSUPPORTED_CONNECT;
    } else if (err == OHOS::AudioStandard::ERR_NOT_SUPPORTED) {
        return AUDIOSUITE_ERROR_UNSUPPORTED_OPERATION;
    } else if (err == OHOS::AudioStandard::ERR_AUDIO_SUITE_CREATED_EXCEED_SYSTEM_LIMITS) {
        return AUDIOSUITE_ERROR_CREATED_EXCEED_SYSTEM_LIMITS;
    } else if (err == (int32_t)AUDIOSUITE_ERROR_REQUIRED_PARAMETERS_MISSING) {
        return AUDIOSUITE_ERROR_REQUIRED_PARAMETERS_MISSING;
    }
    return AUDIOSUITE_ERROR_SYSTEM;
}

OH_AudioSuite_Result OH_AudioSuiteEngine_Create(OH_AudioSuiteEngine **audioSuiteEngine)
{
    CHECK_AND_RETURN_RET_LOG(audioSuiteEngine != nullptr,
        AUDIOSUITE_ERROR_INVALID_PARAM, "Create suiteEngine audioSuiteEngine is nullptr");

    OHAudioSuiteEngine *suiteEngine = OHAudioSuiteEngine::GetInstance();
    CHECK_AND_RETURN_RET_LOG(suiteEngine != nullptr,
        AUDIOSUITE_ERROR_INVALID_STATE, "Get suiteEngine suiteEngine is nullptr");

    int32_t ret = suiteEngine->CreateEngine();
    CHECK_AND_RETURN_RET_LOG(ret == AUDIOSUITE_SUCCESS,
        ConvertError(ret), "Create suiteEngine failed, ret is %{public}d.", ret);

    *audioSuiteEngine = (OH_AudioSuiteEngine *)suiteEngine;
    return AUDIOSUITE_SUCCESS;
}

OH_AudioSuite_Result OH_AudioSuiteEngine_Destroy(OH_AudioSuiteEngine *audioSuiteEngine)
{
    CHECK_AND_RETURN_RET_LOG(audioSuiteEngine != nullptr,
        AUDIOSUITE_ERROR_INVALID_PARAM, "Destroy audioSuiteEngine is nullptr");
    OHAudioSuiteEngine *suiteEngine = ConvertAudioSuiteEngine(audioSuiteEngine);
    CHECK_AND_RETURN_RET_LOG(suiteEngine != nullptr,
        AUDIOSUITE_ERROR_INVALID_PARAM, "Destroy suiteEngine is nullptr");
    if (suiteEngine != OHAudioSuiteEngine::GetInstance()) {
        return AUDIOSUITE_ERROR_INVALID_PARAM;
    }
    int32_t error = suiteEngine->DestroyEngine();
    return ConvertError(error);
}

OH_AudioSuite_Result OH_AudioSuiteEngine_CreatePipeline(
    OH_AudioSuiteEngine *audioSuiteEngine,
    OH_AudioSuitePipeline **audioSuitePipeline, OH_AudioSuite_PipelineWorkMode workMode)
{
    CHECK_AND_RETURN_RET_LOG(audioSuiteEngine != nullptr,
        AUDIOSUITE_ERROR_INVALID_PARAM, "CreatePipeline audioSuiteEngine is nullptr");
    OHAudioSuiteEngine *suiteEngine = ConvertAudioSuiteEngine(audioSuiteEngine);
    CHECK_AND_RETURN_RET_LOG(suiteEngine != nullptr,
        AUDIOSUITE_ERROR_INVALID_PARAM, "CreatePipeline suiteEngine is nullptr");

    int32_t error = suiteEngine->CreatePipeline(audioSuitePipeline, workMode);
    return ConvertError(error);
}

OH_AudioSuite_Result OH_AudioSuiteEngine_DestroyPipeline(OH_AudioSuitePipeline *audioSuitePipeline)
{
    OHAudioSuitePipeline *pipeline = ConvertAudioSuitePipeline(audioSuitePipeline);
    CHECK_AND_RETURN_RET_LOG(pipeline != nullptr,
        AUDIOSUITE_ERROR_INVALID_PARAM, "DestroyPipeline pipeline is nullptr");

    OHAudioSuiteEngine *suiteEngine = OHAudioSuiteEngine::GetInstance();
    CHECK_AND_RETURN_RET_LOG(suiteEngine != nullptr,
        AUDIOSUITE_ERROR_ENGINE_NOT_EXIST, "DestroyPipeline suiteEngine is nullptr");
    int32_t error = suiteEngine->DestroyPipeline(pipeline);
    return ConvertError(error);
}

OH_AudioSuite_Result OH_AudioSuiteEngine_StartPipeline(OH_AudioSuitePipeline *audioSuitePipeline)
{
    OHAudioSuitePipeline *pipeline = ConvertAudioSuitePipeline(audioSuitePipeline);
    CHECK_AND_RETURN_RET_LOG(pipeline != nullptr,
        AUDIOSUITE_ERROR_INVALID_PARAM, "StartPipeline pipeline is nullptr");

    OHAudioSuiteEngine *suiteEngine = OHAudioSuiteEngine::GetInstance();
    CHECK_AND_RETURN_RET_LOG(suiteEngine != nullptr,
        AUDIOSUITE_ERROR_ENGINE_NOT_EXIST, "StartPipeline suiteEngine is nullptr");

    int32_t error = suiteEngine->StartPipeline(pipeline);
    return ConvertError(error);
}

OH_AudioSuite_Result OH_AudioSuiteEngine_StopPipeline(OH_AudioSuitePipeline *audioSuitePipeline)
{
    OHAudioSuitePipeline *pipeline = ConvertAudioSuitePipeline(audioSuitePipeline);
    CHECK_AND_RETURN_RET_LOG(pipeline != nullptr,
        AUDIOSUITE_ERROR_INVALID_PARAM, "StopPipeline pipeline is nullptr");

    OHAudioSuiteEngine *suiteEngine = OHAudioSuiteEngine::GetInstance();
    CHECK_AND_RETURN_RET_LOG(suiteEngine != nullptr,
        AUDIOSUITE_ERROR_ENGINE_NOT_EXIST, "StopPipeline suiteEngine is nullptr");

    int32_t error = suiteEngine->StopPipeline(pipeline);
    return ConvertError(error);
}

OH_AudioSuite_Result OH_AudioSuiteEngine_GetPipelineState(
    OH_AudioSuitePipeline *audioSuitePipeline, OH_AudioSuite_PipelineState *pipelineState)
{
    OHAudioSuitePipeline *pipeline = ConvertAudioSuitePipeline(audioSuitePipeline);
    CHECK_AND_RETURN_RET_LOG(pipeline != nullptr,
        AUDIOSUITE_ERROR_INVALID_PARAM, "GetPipelineState pipeline is nullptr");
    CHECK_AND_RETURN_RET_LOG(pipelineState != nullptr,
        AUDIOSUITE_ERROR_INVALID_PARAM, "GetPipelineState pipelineState is nullptr");

    OHAudioSuiteEngine *suiteEngine = OHAudioSuiteEngine::GetInstance();
    CHECK_AND_RETURN_RET_LOG(suiteEngine != nullptr,
        AUDIOSUITE_ERROR_ENGINE_NOT_EXIST, "GetPipelineState suiteEngine is nullptr");

    int32_t error = suiteEngine->GetPipelineState(pipeline, pipelineState);
    return ConvertError(error);
}

OH_AudioSuite_Result OH_AudioSuiteEngine_RenderFrame(OH_AudioSuitePipeline *audioSuitePipeline,
    void *audioData, int32_t requestFrameSize, int32_t *responseSize, bool *finishedFlag)
{
    OHAudioSuitePipeline *pipeline = ConvertAudioSuitePipeline(audioSuitePipeline);
    CHECK_AND_RETURN_RET_LOG(pipeline != nullptr,
        AUDIOSUITE_ERROR_INVALID_PARAM, "RenderFrame pipeline is nullptr");
    CHECK_AND_RETURN_RET_LOG(responseSize != nullptr,
        AUDIOSUITE_ERROR_INVALID_PARAM, "RenderFrame responseSize is nullptr");
    CHECK_AND_RETURN_RET_LOG(finishedFlag != nullptr,
        AUDIOSUITE_ERROR_INVALID_PARAM, "RenderFrame finishedFlag is nullptr");
    CHECK_AND_RETURN_RET_LOG(requestFrameSize > 0,
        AUDIOSUITE_ERROR_INVALID_PARAM, "RenderFrame requestFrameSize is zero");

    OHAudioSuiteEngine *suiteEngine = OHAudioSuiteEngine::GetInstance();
    CHECK_AND_RETURN_RET_LOG(suiteEngine != nullptr,
        AUDIOSUITE_ERROR_ENGINE_NOT_EXIST, "RenderFrame suiteEngine is nullptr");
    int32_t error = suiteEngine->RenderFrame(pipeline,
        static_cast<uint8_t *>(audioData), requestFrameSize, responseSize, finishedFlag);
    return ConvertError(error);
}

OH_AudioSuite_Result OH_AudioSuiteEngine_MultiRenderFrame(OH_AudioSuitePipeline *audioSuitePipeline,
    OH_AudioDataArray *audioDataArray, int32_t *responseSize, bool *finishedFlag)
{
    OHAudioSuitePipeline *pipeline = ConvertAudioSuitePipeline(audioSuitePipeline);
    OHOS::AudioStandard::AudioSuite::AudioDataArray *dataFrame = ConvertAudioDataArray(audioDataArray);
    CHECK_AND_RETURN_RET_LOG(pipeline != nullptr,
        AUDIOSUITE_ERROR_INVALID_PARAM, "MultiRenderFrame pipeline is nullptr");
    CHECK_AND_RETURN_RET_LOG(dataFrame != nullptr,
        AUDIOSUITE_ERROR_INVALID_PARAM, "MultiRenderFrame audioDataArray is nullptr");
    CHECK_AND_RETURN_RET_LOG(responseSize != nullptr,
        AUDIOSUITE_ERROR_INVALID_PARAM, "MultiRenderFrame responseSize is nullptr");
    CHECK_AND_RETURN_RET_LOG(finishedFlag != nullptr,
        AUDIOSUITE_ERROR_INVALID_PARAM, "MultiRenderFrame finishedFlag is nullptr");
    CHECK_AND_RETURN_RET_LOG(dataFrame->audioDataArray != nullptr,
        AUDIOSUITE_ERROR_INVALID_PARAM, "MultiRenderFrame audioDataArray is nullptr");
    CHECK_AND_RETURN_RET_LOG(dataFrame->requestFrameSize > 0,
        AUDIOSUITE_ERROR_INVALID_PARAM, "MultiRenderFrame requestFrameSize is zero");
    CHECK_AND_RETURN_RET_LOG(dataFrame->arraySize > 0,
        AUDIOSUITE_ERROR_INVALID_PARAM, "MultiRenderFrame arraySize is zero");

    OHAudioSuiteEngine *suiteEngine = OHAudioSuiteEngine::GetInstance();
    CHECK_AND_RETURN_RET_LOG(suiteEngine != nullptr,
        AUDIOSUITE_ERROR_ENGINE_NOT_EXIST, "MultiRenderFrame suiteEngine is nullptr");
    int32_t error = suiteEngine->MultiRenderFrame(pipeline,
        dataFrame, responseSize, finishedFlag);
    return ConvertError(error);
}

OH_AudioSuite_Result OH_AudioSuiteEngine_CreateNode(
    OH_AudioSuitePipeline *audioSuitePipeline, OH_AudioNodeBuilder *builder, OH_AudioNode **audioNode)
{
    OHAudioSuitePipeline *pipeline = ConvertAudioSuitePipeline(audioSuitePipeline);
    CHECK_AND_RETURN_RET_LOG(pipeline != nullptr,
        AUDIOSUITE_ERROR_INVALID_PARAM, "CreateNode pipeline is nullptr");
    OHAudioSuiteNodeBuilder  *nodeBuilder = ConvertAudioSuitBuilder(builder);
    CHECK_AND_RETURN_RET_LOG(nodeBuilder != nullptr,
        AUDIOSUITE_ERROR_INVALID_PARAM, "CreateNode builder is nullptr");

    OHAudioSuiteEngine *suiteEngine = OHAudioSuiteEngine::GetInstance();
    CHECK_AND_RETURN_RET_LOG(suiteEngine != nullptr,
        AUDIOSUITE_ERROR_ENGINE_NOT_EXIST, "CreateNode suiteEngine is nullptr");

    int32_t error = suiteEngine->CreateNode(pipeline, nodeBuilder, audioNode);
    return ConvertError(error);
}

OH_AudioSuite_Result OH_AudioSuiteEngine_DestroyNode(OH_AudioNode *audioNode)
{
    OHAudioNode *node = ConvertAudioNode(audioNode);
    CHECK_AND_RETURN_RET_LOG(node != nullptr,
        AUDIOSUITE_ERROR_INVALID_PARAM, "DestroyNode node is nullptr");

    OHAudioSuiteEngine *suiteEngine = OHAudioSuiteEngine::GetInstance();
    CHECK_AND_RETURN_RET_LOG(suiteEngine != nullptr,
        AUDIOSUITE_ERROR_ENGINE_NOT_EXIST, "DestroyNode suiteEngine is nullptr");
        
    int32_t error = suiteEngine->DestroyNode(node);
    return ConvertError(error);
}

OH_AudioSuite_Result OH_AudioSuiteEngine_GetNodeBypassStatus(
    OH_AudioNode *audioNode, bool *bypassStatus)
{
    OHAudioNode *node = ConvertAudioNode(audioNode);
    CHECK_AND_RETURN_RET_LOG(node != nullptr,
        AUDIOSUITE_ERROR_INVALID_PARAM, "GetNodeBypassStatus node is nullptr");
    CHECK_AND_RETURN_RET_LOG(bypassStatus != nullptr,
        AUDIOSUITE_ERROR_INVALID_PARAM, "GetNodeBypassStatus audioNodeEnable is nullptr");

    OHAudioSuiteEngine *suiteEngine = OHAudioSuiteEngine::GetInstance();
    CHECK_AND_RETURN_RET_LOG(suiteEngine != nullptr,
        AUDIOSUITE_ERROR_ENGINE_NOT_EXIST, "GetNodeBypassStatus suiteEngine is nullptr");

    int32_t error = suiteEngine->GetNodeBypassStatus(node, bypassStatus);
    return ConvertError(error);
}

OH_AudioSuite_Result OH_AudioSuiteEngine_BypassEffectNode(OH_AudioNode *audioNode, bool bypass)
{
    OHAudioNode *node = ConvertAudioNode(audioNode);
    CHECK_AND_RETURN_RET_LOG(node != nullptr,
        AUDIOSUITE_ERROR_INVALID_PARAM, "BypassEffectNode node is nullptr");

    OHAudioSuiteEngine *suiteEngine = OHAudioSuiteEngine::GetInstance();
    CHECK_AND_RETURN_RET_LOG(suiteEngine != nullptr,
        AUDIOSUITE_ERROR_ENGINE_NOT_EXIST, "DestroyNode suiteEngine is nullptr");

    int32_t error = suiteEngine->BypassEffectNode(node, bypass);
    return ConvertError(error);
}

OH_AudioSuite_Result OH_AudioSuiteEngine_SetAudioFormat(OH_AudioNode *audioNode, OH_AudioFormat *audioFormat)
{
    OHAudioNode *node = ConvertAudioNode(audioNode);
    CHECK_AND_RETURN_RET_LOG(node != nullptr,
        AUDIOSUITE_ERROR_INVALID_PARAM, "SetAudioFormat node is nullptr");
    CHECK_AND_RETURN_RET_LOG(audioFormat != nullptr,
        AUDIOSUITE_ERROR_INVALID_PARAM, "SetAudioFormat audioFormat is nullptr");

    OHAudioSuiteEngine *suiteEngine = OHAudioSuiteEngine::GetInstance();
    CHECK_AND_RETURN_RET_LOG(suiteEngine != nullptr,
        AUDIOSUITE_ERROR_ENGINE_NOT_EXIST, "SetAudioFormat suiteEngine is nullptr");

    int32_t error = suiteEngine->SetAudioFormat(node, audioFormat);
    return ConvertError(error);
}

OH_AudioSuite_Result OH_AudioSuiteEngine_ConnectNodes(
    OH_AudioNode* sourceAudioNode, OH_AudioNode* destAudioNode)
{
    OHAudioNode *srcNode = ConvertAudioNode(sourceAudioNode);
    CHECK_AND_RETURN_RET_LOG(srcNode != nullptr,
        AUDIOSUITE_ERROR_INVALID_PARAM, "ConnectNodes srcNode is nullptr");
    OHAudioNode *destNode = ConvertAudioNode(destAudioNode);
    CHECK_AND_RETURN_RET_LOG(destNode != nullptr,
        AUDIOSUITE_ERROR_INVALID_PARAM, "ConnectNodes destNode is nullptr");

    OHAudioSuiteEngine *suiteEngine = OHAudioSuiteEngine::GetInstance();
    CHECK_AND_RETURN_RET_LOG(suiteEngine != nullptr,
        AUDIOSUITE_ERROR_ENGINE_NOT_EXIST, "ConnectNodes suiteEngine is nullptr");

    int32_t error = suiteEngine->ConnectNodes(srcNode, destNode);
    return ConvertError(error);
}

OH_AudioSuite_Result OH_AudioSuiteEngine_DisconnectNodes(OH_AudioNode *sourceAudioNode, OH_AudioNode *destAudioNode)
{
    OHAudioNode *srcNode = ConvertAudioNode(sourceAudioNode);
    CHECK_AND_RETURN_RET_LOG(srcNode != nullptr,
        AUDIOSUITE_ERROR_INVALID_PARAM, "DisConnectNodes srcNode is nullptr");
    OHAudioNode *destNode = ConvertAudioNode(destAudioNode);
    CHECK_AND_RETURN_RET_LOG(destNode != nullptr,
        AUDIOSUITE_ERROR_INVALID_PARAM, "DisConnectNodes destNode is nullptr");

    OHAudioSuiteEngine *suiteEngine = OHAudioSuiteEngine::GetInstance();
    CHECK_AND_RETURN_RET_LOG(suiteEngine != nullptr,
        AUDIOSUITE_ERROR_ENGINE_NOT_EXIST, "DisConnectNodes suiteEngine is nullptr");

    int32_t error = suiteEngine->DisConnectNodes(srcNode, destNode);
    return ConvertError(error);
}

OH_AudioSuite_Result OH_AudioSuiteEngine_SetEqualizerFrequencyBandGains(
    OH_AudioNode *audioNode, OH_EqualizerFrequencyBandGains frequencyBandGains)
{
    OHAudioNode *node = ConvertAudioNode(audioNode);
    CHECK_AND_RETURN_RET_LOG(node != nullptr, AUDIOSUITE_ERROR_INVALID_PARAM,
        "SetEqualizerFrequencyBandGains node is nullptr");
    OHAudioSuiteEngine *suiteEngine = OHAudioSuiteEngine::GetInstance();
    CHECK_AND_RETURN_RET_LOG(suiteEngine != nullptr,
        AUDIOSUITE_ERROR_ENGINE_NOT_EXIST, "SetEqualizerFrequencyBandGains suiteEngine is nullptr");

    int32_t error = suiteEngine->SetEqualizerFrequencyBandGains(node, frequencyBandGains);
    return ConvertError(error);
}

OH_AudioSuite_Result OH_AudioSuiteEngine_SetSoundFieldType(OH_AudioNode *audioNode, OH_SoundFieldType soundFieldType)
{
    OHAudioNode *node = ConvertAudioNode(audioNode);
    CHECK_AND_RETURN_RET_LOG(node != nullptr, AUDIOSUITE_ERROR_INVALID_PARAM, "SetSoundFieldType node is nullptr");
    OHAudioSuiteEngine *suiteEngine = OHAudioSuiteEngine::GetInstance();
    CHECK_AND_RETURN_RET_LOG(suiteEngine != nullptr,
        AUDIOSUITE_ERROR_ENGINE_NOT_EXIST, "SetSoundFieldType suiteEngine is nullptr");

    int32_t error = suiteEngine->SetSoundFieldType(node, soundFieldType);
    return ConvertError(error);
}

OH_AudioSuite_Result OH_AudioSuiteEngine_SetEnvironmentType(OH_AudioNode *audioNode, OH_EnvironmentType environmentType)
{
    OHAudioNode *node = ConvertAudioNode(audioNode);
    CHECK_AND_RETURN_RET_LOG(node != nullptr, AUDIOSUITE_ERROR_INVALID_PARAM, "SetEnvironmentType node is nullptr");
    OHAudioSuiteEngine *suiteEngine = OHAudioSuiteEngine::GetInstance();
    CHECK_AND_RETURN_RET_LOG(suiteEngine != nullptr,
        AUDIOSUITE_ERROR_ENGINE_NOT_EXIST, "SetEnvironmentType suiteEngine is nullptr");

    int32_t error = suiteEngine->SetEnvironmentType(node, environmentType);
    return ConvertError(error);
}

OH_AudioSuite_Result OH_AudioSuiteEngine_SetVoiceBeautifierType(
    OH_AudioNode *audioNode, OH_VoiceBeautifierType voiceBeautifierType)
{
    OHAudioNode *node = ConvertAudioNode(audioNode);
    CHECK_AND_RETURN_RET_LOG(node != nullptr, AUDIOSUITE_ERROR_INVALID_PARAM,
        "SetVoiceBeautifierType node is nullptr");
    OHAudioSuiteEngine *suiteEngine = OHAudioSuiteEngine::GetInstance();
    CHECK_AND_RETURN_RET_LOG(suiteEngine != nullptr,
        AUDIOSUITE_ERROR_ENGINE_NOT_EXIST, "SetVoiceBeautifierType suiteEngine is nullptr");

    int32_t error = suiteEngine->SetVoiceBeautifierType(node, voiceBeautifierType);
    return ConvertError(error);
}

OH_AudioSuite_Result OH_AudioSuiteEngine_GetEnvironmentType(
    OH_AudioNode *audioNode, OH_EnvironmentType *environmentType)
{
    OHAudioNode *node = ConvertAudioNode(audioNode);
    CHECK_AND_RETURN_RET_LOG(node != nullptr,
        AUDIOSUITE_ERROR_INVALID_PARAM, "GetEnvironmentType node is nullptr");
    CHECK_AND_RETURN_RET_LOG(environmentType != nullptr,
        AUDIOSUITE_ERROR_INVALID_PARAM, "GetEnvironmentType environmentType is nullptr");

    OHAudioSuiteEngine *suiteEngine = OHAudioSuiteEngine::GetInstance();
    CHECK_AND_RETURN_RET_LOG(suiteEngine != nullptr,
        AUDIOSUITE_ERROR_ENGINE_NOT_EXIST, "GetEnvironmentType suiteEngine is nullptr");

    int32_t error = suiteEngine->GetEnvironmentType(node, environmentType);
    return ConvertError(error);
}

OH_AudioSuite_Result OH_AudioSuiteEngine_GetSoundFieldType(
    OH_AudioNode *audioNode, OH_SoundFieldType *soundFieldType)
{
    OHAudioNode *node = ConvertAudioNode(audioNode);
    CHECK_AND_RETURN_RET_LOG(node != nullptr,
        AUDIOSUITE_ERROR_INVALID_PARAM, "GetSoundFiledType node is nullptr");
    CHECK_AND_RETURN_RET_LOG(soundFieldType != nullptr,
        AUDIOSUITE_ERROR_INVALID_PARAM, "GetSoundFiledType soundFieldType is nullptr");

    OHAudioSuiteEngine *suiteEngine = OHAudioSuiteEngine::GetInstance();
    CHECK_AND_RETURN_RET_LOG(suiteEngine != nullptr,
        AUDIOSUITE_ERROR_ENGINE_NOT_EXIST, "GetSoundFiledType suiteEngine is nullptr");

    int32_t error = suiteEngine->GetSoundFiledType(node, soundFieldType);
    return ConvertError(error);
}

OH_AudioSuite_Result OH_AudioSuiteEngine_GetEqualizerFrequencyBandGains(
    OH_AudioNode *audioNode, OH_EqualizerFrequencyBandGains *frequencyBandGains)
{
    OHAudioNode *node = ConvertAudioNode(audioNode);
    CHECK_AND_RETURN_RET_LOG(node != nullptr,
        AUDIOSUITE_ERROR_INVALID_PARAM, "GetEqualizerFrequencyBandGains node is nullptr");
    CHECK_AND_RETURN_RET_LOG(frequencyBandGains != nullptr,
        AUDIOSUITE_ERROR_INVALID_PARAM, "GetEqualizerFrequencyBandGains frequencyBandGains is nullptr");

    OHAudioSuiteEngine *suiteEngine = OHAudioSuiteEngine::GetInstance();
    CHECK_AND_RETURN_RET_LOG(suiteEngine != nullptr,
        AUDIOSUITE_ERROR_ENGINE_NOT_EXIST, "GetEqualizerFrequencyBandGains suiteEngine is nullptr");

    int32_t error = suiteEngine->GetEqualizerFrequencyBandGains(node, frequencyBandGains);
    return ConvertError(error);
}

OH_AudioSuite_Result OH_AudioSuiteEngine_GetVoiceBeautifierType(
    OH_AudioNode *audioNode, OH_VoiceBeautifierType *voiceBeautifierType)
{
    OHAudioNode *node = ConvertAudioNode(audioNode);
    CHECK_AND_RETURN_RET_LOG(node != nullptr,
        AUDIOSUITE_ERROR_INVALID_PARAM, "GetVoiceBeautifierType node is nullptr");
    CHECK_AND_RETURN_RET_LOG(voiceBeautifierType != nullptr,
        AUDIOSUITE_ERROR_INVALID_PARAM, "GetVoiceBeautifierType voiceBeautifierType is nullptr");

    OHAudioSuiteEngine *suiteEngine = OHAudioSuiteEngine::GetInstance();
    CHECK_AND_RETURN_RET_LOG(suiteEngine != nullptr,
        AUDIOSUITE_ERROR_ENGINE_NOT_EXIST, "GetVoiceBeautifierType suiteEngine is nullptr");

    int32_t error = suiteEngine->GetVoiceBeautifierType(node, voiceBeautifierType);
    return ConvertError(error);
}

OH_AudioSuite_Result OH_AudioSuiteEngine_IsNodeTypeSupported(OH_AudioNode_Type nodeType, bool *isSupported)
{
    AUDIO_INFO_LOG("IsNodeTypeSupported enter.");
    using namespace OHOS::AudioStandard::AudioSuite;
    CHECK_AND_RETURN_RET_LOG(
        isSupported != nullptr, AUDIOSUITE_ERROR_INVALID_PARAM, "isSupported is nullptr.");
    AudioSuiteCapabilities &audioSuiteCapabilities = AudioSuiteCapabilities::getInstance();
    int32_t error = audioSuiteCapabilities.IsNodeTypeSupported(static_cast<AudioNodeType>(nodeType), isSupported);
    AUDIO_INFO_LOG("IsNodeTypeSupported leave with code: %{public}d.", error);
    return ConvertError(error);
}

namespace OHOS {
namespace AudioStandard {

using namespace OHOS::AudioStandard::AudioSuite;

static constexpr int32_t EQ_FREQUENCY_BAND_GAINS_MIN = -10;
static constexpr int32_t EQ_FREQUENCY_BAND_GAINS_MAX = 10;

int32_t OHSuiteInputNodeRequestDataCallBack::OnRequestDataCallBack(
    void *audioData, int32_t audioDataSize, bool *finished)
{
    CHECK_AND_RETURN_RET_LOG(callback_ != nullptr, 0, "OnRequestDataCallBack callback is nullptr");
    CHECK_AND_RETURN_RET_LOG(audioNode_ != nullptr, 0, "OnRequestDataCallBack OH_audioNode is nullptr");

    return callback_(audioNode_, userData_, audioData, audioDataSize, finished);
}

OHAudioSuiteEngine::~OHAudioSuiteEngine()
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    std::unordered_set<OHAudioSuitePipeline*> tempPipelines(pipelines_.begin(), pipelines_.end());
    for (auto pipeline : tempPipelines) {
        RemovePipeline(pipeline);
    }
    pipelines_.clear();
}

OHAudioSuiteEngine *OHAudioSuiteEngine::GetInstance()
{
    static OHAudioSuiteEngine manager;
    return &manager;
}

int32_t OHAudioSuiteEngine::CreateEngine()
{
    return IAudioSuiteManager::GetAudioSuiteManager().Init();
}

int32_t OHAudioSuiteEngine::DestroyEngine()
{
    int32_t ret = IAudioSuiteManager::GetAudioSuiteManager().DeInit();
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "DestroyEngine DeInit failed, ret = %{public}d.", ret);
    return ret;
}

int32_t OHAudioSuiteEngine::CreatePipeline(
    OH_AudioSuitePipeline **audioSuitePipeline, OH_AudioSuite_PipelineWorkMode ohWorkMode)
{
    CHECK_AND_RETURN_RET_LOG(audioSuitePipeline != nullptr,
        ERR_INVALID_PARAM, "CreatePipeline pipeline is nullptr");

    uint32_t pipelineId = INVALID_PIPELINE_ID;
    PipelineWorkMode workMode = static_cast<PipelineWorkMode>(ohWorkMode);
    int32_t ret = IAudioSuiteManager::GetAudioSuiteManager().CreatePipeline(pipelineId, workMode);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "CreatePipeline failed, ret = %{public}d.", ret);
    CHECK_AND_RETURN_RET_LOG(pipelineId != INVALID_PIPELINE_ID, ret, "CreatePipeline failed, pipelineId invailed");

    OHAudioSuitePipeline *audioPipeline = new OHAudioSuitePipeline(pipelineId);
    CHECK_AND_RETURN_RET_LOG(audioPipeline != nullptr, ERR_MEMORY_ALLOC_FAILED,
        "CreatePipeline pipeline failed, malloc failed.");
    *audioSuitePipeline = (OH_AudioSuitePipeline *)audioPipeline;
    AddPipeline(audioPipeline);
    return SUCCESS;
}

int32_t OHAudioSuiteEngine::DestroyPipeline(OHAudioSuitePipeline *audioPipeline)
{
    CHECK_AND_RETURN_RET_LOG(audioPipeline != nullptr, ERR_INVALID_PARAM,
        "DestroyPipeline failed, audioPipeline is nullptr.");
    CHECK_AND_RETURN_RET_LOG(IsPipelineExists(audioPipeline), ERR_AUDIO_SUITE_PIPELINE_NOT_EXIST,
        "OHAudioSuiteEngine::The pipeline does not exist.");
    uint32_t pipelineId = audioPipeline->GetPipelineId();
    int32_t ret = IAudioSuiteManager::GetAudioSuiteManager().DestroyPipeline(pipelineId);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "DestroyPipeline failed, ret = %{public}d.", ret);
    RemovePipeline(audioPipeline);
    return ret;
}

int32_t OHAudioSuiteEngine::StartPipeline(OHAudioSuitePipeline *audioPipeline)
{
    CHECK_AND_RETURN_RET_LOG(audioPipeline != nullptr, ERR_INVALID_PARAM,
        "StartPipeline failed, audioPipeline is nullptr.");

    uint32_t pipelineId = audioPipeline->GetPipelineId();
    int32_t ret = IAudioSuiteManager::GetAudioSuiteManager().StartPipeline(pipelineId);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "StartPipeline failed, ret = %{public}d.", ret);
    return ret;
}

int32_t OHAudioSuiteEngine::StopPipeline(OHAudioSuitePipeline *audioPipeline)
{
    CHECK_AND_RETURN_RET_LOG(audioPipeline != nullptr, ERR_INVALID_PARAM,
        "StopPipeline failed, audioPipeline is nullptr.");

    uint32_t pipelineId = audioPipeline->GetPipelineId();
    int32_t ret = IAudioSuiteManager::GetAudioSuiteManager().StopPipeline(pipelineId);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "StopPipeline failed, ret = %{public}d.", ret);
    return ret;
}

int32_t OHAudioSuiteEngine::GetPipelineState(OHAudioSuitePipeline *audioPipeline, OH_AudioSuite_PipelineState *state)
{
    CHECK_AND_RETURN_RET_LOG(audioPipeline != nullptr, ERR_INVALID_PARAM,
        "GetPipelineState failed, audioPipeline is nullptr.");
    CHECK_AND_RETURN_RET_LOG(state != nullptr, ERR_INVALID_PARAM, "GetPipelineState failed, state is nullptr.");

    uint32_t pipelineId = audioPipeline->GetPipelineId();
    AudioSuitePipelineState pipelineState = PIPELINE_STOPPED;
    int32_t ret = IAudioSuiteManager::GetAudioSuiteManager().GetPipelineState(pipelineId, pipelineState);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "GetPipelineState failed, ret = %{public}d.", ret);

    *state = static_cast<OH_AudioSuite_PipelineState>(pipelineState);
    return SUCCESS;
}

int32_t OHAudioSuiteEngine::RenderFrame(OHAudioSuitePipeline *audioPipeline,
    uint8_t *audioData, int32_t requestFrameSize, int32_t *responseSize, bool *finishedFlag)
{
    Trace trace("OHAudioSuiteEngine::RenderFrame Start");
    CHECK_AND_RETURN_RET_LOG(audioPipeline != nullptr, ERR_INVALID_PARAM,
        "RenderFrame failed, audioPipeline is nullptr.");

    uint32_t pipelineId = audioPipeline->GetPipelineId();

    AUDIO_INFO_LOG("OHAudioSuiteEngine::RenderFrame enter start.");
    int32_t ret = IAudioSuiteManager::GetAudioSuiteManager().RenderFrame(
        pipelineId, audioData, requestFrameSize, responseSize, finishedFlag);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "RenderFrame failed, ret = %{public}d.", ret);
    trace.End();
    return ret;
}

int32_t OHAudioSuiteEngine::MultiRenderFrame(OHAudioSuitePipeline *audioPipeline,
    AudioSuite::AudioDataArray *audioDataArray, int32_t *responseSize, bool *finishedFlag)
{
    Trace trace("OHAudioSuiteEngine::MultiRenderFrame Start");
    CHECK_AND_RETURN_RET_LOG(audioPipeline != nullptr, ERR_INVALID_PARAM,
        "MultiRenderFrame failed, audioPipeline is nullptr.");

    uint32_t pipelineId = audioPipeline->GetPipelineId();

    AUDIO_INFO_LOG("OHAudioSuiteEngine::MultiRenderFrame enter start.");
    int32_t ret = IAudioSuiteManager::GetAudioSuiteManager().MultiRenderFrame(
        pipelineId, audioDataArray, responseSize, finishedFlag);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "MultiRenderFrame failed, ret = %{public}d.", ret);
    trace.End();
    return ret;
}

int32_t OHAudioSuiteEngine::CreateNode(
    OHAudioSuitePipeline *audioSuitePipeline, OHAudioSuiteNodeBuilder *builder, OH_AudioNode **audioNode)
{
    CHECK_AND_RETURN_RET_LOG(audioSuitePipeline != nullptr, ERR_INVALID_PARAM,
        "CreateNode failed, audioSuitePipeline is nullptr.");
    CHECK_AND_RETURN_RET_LOG(builder != nullptr, ERR_INVALID_PARAM, "CreateNode failed, builder is nullptr.");
    CHECK_AND_RETURN_RET_LOG(audioNode != nullptr, ERR_INVALID_PARAM, "CreateNode audioNode is nullptr");

    uint32_t pipelineId = audioSuitePipeline->GetPipelineId();
    AudioNodeBuilder nodeCfg;
    nodeCfg.nodeType = builder->GetNodeType();
    if (builder->IsSetFormat()) {
        nodeCfg.nodeFormat = builder->GetNodeFormat();
    }

    if (builder->GetNodeType() == NODE_TYPE_INPUT) {
        CHECK_AND_RETURN_RET_LOG(builder->IsSetRequestDataCallback() && builder->IsSetFormat(),
            static_cast<int32_t>(AUDIOSUITE_ERROR_REQUIRED_PARAMETERS_MISSING),
            "Create input Node must set RequestDataCallback and audio format.");
    } else if (builder->GetNodeType() == NODE_TYPE_OUTPUT) {
        CHECK_AND_RETURN_RET_LOG(builder->IsSetFormat(),
            static_cast<int32_t>(AUDIOSUITE_ERROR_REQUIRED_PARAMETERS_MISSING),
            "Create output Node, must set aduio format.");
        CHECK_AND_RETURN_RET_LOG(!builder->IsSetRequestDataCallback(), ERR_NOT_SUPPORTED,
            "Create output Node, can not set RequestDataCallback.");
    } else {
        CHECK_AND_RETURN_RET_LOG(!builder->IsSetRequestDataCallback() && !builder->IsSetFormat(), ERR_NOT_SUPPORTED,
            "Create effect Node, can not set RequestDataCallback and format.");
    }

    uint32_t nodeId = INVALID_NODE_ID;
    int32_t ret = IAudioSuiteManager::GetAudioSuiteManager().CreateNode(pipelineId, nodeCfg, nodeId);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "CreateNode failed, ret = %{public}d.", ret);
    CHECK_AND_RETURN_RET_LOG(nodeId != INVALID_PIPELINE_ID, ERR_OPERATION_FAILED, "CreateNode failed, nodeId invail");

    if (builder->IsSetRequestDataCallback()) {
        std::shared_ptr<OHSuiteInputNodeRequestDataCallBack> callback =
            std::make_shared<OHSuiteInputNodeRequestDataCallBack>(reinterpret_cast<OH_AudioNode *>(audioNode),
                builder->GetRequestDataCallback(), builder->GetCallBackUserData());
        ret = IAudioSuiteManager::GetAudioSuiteManager().SetRequestDataCallback(nodeId, callback);
        if (ret != SUCCESS) {
            AUDIO_ERR_LOG("CreateNode SetRequestDataCallback failed, ret = %{public}d.", ret);
            IAudioSuiteManager::GetAudioSuiteManager().DestroyNode(nodeId);
            return ret;
        }
    }

    OHAudioNode *node = new OHAudioNode(nodeId, nodeCfg.nodeType);
    if (node == nullptr) {
        AUDIO_ERR_LOG("CreateNode failed, malloc failed.");
        IAudioSuiteManager::GetAudioSuiteManager().DestroyNode(nodeId);
        return ERR_MEMORY_ALLOC_FAILED;
    }

    *audioNode = (OH_AudioNode *)node;
    audioSuitePipeline->AddNode(node);
    return SUCCESS;
}

int32_t OHAudioSuiteEngine::DestroyNode(OHAudioNode *node)
{
    CHECK_AND_RETURN_RET_LOG(node != nullptr, ERR_INVALID_PARAM, "DestroyNode failed, node is nullptr.");
    CHECK_AND_RETURN_RET_LOG(IsNodeExists(node), ERR_AUDIO_SUITE_NODE_NOT_EXIST,
                             "OHAudioSuiteEngine::The node does not exist.");
    uint32_t nodeId = node->GetNodeId();
    int32_t ret = IAudioSuiteManager::GetAudioSuiteManager().DestroyNode(nodeId);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "DestroyNode failed, ret = %{public}d.", ret);

    RemoveNode(node);
    return ret;
}

int32_t OHAudioSuiteEngine::GetNodeBypassStatus(OHAudioNode *node, bool *bypassStatus)
{
    CHECK_AND_RETURN_RET_LOG(node != nullptr, ERR_INVALID_PARAM, "GetNodeBypassStatus failed, node is nullptr.");
    CHECK_AND_RETURN_RET_LOG(bypassStatus != nullptr, ERR_INVALID_PARAM,
        "GetNodeBypassStatus failed, bypassStatus is nullptr.");
    CHECK_AND_RETURN_RET_LOG((node->GetNodeType() != NODE_TYPE_INPUT) && (node->GetNodeType() != NODE_TYPE_OUTPUT),
        ERR_NOT_SUPPORTED, "GetNodeBypassStatus failed, node type %{public}d not support option.",
        static_cast<int32_t>(node->GetNodeType()));
    uint32_t nodeId = node->GetNodeId();

    bool bypass = false;
    int32_t ret = IAudioSuiteManager::GetAudioSuiteManager().GetNodeBypassStatus(nodeId, bypass);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "GetNodeBypassStatus failed, ret = %{public}d.", ret);

    *bypassStatus = bypass;
    return ret;
}

int32_t OHAudioSuiteEngine::BypassEffectNode(OHAudioNode *node, bool bypass)
{
    CHECK_AND_RETURN_RET_LOG(node != nullptr, ERR_INVALID_PARAM, "EnableNode failed, node is nullptr.");
    CHECK_AND_RETURN_RET_LOG((node->GetNodeType() != NODE_TYPE_INPUT) && (node->GetNodeType() != NODE_TYPE_OUTPUT),
        ERR_NOT_SUPPORTED, "BypassEffectNode failed, enable type %{public}d not support option.",
        static_cast<int32_t>(node->GetNodeType()));
    uint32_t nodeId = node->GetNodeId();

    int32_t ret = IAudioSuiteManager::GetAudioSuiteManager().BypassEffectNode(nodeId, bypass);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "BypassEffectNode failed, ret = %{public}d.", ret);
    return ret;
}

int32_t OHAudioSuiteEngine::SetAudioFormat(OHAudioNode *node, OH_AudioFormat *audioFormat)
{
    CHECK_AND_RETURN_RET_LOG(node != nullptr, ERR_INVALID_PARAM, "SetAudioFormat failed, node is nullptr.");
    CHECK_AND_RETURN_RET_LOG(audioFormat != nullptr, ERR_INVALID_PARAM,
        "SetAudioFormat failed, audioFormat is nullptr.");
    CHECK_AND_RETURN_RET_LOG((node->GetNodeType() == NODE_TYPE_INPUT) || (node->GetNodeType() == NODE_TYPE_OUTPUT),
        ERR_NOT_SUPPORTED, "SetAudioFormat failed, enable type %{public}d not support.",
        static_cast<int32_t>(node->GetNodeType()));
    CHECK_AND_RETURN_RET(CheckAudioFormat(*audioFormat), AUDIOSUITE_ERROR_UNSUPPORTED_FORMAT);

    uint32_t nodeId = node->GetNodeId();
    AudioFormat format;
    format.audioChannelInfo.channelLayout = static_cast<AudioChannelLayout>(audioFormat->channelLayout);
    format.audioChannelInfo.numChannels = audioFormat->channelCount;
    format.encodingType = static_cast<AudioStreamEncodingType>(audioFormat->encodingType);
    format.format = static_cast<AudioSampleFormat>(audioFormat->sampleFormat);
    format.rate =  static_cast<AudioSamplingRate>(audioFormat->samplingRate);
    int32_t ret = IAudioSuiteManager::GetAudioSuiteManager().SetAudioFormat(nodeId, format);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "SetAudioFormat failed, ret = %{public}d.", ret);
    return ret;
}

int32_t OHAudioSuiteEngine::ConnectNodes(OHAudioNode *srcNode, OHAudioNode *destNode)
{
    CHECK_AND_RETURN_RET_LOG(srcNode != nullptr, ERR_INVALID_PARAM, "ConnectNodes failed, srcNode is nullptr.");
    CHECK_AND_RETURN_RET_LOG(destNode != nullptr, ERR_INVALID_PARAM, "ConnectNodes failed, srcNode is nullptr.");
    uint32_t srcNodeId = srcNode->GetNodeId();
    uint32_t destNodeId = destNode->GetNodeId();
    int32_t ret = IAudioSuiteManager::GetAudioSuiteManager().ConnectNodes(srcNodeId, destNodeId);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "ConnectNodes failed, ret = %{public}d.", ret);
    return ret;
}

int32_t OHAudioSuiteEngine::DisConnectNodes(OHAudioNode *srcNode, OHAudioNode *destNode)
{
    CHECK_AND_RETURN_RET_LOG(srcNode != nullptr, ERR_INVALID_PARAM, "DisConnectNodes failed, srcNode is nullptr.");
    CHECK_AND_RETURN_RET_LOG(destNode != nullptr, ERR_INVALID_PARAM, "DisConnectNodes failed, srcNode is nullptr.");
    uint32_t srcNodeId = srcNode->GetNodeId();
    uint32_t destNodeId = destNode->GetNodeId();
    int32_t ret = IAudioSuiteManager::GetAudioSuiteManager().DisConnectNodes(srcNodeId, destNodeId);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "DisConnectNodes failed, ret = %{public}d.", ret);
    return ret;
}

int32_t OHAudioSuiteEngine::SetEqualizerFrequencyBandGains(
    OHAudioNode *node, OH_EqualizerFrequencyBandGains frequencyBandGains)
{
    CHECK_AND_RETURN_RET_LOG(node != nullptr, ERR_INVALID_PARAM,
        "SetEqualizerFrequencyBandGains failed, node is nullptr.");
    CHECK_AND_RETURN_RET_LOG(IsNodeExists(node),
        ERR_AUDIO_SUITE_NODE_NOT_EXIST, "SetEqualizerFrequencyBandGains node is not exist");
    CHECK_AND_RETURN_RET_LOG(node->GetNodeType() == NODE_TYPE_EQUALIZER, ERR_NOT_SUPPORTED,
        "SetEqualizerFrequencyBandGains failed, node type = %d{public}d must is EQUALIZER type.",
        static_cast<int32_t>(node->GetNodeType()));

    uint32_t nodeId = node->GetNodeId();
    AudioEqualizerFrequencyBandGains audioGains;
    size_t ohGainsNum = sizeof(frequencyBandGains.gains) / sizeof(frequencyBandGains.gains[0]);
    size_t gainsNum = sizeof(audioGains.gains) / sizeof(audioGains.gains[0]);

    for (uint32_t idx = 0; idx < (ohGainsNum < gainsNum ? ohGainsNum : gainsNum); idx++) {
        audioGains.gains[idx] = frequencyBandGains.gains[idx];
        CHECK_AND_RETURN_RET_LOG((audioGains.gains[idx] >= EQ_FREQUENCY_BAND_GAINS_MIN) &&
            (audioGains.gains[idx] <= EQ_FREQUENCY_BAND_GAINS_MAX), ERR_INVALID_PARAM,
            "SetEqualizerFrequencyBandGains failed, input value %{public}d not in -10, 10.", audioGains.gains[idx]);
    }

    int32_t ret = IAudioSuiteManager::GetAudioSuiteManager().SetEqualizerFrequencyBandGains(nodeId, audioGains);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "SetEqualizerFrequencyBandGains failed, ret = %{public}d.", ret);
    return ret;
}

int32_t OHAudioSuiteEngine::SetSoundFieldType(
    OHAudioNode *node, OH_SoundFieldType soundFieldType)
{
    CHECK_AND_RETURN_RET_LOG(node != nullptr, ERR_INVALID_PARAM,
        "SetSoundFieldType failed, node is nullptr.");
    CHECK_AND_RETURN_RET_LOG(node->GetNodeType() == NODE_TYPE_SOUND_FIELD, ERR_NOT_SUPPORTED, "SetSoundFieldType "
        "failed, node type = %d{public}d must is SOUND_FIELD type.", static_cast<int32_t>(node->GetNodeType()));

    uint32_t nodeId = node->GetNodeId();
    int32_t ret = IAudioSuiteManager::GetAudioSuiteManager().SetSoundFieldType(
        nodeId, static_cast<SoundFieldType>(soundFieldType));
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "SetSoundFieldType failed, ret = %{public}d.", ret);
    return ret;
}

int32_t OHAudioSuiteEngine::SetEnvironmentType(
    OHAudioNode *node, OH_EnvironmentType environmentType)
{
    CHECK_AND_RETURN_RET_LOG(node != nullptr, ERR_INVALID_PARAM, "SetEnvironmentType failed, node is nullptr.");
    CHECK_AND_RETURN_RET_LOG(node->GetNodeType() == NODE_TYPE_ENVIRONMENT_EFFECT, ERR_NOT_SUPPORTED,
        "SetEnvironmentType failed, node type = %d{public}d must is ENVIRONMENT_EFFECT type.",
        static_cast<int32_t>(node->GetNodeType()));

    uint32_t nodeId = node->GetNodeId();
    int32_t ret = IAudioSuiteManager::GetAudioSuiteManager().SetEnvironmentType(
        nodeId, static_cast<EnvironmentType>(environmentType));
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "SetEnvironmentType failed, ret = %{public}d.", ret);
    return ret;
}

int32_t OHAudioSuiteEngine::SetVoiceBeautifierType(
    OHAudioNode *node, OH_VoiceBeautifierType voiceBeautifierType)
{
    CHECK_AND_RETURN_RET_LOG(node != nullptr, ERR_INVALID_PARAM, "SetVoiceBeautifierType failed, node is nullptr.");
    CHECK_AND_RETURN_RET_LOG(node->GetNodeType() == NODE_TYPE_VOICE_BEAUTIFIER, ERR_NOT_SUPPORTED,
        "SetVoiceBeautifierType failed, node type = %d{public}d must is VOICE_BEAUTIFIER type.",
        static_cast<int32_t>(node->GetNodeType()));

    uint32_t nodeId = node->GetNodeId();
    int32_t ret = IAudioSuiteManager::GetAudioSuiteManager().SetVoiceBeautifierType(
        nodeId, static_cast<VoiceBeautifierType>(voiceBeautifierType));
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "SetVoiceBeautifierType failed, ret = %{public}d.", ret);
    return ret;
}

int32_t OHAudioSuiteEngine::GetEnvironmentType(OHAudioNode *node, OH_EnvironmentType *environmentType)
{
    CHECK_AND_RETURN_RET_LOG(node != nullptr, ERR_INVALID_PARAM, "GetEnvironmentType failed, node is nullptr.");
    CHECK_AND_RETURN_RET_LOG(environmentType != nullptr, ERR_INVALID_PARAM,
        "GetEnvironmentType failed, environmentType is nullptr.");
    CHECK_AND_RETURN_RET_LOG((node->GetNodeType() == NODE_TYPE_ENVIRONMENT_EFFECT),
        ERR_NOT_SUPPORTED, "GetEnvironmentType failed, enable type %{public}d not support option.",
        static_cast<int32_t>(node->GetNodeType()));
    uint32_t nodeId = node->GetNodeId();

    EnvironmentType environment = AUDIO_SUITE_ENVIRONMENT_TYPE_CLOSE;
    int32_t ret = IAudioSuiteManager::GetAudioSuiteManager().GetEnvironmentType(nodeId, environment);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "GetEnvironmentType failed, ret = %{public}d.", ret);

    *environmentType = static_cast<OH_EnvironmentType>(environment);
    return ret;
}

int32_t OHAudioSuiteEngine::GetSoundFiledType(OHAudioNode *node, OH_SoundFieldType *soundFieldType)
{
    CHECK_AND_RETURN_RET_LOG(node != nullptr, ERR_INVALID_PARAM, "GetSoundFiledType failed, node is nullptr.");
    CHECK_AND_RETURN_RET_LOG(soundFieldType != nullptr, ERR_INVALID_PARAM,
        "GetSoundFiledType failed, soundFieldType is nullptr.");
    CHECK_AND_RETURN_RET_LOG((node->GetNodeType() == NODE_TYPE_SOUND_FIELD),
        ERR_NOT_SUPPORTED, "GetSoundFiledType failed, enable type %{public}d not support option.",
        static_cast<int32_t>(node->GetNodeType()));
    uint32_t nodeId = node->GetNodeId();

    SoundFieldType soundField = AUDIO_SUITE_SOUND_FIELD_CLOSE;
    int32_t ret = IAudioSuiteManager::GetAudioSuiteManager().GetSoundFiledType(nodeId, soundField);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "GetSoundFiledType failed, ret = %{public}d.", ret);

    *soundFieldType = static_cast<OH_SoundFieldType>(soundField);
    return ret;
}

int32_t OHAudioSuiteEngine::GetEqualizerFrequencyBandGains(OHAudioNode *node,
    OH_EqualizerFrequencyBandGains *frequencyBandGains)
{
    CHECK_AND_RETURN_RET_LOG(node != nullptr, ERR_INVALID_PARAM,
        "GetEqualizerFrequencyBandGains failed, node is nullptr.");
    CHECK_AND_RETURN_RET_LOG(frequencyBandGains != nullptr, ERR_INVALID_PARAM,
        "GetEqualizerFrequencyBandGains failed, frequencyBandGains is nullptr.");
    CHECK_AND_RETURN_RET_LOG(node->GetNodeType() == NODE_TYPE_EQUALIZER, ERR_NOT_SUPPORTED,
        "GetEqualizerFrequencyBandGains failed, node type = %d{public}d must is EQUALIZER type.",
        static_cast<int32_t>(node->GetNodeType()));
    uint32_t nodeId = node->GetNodeId();

    AudioEqualizerFrequencyBandGains frequency = {
        .gains = {0}
    };
    int32_t ret = IAudioSuiteManager::GetAudioSuiteManager().GetEqualizerFrequencyBandGains(
        nodeId, frequency);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS,
        ret, "GetEqualizerFrequencyBandGains failed, ret = %{public}d.", ret);

    size_t ohGainsNum = sizeof(frequencyBandGains->gains) / sizeof(frequencyBandGains->gains[0]);
    size_t gainsNum = sizeof(frequency.gains) / sizeof(frequency.gains[0]);
    for (uint32_t idx = 0; idx < (ohGainsNum < gainsNum ? ohGainsNum : gainsNum); idx++) {
        frequencyBandGains->gains[idx] = frequency.gains[idx];
    }

    return ret;
}

int32_t OHAudioSuiteEngine::GetVoiceBeautifierType(OHAudioNode *node,
    OH_VoiceBeautifierType *voiceBeautifierType)
{
    CHECK_AND_RETURN_RET_LOG(node != nullptr, ERR_INVALID_PARAM, "GetVoiceBeautifierType failed, node is nullptr.");
    CHECK_AND_RETURN_RET_LOG(voiceBeautifierType != nullptr, ERR_INVALID_PARAM,
        "GetVoiceBeautifierType failed, voiceBeautifierType is nullptr.");
    CHECK_AND_RETURN_RET_LOG(node->GetNodeType() == NODE_TYPE_VOICE_BEAUTIFIER, ERR_NOT_SUPPORTED,
        "GetVoiceBeautifierType failed, node type = %d{public}d must is VOICE_BEAUTIFIER type.",
        static_cast<int32_t>(node->GetNodeType()));
    uint32_t nodeId = node->GetNodeId();

    VoiceBeautifierType voiceBeautifier = AUDIO_SUITE_VOICE_BEAUTIFIER_TYPE_CLEAR;
    int32_t ret = IAudioSuiteManager::GetAudioSuiteManager().GetVoiceBeautifierType(nodeId, voiceBeautifier);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "GetVoiceBeautifierType failed, ret = %{public}d.", ret);

    *voiceBeautifierType = static_cast<OH_VoiceBeautifierType>(voiceBeautifier);
    return ret;
}

void OHAudioSuiteEngine::AddPipeline(OHAudioSuitePipeline *pipeline)
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    pipelines_.insert(pipeline);
}

void OHAudioSuiteEngine::RemovePipeline(OHAudioSuitePipeline *pipeline)
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    if (pipeline != nullptr) {
        pipelines_.erase(pipeline);
        delete pipeline;
    }
}

bool OHAudioSuiteEngine::IsPipelineExists(OHAudioSuitePipeline *pipeline)
{
    if (pipeline == nullptr) {
        return false;
    }

    std::lock_guard<std::recursive_mutex> lock(mutex_);
    return pipelines_.find(pipeline) != pipelines_.end();
}

void OHAudioSuitePipeline::AddNode(OHAudioNode *node)
{
    std::lock_guard<std::mutex> lock(mutex_);
    nodes_.insert(node);
}

bool OHAudioSuitePipeline::IsNodeExists(OHAudioNode *node)
{
    std::lock_guard<std::mutex> lock(mutex_);
    return nodes_.find(node) != nodes_.end();
}

void OHAudioSuitePipeline::RemoveNode(OHAudioNode *node)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (node != nullptr) {
        nodes_.erase(node);
        delete node;
    }
}
OHAudioSuitePipeline::~OHAudioSuitePipeline()
{
    std::lock_guard<std::mutex> lock(mutex_);
    for (OHAudioNode* node : nodes_) {
        if (node != nullptr) {
            delete node;
        }
    }
    nodes_.clear();
}

bool OHAudioSuiteEngine::IsNodeExists(OHAudioNode *node)
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    for (auto pipeline : pipelines_) {
        if (pipeline && node && pipeline->IsNodeExists(node)) {
            return true;
        }
    }
    return false;
}

void OHAudioSuiteEngine::RemoveNode(OHAudioNode *node)
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    for (auto pipeline : pipelines_) {
        if (pipeline && node && pipeline->IsNodeExists(node)) {
            pipeline->RemoveNode(node);
        }
    }
}

}  // namespace AudioStandard
}  // namespace OHOS
